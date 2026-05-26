
# 250224 最新版本
## 编译运行指令

```shell
make
./server
./client
```

## 修改参数
- 在 `server.c / client.c` 中搜索关键字 `TARGET_IP` 和 `TARGET_PORT`

```c
#define TARGET_IP "192.168.3.212"  // 替换为特定IP地址
#define TARGET_PORT "54321"           // 替换为特定端口号
```


## 功能描述 

```sh
client ---------1. cm request ----------> server 
client ---------2. RDMA_Send (file_path,virtual_address,remote_key,file_request_size)-----------> server
client <--------3. server open file, read file to local_buffer, and then RDMA_Write to remote client  ----------- server
client <--------4. RDMA_Send, send_completion_ack,1B(0x11)的确认信号  ----------- server
client ---------5. Recv (0x11) 后，read buffer ,out buffer content to a file

```
