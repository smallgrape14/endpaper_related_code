
## 0731
```sh
专利评审时候，用的GDS测试结果来自于 DALI.py 程序， 测试脚本是 run.sh
可以设置两个参数， USE_GDS = 1  或者 USE_GDS == 0 
1: 表示pipeline 数据加载的时候 device ="gpu"
0：表示pipeline 数据加载的时候 device ="cpu"
 

```


## 0728
```sh
GDS 相关配置

1.检查 nvidia_fs 模块是否正确插入
lsmod | grep nvidia_fs
modinfo nvidia_fs

ls /dev/nvidia-fs  # 设备文件应存在
```
### 替代方案
```sh
💎 四、替代方案
若问题持续，改用 ​​NVIDIA DALI 库​​（内置 GDS 支持）：

import nvidia.dali.fn as fn

# 自动启用 GDS
images = fn.readers.file(file_root="/mnt/beegfs/data", use_direct_io=True)
```

### GDS 测试,读写正常
```sh
/usr/local/cuda/gds/tools/gdscheck -p
/usr/local/cuda/gds/tools/gdsio --help

/usr/local/cuda/gds/tools/gdsio -D /mnt/beegfs/gdsio_0728 -w 1 -d 0 -I 1 -x 1 -s 64M -i 1M

/usr/local/cuda/gds/tools/gdsio -D /mnt/beegfs/gdsio_0728 -w 1 -d 0 -I 0 -x 1 -s 64M -i 1M

/usr/local/cuda-11.7/gds/tools/gdsio -D /mnt/beegfs/gdsio_0728 -w 1 -d 0 -I 0 -
x 0 -s 64M -i 1M
```
## Error

### Error1: 
```sh
~/xyp/imagenet_code/GDS$ sudo ./run2.sh 
Traceback (most recent call last):
  File "train.py", line 121, in <module>
    main()
  File "train.py", line 102, in main
    handle = register_file(fd)
  File "train.py", line 44, in register_file
    raise RuntimeError(f"cuFileHandleRegister failed: error code {err.error}")
RuntimeError: cuFileHandleRegister failed: error code 5008
```

### Error2:
```sh
[报错]
/usr/local/lib/python3.8/dist-packages/nvidia/dali/plugin/base_iterator.py:208: Warning: Please set `reader_name` and don't set last_batch_padded and size manually whenever possible. This may lead, in some situations, to missing some samples or returning duplicated ones. Check the Sharding section of the documentation for more details.

```

```sh
[解决方式]： 去掉size=size

class DaliGDSDataset(IterableDataset):
    def __init__(self, pipeline, size):
        self.dali_iterator = DALIGenericIterator(
            pipelines=[pipeline],
            output_map=["data"],
            # size=size,
            auto_reset=True
        )

```

### Error3:
```sh
use_o_direct (bool, optional, default = False) –
  If set to True, the data will be read directly from the storage bypassing system cache.
  Mutually exclusive with dont_use_mmap=False.

```

### Error4: acc 不高 ，没有随着迭代 上升
```sh
【解决方式】注释下面的shuffle ,根本原因是获取标签和文件实际上是不一致的，不是对应的，文件对应的标签不正确
def data_generator(data_dir, batch_size,batch_files):
    all_data = scan_directory(data_dir)
    total_samples = len(all_data)
    
    while True:
        # random.shuffle(all_data)
```
### Error5:
```sh
[报错]
/usr/local/lib/python3.8/dist-packages/torch/utils/data/dataloader.py:641: UserWarning: Length of IterableDataset <__main__.DaliGDSDataset object at 0x7fe637b1f7c0> was reported to be 163(when accessing len(dataloader)), but 228 samples have been fetched. 
  warnings.warn(warn_msg)
【解决】
class DaliGDSDataset(IterableDataset):
    def __init__(self, pipeline, size):

        self.dali_iterator = DALIGenericIterator(
            pipelines=[pipeline],
            output_map=["images","labels"],
            size=size, # 设置这个参数 == total_file_num == total_samples
            auto_reset=True
        )
```

### Error6:
```sh
[报错]
batchsize= 16 变化增大 报错：
  File "DALI_acc.py", line 279, in gds_pipeline
    images = fn.readers.numpy(

encountered:

Assert on "ref_batch_size == output_batch_size" failed: Batch size has to be uniform across one iteration. Expected: 16; Actual: 32
C++ context: [/opt/dali/dali/pipeline/operator/operator.cc:59] 
Current pipeline object is no longer valid.

【解决】
@pipeline_def(batch_size=16, num_threads=4, device_id=0)
def gds_pipeline(data_dir,files,labels):
    
    batch_files=[]
    # 使用fn.external_source生成数据节点
    _, labels = fn.external_source(
        # source=lambda: data_generator(data_dir,batch_size=16),
        source=data_generator(data_dir,batch_size=16,batch_files=batch_files), # 原因是这里的batchsize是硬编码的 ，改成由参数指定
        
        num_outputs=2,  
        # cycle='raise',  
        device='cpu'     
        # batch=False
    )
```

### Error:
```sh
batchsize 调整到 512 报错

  File "DALI_acc.py", line 279, in gds_pipeline
    images = fn.readers.numpy(

encountered:

Error in thread 0: CUFile open failed: /mnt/beegfs/test_dir/train/n02138441/n02138441_5631.JPEG.npy
Current pipeline object is no longer valid.

莫名其妙又好了，好像没有修改什么代码

```
### Error:meta 崩了导致的问题
```sh
Traceback (most recent call last):
  File "DALI.py", line 887, in <module>
    main()
  File "DALI.py", line 409, in main
    main_worker(args.gpu, ngpus_per_node, args)
  File "DALI.py", line 530, in main_worker
    classes = sorted(os.listdir(train_dir))
OSError: [Errno 70] Communication error on send: '/mnt/beegfs/test_dir/train'
```
## Reference

### GDS DALI
https://docs.nvidia.com/deeplearning/dali/user-guide/docs/operations/nvidia.dali.fn.readers.numpy.html

#### github之类的提问
https://github.com/NVIDIA/DALI/issues/3972
https://github.com/NVIDIA/DALI/issues/4720
https://github.com/NVIDIA/DALI_extra
https://discuss.pytorch.org/t/feature-request-nvidia-gds-support-for-pytorch-iterabledataset-checkpointing/211945

#### API
https://docs.nvidia.com/deeplearning/dali/user-guide/docs/examples/general/data_loading/numpy_reader.html
https://docs.nvidia.com/deeplearning/dali/user-guide/docs/examples/general/data_loading/numpy_reader.html
https://docs.nvidia.com/deeplearning/dali/archives/dali_1_14_0/user-guide/docs/operations/nvidia.dali.fn.external_source.html
https://docs.nvidia.com/deeplearning/dali/user-guide/docs/operations/nvidia.dali.fn.readers.numpy.html

### GDS KVIKIO

https://github.com/Project-MONAI/tutorials/blob/main/modules/GDS_dataset.ipynb
https://github.com/rapidsai/kvikio#install
https://docs.rapids.ai/api/kvikio/nightly/install/

#### 操作指南
```sh

# 1.安装
python3 -m pip install --extra-index-url https://test.pypi.org/simple/ cucim==0.0.233

python3 -m pip install scipy 'scikit-image<0.20.0'
```

#### Error
##### Error1:
```sh
[Error] cuFileHandleRegister fd: 98 (/home/oem/xyp/imagenet_code/GDS/CUCIM/input.raw), status: GPUDirect Storage not supported on current file. Would work with cuCIM's compatibility mode.
[Error] cuFileHandleRegister fd: 98 (output.raw), status: GPUDirect Storage not supported on current file. Would work with cuCIM's compatibility mode.

```

[原因分析]：为什么这个也是 cufilehandle 注册失败？
```sh
文件未对齐
nvme ssd 没正确挂载
```

##### Error2:  /usr/local/cuda/gds/tools/gdscheck -p 显示nvme unsported 应该怎么办

```sh
mount | grep -i "/dev/nvme"
/dev/nvme0n1p2 on /mnt/nvme0 type ext4 (rw,relatime,data=ordered)
```




### GDS 是否处于兼容模式
```sh
​​通过环境变量检查​​
GDS 的兼容模式通常由环境变量 CUCIM_ENABLE_COMPATIBILITY_MODE 控制：

​​查看当前环境变量​​：
echo $CUCIM_ENABLE_COMPATIBILITY_MODE
若输出 1：表示​​强制启用兼容模式​​（数据通过 CPU 内存中转）。
若输出为空或 0：表示​​默认尝试高性能模式​​（但可能因硬件/配置失败自动回退）。
💡 此变量是显式启用兼容模式的标志，由用户主动设置。
```

【诗雨】
```sh
/etc/cufile.json


```





###  cuCIM
https://github.com/NVIDIA/MagnumIO/tree/main/gds/readers/cucim-gds
https://nbviewer.org/github/rapidsai/cucim/blob/cucim_gds_reader/notebooks/Accessing_File_with_GDS.ipynb
### COCO Reader
https://docs.nvidia.com/deeplearning/dali/user-guide/docs/examples/general/data_loading/coco_reader.html


