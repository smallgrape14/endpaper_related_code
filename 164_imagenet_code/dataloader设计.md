## 目标是解析出传统dataloader的设计细节，

试着分析出，
1. prefetch 参数提升，为什么性能没有明显提升
2. pin memory 开启后性能更差
查看网上有没有相关的提问
3. 最最重要的是：当前dataloader的设计瓶颈，从相关优化工作中汲取优化的经验，看看能不能解决这个问题


## dataloader 参数详细说明

任务一：分析传统dataloader 的多进程数据加载的模式是怎样的，已经知道prefetch的设计仅使用在多进程模式的数据加载，pretch_factor=2 表示每个 worker 提前加载 的 sample 数量,是否意味这个每个work进程提前加载的batch数据是互相独立的，互不共享
**重要的问题：**传统的dataloader 多进程的数据加载中 数据加载 和训练进程也是异步的模式，其是否会存在我们系统当前的这个问题，异步不同进程之间的资源竞争，（目前还没有确定当前的系统开启异步后训练时间骤增的原因所在）
    - [想法] 有没有可能是两个程序再操控同一块连续的GPU显存，只是训练进程和数据记载进程操作的是不同偏移量的GPU显存
            - 可问题是需要多块显存就需要，多次和GPUNetIO代理进行RDMA Write报文的解析，破解目标的GPU virtual address 和 remote key,如果不是

两个数据加载模式
dataloader/ train 完全串行  VS  dataloader/ train 预取异步的设计，在训练的同时也在从远端加载文件数据到同一大块的GPU显存
    竞争的资源很可能就是 ：A_Buffer 很可能就是这个Buffer的问题，
    如何证明是这个竞争同一个大GPU内存缓冲区资源导致的性能问题：
        验证思路：缓冲区数量和prefetch参数数量相同 ，2个预取的话，就用开辟两块GPU缓冲区域，
            已知如果是多块GPU显存的话，就需要多次GPUNetIO-DPU代理程序的拦截操作，这个比较麻烦，有一个办法，第一块A_BUF中数据直接复制第一块的文件数据，或者直接从本地读进去

**有个重要的问题：**
    - 参考的博客：https://blog.csdn.net/weixin_47364682/article/details/119744920
    - 关于相关 dataloader性能对比：论文：[An Overview of the Data-Loader Landscape:Comparative Performance Analysis] https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=10825421 
        - **从该论文中收获的一些重点结论：**
            - 还没有看论文里使用的CPU核心数量最高有多少，以及使用的是什么型号的GPU，和我们baseline的测试结果类似：num_worker=8~16时候就达到性能峰值
    - 现在已经知道**传统 dataloader 多进程的工作模式**，关键是三种queue 用于主进程和其余子进程的通信，主进程将生成的批次索引存放于每个worker的独立index queue,
        worker 通过（轮询？）方式检测index queque，根据索引去加载数据到 data queue ,（这个data queue 是共享的方式一共只有一个）
        当data_queue中批次数据被消耗了之后，主进程主动去加载新的批次索引给对应的子进程
        **问题：数据加载进程和训练进程的合作模式是怎样的？**
            - 训练进程之间消耗data_queue的数据吗，轮询data_queue？（CPU队列）从CPU->GPU->训练
    - **顺序保障机制​​**:主进程严格按采样器顺序从 data_queue 取数据。若某个 Worker 延迟，主进程会等待其批次就绪，避免乱序
    - **资源竞争与性能陷阱**
        1. ​​进程间通信开销​​
        多进程需通过队列传输数据和索引，过量 Worker（如超过 CPU 核心数）会导致上下文切换开销激增，反而降低速度。
        2. ​​内存消耗​​
        每个 Worker 独立缓存数据，num_workers 或 prefetch_factor 过高可能引发内存溢出（OOM）。
```sh
                                               +-----------+
               indices  -------------+ indices | Worker    | Data
             +--------->+index queue +-------->+ Process   +------+
             |          |            |         |           |      |
             |          -------------+         +-----------+      |
             |                                                    |   +------------+
             |                                                    |   |            |
+---------+  |                                                    +--->            |
| Main    |  | indices  -------------+ indices +-----------+          |            |
| Process +------------>+index queue +-------->+ Worker    | Data     | Data Queue |
|         |  |          |            |         | Process   +---------->            |
+---------+  |          -------------+         |           |          |            |
             |                                 +-----------+      +--->            |
             |                                                    |   +------------+
             |                                                    |
             | indices  -------------+ indices +-----------+      |
             +--------->+index queue +-------->+ Worker    | Data |
                        |            |         | Process   +------+
                        -------------+         |           |
                                               +-----------+


```

## 性能分析工具 
PyTorch Profiler
- [1] https://help.aliyun.com/zh/ack/cloud-native-ai-suite/use-cases/use-pytorch-profiler-to-realize-performance-analysis-and-troubleshooting-of-large-models
- [2] https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/80RC1alpha001/devaids/auxiliarydevtool/atlasprofiling_16_0006.html
- [3] https://pytorch.ac.cn/tutorials/recipes/recipes/profiler_recipe.html
- [4] https://docs.pytorch.org/tutorials/intermediate/tensorboard_profiler_tutorial.html
分析参数解读，重点是SM使用情况

> PyTorch Profiler涉及的GPU利用率指标说明
    主要涉及到 GPU Utilization，Est. SM Efficiency，Est. Achieved Occupancy，Kernel Time using Tensor Cores 这几个概念。文档：https://github.com/pytorch/kineto/blob/main/tb_plugin/docs/gpu_utilization.md
    **GPUUtilization** ：GPU 繁忙时间 / 所有步骤时间。数值越高越好。所有步骤时间是所有分析步骤（或称为迭代）的总时间。 GPU 繁忙时间是在“所有步骤时间”中至少有一个 GPU kernel在此 GPU 上运行的时间。 然而，这个高级别的利用率指标是粗糙的。它不能显示有多少个流多处理器（SM）正在使用。 例如，一个持续运行单线程的kernel将获得 100% 的 GPU 利用率。
    **Est. SM Efficiency** ：预估SM效率。数值越高越好。此指标为kernel的 SM 效率，SM_Eff_K = min（该kernel的block数 / 该 GPU 的 SM 数，100%）。 这个总数是所有kernel的 SM_Eff_K 乘以kernel执行持续时间后的总和，然后除以“所有步骤时间”。 它显示了 GPU 流多处理器的利用率。 虽然它比上面的“GPU 利用率”更精细，但它仍然不能完全展示全部情况。 例如，每个块只有一个线程的kernel无法完全利用每个 SM。  
    **Est. Achieved Occupancy** ：对于大多数情况，如内存带宽受限的kernel，更高的值通常意味着更好的性能，特别是当初始值非常低时。参考资料(http://developer.download.nvidia.com/GTC/PDF/GTC2012/PresentationPDF/S0514-GTC2012-GPU-Performance-Analysis.pdf)。占用率的定义在此处(https://docs.nvidia.com/gameworks/content/developertools/desktop/analysis/report/cudaexperiments/kernellevel/achievedoccupancy.htm)。 Occupancy是一个 SM 上活跃 warps 的比率与该 SM 支持的最大活跃 warps 数的比率。kernel的理论Occupancy是该kernel的上限占用率，受多种因素限制，如kernel形状、kernel使用的资源和 GPU 的计算能力。 kernel的预估实现Occupancy，OCC_K = min（kernel的线程数 / SM 数 / 每 SM 最大线程数，kernel的理论Occupancy）。 这个总数是所有kernel的 OCC_K 使用kernel执行持续时间作为权重的加权和。它显示了细粒度的低级 GPU 利用率。
    **Kernel Time using Tensor Cores** ：用于Tensor Core kernel的总 GPU 时间 / 所有kernel的总 GPU 时间。数值越高越好。 Tensor Core是 Volta GPU（如 Titan V）及以后 GPU 提供的混合精度浮点运算操作。 cuDNN 和 cuBLAS 库包含了多数卷积和 GEMM 操作的几个启用了张量核心的 GPU kernel。 这个数字显示了 GPU 上所有kernel中使用张量核心的时间比例。

### 性能最为相关的三个参数
1. num_workers	要用于数据加载的子进程数，0 表示将在主进程中加载数据
2. pin_memory	如果为 True，则 DataLoader 在将张量返回之前将其复制到 CUDA 固定的内存中
3. prefetch_factor	每个 worker 提前加载 的 sample 数量
4. persistent_workers (bool, optional) – If True, the data loader will not shut down the worker processes after a dataset has been consumed once. This allows to maintain the workers Dataset instances alive. (default: False)
#### 相关知识
##### num_workers
num_workers (int, optional) – how many subprocesses to use for data loading. 0 means that the data will be loaded in the main process. (default: 0)
##### prefetch_factor
默认值是 2
通过源码可以看到，prefetch 功能仅适用于 多进程 加载中（下面会由多进程 dataloader 的代码分析）
- [官方文档]
    - https://docs.pytorch.org/docs/stable/data.html#torch.utils.data.DataLoader
    - prefetch_factor (int, optional, keyword-only arg) – Number of batches loaded in advance by each worker. 2 means there will be a total of 2 * num_workers batches prefetched across all workers. (default value depends on the set value for num_workers. If value of num_workers=0 default is None. Otherwise, if value of num_workers > 0 default is 2)
    - 翻译：prefetch_factor（int，可选，仅关键字 - gry arg） - 每个工人提前加载的批次数量。 2意味着将总共有2个 * NUM_WORKERS批处理在所有工人中。 （默认值取决于num_workers的设定值。如果num_workers = 0默认值无。
#####  锁页内存 (Memory Pinning)
这里首先解释一下锁页内存的概念。

主机中的内存，有两种存在方式，一是锁页，二是不锁页，锁页内存存放的内容在任何情况下都不会与主机的虚拟内存进行交换（注：虚拟内存就是硬盘），而不锁页内存在主机内存不足时，数据会存放在虚拟内存中。主机到GPU副本源自固定（页面锁定）内存时，速度要快得多。CPU张量和存储暴露了一种 pin_memory() 方法，该方法返回对象的副本，并将数据放在固定的区域中。

而显卡中的显存全部是锁页内存！当计算机的内存充足的时候，可以设置 pin_memory=True。设置 pin_memory=True，则意味着生成的 Tensor 数据最开始是属于内存中的锁页内存，这样将内存的Tensor转义到GPU的显存就会更快一些。同时，由于 pin_memory 的作用是将张量返回之前将其复制到 CUDA 固定的内存中，所以只有在 CUDA 环境支持下才有用。





## 当前实现异步流水线的GPUNetIO程序

当前存在的问题
1. train时间相对于没有开启异步流水线明显慢
    原因：大致是在GPUNetIO数据加载程序 和 train 程序之间存在某种资源竞争
    尝试方法：分析进程资源的情况:如果 GPUNetIO程序只使用了一个SM ，按理来说应该不会存在太多计算资源的竞争，可是需要确定GPUNetIO程序使用的实际数量
        - 使用 PyTorch Profiler TensorBoard 插件 
    GPU资源分成两类就是：计算资源和内存资源
    AI 提示词语：
        有两个程序 一个是用GPU发起的数据加载程序，另一个是训练程序，实现了数据预取的设计，使得数据加载程序和训练程序可以异步执行，但是训练时间却在开启异步后显著提升，这是为什么，是数据加载程序和训练程序之间的资源竞争吗 
2. 


### the tools Maybe useful 

1. 如何测试训练过程的瓶颈
    如果现在程序运行速度很慢，那应该如何判断瓶颈在哪里呢？PyTorch中提供了工具，非常方便的可以查看设计的代码在各个部分运行所消耗的时间。
    操作步骤：
        （1）可以使用PyTorch中**bottleneck**工具，
            瓶颈测试：https://pytorch.org/docs/stable/bottleneck.html
            具体使用方法如下：
            python -m torch.utils.bottleneck /path/to/source/script.py [args]
        （2）也可用**cProfile**这样的工具来测试瓶颈所在
            python -m cProfile -o 100_percent_gpu_utilization.prof train.py
                这样就得到了文件100_percent_gpu_utilization.prof
            snakeviz 100_percent_gpu_utilization.prof
                对其进行可视化（用到了snakeviz包，pip install snakeviz即可）


## dataloader 库

Torchdata[29]
Deep Lake [30]
FFCV [10]  最快，为什么？
- 如前所述 是一个drop in的数据加载系统，通过使用.beton格式和即时（JIT）汇编
## Decode 等预处理在DALI上的问题


> 或许真的有相关工作是修改DALI使其满足特定功能的
    DALI 技术
    以前都是cpu处理加载图片，这个技术就是把这个步骤放在gpu上进行，想想就知道速度有多快了。缺点是有点占显存。
    不过比较麻烦的是，DaLi只能进行一些较简单的crop，resize之类的操作，像RandAugment 或CTAugment这些比较复杂的图像增强则需要自行修改源代码实现。
    具体可看下这个项目GitHub - tanglang96/DataLoaders_DALI: PyTorch DataLoaders implemented with DALI for accelerating image preprocessing


### jpeg GPU decode/encode 论文 

[1] Accelerating JPEG Decompression on GPUs
    - https://arxiv.org/pdf/2111.09219

## 一些可能用到的 注意事项

当前还不支持在子进程中进行 GPU Tensor 的操作，请不要在子进程流程中使用 GPU Tensor，例如 dataset 中的预处理，collate_fn 等，numpy array 和 CPU Tensor 操作已支持。