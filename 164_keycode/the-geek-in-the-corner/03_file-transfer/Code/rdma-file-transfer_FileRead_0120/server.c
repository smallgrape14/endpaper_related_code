#include <fcntl.h>
#include <sys/stat.h>

#include "common.h"
#include "messages.h"

#define MAX_FILE_NAME 256

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

// static void write_remote(struct rdma_cm_id *id, uint32_t len)
// {
//   // struct client_context *ctx = (struct client_context *)id->context;
//   struct conn_context *ctx = (struct conn_context *)id->context;

//   struct ibv_send_wr wr, *bad_wr = NULL;
//   struct ibv_sge sge;

//   memset(&wr, 0, sizeof(wr));

//   wr.wr_id = (uintptr_t)id;
//   wr.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;//RDMA写操作
//   wr.send_flags = IBV_SEND_SIGNALED;
//   wr.imm_data = htonl(len);//PS: 这里的len是文件数据的长度
//   wr.wr.rdma.remote_addr = ctx->peer_addr;//对方的buffer地址
//   wr.wr.rdma.rkey = ctx->peer_rkey;//对方的buffer rkey

//   if (len) {
//     wr.sg_list = &sge;
//     wr.num_sge = 1;

//     sge.addr = (uintptr_t)ctx->buffer;
//     sge.length = len;
//     sge.lkey = ctx->buffer_mr->lkey;
//   }

//   TEST_NZ(ibv_post_send(id->qp, &wr, &bad_wr));
// }

static void send_message(struct rdma_cm_id *id)//告诉客户端，文件数据写到这里
{
  struct conn_context *ctx = (struct conn_context *)id->context;

  struct ibv_send_wr wr, *bad_wr = NULL;
  struct ibv_sge sge;

  memset(&wr, 0, sizeof(wr));

  wr.wr_id = (uintptr_t)id;
  wr.opcode = IBV_WR_SEND;
  wr.sg_list = &sge;
  wr.num_sge = 1;
  wr.send_flags = IBV_SEND_SIGNALED;//IBV_SEND_SIGNALED 标志表示发送完成后，网卡将生成一个完成队列条目（CQE），以便应用程序可以轮询完成队列（CQ）来获取完成状态

  sge.addr = (uintptr_t)ctx->msg;
  sge.length = sizeof(*ctx->msg);
  sge.lkey = ctx->msg_mr->lkey;

  TEST_NZ(ibv_post_send(id->qp, &wr, &bad_wr));
}

static void post_receive(struct rdma_cm_id *id)
{
  struct ibv_recv_wr wr, *bad_wr = NULL;

  memset(&wr, 0, sizeof(wr));

  wr.wr_id = (uintptr_t)id;
  wr.sg_list = NULL;
  wr.num_sge = 0;

  TEST_NZ(ibv_post_recv(id->qp, &wr, &bad_wr));
}

static void on_pre_conn(struct rdma_cm_id *id)
{
  struct conn_context *ctx = (struct conn_context *)malloc(sizeof(struct conn_context));

  id->context = ctx;

  ctx->file_name[0] = '\0'; // take this to mean we don't have the file name

  posix_memalign((void **)&ctx->buffer, sysconf(_SC_PAGESIZE), BUFFER_SIZE);
  TEST_Z(ctx->buffer_mr = ibv_reg_mr(rc_get_pd(), ctx->buffer, BUFFER_SIZE, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE|IBV_ACCESS_REMOTE_READ));

  posix_memalign((void **)&ctx->msg, sysconf(_SC_PAGESIZE), sizeof(*ctx->msg));
  TEST_Z(ctx->msg_mr = ibv_reg_mr(rc_get_pd(), ctx->msg, sizeof(*ctx->msg), 0));

  post_receive(id);
}

static void on_connection(struct rdma_cm_id *id)
{
  struct conn_context *ctx = (struct conn_context *)id->context;

  ctx->msg->id = MSG_MR;
  ctx->msg->data.mr.addr = (uintptr_t)ctx->buffer_mr->addr;
  ctx->msg->data.mr.rkey = ctx->buffer_mr->rkey;

  send_message(id);
}

static void on_completion(struct ibv_wc *wc)
{
  struct rdma_cm_id *id = (struct rdma_cm_id *)(uintptr_t)wc->wr_id;
  struct conn_context *ctx = (struct conn_context *)id->context;

  if (wc->opcode == IBV_WC_RECV_RDMA_WITH_IMM) {
    printf("received RDMA write with immediate data\n");
    uint32_t size = ntohl(wc->imm_data);

    if (size == 0) {//长度为0
      printf("received 0-length message, closing connection\n");
      ctx->msg->id = MSG_DONE;
      send_message(id);

      // don't need post_receive() since we're done with this connection

    } 
    /*
    else if (ctx->file_name[0]) {//第二次进入这个分支，从指定内存缓冲区中取出文件数据并写入到文件中
      ssize_t ret;

      printf("received %i bytes.\n", size);

      ret = write(ctx->fd, ctx->buffer, size);//写入到对应的文件中

      if (ret != size)
        rc_die("write() failed");

      post_receive(id);

      ctx->msg->id = MSG_READY;
      send_message(id);

    }
    */
     else { //读文件数据进入的是这个分支
      size = (size > MAX_FILE_NAME) ? MAX_FILE_NAME : size;
      memcpy(ctx->file_name, ctx->buffer, size);
      ctx->file_name[size - 1] = '\0';

      printf("opening file %s\n", ctx->file_name);

      ctx->fd = open(ctx->file_name,O_RDONLY);
      long size = getFileSize(ctx->file_name);
      int read_res = read(ctx->fd, ctx->buffer, BUFFER_SIZE<size?BUFFER_SIZE:size);
      printf("read_res: %d\n", read_res);
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

      // post_receive(id);

      ctx->msg->id = MSG_READY;//通知对方可以开始发送文件数据了
                                //变成通知对方可以开始接收文件数据了

      // ctx->peer_addr = ctx->msg->data.mr.addr;//对方的buffer地址
      // ctx->peer_rkey = ctx->msg->data.mr.rkey;//对方的buffer rkey

      send_message(id);
      // write_remote(id, read_res);//写入远端
    }
  }
}

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
    on_completion,
    on_disconnect);

  printf("waiting for connections. interrupt (^C) to exit.\n");

  rc_server_loop(DEFAULT_PORT);

  return 0;
}
