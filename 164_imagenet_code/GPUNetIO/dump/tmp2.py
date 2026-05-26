# 导入DALI核心库和依赖
import cupy as cp
from nvidia.dali import pipeline_def
import nvidia.dali.fn as fn
import nvidia.dali.types as types
from nvidia.dali.plugin.pytorch import DALIGenericIterator

@pipeline_def(batch_size=32, num_threads=4, device_id=0)
def gpu_jpeg_preprocess_pipeline():
    """
    GPU内存JPEG预处理流水线定义
    - 输入：已存在于GPU内存的JPEG字节流（CUDA数组或PyTorch CUDA张量）
    - 输出：标准化后的RGB图像张量（GPU显存）
    """
    # 步骤1：从GPU内存读取JPEG字节流（需符合DALI的TensorGPU格式）
    # device='gpu'表示直接从GPU内存读取，避免数据拷贝
    jpeg_data = fn.external_source(device="gpu", name="gpu_jpeg_input", no_copy=True)

    # 步骤2：混合模式JPEG解码（自动选择硬件/CUDA解码）
    # device='mixed'表示在GPU上执行解码，hw_decoder_load=0.7表示70%负载分配给硬件解码器（A100等支持）
    decoded = fn.decoders.image(
        jpeg_data,
        device="mixed",
        output_type=types.RGB,
        hw_decoder_load=0.7  # 根据GPU负载调整硬件解码比例[1](@ref)
    )

    # 步骤3：GPU上的图像预处理
    # 调整尺寸至224x224，使用双线性插值
    resized = fn.resize(
        decoded,
        resize_x=224,
        resize_y=224,
        interp_type=types.INTERP_LINEAR
    )
    
    # 归一化与镜像增强（ImageNet标准参数）
    normalized = fn.crop_mirror_normalize(
        resized,
        mean=[0.485 * 255, 0.456 * 255, 0.406 * 255],  # 均值反归一化到0-255范围
        std=[0.229 * 255, 0.224 * 255, 0.225 * 255],    # 标准差反归一化
        mirror=fn.random.coin_flip(probability=0.5),    # 50%概率水平翻转
        dtype=types.FLOAT,                              # 输出浮点张量
        output_layout=types.NCHW                        # 转换为PyTorch的NCHW格式
    )
    return normalized

def gpu_data_generator(jpeg_buffers):
    """
    GPU数据生成器（模拟生产环境的数据输入）
    :param jpeg_buffers: 预加载到GPU内存的JPEG字节流列表（需为cupy数组或PyTorch CUDA张量）
    """
    for batch in jpeg_buffers:
        # 将数据转换为DALI兼容的GPU数组格式（零拷贝）
        yield cp.asarray(batch)  # 若使用PyTorch可替换为 torch.as_tensor(batch).cuda()

if __name__ == "__main__":
    # 初始化预处理流水线
    pipe = gpu_jpeg_preprocess_pipeline()
    pipe.build()  # 编译优化计算图[7](@ref)

    # 模拟输入数据（实际场景应从多进程队列或共享内存获取）
    # 示例：生成随机JPEG字节流（GPU内存）
    dummy_jpegs = [cp.random.bytes(1024 * 1024).astype(cp.uint8) for _ in range(100)]  # 100张1MB的模拟JPEG
    
    # 将数据送入流水线
    pipe.feed_input("gpu_jpeg_input", gpu_data_generator(dummy_jpegs))

    # 创建PyTorch兼容的数据迭代器
    dali_iter = DALIGenericIterator(
        pipelines=pipe,
        output_map=["processed_images"],
        auto_reset=True,          # 迭代完成后自动重置
        dynamic_shape=True,        # 支持可变尺寸输入[2](@ref)
        last_batch_policy="PARTIAL"# 允许最后批次不足
    )

    # 使用示例
    for idx, data in enumerate(dali_iter):
        batch = data[0]["processed_images"]  # 直接获取GPU上的预处理结果
        print(f"Batch {idx} shape: {batch.shape}, dtype: {batch.dtype}")
        # 此处可接入PyTorch训练流程，如：model(batch)