## 250506 功能描述 

```sh
主要是模拟GPUNetIO 加载模式的数据移动路径，做个简单的验证

1. 从指定目录中读取10个文件，加载到GPU显存中，并且IPC的方式生成对应的内存句柄共享给训练的python 进程
    make
    ./export_gpu_data
2. python 进程读取IPC句柄获取对应的GPUmem 中的文件数据，由于只读了一个目录（即只有一个类别），也没有加载验证集，所loss =0 acc=1 ，label的获取还属于硬编码的阶段，具体的标签获取需要通过 GPUNetIO 往共享内存中 file_info 增加对应的label字段
    sudo python3 train2.py 

PS： 如果有时间的话，可以考虑一个对应的思路，检验数据加载的正确性，共享内存是否可以正确加载数据
```