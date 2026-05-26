#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <rdma/rdma_cma.h>

#define TEST_NZ(x) do { if ( (x)) die("error: " #x " failed (returned non-zero)." ); } while (0)
#define TEST_Z(x)  do { if (!(x)) die("error: " #x " failed (returned zero/null)."); } while (0)

const int BUFFER_SIZE = 1024;
const int TIMEOUT_IN_MS = 500; /* ms */

struct context {
  struct ibv_context *ctx;
  struct ibv_pd *pd;
  struct ibv_cq *cq;
  struct ibv_comp_channel *comp_channel;

  pthread_t cq_poller_thread;
};

struct connection {
  struct rdma_cm_id *id;
  struct ibv_qp *qp;

  struct ibv_mr *recv_mr;
  struct ibv_mr *recv_mr2;

  struct ibv_mr *send_mr;

  char *recv_region;
  char *recv_region2;

  char *send_region;

  int num_completions;
  char file_path[256];       // 文件路径
    uint64_t remote_addr;      // 要写入的客户端缓冲区地址
    uint32_t rkey;             // 客户端缓冲区的 rkey
    size_t file_size;          // 请求文件长度
};

static void die(const char *reason);

static void build_context(struct ibv_context *verbs);
static void build_qp_attr(struct ibv_qp_init_attr *qp_attr);
static void * poll_cq(void *);
static void post_receives(struct connection *conn);
static void register_memory(struct connection *conn);

static int on_addr_resolved(struct rdma_cm_id *id);
static void on_completion(struct ibv_wc *wc);
static int on_connection(void *context);
static int on_disconnect(struct rdma_cm_id *id);
static int on_event(struct rdma_cm_event *event);
static int on_route_resolved(struct rdma_cm_id *id);

static struct context *s_ctx = NULL;
#define TARGET_IP "192.168.3.212"  // 替换为特定IP地址

#define TARGET_PORT "54321"           // 替换为特定端口号
// int main(int argc, char **argv)
int main()

{
  struct addrinfo *addr;
  struct rdma_cm_event *event = NULL;
  struct rdma_cm_id *conn= NULL;
  struct rdma_event_channel *ec = NULL;

  // if (argc != 3)
  //   die("usage: client <server-address> <server-port>");

    
  // TEST_NZ(getaddrinfo(argv[1], argv[2], NULL, &addr));
  TEST_NZ(getaddrinfo(TARGET_IP, TARGET_PORT, NULL, &addr));


  TEST_Z(ec = rdma_create_event_channel());
  TEST_NZ(rdma_create_id(ec, &conn, NULL, RDMA_PS_TCP));
  // printf("vebres: %p\n", conn->verbs);
  TEST_NZ(rdma_resolve_addr(conn, NULL, addr->ai_addr, TIMEOUT_IN_MS));
  // printf("vebres: %p\n", conn->verbs);


  freeaddrinfo(addr);
  // printf("vebres: %p\n", conn->verbs);

  while (rdma_get_cm_event(ec, &event) == 0) {
    struct rdma_cm_event event_copy;

    memcpy(&event_copy, event, sizeof(*event));
    rdma_ack_cm_event(event);
    // printf("2 vebres: %p\n", conn->verbs);

    if (on_event(&event_copy))
      break;
  }

  rdma_destroy_event_channel(ec);

  return 0;
}

void die(const char *reason)
{
  fprintf(stderr, "%s\n", reason);
  exit(EXIT_FAILURE);
}

void build_context(struct ibv_context *verbs)
{
  // printf("[build_contxt]verbs: %p\n", verbs);


  if (s_ctx) {
    if (s_ctx->ctx != verbs)
      die("cannot handle events in more than one context.");

    return;
  }

  s_ctx = (struct context *)malloc(sizeof(struct context));

  s_ctx->ctx = verbs;

  TEST_Z(s_ctx->pd = ibv_alloc_pd(s_ctx->ctx));
  TEST_Z(s_ctx->comp_channel = ibv_create_comp_channel(s_ctx->ctx));
  TEST_Z(s_ctx->cq = ibv_create_cq(s_ctx->ctx, 10, NULL, s_ctx->comp_channel, 0)); /* cqe=10 is arbitrary */
  TEST_NZ(ibv_req_notify_cq(s_ctx->cq, 0));

  TEST_NZ(pthread_create(&s_ctx->cq_poller_thread, NULL, poll_cq, NULL));
}

void build_qp_attr(struct ibv_qp_init_attr *qp_attr)
{
  memset(qp_attr, 0, sizeof(*qp_attr));

  qp_attr->send_cq = s_ctx->cq;
  qp_attr->recv_cq = s_ctx->cq;
  qp_attr->qp_type = IBV_QPT_RC;

  qp_attr->cap.max_send_wr = 10;
  qp_attr->cap.max_recv_wr = 10;
  qp_attr->cap.max_send_sge = 1;
  qp_attr->cap.max_recv_sge = 1;
}

void * poll_cq(void *ctx)
{
  struct ibv_cq *cq;
  struct ibv_wc wc;

  while (1) {
    TEST_NZ(ibv_get_cq_event(s_ctx->comp_channel, &cq, &ctx));
    ibv_ack_cq_events(cq, 1);
    TEST_NZ(ibv_req_notify_cq(cq, 0));

    while (ibv_poll_cq(cq, 1, &wc))
      on_completion(&wc);
  }

  return NULL;
}

void post_receives(struct connection *conn)
{

  struct ibv_recv_wr wr, *bad_wr = NULL;
  struct ibv_sge sge;

  wr.wr_id = (uintptr_t)conn;
  wr.next = NULL;
  wr.sg_list = &sge;
  wr.num_sge = 1;

  sge.addr = (uintptr_t)conn->recv_region2;
  sge.length = 1;//
  sge.lkey = conn->recv_mr2->lkey;

  TEST_NZ(ibv_post_recv(conn->qp, &wr, &bad_wr));
}

void register_memory(struct connection *conn)
{
  conn->send_region = malloc(BUFFER_SIZE);
  conn->recv_region = malloc(BUFFER_SIZE);
  conn->recv_region2 = malloc(BUFFER_SIZE);


  TEST_Z(conn->send_mr = ibv_reg_mr(
    s_ctx->pd, 
    conn->send_region, 
    BUFFER_SIZE, 
    0));

  TEST_Z(conn->recv_mr2 = ibv_reg_mr(
    s_ctx->pd, 
    conn->recv_region2, 
    BUFFER_SIZE, 
    IBV_ACCESS_LOCAL_WRITE|IBV_ACCESS_REMOTE_WRITE));
  
    TEST_Z(conn->recv_mr = ibv_reg_mr(
    s_ctx->pd, 
    conn->recv_region, 
    BUFFER_SIZE, 
    IBV_ACCESS_LOCAL_WRITE));
  
  // 增大缓冲区至文件传输需求（例如 1MB）
    const size_t BUFFER_SIZE =  1024 * 1024; 
    conn->recv_region = malloc(BUFFER_SIZE);
    
    // 注册 MR 时添加远程写权限
    conn->recv_mr = ibv_reg_mr(s_ctx->pd, conn->recv_region, BUFFER_SIZE, 
                              IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE); // [3,6](@ref)
}

int on_addr_resolved(struct rdma_cm_id *id)
{
  struct ibv_qp_init_attr qp_attr;
  struct connection *conn;

  printf("address resolved.\n");

  build_context(id->verbs);
  build_qp_attr(&qp_attr);

  TEST_NZ(rdma_create_qp(id, s_ctx->pd, &qp_attr));

  id->context = conn = (struct connection *)malloc(sizeof(struct connection));

  conn->id = id;
  conn->qp = id->qp;
  conn->num_completions = 0;

  register_memory(conn);
  post_receives(conn);

  TEST_NZ(rdma_resolve_route(id, TIMEOUT_IN_MS));

  return 0;
}

void on_completion(struct ibv_wc *wc)
{
  struct connection *conn = (struct connection *)(uintptr_t)wc->wr_id;

  if (wc->status != IBV_WC_SUCCESS)
    die("on_completion: status is not IBV_WC_SUCCESS.");

  if (wc->opcode & IBV_WC_RECV)
    {
      // 接收到服务端完成通知
      // printf("接收到服务端完成通知 \n");
        printf("buffer1 : %s \n",conn->recv_region);
      //   printf("buffer2 : %s\n",conn->recv_region2);
      //   printf("file_size = %d \n",conn->file_size);

      // FILE *fp = fopen("received_file.bin", "wb");

      //   fwrite(conn->recv_region, 1, conn->file_size, fp); // [5](@ref)
      //   fclose(fp);
      //   printf("File saved to received_file.bin\n");

      if (conn->recv_region2[0] == 0x11)

      {
        printf("接收到服务端完成通知 \n");
        // printf("buffer : %c",conn->recv_region2);
        printf("file_size = %lu ",conn->file_size);
        FILE *fp = fopen("received_file2.bin", "wb");

        fwrite(conn->recv_region, 1, conn->file_size, fp); // [5](@ref)
        fclose(fp);
        printf("File saved to received_file2.bin\n");
        }
    }
    
  else if (wc->opcode == IBV_WC_SEND)
    printf("send completed successfully.\n");
  else
    die("on_completion: completion isn't a send or a receive.");

  if (++conn->num_completions == 5)
    rdma_disconnect(conn->id);
}

int on_connection(void *context)
{
  struct connection *conn = (struct connection *)context;

  // 填充元数据
    strcpy(conn->file_path, "/mnt/beegfs/test_250220/image_data.csv"); // 实际文件路径
    conn->remote_addr = (uint64_t)conn->recv_region; 
    conn->rkey = conn->recv_mr->rkey; 
    conn->file_size = 1* 1024; // 实际文件大小
    
    // 发送元数据（使用 Send 操作）
    memcpy(conn->send_region, &conn->file_path, sizeof(conn->file_path));
    memcpy(conn->send_region + 256, &conn->remote_addr, sizeof(uint64_t));
    memcpy(conn->send_region + 264, &conn->rkey, sizeof(uint32_t));
    memcpy(conn->send_region + 268, &conn->file_size, sizeof(size_t));

  
    printf("Received request:\n");
    printf("File path: %s\n", conn->file_path);
    printf("Remote addr: 0x%lx\n", conn->remote_addr);
    printf("Rkey: 0x%x\n", conn->rkey);
    printf("File size: %lu\n", conn->file_size);
  struct ibv_send_wr wr, *bad_wr = NULL;
  struct ibv_sge sge;

  // snprintf(conn->send_region, BUFFER_SIZE, "message from client side with pid %d", getpid());

  printf("connected. posting send...\n");

  memset(&wr, 0, sizeof(wr));

  wr.wr_id = (uintptr_t)conn;
  wr.opcode = IBV_WR_SEND;
  wr.sg_list = &sge;
  wr.num_sge = 1;
  wr.send_flags = IBV_SEND_SIGNALED;

  sge.addr = (uintptr_t)conn->send_region;
  sge.length = BUFFER_SIZE;
  sge.lkey = conn->send_mr->lkey;

  TEST_NZ(ibv_post_send(conn->qp, &wr, &bad_wr));

  return 0;
}

int on_disconnect(struct rdma_cm_id *id)
{
  struct connection *conn = (struct connection *)id->context;

  printf("disconnected.\n");

  rdma_destroy_qp(id);

  ibv_dereg_mr(conn->send_mr);
  ibv_dereg_mr(conn->recv_mr);
  ibv_dereg_mr(conn->recv_mr2);


  free(conn->send_region);
  free(conn->recv_region);
  free(conn->recv_region2);


  free(conn);

  rdma_destroy_id(id);

  return 1; /* exit event loop */
}

int on_event(struct rdma_cm_event *event)
{
  int r = 0;

  if (event->event == RDMA_CM_EVENT_ADDR_RESOLVED)
    r = on_addr_resolved(event->id);
  else if (event->event == RDMA_CM_EVENT_ROUTE_RESOLVED)
    r = on_route_resolved(event->id);
  else if (event->event == RDMA_CM_EVENT_ESTABLISHED)
    r = on_connection(event->id->context);
  else if (event->event == RDMA_CM_EVENT_DISCONNECTED)
    r = on_disconnect(event->id);
  else
    die("on_event: unknown event.");

  return r;
}

int on_route_resolved(struct rdma_cm_id *id)
{
  struct rdma_conn_param cm_params;

  printf("route resolved.\n");

  memset(&cm_params, 0, sizeof(cm_params));
  TEST_NZ(rdma_connect(id, &cm_params));

  return 0;
}
