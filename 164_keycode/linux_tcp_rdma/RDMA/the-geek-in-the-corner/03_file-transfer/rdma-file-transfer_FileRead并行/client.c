#include <fcntl.h>
#include <libgen.h>

#include "common.h"
#include "messages.h"
#include <sys/stat.h>
#include <stdio.h>
#include <infiniband/verbs.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int file_num=2;
int file_idx=0;
char file_name[MAX_FILE_NUM][MAX_FILE_NAME]={
"/home/oem/xyp/the-geek-in-the-corner/FashionMNIST/raw/t10k-images-idx3-ubyte",
"/home/oem/xyp/the-geek-in-the-corner/FashionMNIST/raw/t10k-labels-idx1-ubyte",
"/home/oem/xyp/the-geek-in-the-corner/FashionMNIST/raw/train-images-idx3-ubyte",
"/home/oem/xyp/the-geek-in-the-corner/FashionMNIST/raw/train-labels-idx1-ubyte"
};

long getFileSize(const char *fileName) {
    struct stat fileStat;
    if (stat(fileName, &fileStat) == -1) {
        perror("Failed to get file size");
        return -1;
    }
    return fileStat.st_size;
}

struct client_context
{
  char *buffer;
  struct ibv_mr *buffer_mr;

  struct message *msg;
  struct ibv_mr *msg_mr;

  uint64_t peer_addr;
  uint32_t peer_rkey;

  int fd;
  const char *file_name;
};

// #define TEST_NZ(x) do { if ((x) != 0) { perror("Error"); exit(1); } } while (0)
/*
static void read_remote(struct rdma_cm_id *id, uint32_t len, const char *output_file)
{
    struct client_context *ctx = (struct client_context *)id->context;

    struct ibv_send_wr wr, *bad_wr = NULL;
    struct ibv_sge sge;

    memset(&wr, 0, sizeof(wr));

    wr.wr_id = (uintptr_t)id;
    // wr.imm_data=0;
    wr.opcode = IBV_WR_RDMA_READ; // RDMA Read 操作
    wr.send_flags = IBV_SEND_SIGNALED;
    wr.wr.rdma.remote_addr = ctx->peer_addr; // 对端的 buffer 地址
    wr.wr.rdma.rkey = ctx->peer_rkey;        // 对端的 buffer rkey

    if (len) {
        wr.sg_list = &sge;
        wr.num_sge = 1;

        sge.addr = (uintptr_t)ctx->buffer; // 本地缓冲区地址，用于存储读取的数据
        sge.length = len;                  // 读取的数据长度
        sge.lkey = ctx->buffer_mr->lkey;   // 本地缓冲区的 lkey
    }

    TEST_NZ(ibv_post_send(id->qp, &wr, &bad_wr));

    // 等待 RDMA Read 完成
    // struct ibv_wc wc;
    // do {
    //     TEST_NZ(ibv_poll_cq(ctx->cq, 1, &wc));
    // } while (wc.wr_id != (uintptr_t)id);

    // 将读取的数据写入本地文件
    FILE *file = fopen(output_file, "wb");
    if (!file) {
        perror("Failed to open output file");
        exit(1);
    }

    fwrite(ctx->buffer, 1, len, file);
    fclose(file);

    printf("Data read from remote memory and written to %s\n", output_file);

}

static void write_remote(struct rdma_cm_id *id, uint32_t len)
{
  struct client_context *ctx = (struct client_context *)id->context;

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
static void post_receive(struct rdma_cm_id *id)
{
  struct client_context *ctx = (struct client_context *)id->context;

  struct ibv_recv_wr wr, *bad_wr = NULL;
  struct ibv_sge sge;

  memset(&wr, 0, sizeof(wr));

  wr.wr_id = (uintptr_t)id;
  wr.sg_list = &sge;
  wr.num_sge = 1;

  sge.addr = (uintptr_t)ctx->msg;
  sge.length = sizeof(*ctx->msg);
  sge.lkey = ctx->msg_mr->lkey;

  TEST_NZ(ibv_post_recv(id->qp, &wr, &bad_wr));
}

/*
  static void send_next_chunk(struct rdma_cm_id *id)
  {
    struct client_context *ctx = (struct client_context *)id->context;

    ssize_t size = 0;

    size = read(ctx->fd, ctx->buffer, BUFFER_SIZE);

    if (size == -1)
      rc_die("read() failed\n");

    write_remote(id, size);
  }
*/

/*
static void send_file_name(struct rdma_cm_id *id,int file_idx)
{
  struct client_context *ctx = (struct client_context *)id->context;

  // strcpy(ctx->buffer, ctx->file_name);
  // write_remote(id, strlen(ctx->file_name) + 1);
    strcpy(ctx->buffer, file_name[file_idx]);
  write_remote(id, strlen(file_name[file_idx]) + 1);
}
*/
static void on_pre_conn(struct rdma_cm_id *id)
{
  printf("[Client] on_pre_conn \n");

  struct client_context *ctx = (struct client_context *)id->context;

  posix_memalign((void **)&ctx->buffer, sysconf(_SC_PAGESIZE), BUFFER_SIZE);
  TEST_Z(ctx->buffer_mr = ibv_reg_mr(rc_get_pd(), ctx->buffer, BUFFER_SIZE, IBV_ACCESS_LOCAL_WRITE));

  posix_memalign((void **)&ctx->msg, sysconf(_SC_PAGESIZE), sizeof(*ctx->msg));
  TEST_Z(ctx->msg_mr = ibv_reg_mr(rc_get_pd(), ctx->msg, sizeof(*ctx->msg), IBV_ACCESS_LOCAL_WRITE));//TODO

  post_receive(id);
}
/*
static void on_completion(struct ibv_wc *wc)
{
  struct rdma_cm_id *id = (struct rdma_cm_id *)(uintptr_t)(wc->wr_id);
  struct client_context *ctx = (struct client_context *)id->context;

  if (wc->opcode & IBV_WC_RECV) {// 1th 发送文件名字消息
    if (ctx->msg->id == MSG_MR) {
      ctx->peer_addr = ctx->msg->data.mr.addr;//对方的buffer地址
      ctx->peer_rkey = ctx->msg->data.mr.rkey;//对方的buffer rkey

      printf("1.received MR, sending file name\n");
      send_file_name(id,file_idx);
    } else if (ctx->msg->id == MSG_READY) {// 2th 发送文件数据
      printf("2.received READY, Reading file data\n");
      // send_next_chunk(id);
      uint32_t read_length = getFileSize(file_name[file_idx]); // 读取的数据长度
      char output_file[100] = "output/out_"; // 输出文件路径
      char *last_slash = strrchr(file_name[file_idx], '/');
        if (last_slash != NULL) {
            // 提取文件名部分
            strcat(output_file, last_slash + 1);  // 跳过 '/' 后的部分
        } 
      printf("file[%d] output_file:%s , read_length %d\n",file_idx,output_file,read_length);
      read_remote(id, read_length, output_file);

      //文件读取完成，发出下一个文件读取请求
      file_idx++;
      if(file_idx<file_num)
      {
        send_file_name(id,file_idx);
      }
      else
      {
        printf("received DONE, disconnecting\n");
        rc_disconnect(id);
        return ;
      }

    } else if (ctx->msg->id == MSG_DONE) {
      printf("received DONE, disconnecting\n");
      rc_disconnect(id);
      return;
    }

    post_receive(id);
  }
}
*/
// 线程参数结构
typedef struct {
    struct ibv_qp *qp;  // 队列对
    char file_path[MAX_FILE_NAME];  // 文件路径
    uint32_t client_rkey;  // 客户端的 RKey
    uint64_t client_remote_addr;  // 客户端的远程地址
    void *cmem;  // 客户端内存区域
    struct ibv_mr *cmem_mr;  // 客户端内存区域的 MR
    int file_idx;
    struct rdma_cm_id *id;
} FileRequestThreadParam;
// 发送文件请求消息到服务器
/*
static void post_receive_qp(struct ibv_qp *qp,int file_idx)
{
  struct ibv_recv_wr wr, *bad_wr = NULL;

  memset(&wr, 0, sizeof(wr));

  wr.wr_id = (uintptr_t)file_idx;
  wr.sg_list = NULL;
  wr.num_sge = 0;

  TEST_NZ(ibv_post_recv(qp, &wr, &bad_wr));
}
*/
// 接收消息的函数
static void receive_message(struct rdma_cm_id *id)
{
    struct client_context *ctx = (struct client_context *)id->context;
    struct ibv_recv_wr wr, *bad_wr = NULL;
    struct ibv_sge sge;

    // 初始化接收工作请求
    memset(&wr, 0, sizeof(wr));
    wr.wr_id = (uintptr_t)id;
    wr.sg_list = &sge;
    wr.num_sge = 1;

    // 设置接收缓冲区
    sge.addr = (uintptr_t)ctx->msg;
    sge.length = sizeof(*ctx->msg); // 确保与发送端一致
    sge.lkey = ctx->msg_mr->lkey;

    // 提交接收请求
    if (ibv_post_recv(id->qp, &wr, &bad_wr)) {
        fprintf(stderr, "Failed to post receive request\n");
        exit(1);
    }
}

void send_file_request(struct ibv_qp *qp, const char *file_path, uint32_t client_rkey, uint64_t client_remote_addr,int file_idx,struct rdma_cm_id *id) {
    printf("发送文件请求之前，先post_recv 来感知 rdma write 的完成 \n");

    // post_receive_qp(qp,file_idx);
    receive_message(id);
    
    
    
    
    
    
    printf("1.[send_file_request] file_path '%s'\n", file_path);
    char send_buffer[1024];
    memcpy(send_buffer, &client_rkey, sizeof(uint32_t));
    // printf("[send_file_request] client_rkey '%d', client_remote_addr %lu,file_path %s \n", client_rkey, client_remote_addr, file_path);
    memcpy(send_buffer + sizeof(uint32_t), &client_remote_addr, sizeof(uint64_t));
    memcpy(send_buffer + sizeof(uint32_t) + sizeof(uint64_t), file_path, MAX_FILE_NAME);

    struct ibv_send_wr wr, *bad_wr = NULL;
    struct ibv_sge sge;

    memset(&wr, 0, sizeof(wr));
    memset(&sge, 0, sizeof(sge));

    wr.wr_id = (uintptr_t)file_idx;
    wr.opcode = IBV_WR_SEND;
    wr.send_flags = IBV_SEND_SIGNALED;

    sge.addr = (uintptr_t)send_buffer;
    sge.length = sizeof(send_buffer);
    sge.lkey = ibv_reg_mr(qp->pd, send_buffer, sizeof(send_buffer), IBV_ACCESS_LOCAL_WRITE|IBV_ACCESS_REMOTE_READ)->lkey;
    // printf("send_buffer print begin \n");
    // for(int i=0;i<sizeof(send_buffer);i++)
    // {
    //     printf("%02x ",send_buffer[i]);
    // }
    // printf("send_buffer end  \n");

    wr.sg_list = &sge;
    wr.num_sge = 1;

    if (ibv_post_send(qp, &wr, &bad_wr)) {
        perror("ibv_post_send failed");
        exit(EXIT_FAILURE);
    }

    struct ibv_wc wc;
    while (ibv_poll_cq(qp->send_cq, 1, &wc) == 0||wc.wr_id!=file_idx) {

        usleep(100);
    }
    // printf("send_file_request success\n");
    // printf("wc.wr_id %lu\n",wc.wr_id);
    if (wc.status != IBV_WC_SUCCESS) {
        fprintf(stderr, "Send failed: %s\n", ibv_wc_status_str(wc.status));
    }
}
// 保存文件到磁盘
void save_file(const char *file_path, void *buffer, size_t size) {
    FILE *file = fopen(file_path, "wb");
    if (!file) {
        perror("fopen failed");
        return;
    }

    fwrite(buffer, 1, size, file);
    fclose(file);
    printf("File saved to '%s'\n", file_path);
}

// 线程函数：处理单个文件请求
void *handle_file_request(void *arg) {
    FileRequestThreadParam *param = (FileRequestThreadParam *)arg;

    // 注册内存区域
    // printf(" 注册内存区域 \n");
    param->cmem = malloc(MAX_FILE_SIZE);
    param->cmem_mr = ibv_reg_mr(param->qp->pd, param->cmem, MAX_FILE_SIZE, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE);
    if (!param->cmem_mr) {
        perror("ibv_reg_mr failed");
        free(param->cmem);
        return NULL;
    }

    // 更新客户端的 rkey 和 remote_addr
    param->client_rkey = param->cmem_mr->rkey;
    param->client_remote_addr = (uintptr_t)param->cmem;

    // 发送文件请求
    // printf(" 发送文件请求 \n");
    send_file_request(param->qp, param->file_path, param->client_rkey, param->client_remote_addr,param->file_idx,param->id);

    // 等待 RDMA Write 完成
    printf("等待 RDMA Write 完成 \n");
    struct ibv_wc wc;

    while (ibv_poll_cq(param->qp->recv_cq, 1, &wc) == 0) {
      // printf("轮询中 \n");
      // for(int i=0;i<100;i++)
      // {
      //   // printf("%02x ",(unsigned char)((char*)(param->client_remote_addr)[i]));
      //   printf("%02x ", (unsigned char)((char*)param->client_remote_addr)[i]);
      // }
        usleep(100);
    }
    printf("2.等待 RDMA Write 完成 轮询结束\n");
    if (wc.status != IBV_WC_SUCCESS) {
        fprintf(stderr, "RDMA Write failed: %s\n", ibv_wc_status_str(wc.status));
    }
    struct rdma_cm_id *tmp_id = (struct rdma_cm_id *)(uintptr_t)wc.wr_id;
    struct client_context *tmp_ctx = (struct client_context *)tmp_id->context;
    // 检查消息内容
    if (tmp_ctx->msg->id == MSG_DONE) {
        printf("Received MSG_DONE message\n");
        printf("File name: %s\n", tmp_ctx->msg->file_name);
       // 保存文件到磁盘
        printf(" 3.保存文件到磁盘 \n");
        char output_file[100] = "output/out_"; // 输出文件路径
          char *last_slash = strrchr(tmp_ctx->msg->file_name, '/');
            if (last_slash != NULL) {
                // 提取文件名部分
                strcat(output_file, last_slash + 1);  // 跳过 '/' 后的部分
            } 
        off_t file_size=0;
        memcpy(&file_size,param->cmem, sizeof(off_t));
        // printf("4. file[%d] output_file:%s ,file_size %ld \n",file_idx,output_file,file_size);
        printf("4. file_size %ld \n",file_size);
        save_file(output_file, param->cmem+sizeof(off_t), file_size);//TODO

    } else {
        printf("Received unknown message ID: %d\n", tmp_ctx->msg->id);
    }
   

    // 清理资源
    ibv_dereg_mr(param->cmem_mr);
    free(param->cmem);

    printf("5. File '%s' received and saved.\n", param->file_path);
    return NULL;
}

// static void on_completion(struct ibv_wc *wc)
static void on_completion(struct rdma_cm_id *id)
{
  // struct rdma_cm_id *id = (struct rdma_cm_id *)(uintptr_t)(wc->wr_id);
  // struct client_context *ctx = (struct client_context *)id->context;
 printf("1. sending file name, send Read file request\n");
    
    // printf("wc.wr_id %lu\n",wc->wr_id);
    // 创建线程处理每个文件请求
    pthread_t threads[MAX_FILE_NUM];
    FileRequestThreadParam params[MAX_FILE_NUM];

    for (int i = 0; i < file_num && i < MAX_FILE_NUM; i++) {
        params[i].qp =id->qp;
        strcpy(params[i].file_path, file_name[i]);
        params[i].file_idx=i;
        params[i].id=id;
        pthread_create(&threads[i], NULL, handle_file_request, &params[i]);
    }

    // 等待所有线程完成
    for (int i = 0; i < file_num && i < MAX_FILE_NUM; i++) {
        pthread_join(threads[i], NULL);
    }

/*
  if (wc->opcode & IBV_WC_RECV) {// 1th 发送文件名字消息
    if (ctx->msg->id == MSG_MR) {//这个信号是对方发送的内存注册信息 但我们暂时将其当作可以开始传输文件数据的信号
      ctx->peer_addr = ctx->msg->data.mr.addr;//对方的buffer地址
      ctx->peer_rkey = ctx->msg->data.mr.rkey;//对方的buffer rkey

      printf("1.received MR, sending file name, send Read file request\n");
    
    printf("wc.wr_id %lu\n",wc->wr_id);
    // 创建线程处理每个文件请求
    pthread_t threads[MAX_FILE_NUM];
    FileRequestThreadParam params[MAX_FILE_NUM];

    for (int i = 0; i < file_num && i < MAX_FILE_NUM; i++) {
        params[i].qp =id->qp;
        strcpy(params[i].file_path, file_name[i]);
        pthread_create(&threads[i], NULL, handle_file_request, &params[i]);
    }

    // 等待所有线程完成
    for (int i = 0; i < file_num && i < MAX_FILE_NUM; i++) {
        pthread_join(threads[i], NULL);
    }
    
    
    } else if (ctx->msg->id == MSG_READY) {// 2th 发送文件数据
      printf("2.received READY, Reading file data\n");
      // send_next_chunk(id);
      uint32_t read_length = getFileSize(file_name[file_idx]); // 读取的数据长度
      char output_file[100] = "output/out_"; // 输出文件路径
      char *last_slash = strrchr(file_name[file_idx], '/');
        if (last_slash != NULL) {
            // 提取文件名部分
            strcat(output_file, last_slash + 1);  // 跳过 '/' 后的部分
        } 
      printf("file[%d] output_file:%s , read_length %d\n",file_idx,output_file,read_length);
      read_remote(id, read_length, output_file);

      //文件读取完成，发出下一个文件读取请求
      file_idx++;
      if(file_idx<file_num)
      {
        send_file_name(id,file_idx);
      }
      else
      {
        printf("received DONE, disconnecting\n");
        rc_disconnect(id);
        return ;
      }

    } else if (ctx->msg->id == MSG_DONE) {
      printf("received DONE, disconnecting\n");
      rc_disconnect(id);
      return;
    }

    post_receive(id);
  }
*/
}



int main(int argc, char **argv)
{
  struct client_context ctx;

  if (argc != 3) {
    fprintf(stderr, "usage: %s <server-address> <file-name>\n", argv[0]);
    return 1;
  }

  ctx.file_name = basename(argv[2]);
  ctx.fd = open(argv[2], O_RDONLY);

  if (ctx.fd == -1) {
    fprintf(stderr, "unable to open input file \"%s\"\n", ctx.file_name);
    return 1;
  }

  rc_init(
    on_pre_conn,
    NULL, // on connect
    on_completion,
    NULL); // on disconnect

  rc_client_loop(argv[1], DEFAULT_PORT, &ctx);

  close(ctx.fd);

  return 0;
}

