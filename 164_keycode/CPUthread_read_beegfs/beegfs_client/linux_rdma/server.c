#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <rdma/rdma_cma.h>

#define TEST_NZ(x) do { if ( (x)) die("error: " #x " failed (returned non-zero)." ); } while (0)
#define TEST_Z(x)  do { if (!(x)) die("error: " #x " failed (returned zero/null)."); } while (0)

const int BUFFER_SIZE = 1024;

struct context {
  struct ibv_context *ctx;
  struct ibv_pd *pd;
  struct ibv_cq *cq;
  struct ibv_comp_channel *comp_channel;

  pthread_t cq_poller_thread;
};

struct connection {
  struct ibv_qp *qp;

  struct ibv_mr *recv_mr;
  struct ibv_mr *send_mr;
  struct ibv_mr *send_mr2;


  char *recv_region;
  char *send_region;
  char *send_region2;


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

static void on_completion(struct ibv_wc *wc);
static int on_connect_request(struct rdma_cm_id *id);
static int on_connection(void *context);
static int on_disconnect(struct rdma_cm_id *id);
static int on_event(struct rdma_cm_event *event);

static struct context *s_ctx = NULL;
// #define TARGET_IP "192.168.5.209"  // 替换为特定IP地址
#define TARGET_IP "192.168.3.212"  // 替换为特定IP地址

#define TARGET_PORT 54321           // 替换为特定端口号
int main(int argc, char **argv)
{
#if _USE_IPV6
  struct sockaddr_in6 addr;
#else
  struct sockaddr_in addr;
#endif
  struct rdma_cm_event *event = NULL;
  struct rdma_cm_id *listener = NULL;
  struct rdma_event_channel *ec = NULL;
  uint16_t port = 0;

  memset(&addr, 0, sizeof(addr));
#if _USE_IPV6
  addr.sin6_family = AF_INET6;
  addr.sin6_port = htons(TARGET_PORT);
  addr.sin6_addr = in6addr_any; // 监听所有IPv6地址
  inet_pton(AF_INET6, TARGET_IP, &addr.sin6_addr); // 设置目标IPv6地址
#else
  addr.sin_family = AF_INET;
  addr.sin_port = htons(TARGET_PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY); // 监听所有IPv4地址
  inet_pton(AF_INET, TARGET_IP, &addr.sin_addr); // 设置目标IPv4地址
#endif

  TEST_Z(ec = rdma_create_event_channel());
  TEST_NZ(rdma_create_id(ec, &listener, NULL, RDMA_PS_TCP));
  TEST_NZ(rdma_bind_addr(listener, (struct sockaddr *)&addr));
  TEST_NZ(rdma_listen(listener, 10)); /* backlog=10 is arbitrary */

  port = ntohs(rdma_get_src_port(listener));

  printf("listening on port %d.\n", port);

  while (rdma_get_cm_event(ec, &event) == 0) {
    struct rdma_cm_event event_copy;

    memcpy(&event_copy, event, sizeof(*event));
    rdma_ack_cm_event(event);

    if (on_event(&event_copy))
      break;
  }

  rdma_destroy_id(listener);
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

  sge.addr = (uintptr_t)conn->recv_region;
  sge.length = BUFFER_SIZE;
  sge.lkey = conn->recv_mr->lkey;

  TEST_NZ(ibv_post_recv(conn->qp, &wr, &bad_wr));
}

void register_memory(struct connection *conn)
{
  printf("register_memory \n");
  conn->send_region = malloc(BUFFER_SIZE);
  conn->send_region2 = malloc(BUFFER_SIZE);
  // printf("send_region = %p \n",conn->send_region);
  if(conn->send_region==NULL)
    printf("[send_region ] error\n");
  conn->recv_region = malloc(BUFFER_SIZE);

  TEST_Z(conn->send_mr = ibv_reg_mr(
    s_ctx->pd,
    conn->send_region,
    BUFFER_SIZE,
    IBV_ACCESS_LOCAL_WRITE|IBV_ACCESS_REMOTE_READ));

      TEST_Z(conn->send_mr2 = ibv_reg_mr(
    s_ctx->pd,
    conn->send_region2,
    BUFFER_SIZE,
    IBV_ACCESS_LOCAL_WRITE|IBV_ACCESS_REMOTE_READ));

if(conn->send_region==NULL)
  printf("[send_region ]error2\n");

  TEST_Z(conn->recv_mr = ibv_reg_mr(
    s_ctx->pd,
    conn->recv_region,
    BUFFER_SIZE,
    IBV_ACCESS_LOCAL_WRITE));
}
// 在 RDMA WRITE 完成后发送确认消息
void send_completion_ack(struct connection *conn) {
  printf("send_completion_ack \n");
  // printf("----------1-------\n");
  memset(conn->send_region2,BUFFER_SIZE,0);
  // printf(" conn->send_region2[0] = %02x", conn->send_region2[0]);
  conn->send_region2[0] = (unsigned char)0x11; // 确认信号标识
  // printf(" conn->send_region2[0] = %02x", conn->send_region2[0]);

    struct ibv_send_wr wr = {};
  // printf("----------2-------\n");
  if(conn->send_region2==NULL)
  {
    printf("error \n");
  }
    struct ibv_sge sge = {
        .addr = (uintptr_t)conn->send_region2,
        .length = 1, // 发送1字节确认信号
        .lkey = conn->send_mr2->lkey
    };
  // printf("----------3-------\n");

    wr.opcode = IBV_WR_SEND;
    wr.send_flags = IBV_SEND_SIGNALED;
    wr.sg_list = &sge;
    wr.num_sge = 1;
  // printf("----------4-------\n");

    TEST_NZ(ibv_post_send(conn->qp, &wr, NULL));
  // printf("----------5-------\n");

}
void on_completion(struct ibv_wc *wc)
{
  if (wc->status != IBV_WC_SUCCESS)
    die("on_completion: status is not IBV_WC_SUCCESS.");

  if (wc->opcode & IBV_WC_RECV) {
    struct connection *conn = (struct connection *)(uintptr_t)wc->wr_id;
        // printf("conn %p \n",conn);

  printf("开始接收文件信息 \n");
   // 解析元数据
        memcpy(&conn->file_path, conn->recv_region, 256);
        memcpy(&conn->remote_addr, conn->recv_region + 256, sizeof(uint64_t));
        memcpy(&conn->rkey, conn->recv_region + 264, sizeof(uint32_t));
        memcpy(&conn->file_size, conn->recv_region + 268, sizeof(size_t));
        
            printf("Received request:\n");
          printf("\tFile path: %s\n", conn->file_path);
          printf("\tRemote addr: 0x%lx\n", conn->remote_addr);
          printf("\tRkey: 0x%x\n", conn->rkey);
          printf("\tFile size: %u\n", conn->file_size);
        // 读取本地文件

        FILE *fp = fopen(conn->file_path, "rb");
        if (fp == NULL) {
            perror("文件打开失败");
            exit(EXIT_FAILURE);
        }
        printf("----------0--------- \n");
        // printf("fp = %p \n",fp);
        // printf("send_region = %p \n",conn->send_region);
        if (conn->send_region == NULL) {
            perror("内存分配失败");
            exit(EXIT_FAILURE);
        }
        // printf("----------0.5----------\n");
        fread(conn->send_region, 1, conn->file_size, fp); // [5](@ref)
        fclose(fp);
        // printf("----------1--------- \n");
        
        // 发起 RDMA WRITE
        struct ibv_send_wr wr = {};
        wr.wr_id =(uintptr_t)conn;
        struct ibv_sge sge = {
            .addr = (uintptr_t)conn->send_region,
            .length = conn->file_size,
            .lkey = conn->send_mr->lkey
        };
        wr.wr.rdma.remote_addr = conn->remote_addr;
        wr.wr.rdma.rkey = conn->rkey;
        wr.opcode = IBV_WR_RDMA_WRITE; // [4,7](@ref)
        wr.send_flags = IBV_SEND_SIGNALED;
        wr.sg_list = &sge;
        wr.num_sge = 1;
        // printf("----------2--------- \n");

        ibv_post_send(conn->qp, &wr, NULL); // [4](@ref)
        printf("RDMA Write finish \n");

  } else if (wc->opcode == IBV_WC_SEND) {
    printf("send completed successfully.\n");
  }
  else if (wc->opcode == IBV_WC_RDMA_WRITE) {
        // RDMA WRITE 完成，发送确认
        // printf("----------3--------- \n");

        if (wc->wr_id == 0) {
    printf("Error: wc->wr_id is NULL\n");
    return;
}
        // printf("----------4--------- \n");

        struct connection *conn = (struct connection *)(uintptr_t)wc->wr_id;
        printf("RDMA WRITE 完成，发送确认 \n");
        // printf("conn %p \n",conn);
        // printf("send_region %s \n",conn->send_region);

        // printf("send_region2 %s \n",conn->send_region2);
        send_completion_ack((struct connection *)(uintptr_t)wc->wr_id);
    } 
}

int on_connect_request(struct rdma_cm_id *id)
{
  struct ibv_qp_init_attr qp_attr;
  struct rdma_conn_param cm_params;
  struct connection *conn;

  printf("received connection request.\n");

  build_context(id->verbs);
  build_qp_attr(&qp_attr);

  TEST_NZ(rdma_create_qp(id, s_ctx->pd, &qp_attr));

  id->context = conn = (struct connection *)malloc(sizeof(struct connection));
  conn->qp = id->qp;

  register_memory(conn);
  // printf("send_region2 %p",conn->send_region2);

  post_receives(conn);

  memset(&cm_params, 0, sizeof(cm_params));
  TEST_NZ(rdma_accept(id, &cm_params));
  // post_receives(conn);


  return 0;
}

int on_connection(void *context)
{
  // struct connection *conn = (struct connection *)context;
  // struct ibv_send_wr wr, *bad_wr = NULL;
  // struct ibv_sge sge;
  // // post_receives(conn);
  // snprintf(conn->send_region, BUFFER_SIZE, "message from server side with pid %d", getpid());

  printf("connected. 已经建立...\n");

  // memset(&wr, 0, sizeof(wr));

  // wr.opcode = IBV_WR_SEND;
  // wr.sg_list = &sge;
  // wr.num_sge = 1;
  // wr.send_flags = IBV_SEND_SIGNALED;

  // sge.addr = (uintptr_t)conn->send_region;
  // sge.length = BUFFER_SIZE;
  // sge.lkey = conn->send_mr->lkey;

  // TEST_NZ(ibv_post_send(conn->qp, &wr, &bad_wr));

  return 0;
}

int on_disconnect(struct rdma_cm_id *id)
{
  struct connection *conn = (struct connection *)id->context;

  printf("peer disconnected.\n");

  rdma_destroy_qp(id);

  ibv_dereg_mr(conn->send_mr);
  ibv_dereg_mr(conn->send_mr2);

  ibv_dereg_mr(conn->recv_mr);

  free(conn->send_region);
  free(conn->send_region2);

  free(conn->recv_region);
  // free(conn->recv_region2);


  free(conn);

  rdma_destroy_id(id);

  return 0;
}

int on_event(struct rdma_cm_event *event)
{
  int r = 0;

  if (event->event == RDMA_CM_EVENT_CONNECT_REQUEST)
    r = on_connect_request(event->id);
  else if (event->event == RDMA_CM_EVENT_ESTABLISHED)
    r = on_connection(event->id->context);
  else if (event->event == RDMA_CM_EVENT_DISCONNECTED)
    r = on_disconnect(event->id);
  else
    die("on_event: unknown event.");

  return r;
}

