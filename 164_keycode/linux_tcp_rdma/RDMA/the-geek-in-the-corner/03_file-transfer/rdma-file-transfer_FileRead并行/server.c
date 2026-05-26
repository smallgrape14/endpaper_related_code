#include <fcntl.h>
#include <sys/stat.h>

#include "common.h"
#include "messages.h"

// #define MAX_FILE_NAME 256
#define MAX_RECV_WR 2  // TODO @@最大同时接收的请求数量
// #include <sys/stat.h>
#include <stdio.h>

long getFileSize(const char *fileName) {
    struct stat fileStat;
    if (stat(fileName, &fileStat) == -1) {
        perror("Failed to get file size");
        return -1;
    }
    return fileStat.st_size;
}

struct conn_context
{
  char *buffer;
  struct ibv_mr *buffer_mr;

  struct message *msg;
  struct ibv_mr *msg_mr;

  uint64_t peer_addr;//xyp insert
  uint32_t peer_rkey; //xyp insert

  int fd;
  char file_name[MAX_FILE_NAME];
};

/*
  static void write_remote(struct rdma_cm_id *id, uint32_t len)
  {
    // struct client_context *ctx = (struct client_context *)id->context;
    struct conn_context *ctx = (struct conn_context *)id->context;

    struct ibv_send_wr wr, *bad_wr = NULL;
    struct ibv_sge sge;

    memset(&wr, 0, sizeof(wr));

    wr.wr_id = (uintptr_t)id;
    wr.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;//RDMA写操作
    wr.send_flags = IBV_SEND_SIGNALED;
    wr.imm_data = htonl(len);//PS: 这里的len是文件数据的长度
    wr.wr.rdma.remote_addr = ctx->peer_addr;//对方的buffer地址
    wr.wr.rdma.rkey = ctx->peer_rkey;//对方的buffer rkey

    if (len) {
      wr.sg_list = &sge;
      wr.num_sge = 1;

      sge.addr = (uintptr_t)ctx->buffer;
      sge.length = len;
      sge.lkey = ctx->buffer_mr->lkey;
    }

    TEST_NZ(ibv_post_send(id->qp, &wr, &bad_wr));
  }
*/
/*
static void post_receive(struct rdma_cm_id *id)
{
  struct ibv_recv_wr wr, *bad_wr = NULL;

  memset(&wr, 0, sizeof(wr));

  wr.wr_id = (uintptr_t)id;
  wr.sg_list = NULL;
  wr.num_sge = 0;

  TEST_NZ(ibv_post_recv(id->qp, &wr, &bad_wr));
}

*/

static void send_message(struct rdma_cm_id *id,char *file_name)//告诉客户端，文件数据写到这里
{

  struct conn_context *ctx = (struct conn_context *)id->context;
  ctx->msg->id = MSG_DONE;
  memcpy(ctx->msg->file_name,file_name,strlen(file_name));
  ctx->msg->file_name[strlen(file_name)]='\0';
  struct ibv_send_wr wr, *bad_wr = NULL;
  struct ibv_sge sge;

  memset(&wr, 0, sizeof(wr));

  wr.wr_id = (uintptr_t)id;
  printf("send_message wr_id %lu\n",wr.wr_id);
  // post_receive(id);
  // post_receive(id);

  wr.opcode = IBV_WR_SEND;
  wr.sg_list = &sge;
  wr.num_sge = 1;
  wr.send_flags = IBV_SEND_SIGNALED;//IBV_SEND_SIGNALED 标志表示发送完成后，网卡将生成一个完成队列条目（CQE），以便应用程序可以轮询完成队列（CQ）来获取完成状态

  sge.addr = (uintptr_t)ctx->msg;
  sge.length =sizeof(*ctx->msg);
  sge.lkey = ctx->msg_mr->lkey;

  TEST_NZ(ibv_post_send(id->qp, &wr, &bad_wr));

  struct ibv_wc wc;
    // while (ibv_poll_cq(id->qp->recv_cq, 1, &wc) == 0) {
    //     usleep(100);
    // }
    // printf("send_message success\n");
    // printf("wc.wr_id %lu\n",wc.wr_id);
    // if (wc.status != IBV_WC_SUCCESS) {
    //     fprintf(stderr, "send_message failed: %s\n", ibv_wc_status_str(wc.status));
    // }


    while (ibv_poll_cq(id->qp->send_cq, 1, &wc) == 0) {
        usleep(100);
    }
    printf("send_message success\n");
    printf("wc.wr_id %lu\n",wc.wr_id);
    if (wc.status != IBV_WC_SUCCESS) {
        fprintf(stderr, "send_message failed: %s\n", ibv_wc_status_str(wc.status));
    }
}



// 用于存储客户端发送的文件请求信息
typedef struct {
    char file_path[MAX_FILE_NAME];
    uint32_t client_rkey;
    uint64_t client_remote_addr;
    struct ibv_qp* qp;
    struct ibv_pd* pd;
    struct rdma_cm_id *id;
} FileRequest;

// 从接收缓冲区中解析文件请求信息
void parse_file_request(const char* buffer, FileRequest* request) {
  
  // printf("buffer %s ,len %ld\n",buffer,strlen(buffer));
    memcpy(&request->client_rkey, buffer, sizeof(uint32_t));
    memcpy(&request->client_remote_addr, buffer + sizeof(uint32_t), sizeof(uint64_t));
    strncpy(request->file_path, buffer + sizeof(uint32_t) + sizeof(uint64_t), MAX_FILE_NAME);
    request->file_path[MAX_FILE_NAME - 1] = '\0'; // 确保字符串以null结尾
    printf("[Server] parse_file_request: %s\n", request->file_path);
    printf("[send_file_request] client_rkey '%u', client_remote_addr %lu,file_path %s \n", request->client_rkey, request->client_remote_addr, request->file_path);

}

// 线程函数：处理单个文件请求
void* process_file_request(void* arg) {
    FileRequest request = *(FileRequest*)arg;
    free(arg); // 释放传递给线程的动态分配的内存

    printf("[Thread] Processing file: %s\n", request.file_path);

    // 打开文件
    int fd = open(request.file_path, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        return NULL;
    }

    // 获取文件大小
    off_t file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    // 分配本地内存并注册为RDMA可访问
    char* smem = malloc(file_size);
    if (!smem) {
        perror("Failed to allocate memory");
        close(fd);
        return NULL;
    }

    struct ibv_mr* mr = ibv_reg_mr(request.pd, smem, file_size, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE);
    if (!mr) {
        perror("Failed to register memory region");
        free(smem);
        close(fd);
        return NULL;
    }

    // 读取文件内容
    // int tobe_read = 1024;
    memcpy(smem, &file_size, sizeof(off_t));
    ssize_t bytes_read = read(fd, smem+sizeof(off_t), file_size);//TODO  : file_size 换 tobe_read 
    printf("bytes_read %ld,filesize %ld ,  \n",bytes_read,file_size);
    // for(int i=0;i<100;i++)
    // {
    //   printf("%02x ",smem[i]);
    // }
    // printf("\n");
    if (bytes_read != file_size) {
        perror("Failed to read file");
        ibv_dereg_mr(mr);
        free(smem);
        close(fd);
        return NULL;
    }

    // 提交RDMA写操作
    struct ibv_sge sge = {
        .addr = (uintptr_t)smem,
        .length = file_size,
        .lkey = mr->lkey
    };

    struct ibv_send_wr wr = {
        .wr_id = 0,
        .opcode = IBV_WR_RDMA_WRITE,
        .send_flags = IBV_SEND_SIGNALED,
        .wr.rdma.remote_addr = request.client_remote_addr,
        .wr.rdma.rkey = request.client_rkey
    };
    // printf("打印客户端的地址信息 \n");
    // printf("request.client_remote_addr %lu,request.client_rkey %u\n",request.client_remote_addr,request.client_rkey);

    wr.sg_list = &sge;
    wr.num_sge = 1;

    struct ibv_send_wr* bad_wr = NULL;
    if (ibv_post_send(request.qp, &wr, &bad_wr)) {
        perror("Failed to post RDMA write");
    }

    // 等待RDMA写操作完成
    struct ibv_wc wc;
    while (ibv_poll_cq(request.qp->send_cq, 1, &wc) == 0) {
        usleep(100);
    }
    if (wc.status != IBV_WC_SUCCESS) {
        fprintf(stderr, "RDMA write failed: %s\n", ibv_wc_status_str(wc.status));
    }

    // 清理资源
    ibv_dereg_mr(mr);
    free(smem);
    close(fd);

    printf("[Thread] Finished processing file: %s\n", request.file_path);
    send_message(request.id, request.file_path);
    return NULL;
}

void receive_file_requests(struct ibv_qp* qp, struct ibv_pd* pd,struct rdma_cm_id *id) 
{
    printf("1.receive_file_requests \n");
    int Request_size = 1024;
    char recv_buffers[MAX_RECV_WR][Request_size];
    struct ibv_mr* mrs[MAX_RECV_WR];
    struct ibv_sge sg_list[MAX_RECV_WR];
    struct ibv_recv_wr recv_wr[MAX_RECV_WR], *bad_wr = NULL;
    
    // 提前注册所有接收缓冲区
    // printf("[Server] 1.提前注册所有接收缓冲区 \n");
    for (int i = 0; i < MAX_RECV_WR; i++) {
        mrs[i] = ibv_reg_mr(pd, recv_buffers[i], Request_size, IBV_ACCESS_LOCAL_WRITE|IBV_ACCESS_REMOTE_READ);
        if (!mrs[i]) {
            perror("Failed to register receive buffer");
            exit(EXIT_FAILURE);
        }


        sg_list[i] = (struct ibv_sge) {
            .addr = (uintptr_t)recv_buffers[i],
            .length = Request_size,
            .lkey = mrs[i]->lkey
        };

        recv_wr[i] = (struct ibv_recv_wr) {
            .wr_id = i,
            .sg_list = &sg_list[i],
            .num_sge = 1
        };

    }

    // 提交所有接收请求
    // printf("[Server] 2.提交所有接收请求 \n");

  for(int i = 0; i < MAX_RECV_WR; i++) {
    recv_wr[i].wr_id = i;  // 确保 wr_id 的值正确
    // printf("recv_wr[i].wr_id %lu\n",recv_wr[i].wr_id);
    if (ibv_post_recv(qp, &recv_wr[i], &bad_wr)) {
        perror("Failed to post receive");
        exit(EXIT_FAILURE);
    }
  }
    // send_message(id);
    // 轮询完成队列，处理所有接收完成的请求
    printf("[Server] 3. 轮询完成队列，处理所有接收完成的请求 \n");
// int timeout = 100000;  // 超时时间（毫秒）
// int poll_count = 0;
    struct ibv_wc wc;
    while (1) {
          // printf("---------- 04 -----------\n");
        int ret=ibv_poll_cq(qp->recv_cq, 1, &wc);
        if(ret)
        printf("4. ret %d\n",ret);
       
        while (ret > 0) {
          // printf("---------- 5 -----------\n");
          FileRequest request;
            request.qp = qp;
            request.pd = pd;

          int wr_id = wc.wr_id;
          printf("4. wr_id %lu\n",wc.wr_id);

            if (wc.status != IBV_WC_SUCCESS) {
                fprintf(stderr, "Receive failed: %s\n", ibv_wc_status_str(wc.status));
                break;
            }
            if(wc.wr_id >= MAX_RECV_WR) {
                fprintf(stderr, "Invalid wr_id: %lu\n", wc.wr_id);
                break;
            }
            else
            {
              printf("[Server] 5. 解析文件请求信息, wr_id %d,wc->wr_id %lu\n",wr_id,wc.wr_id);
              request.id = id;
              parse_file_request(recv_buffers[wr_id], &request);
              // parse_file_request(recv_buffers[0], &request);
              // parse_file_request(recv_buffers[1], &request);
              // parse_file_request(recv_buffers[2], &request);
              // parse_file_request(recv_buffers[3], &request);
            }


          // printf("---------- 6 -----------\n");
                      
            




            // 创建线程处理文件请求
            printf("[Server] 6. 创建线程处理文件请求 \n");

            FileRequest* thread_request = malloc(sizeof(FileRequest));
            *thread_request = request;
            pthread_t thread;
            pthread_create(&thread, NULL, process_file_request, thread_request);
            pthread_detach(thread);
            // break;
            // 重新提交接收请求
            if (ibv_post_recv(qp, &recv_wr[wr_id], &bad_wr)) {
                perror("Failed to repost receive");
            }
           ret= ibv_poll_cq(qp->recv_cq, 1, &wc);
        }
        // printf("---------- 7 -----------\n");
        usleep(100); // 避免CPU占用过高
        //  poll_count += 100;
        // if (poll_count > timeout) {
        //     fprintf(stderr, "Timeout waiting for completion event\n");
        //     exit(EXIT_FAILURE);
        // }
    }
}

/*

串行接收逻辑代码
// 接收客户端发送的文件请求消息并处理
void receive_file_request(struct ibv_qp* qp, struct ibv_pd* pd) {
    char recv_buffer[BUFFER_SIZE];
    struct ibv_mr* mr = ibv_reg_mr(pd, recv_buffer, BUFFER_SIZE, IBV_ACCESS_LOCAL_WRITE);
    if (!mr) {
        perror("Failed to register receive buffer");
        return;
    }

    struct ibv_sge sge = {
        .addr = (uintptr_t)recv_buffer,
        .length = BUFFER_SIZE,
        .lkey = mr->lkey
    };

    struct ibv_recv_wr wr = {
        .wr_id = 0,
        .sg_list = &sge,
        .num_sge = 1
    };

    struct ibv_recv_wr* bad_wr = NULL;
    if (ibv_post_recv(qp, &wr, &bad_wr)) {
        perror("Failed to post receive");
        ibv_dereg_mr(mr);
        return;
    }

    // 等待接收完成
    struct ibv_wc wc;
    while (ibv_poll_cq(qp->recv_cq, 1, &wc) == 0) {
        usleep(100);
    }
    if (wc.status != IBV_WC_SUCCESS) {
        fprintf(stderr, "Receive failed: %s\n", ibv_wc_status_str(wc.status));
        ibv_dereg_mr(mr);
        return;
    }

    // 解析文件请求信息
    FileRequest request;
    request.qp = qp;
    request.pd = pd;
    parse_file_request(recv_buffer, &request);

    // 为每个文件请求创建一个线程
    FileRequest* thread_request = malloc(sizeof(FileRequest));
    *thread_request = request;
    pthread_t thread;
    pthread_create(&thread, NULL, process_file_request, thread_request);
    pthread_detach(thread); // 分离线程，避免等待线程结束

    printf("[Main] Created thread to process file: %s\n", request.file_path);

    // 清理资源
    ibv_dereg_mr(mr);
}
*/




static void on_pre_conn(struct rdma_cm_id *id)
{
  printf("[Server] on_pre_conn \n");

  struct conn_context *ctx = (struct conn_context *)malloc(sizeof(struct conn_context));

  id->context = ctx;

  ctx->file_name[0] = '\0'; // take this to mean we don't have the file name

  posix_memalign((void **)&ctx->buffer, sysconf(_SC_PAGESIZE), BUFFER_SIZE);
  TEST_Z(ctx->buffer_mr = ibv_reg_mr(rc_get_pd(), ctx->buffer, BUFFER_SIZE, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE|IBV_ACCESS_REMOTE_READ));

  posix_memalign((void **)&ctx->msg, sysconf(_SC_PAGESIZE), sizeof(*ctx->msg));
  TEST_Z(ctx->msg_mr = ibv_reg_mr(rc_get_pd(), ctx->msg, sizeof(*ctx->msg), 0));

  // post_receive(id);
}

static void on_connection(struct rdma_cm_id *id)
{
  printf("[Server] on_connection \n");
  struct conn_context *ctx = (struct conn_context *)id->context;

  ctx->msg->id = MSG_MR;
  ctx->msg->data.mr.addr = (uintptr_t)ctx->buffer_mr->addr;
  ctx->msg->data.mr.rkey = ctx->buffer_mr->rkey;
  //TODO 这里应该加上ibv_post_recv接收的逻辑
  // send_message(id);
  receive_file_requests(id->qp, id->qp->pd,id);

}

/*
static void on_completion(struct ibv_wc *wc)
{
  struct rdma_cm_id *id = (struct rdma_cm_id *)(uintptr_t)wc->wr_id;
  struct conn_context *ctx = (struct conn_context *)id->context;
  printf("opcode: %d\n", wc->opcode);
  if (wc->opcode == IBV_WC_RECV_RDMA_WITH_IMM) {
    printf("received RDMA write with immediate data\n");
    uint32_t size = ntohl(wc->imm_data);

    if (size == 0) {//长度为0
      printf("2. received 0-length message, closing connection\n");
      ctx->msg->id = MSG_DONE;
      send_message(id);

      // don't need post_receive() since we're done with this connection

    } 
   
    // else if (ctx->file_name[0]) {//第二次进入这个分支，从指定内存缓冲区中取出文件数据并写入到文件中
    //   ssize_t ret;

    //   printf("received %i bytes.\n", size);

    //   ret = write(ctx->fd, ctx->buffer, size);//写入到对应的文件中

    //   if (ret != size)
    //     rc_die("write() failed");

    //   post_receive(id);

    //   ctx->msg->id = MSG_READY;
    //   send_message(id);

    // }
    
     else { //读文件数据进入的是这个分支
      size = (size > MAX_FILE_NAME) ? MAX_FILE_NAME : size;
      memcpy(ctx->file_name, ctx->buffer, size);
      ctx->file_name[size - 1] = '\0';

      printf("1. opening file %s\n", ctx->file_name);

      ctx->fd = open(ctx->file_name,O_RDONLY);
      long total_size = getFileSize(ctx->file_name);
      int offset = 0;
      do
      {
        int to_read = total_size - offset;
        int read_res = pread(ctx->fd, ctx->buffer+offset, BUFFER_SIZE<to_read?BUFFER_SIZE:to_read, offset);
        printf("read_res: %d\n", read_res);
        offset += read_res;
        post_receive(id);
        ctx->msg->id = MSG_READY;
        send_message(id);
      }while(offset < total_size);

      // int read_res = read(ctx->fd, ctx->buffer, BUFFER_SIZE<total_size?BUFFER_SIZE:total_size);
      // printf("read_res: %d\n", read_res);
      // ctx->fd = open(ctx->file_name, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

      if (ctx->fd == -1)
        {
          switch (errno) {
            case EEXIST:
                fprintf(stderr, "File already exists: %s\n", ctx->file_name);
                break;
            case EACCES:
                fprintf(stderr, "Permission denied: %s\n", ctx->file_name);
                break;
            case ENOENT:
                fprintf(stderr, "File or directory not found: %s\n", ctx->file_name);
                break;
            case ENOSPC:
                fprintf(stderr, "No space left on device\n");
                break;
            case EMFILE:
                fprintf(stderr, "Too many open files in process\n");
                break;
            case ENFILE:
                fprintf(stderr, "Too many open files in system\n");
                break;
            default:
                fprintf(stderr, "open() failed: %s\n", strerror(errno));
                break;
        }
          rc_die("open() failed");
        }

      post_receive(id);

      ctx->msg->id = MSG_READY;//通知对方可以开始发送文件数据了
                                //变成通知对方可以开始接收文件数据了

      // ctx->peer_addr = ctx->msg->data.mr.addr;//对方的buffer地址
      // ctx->peer_rkey = ctx->msg->data.mr.rkey;//对方的buffer rkey

      send_message(id);
      // write_remote(id, read_res);//写入远端

    }
  }
  else if(wc->opcode ==IBV_WC_RDMA_READ)
  {
    printf("2.received RDMA read\n");
    printf("received %i bytes.\n", wc->byte_len);
    post_receive(id);
    ctx->msg->id = MSG_READY;
    send_message(id);
  }
}
*/



// static void on_completion(struct ibv_wc *wc)
// {
//   struct rdma_cm_id *id = (struct rdma_cm_id *)(uintptr_t)wc->wr_id;
//   struct conn_context *ctx = (struct conn_context *)id->context;
//   printf("opcode: %d\n", wc->opcode);
//   if (wc->opcode ==IBV_WC_RECV)//接收到客户端发送的read请求消息
//   {
//       // rdma_post_recv()
//       // receive_file_request(id->qp, id->qp->pd);
//   }
// }

static void on_disconnect(struct rdma_cm_id *id)
{
  struct conn_context *ctx = (struct conn_context *)id->context;

  close(ctx->fd);

  ibv_dereg_mr(ctx->buffer_mr);
  ibv_dereg_mr(ctx->msg_mr);

  free(ctx->buffer);
  free(ctx->msg);

  printf("finished transferring %s\n", ctx->file_name);

  free(ctx);
}

int main(int argc, char **argv)
{
  rc_init(
    on_pre_conn,
    on_connection,
    // on_completion,
    NULL,
    on_disconnect);

  printf("waiting for connections. interrupt (^C) to exit.\n");

  rc_server_loop(DEFAULT_PORT);

  return 0;
}
