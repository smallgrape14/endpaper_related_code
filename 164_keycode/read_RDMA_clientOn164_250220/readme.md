## 250106 标注
- 注意这个版本是 164 client 针对 162 server 读 beegfs 挂载目录下  test_1217/image_data.csv 的 open (TCP) read (RDMA) 的版本

- 注意 ：非常奇怪！！！ 读 test_1202/image_data.csv open 一直失败，返回 8 说明路径不存在的错误，到底哪个参数有问题，目前还没找到，但是我一换了文件读，立马就成功了


## 250220 注意：标注程序运行成功的最新日期，以防 beegfs 修改配置之后，需要修改配置参数的问题


读 `/mnt/beegfs/test_250220/image_data.csv` 文件

### 描述下 代码结构

```sh
└─test2 
    ├─read.cpp 
    ├─new_read.cpp 
    ├─read_pre.cpp
    ├─RDMAConnection.hpp
    ├─IBVSocket.hpp
    └─open.hpp
```
- 运行代码指令，非常简单 
```sh
make
./test2
#PS： 输出显示 ---------------输出buffer ------------- 到 ---------------输出buffer end------------- 之间 即为 RDMA read 读取到的文件信息，
# 即表示 RDMA read 成功，
# 程序可以直接 ctrl+c 结束，不用管后面的输出（代码逻辑实现的问题，bug没解决，不影响正常通信）
```

## 250228 修改记录

```sh
# 对如下步骤 原来是
162Storage : xx.5.210  <------> 164Client xx.5.209
## 改成 
162Storage : xx.3.216  <------> 164Client xx.3.212

//RDMA Read 修改参数步骤
4. new_read.hpp中
__commkit_prepare_generic()
{   
    printf("--------- 1.创建 socket -------------\n");
    const char* LocalRdmaIP = "192.168.5.209";       //@@本地客户端的RDMA IP,   网口名为 enp130s0np0
                                                   // dell01 的RDMA IP : 192.168.0.203 ,   网口名为 enp152s0np0
                                                   // 164 oem: ens5f0np0 192.168.5.209 mlx5_0 ; ens4f0np0 192.168.3.212 mlx5_2
    
    strncpy(nicAddr->name, "ens5f0np0", IFNAMSIZ);  //@@本地客户端 RDMA 网口名 
                                                        //164 oem : ens5f0np0/ens4f0np0  ;dell02 : enp130s0np0  ; dell 01: enp152s0np0

    ...
    printf("------------------- 2. 尝试通过IP连接-------------------\n");

    const char* RemoteRdmaIP = "192.168.5.210";   //@@存储服务端的RDMA IP   
                                                     // 162 ubuntu 的 RDMA IP : 192.168.5.210 , ens4f0np0
                                                    //存储服务器xfusion3的RDMA IP,网口名 ens2np0

    unsigned short port = 8003;    //存储服务器的port
}

```

### 生成对应的 RDMA_Read 代码

