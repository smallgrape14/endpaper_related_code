## 250407 
功能：
- 模拟beegfs-client 用linux rdma 高并发发送 文件读请求 
```sh

修改读文件数量: 在beegfs/getentryid.hpp 中
#define FILE_NUM 64
修改读目录时候 ： client.cpp 中 和 在beegfs/getentryid.hpp 中
（1）
char inputsearchPath[50] = "test_dir_1000";
（2）
unsigned int origParentUID=1000;//上帝视角的数据设置 test_dir(500个512B文件) small_image (12个文件)

# 运行指南
运行指令：
 
rm client  # 因为修改了 beegfs/目录下的文件 make不会重新编译
make
./client
```


### BUG： post send 超过 16 个之后，send 请求目标是 256个 ，但是只发出了 70多个，recv 接收打印 M_buffer ,和 data_buffer 发现只成功接收到了两个

(已解决)存在问题：超过 15个文件请求会不能完全接收到 storage的 RDMA write 和 RDMA send 消息,
且beegfs-storage处理 read 请求 有报错提示 ，在日志中

```sh
日志报错如下：
writeRes: -1
[ReadChunkFileRDMAMsg (read incremental)] >> Unable to write file data to client. 
[[xyp_debug_0306]] >> Unable to write file data to client.
...

一般情况下 日志报错如此显示:需要检查 vaddr rkey 是否正确，
```

- 进一步说明： 如果设置 FILE_NUM == 16个 ，那么send /recv /打印缓冲区都是正确的
- 并且抓包文件中 client 发送方的ACK 出现了 credit count == 0 的情况

错误的原因：
```sh
完成队列（CQ）容量不足的问题
    ​​CQ深度不足​​
    每个IBV_SEND_SIGNALED请求会产生一个完成事件（CQE）。若CQ深度（如默认值10）小于批量提交的请求数，CQ溢出会导致后续请求失败。
​​信号频率过高​​
    若每个请求均设置IBV_SEND_SIGNALED，密集的完成事件会迅速耗尽CQ资源。优化方案：
解决思路：
    每N个请求触发一次信号（如每16个请求发一次信号）
    使用非阻塞的批量轮询（ibv_poll_cq）替代逐事件处理
```
解决思路：每N个请求触发一次信号（如每16个请求发一次信号）

```c
//思路一： 减少完成事件的信号频率
//关键是这个 ：16个请求发一次完成信号[3](@ref)
// wr_list[i].send_flags = (i%16 == 0) ? IBV_SEND_SIGNALED : 0; //@@@@ 每16个请求发一次完成信号[3](@ref)
  {  //方法二 : 批量提交
    
      struct ibv_send_wr *wr_list =(struct ibv_send_wr *) malloc(FILE_NUM * sizeof(struct ibv_send_wr));
      struct ibv_sge *sge_list =(struct ibv_sge *) malloc(FILE_NUM * sizeof(struct ibv_sge));
      // struct ibv_send_wr *bad_wr = NULL;

      for (int i=0; i<FILE_NUM; i++) {
          // 构建SGE描述符
          memcpy(conn->send_pool[i], read_msg+i*MAX_REQUEST_LEN,read_msg_len[i]);
          sge_list[i].addr = (uintptr_t)conn->send_pool[i];
          sge_list[i].length = read_msg_len[i];
          sge_list[i].lkey = conn->send_mr_pool[i]->lkey;

          // 配置工作请求
          wr_list[i].wr_id = (uintptr_t)conn;
          wr_list[i].next = (i < FILE_NUM-1) ? &wr_list[i+1] : NULL;
          wr_list[i].sg_list = &sge_list[i];
          wr_list[i].num_sge = 1;
          wr_list[i].opcode = IBV_WR_SEND;
          // wr_list[i].send_flags = IBV_SEND_SIGNALED; // 每16个请求发一次完成信号[3](@ref)

          wr_list[i].send_flags = (i%16 == 0) ? IBV_SEND_SIGNALED : 0; //@@@@ 每16个请求发一次完成信号[3](@ref)
          
          // // 设置内联数据优化（适用于小消息）
          // if (read_msg_len[i] <= 256) {
          //     wr_list[i].send_flags |= IBV_SEND_INLINE;  // 减少PCIe总线访问[3,7](@ref)
          // }
      }

    
        // 批量提交全部发送请求
      if (ibv_post_send(conn->qp, &wr_list[0], &bad_wr)) {
          int failed_idx = bad_wr - &wr_list[0];
          fprintf(stderr, "批量提交失败，首个错误索引:%d，错误码:%s", failed_idx, strerror(errno));
      }
  }
//思路二：修改配置
  int cq_size = 1024;  // 设置CQ深度为1024
  TEST_Z(s_ctx->cq = ibv_create_cq(s_ctx->ctx, cq_size, NULL, s_ctx->comp_channel, 0)); /* cqe=10 is arbitrary */

```


