## 0617 nsight system 使用指南
```sh
1. 找到 nsight system 的可执行文件的存放位置
    PS: 不需要专门安装，安装cuda时候会一起安装好
    ex: /usr/local/cuda/bin/nsys 

2. 代码中添加 nvtx 分割待分析的代码段
        c++/c/python 都可以添加
【官方文档】
    https://docs.nvidia.com/nsight-systems/UserGuide/index.html#nvtx-trace
    https://nvidia.github.io/NVTX

    C的添加教程：
        https://nvidia.github.io/NVTX/doxygen/
    C++:的添加教程，C++程序代码中使用nvtx的教程
        https://nvidia.github.io/NVTX/doxygen-cpp/

    python 的添加教程：
        https://nvidia.github.io/NVTX/python/
        （1）pip install nvtx  且 import nvtx 
            或者 from torch.cuda import nvtx
        （2）with nvtx.range("iterate dataloader"):
                目标操作
            
3. 执行的指令 

**164 HOST 上** /usr/local/cuda/bin/nsys 用不了，版本不匹配换成 /opt/nvidia/nsight-systems/2024.5.1/target-linux-x64/nsys
shell:
    /usr/local/cuda/bin/nsys profile --stats=true --trace=cuda,cublas,nvtx --force-overwrite=true -o report_data ./build.sh
C++:
    /usr/local/cuda/bin/nsys profile --stats=true --trace=cuda,cublas,nvtx --force-overwrite=true -o report_data ./target
Python:
    /usr/local/cuda/bin/nsys profile --stats=true --trace=cuda,cublas,nvtx  --force-overwrite=true -o report_data python3 train.py
Python（指定参数）:
    /usr/local/cuda/bin/nsys profile --stats=true --trace=cuda,cublas,nvtx  --force-overwrite=true -o report_data python3 train52_nvtx.py --batch_size 16 --epochs 8 --workers 4 --conn_mode 1 --prefetch 1

/opt/nvidia/nsight-systems/2024.5.1/target-linux-x64/nsys profile --stats=true --trace=cuda,cublas,nvtx  --force-overwrite=true -o report_data_mode1 python3 train52_nvtx.py --batch_size 16 --epochs 8 --workers 4 --conn_mode 1 --prefetch 1

/opt/nvidia/nsight-systems/2024.5.1/target-linux-x64/nsys profile --stats=true --trace=cuda,cublas,nvtx  --force-overwrite=true -o report_data_mode2 python3 train52_nvtx.py --batch_size 16 --epochs 8 --workers 4 --conn_mode 1 --prefetch 1
```

### 问题收集 

用这个/usr/local/cuda/bin/nsys 报错 ：
```sh
    [1/7] [==========47%               ] report_data_mode1.nsys-rep
    Importer error status: Importation failed.
    Import Failed with unexpected exception: /dvs/p4/build/sw/devtools/Agora/Rel/CUDA12.4/QuadD/Host/QdstrmImporter/main.cpp(34): Throw in function {anonymous}::Importer::Importer(const boost::filesystem::path&, const boost::filesystem::path&)
    Dynamic exception type: boost::wrapexcept<QuadDCommon::RuntimeException>
    std::exception::what: RuntimeException
    [QuadDCommon::tag_message*] = Status: AnalysisFailed 如何解决
```


[问题原因]

```sh

1. 检查 CUDA 和 Nsight Systems 兼容性​​
​​CUDA 版本匹配​​
错误路径 /dvs/p4/build/sw/devtools/Agora/Rel/CUDA12.4/... 表明工具链依赖 ​​CUDA 12.4​​。请确认：
nvcc --version  # 查看当前 CUDA 版本
nsys --version  # 确认 Nsight Systems 版本
若版本不一致（如实际使用 CUDA 11.x），需升级 CUDA 或安装匹配的 Nsight Systems 版本
```
【解决方法】：
```

换成这个可执行文件： /opt/nvidia/nsight-systems/2024.5.1/target-linux-x64/nsys
```

## pytorch profiler 工具 的使用

## 火焰图的绘制，分析性能瓶颈

1. 生成对应的性能数据
```sh
pip install cProfile
import cProfile
profiler = cProfile.Profile()
profiler.enable()  # 启动性能分析

# 待测试的函数
profiler.disable()
profiler.dump_stats(f"worker_{gpu}.prof")  # 按GPU编号保存
  
```
2. 绘制成对应的图
```sh
pip install flameprof
flameprof worker_1.prof > worker_1.svg
flameprof xx.prof > xx.svg
```

3. 关于火焰图怎么看，还是不知道