
## 0808 Baseline DEBUG 思路解析记录

每轮epoch的第一个iteration都很慢，这是为什么
每轮epoch的其他iteration就很快，我们发现目前这个读取速度非常快，预取的优势非常大


## 最新版本的代码，代码版本说明

## 0716 版本说明 
```sh
[Baseline]
单GPU ： train52.py 脚本 run.sh
多GPU :  main.py 脚本 run2.sh

```
## 0714 脚本说明
```sh
使用nsys分析工具
【多GPU分布式】：main_nvtx.py nvtx.sh 记录svg数据在 avg_batch_time_dist0707_debug
【单GPU】：train52.py nvtx1.sh 记录svg数据在 avg_batch_time_BeeGFS_验证_0708


```

## 0711 版本说明


docker run --name xyp_2.10.0-devel_250713 --runtime=nvidia --gpus all -e TZ=Asia/Shanghai -v /home/oem/xyp/docker_host_250402:/doca_devel -v /dev/hugepages:/dev/hugepages -v /usr/local/cuda-12.6:/usr/local/cuda -v /opt/mellanox/gdrcopy:/opt/mellanox/gdrcopy --privileged --net=host -it nvcr.io/nvidia/doca/doca:2.10.0-devel-host

### 250626 :关键的三个代码版本说明
1. main.py :pytorch 官方给的代码，我去掉了预处理的逻辑之后，即直接加载numpy文件 ,0626之前的版本 main_0626past.py
2. train.py : GPUNetIO 加载文件程序 对应的训练代码
3. train52.py : Baseline 用的测试代码 0626之前的版本是 train52_0626past.py
4. train54.py : 进一步开启优化，在train52.py 的基础上：使用异步数据拷贝（DataPrefetcher），加载下一个 batch 时就预拷贝数据到 GPU
    0626 增加 main.py. pytorch 官方的example code
5. decode3.py 和 decode3_0626.py ： 是对JPEG 解码预处理后导出到指定目录的程序

### 0616：
0616： 当前最新版本的代码是 train51.py 和 train52.py
    train51.py 是加上nvtx 模块和没加的用于测试worknum prefetch 参数的代码
    train52.py 是进一步开启dataloader /pytorch 的相关优化，看看pytorch dataloader 的异步数据加载上限
    train54.py : 进一步开启优化，在train52.py 的基础上：使用异步数据拷贝（DataPrefetcher），加载下一个 batch 时就预拷贝数据到 GPU
    0626 增加 main.py. pytorch 官方的example code

### 0514
0514： 当前最新版本的代码是 train4.py 和 train41.py



## DEBUG 记录 

### 250714 
```sh
[报错] 模式2： 为main_worker 和  dataloader 的多 worker，指定一些 GPU亲和的CPU核心 
UserWarning: This DataLoader will create 8 worker processes in total. Our suggested max number of worker in current system is 6, which is smaller than what this DataLoader is going to create. Please be aware that excessive worker creation might get DataLoader running slow or even freeze, lower the worker number to avoid potential slowness/freeze if necessary.
  warnings.warn(_create_warning_msg(
/usr/local/lib/python3.8/dist-packages/torch/utils/data/dataloader.py:557: UserWarning: This DataLoader will create 8 worker processes in total. Our suggested max number of worker in current system is 6, which is smaller than what this DataLoader is going to create. Please be aware that excessive worker creation might get DataLoader running slow or even freeze, lower the worker number to avoid potential slowness/freeze if necessary.


/usr/local/lib/python3.8/dist-packages/torch/utils/data/dataloader.py:557: UserWarning: This DataLoader will create 6 worker processes in total. Our suggested max number of worker in current system is 1, which is smaller than what this DataLoader is going to create. Please be aware that excessive worker creation might get DataLoader running slow or even freeze, lower the worker number to avoid potential slowness/freeze if necessary.
  warnings.warn(_create_warning_msg(
/usr/local/lib/python3.8/dist-packages/torch/utils/data/dataloader.py:557: UserWarning: This DataLoader will create 6 worker processes in total. Our suggested max number of worker in current system is 1, which is smaller than what this DataLoader is going to create. Please be aware that excessive worker creation might get DataLoader running slow or even freeze, lower the worker number to avoid potential slowness/freeze if necessary.

```
#### V100不能开启带宽监控？
```sh
[指令]
/opt/nvidia/nsight-systems/2024.5.1/target-linux-x64/nsys profile -o report_main_nvtx_worker512_16 --stats=true --trace=cuda,osrt,nvtx,syscall --gpu-metrics-set=ga100 --gpu-metrics-devices=all --gpu-metrics-frequency=10000 --sample=process-tree  --force-overwrite=true  python3 $PYTHON_SCRIPT -b $batch_size -j $worker_num --epochs $EPOCH_NUM -p $Print_freq -a $MODEL_NAME $TRAIN_DIR --dist-url $DIST_URL --dist-backend $DIST_BACKEND --multiprocessing-distributed --world-size $NODE_NUM --rank $NODE_RANK

[报错]
root@node9:/home/oem/xyp/imagenet_code/Baseline# ./nvtx.sh 
Illegal --gpu-metrics-devices arguments.
None of the installed GPUs are supported:
        Tesla V100-PCIE-32GB PCI[0000:17:00.0] - Unsupported architecture
        Tesla V100-PCIE-32GB PCI[0000:fa:00.0] - Unsupported architecture
See the user guide: https://docs.nvidia.com/nsight-systems/UserGuide/index.html#gpu-metrics

usage: nsys profile [<args>] [application] [<application args>]
Try 'nsys profile --help' for more information.
```
### 250711 DEBUG 分析记录
#### Task1 : 单/多GPU 相同Batchsize下的 数据加载时间有差距的原因是什么 nvtx/profiler分析下，时间差距的原因是什么，训练时间是有点差距，

对比 main_nvtx.py 多GPU模式训练  和 
### 250710
processor : 0~79
physical id     : 0~1
core id         : 0~19

processor 偶数是0CPU 奇数是1CPU
​​processor字段​ :
    表示逻辑处理器的编号，从0开始递增。每个逻辑处理器对应操作系统可调度的最小CPU单元。
​​physical id字段​​
    ​​含义​​：标识物理CPU插槽的编号。在多路（多颗物理CPU）系统中，每个物理CPU有唯一的physical id。
​​core id字段​​
    ​​含义​​：标识同一物理CPU内的物理核心编号。每个物理核心有唯一的core id，与超线程产生的逻辑核心无关。
    若两个逻辑处理器的physical id和core id相同，则属于同一物理核心的超线程兄弟（需结合siblings字段判断是否启用超线程）。
    ​​示例​​：core id : 0表示第一个物理核心

- 参数解析：
        worker_init_fn (Callable, optional): If not ``None``, this will be called on each
            worker subprocess with the worker id (an int in ``[0, num_workers - 1]``) as
            input, after seeding and before data loading. (default: ``None``)
#### 解析
- 查看GPU与CPU的NUMA拓扑关系​​
    使用 nvidia-smi topo -m 命令获取GPU与CPU核心的亲和性信息。该命令会显示：
    ​​GPU与CPU的NUMA节点关联​​：明确GPU连接到哪个NUMA节点（Socket），以及该节点包含的CPU核心范围。
    ​​PCIe连接拓扑​​：显示GPU通过哪个PCIe Root Complex与CPU相连，帮助判断跨NUMA节点的通信延迟。
```sh
(base) oem@node9:~/xyp$ nvidia-smi topo -m 
        GPU0    GPU1    NIC0    NIC1    NIC2    NIC3    CPU Affinity    NUMA Affinity   GPU NUMA ID
GPU0     X      SYS     SYS     SYS     SYS     SYS     0,2,4,6,8,10    0               N/A
GPU1    SYS      X      SYS     SYS     SYS     SYS     1,3,5,7,9,11    1               N/A
```


- ​​确定NUMA节点内的CPU核心​​: 通过 lscpu 命令查看NUMA节点的CPU分配情况：
```sh
(base) oem@node9:~/xyp$ lscpu | grep "NUMA"
NUMA 节点：                         2
NUMA 节点0 CPU：                    0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,78
NUMA 节点1 CPU：                    1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,45,47,49,51,53,55,57,59,61,63,65,67,69,71,73,75,77,79
```
#### 问题：core_id 为什么是从0~19 而不是 0~39 

```sh
根据你提供的 /proc/cpuinfo 输出，结合CPU架构和NUMA设计原理，core_id 未连续从0到39排列的原因如下：

1. ​​物理CPU插槽与核心分布​​
​​双物理CPU插槽​​：physical id 值为0和1，表明系统配备​​两颗物理CPU​​（如双路服务器）。
​​每颗CPU的核心分配​​：
每颗物理CPU包含20个物理核心（core_id 范围0–19）。
例如：
physical id:0 的 core_id 为0、1、2…19
physical id:1 的 core_id 同样为0、1、2…19
​​总物理核心数​​ = 2（插槽） × 20（核心/插槽） = ​​40个物理核心​​。
2. ​​超线程（Hyper-Threading）机制​​
​​逻辑处理器翻倍​​：每个物理核心通过超线程技术虚拟化为两个逻辑处理器（即80个processor编号0-79）。
​​core_id 重复的原因​​：
同一物理核心的两个逻辑处理器​​共享相同的core_id​​（如processor:0和processor:40均对应physical id:0, core_id:0）。
因此，core_id 在每颗CPU内部独立编号（0–19），而非全局连续编号（0–39）
```
####
```sh
(base) oem@node9:~/xyp$ grep -E "processor|core id|physical id" /proc/cpuinfo
processor       : 0
physical id     : 0
core id         : 0
processor       : 1
physical id     : 1
core id         : 0
processor       : 2
physical id     : 0
core id         : 10
```
### 250709

#### 关于原生pytorch dataloader 的研究，解决以下问题

1. 为什么batchsize >=256 这样比较大的batchsize dataloader 的加载效果不是很好
从batchsize=512 /batchsize=256 的datatime 大于两倍 可以看出来，甚至更大的batchsize的倍数是更大的
2. 我们为什么会在大batchsize 表现出比baseline更优异的性能，？大batchsize为什么会体现出我们的性能优势
3.我们open都是串行的，为什么会比baseline的性能好，好的原因在哪里
    我觉得可以从 : pytorch dataloader 多worker的工作流程梳理一下，解释下其并行方式是否和我们是一样的，多个worker并行加载各自一个batchsize的数据，还是多个worker协作加载一个batchsize的数据，
4.预取参数是指的什么，预取一个batchsize的数据，还是 多留一个sample的数据缓冲区


### 250704 ： 
TODLIST：
    1. 主要是测试16GB->32GB 的GPU，是否可以使用超过512的batchsize,
    2. 还有测试Baseline的多GPU分布式实验
    3. 重点是研究下 如何测量数据加载时间训练时间，多GPU分布式训练的时候时间怎么测试的

其实就是 Ours ：batchsize=b Baseline : 得用 batchsize=2*b ,这样才是相同参数下的对比实验
main.py 这个是实现多GPU分布式训练，已经正常载入 beegfs 存储的预处理好的文件 


## 250702

def __getitem__(self, idx):
    print(f"[Worker PID {os.getpid()}] Loading sample {idx}")
    ...
    return sample


## 250701
分布式训练注意事项：
    1. 数据采样器是针对整个dataset进行划分，保证每个GPU训练进程只处理数据集的一部分，似乎不是一个batch的数据划分给不同的进程？
    2. trainloader 中参数需要设置 sampler=train_sampler






## 250626 分布式训练，多GPU训练
sudo python3 main.py -a resnet18 /mnt/beegfs/test_dir -j 0 --epochs 1 -b 16 --gpu 0
### Single node, multiple GPUs:

```bash
python main.py -a resnet50 --dist-url 'tcp://127.0.0.1:FREEPORT' --dist-backend 'nccl' --multiprocessing-distributed --world-size 1 --rank 0 [imagenet-folder with train and val folders]

//修改后：
python3 main.py -a resnet18 --dist-url 'tcp://127.0.0.1:3689' --dist-backend 'nccl' --multiprocessing-distributed --world-size 1 --rank 0 /mnt/beegfs/test_dir --epochs 5 -b 16
```
- **url参数设置**
```
单机多卡训练​​（一台服务器，多张GPU）
# 在启动命令或代码中设置
args.dist_url = 'tcp://127.0.0.1:23456'  # 使用本地回环地址
```
## 250623 关于 pytorch dataloader 的加载



## 250506 说明主要代码功能

```sh
主要是Baseline BeeGFS-Client 加载数据的

1. decode / resize 等预处理 对整个imagenet数据集进行预处理，预处理后导出到一个目录下去
    python3 decode3.py

2. 从目标目录 加载数据集 做AI训练  (优先用main.py 因为是官方的代码，比较标准，各种参数都便于修改)
    sudo python3 main.py -a resnet18 /home/oem/xyp/imagenet_test_decode3 -j 1 --epochs 1 -b 16 --gpu 0


    sudo python3 main2.py -a resnet18 /home/oem/xyp/imagenet_test_decode31 -j 1 --epochs 1 -b 16 --gpu 0
    sudo python3 main2.py -a resnet18 /home/oem/xyp/imagenet_test -j 1 --epochs 1 -b 16 --gpu 0
    sudo python3 main2.py -a resnet18 /mnt/beegfs/test_dir -j 1 --epochs 1 -b 16 --gpu 0



    sudo python3 main.py -a resnet18 /home/oem/xyp/imagenet_decode -j 1 --epochs 1 -b 16 --gpu 0

    sudo python3 main2.py -a resnet18 /mnt/beegfs/test_dir -j 1 --epochs 1 -b 256 --gpu 0


    sudo python3 train3.py
```

## 0512 说明
main2.py 也是同理

新增加 train31.py : 只加载 train 数据集 不管 val 集合，用于和 GPUNetIO 进行对比，检查数据加载的正确性，主要是通过 loss 和 acc值来判断


## 0514 说明 
train4.py 增加时间分解的逻辑

train41.py train4.py 的基础上增加 main函数参数的逻辑，用于脚本 自动测试 

当前最新版本的代码是 train4.py 和 train41.py
```sh
sudo python3 train4.py 
sudo python3 train41.py --batch_size 256 --epochs 10--workers 1
```

## Baseline 数据加载的优化

```sh

数据传输的优化
数据传输从 CPU 到 GPU 是一个相对耗时的操作，尤其是在数据量较大的情况下。为了优化这个过程，可以采取以下措施：

1.异步数据传输：
    使用 non_blocking=True 参数进行异步数据传输，这样可以重叠数据传输和计算时间。例如：

    inputs = inputs.cuda(non_blocking=True)
    labels = labels.cuda(non_blocking=True)
2.数据预取（Prefetching）：
    在训练当前 batch 的数据时，提前加载下一个 batch 的数据到 GPU 内存。这可以通过多线程或多进程实现。
    使用 DataLoader 的 num_workers 参数：
    启用多进程数据加载，可以加快数据从磁盘到 CPU 内存的速度，从而间接减少数据传输到 GPU 的总时间。例如：

    train_loader = DataLoader(train_dataset, batch_size=batch_size, shuffle=True, num_workers=4)
3.使用 pin_memory：
    在数据加载器中启用 pin_memory，这样可以加速从 CPU 到 GPU 的数据传输。例如：
    train_loader = DataLoader(train_dataset, batch_size=batch_size, shuffle=True, num_workers=4, pin_memory=True)

```

## 实验设计