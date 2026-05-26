#include <fcntl.h>
#include <libgen.h>

#include "common.h"
#include "messages.h"
#include <sys/stat.h>
#include <stdio.h>
int file_num=4;
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

static void send_file_name(struct rdma_cm_id *id,int file_idx)
{
  struct client_context *ctx = (struct client_context *)id->context;

  // strcpy(ctx->buffer, ctx->file_name);
  // write_remote(id, strlen(ctx->file_name) + 1);
    strcpy(ctx->buffer, file_name[file_idx]);
  write_remote(id, strlen(file_name[file_idx]) + 1);
}

static void on_pre_conn(struct rdma_cm_id *id)
{
  struct client_context *ctx = (struct client_context *)id->context;

  posix_memalign((void **)&ctx->buffer, sysconf(_SC_PAGESIZE), MAX_FILE_SIZE);
  TEST_Z(ctx->buffer_mr = ibv_reg_mr(rc_get_pd(), ctx->buffer, MAX_FILE_SIZE, IBV_ACCESS_LOCAL_WRITE));

  posix_memalign((void **)&ctx->msg, sysconf(_SC_PAGESIZE), sizeof(*ctx->msg));
  TEST_Z(ctx->msg_mr = ibv_reg_mr(rc_get_pd(), ctx->msg, sizeof(*ctx->msg), IBV_ACCESS_LOCAL_WRITE));//TODO

  post_receive(id);
}

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

