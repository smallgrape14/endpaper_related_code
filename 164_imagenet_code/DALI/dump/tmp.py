from nvidia.dali.pipeline import pipeline_def
import nvidia.dali.types as types
import nvidia.dali.fn as fn
from nvidia.dali.plugin.pytorch import DALIGenericIterator

@pipeline_def(batch_size=128, num_threads=4, device_id=0)
def get_dali_pipeline(data_dir, crop_size):
  images, labels = fn.readers.file(file_root=data_dir, shuffle=True, name="Reader")
  # decode data on the GPU
  images = fn.decoders.image_random_crop(images, device="mixed", output_type=types.RGB)
  # the rest of processing happens on the GPU as well
  images = fn.resize(images, resize_x=crop_size, resize_y=crop_size)
  images = fn.crop_mirror_normalize(images,
                                    mean=[0.485 * 255,0.456 * 255,0.406 * 255],
                                    std=[0.229 * 255,0.224 * 255,0.225 * 255],
                                    mirror=fn.random.coin_flip())
  return images, label

train_data = DALIGenericIterator(
   [get_dali_pipeline(data_dir, (244,244))], ['data', 'label’],
   reader_name='Reader’
)


for i, data in enumerate(train_data):
  x, y = data[0]['data'], data[0]['label’]
  pred = model(x)
  loss = loss_func(pred, y)
  backward(loss, model)





import torch
import time
from nvidia.dali.pipeline import pipeline_def
import nvidia.dali.fn as fn

# 定义时间统计器（参考网页3/9的同步测量方法）
class GPUTimer:
    def __enter__(self):
        self.start = torch.cuda.Event(enable_timing=True)
        self.end = torch.cuda.Event(enable_timing=True)
        self.start.record()
        return self
    
    def __exit__(self, *args):
        self.end.record()
        torch.cuda.synchronize()  # 强制同步确保时间准确
        self.elapsed = self.start.elapsed_time(self.end)  # 单位：毫秒

##--------------------------------------------------------------##--------------------------------------------------------------
# DALI预处理管道（结合网页6/8的优化方案）
@pipeline_def(batch_size=256, num_threads=4, device_id=0)
def create_dali_pipeline(data_dir):
    # 阶段1：磁盘加载与CPU解码
    images, labels = fn.readers.file(file_root=data_dir, random_shuffle=True)
    images = fn.decoders.image(images, device="cpu")  # CPU解码
    
    # 阶段2：GPU预处理
    images = images.gpu()  # 数据转移至GPU
    images = fn.resize(images, resize_x=224, resize_y=224)  # GPU上缩放
    images = fn.crop_mirror_normalize(images, 
        mean=[0.485 * 255, 0.456 * 255, 0.406 * 255],
        std=[0.229 * 255, 0.224 * 255, 0.225 * 255]
    )
    return images, labels

# 初始化DALI迭代器
pipe = create_dali_pipeline("data/train")
loader = DALIGenericIterator(pipe, ["images", "labels"])

for data in loader:
    # 阶段3：数据从DALI缓存到PyTorch张量
    with GPUTimer() as t_transfer:
        images = data[0]["images"]  # 数据已在GPU，但需转PyTorch张量
    
    # 阶段4：自定义GPU预处理（如需要）
    with GPUTimer() as t_preprocess:
        images = custom_gpu_augmentation(images)  # 用户自定义预处理
    
    print(f"传输耗时: {t_transfer.elapsed}ms | 预处理耗时: {t_preprocess.elapsed}ms")

##--------------------------------------------------------------##--------------------------------------------------------------
# 预加载数据到内存
with open("dataset.bin", "rb") as f:
    raw_data = np.frombuffer(f.read(), dtype=np.uint8)

@pipeline_def(batch_size=32, device_id=0)
def external_gpu_pipeline():
    # 直接注入GPU显存[4,6](@ref)
    gpu_data = fn.external_source(
        source=lambda: raw_data,
        device="gpu",  # 关键参数
        no_copy=True   # 避免额外复制
    )
    
    # 全GPU解码流程
    images = fn.decoders.image(
        gpu_data,
        device="gpu",
        output_type=types.RGB
    )
    return images
##--------------------------------------------------------------##--------------------------------------------------------------
