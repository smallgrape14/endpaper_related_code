#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <infiniband/verbs.h>
#include <rdma/rdma_cma.h>

static struct context *s_ctx = NULL;
#define FILE_PATH_SIZE 256
#define BUFFER_SIZE (1024 * 1024)  // 1MB buffer
void die(const char *reason)
{
  fprintf(stderr, "%s\n", reason);
  exit(EXIT_FAILURE);
}

#define TEST_NZ(x) do { if ( (x)) die("error: " #x " failed (returned non-zero)." ); } while (0)
#define TEST_Z(x)  do { if (!(x)) die("error: " #x " failed (returned zero/null)."); } while (0)

typedef struct {
    char file_path[FILE_PATH_SIZE];
    uint64_t remote_addr;
    uint32_t rkey;
    uint32_t file_size;
} RequestInfo;

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
    struct ibv_mr *send_mr;
  
    char *recv_region;
    char *send_region;
  
    int num_completions;
  };

// 初始化RDMA资源
void init_rdma(struct rdma_cm_id *id) {
    struct ibv_pd *pd = ibv_alloc_pd(id->verbs);
    struct ibv_cq *cq = ibv_create_cq(id->verbs, 10, NULL, NULL, 0);
    struct ibv_qp_init_attr attr = {};
    attr.qp_type = IBV_QPT_RC;
    attr.send_cq = cq;
    attr.recv_cq = cq;
    attr.cap.max_send_wr = 10;
    attr.cap.max_recv_wr = 10;
    attr.cap.max_send_sge = 1;
    attr.cap.max_recv_sge = 1;
    rdma_create_qp(id, pd, &attr);
}

// 清理RDMA资源
void cleanup_rdma(struct rdma_cm_id *id) {
    rdma_destroy_qp(id);
    // ibv_destroy_cq(id->verbs);
    // ibv_dealloc_pd(id->verbs);
    // rdma_destroy_id(id);
}

// 处理RDMA事件
void handle_rdma_event(struct rdma_cm_event *event) {
    if (event->event == RDMA_CM_EVENT_ESTABLISHED) {
        printf("Connection established.\n");
    } else if (event->event == RDMA_CM_EVENT_DISCONNECTED) {
        printf("Connection disconnected.\n");
    } else if (event->event == RDMA_CM_EVENT_ADDR_RESOLVED) {
        printf("Address resolved.\n");
    } else if (event->event == RDMA_CM_EVENT_ROUTE_RESOLVED) {
        printf("Route resolved.\n");
    } else if (event->event == RDMA_CM_EVENT_CONNECT_REQUEST) {
        printf("Connection request received.\n");
    } else if (event->event == RDMA_CM_EVENT_CONNECT_RESPONSE) {
        printf("Connection response received.\n");
    } else if (event->event == RDMA_CM_EVENT_CONNECT_ERROR) {
        printf("Connection error.\n");
    }
}
#define TARGET_IP "192.168.3.212"  // 替换为特定IP地址

#define TARGET_PORT 54322           // 替换为特定端口号

void post_receives(struct connection *conn, struct rdma_cm_id *conn_id)
{
  struct ibv_recv_wr wr, *bad_wr = NULL;
  struct ibv_sge sge;

  RequestInfo request;

  wr.wr_id = (uintptr_t)conn;
  wr.next = NULL;
  wr.sg_list = &sge;
  wr.num_sge = 1;

  sge.addr =  (uint64_t)&request;
  sge.length =sizeof(request);//BUFFER_SIZE;// sizeof(request);
  printf("sizeof(request): %d\n", sizeof(request));
  struct ibv_mr * recv_mr =ibv_reg_mr(conn_id->pd, &request,sizeof(request), IBV_ACCESS_LOCAL_WRITE|IBV_ACCESS_REMOTE_WRITE|IBV_ACCESS_REMOTE_READ);
  sge.lkey = recv_mr->lkey;

  TEST_NZ(ibv_post_recv(conn->qp, &wr, &bad_wr));
//   struct ibv_wc wc;// 读取文件内容
        FILE *file = fopen(request.file_path, "rb");
        if (!file) {
            perror("File open failed");
            exit(1);
        }
    
        char *file_buffer =
// while (ibv_poll_cq(conn_id->recv_cq, 1, &wc) == 1) {
//     if (wc.status != IBV_WC_SUCCESS) {
//         die("Failed to receive data");
//     }
//     break;
// }

    printf("Received request:\n");
    printf("File path: %s\n", request.file_path);
    printf("Remote addr: 0x%lx\n", request.remote_addr);
    printf("Rkey: 0x%x\n", request.rkey);
    printf("File size: %u\n", request.file_size);

        // 读取文件内容
        FILE *file = fopen(request.file_path, "rb");
        if (!file) {
            perror("File open failed");
            exit(1);
        }
    
        char *file_buffer = malloc(request.file_size);
        if (!file_buffer) {
            perror("Memory allocation failed");
            exit(1);
        }
    
        fread(file_buffer, 1, request.file_size, file);
        fclose(file);
    
        printf("File read successfully. Size: %u bytes\n", request.file_size);

        // 使用RDMA_Write发送文件内容
    struct ibv_mr *file_mr = ibv_reg_mr(conn_id->pd, file_buffer, request.file_size, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE);
    sge.addr = (uint64_t)file_buffer;
    sge.length = request.file_size;
    sge.lkey = file_mr->lkey;
    struct ibv_send_wr send_wr = {};
    send_wr.wr_id = 2;
    send_wr.opcode = IBV_WR_RDMA_WRITE;
    send_wr.sg_list = &sge;
    send_wr.num_sge = 1;
    send_wr.wr.rdma.remote_addr = request.remote_addr;
    send_wr.wr.rdma.rkey = request.rkey;
    // ibv_post_send(conn_id, &send_wr, NULL);
    TEST_NZ(ibv_post_send(conn->qp, &send_wr, &bad_wr));

    // 等待发送完成
    // while (ibv_poll_cq(conn_id->send_cq, 1, &wc) == 0) {
    //     // 等待完成事件
    // }

    printf("RDMA_Write completed successfully.\n");

        // 清理资源
        ibv_dereg_mr(file_mr);
        ibv_dereg_mr(recv_mr);
        free(file_buffer);

}
void on_completion(struct ibv_wc *wc)
{
  struct connection *conn = (struct connection *)(uintptr_t)wc->wr_id;

  if (wc->status != IBV_WC_SUCCESS)
    die("on_completion: status is not IBV_WC_SUCCESS.");

  if (wc->opcode & IBV_WC_RECV)
    printf("received message: %s\n", conn->recv_region);
  else if (wc->opcode == IBV_WC_SEND)
    printf("send completed successfully.\n");
  else
    die("on_completion: completion isn't a send or a receive.");

  if (++conn->num_completions == 2)
    rdma_disconnect(conn->id);
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
void register_memory(struct connection *conn)
{
  conn->send_region = malloc(BUFFER_SIZE);
  conn->recv_region = malloc(BUFFER_SIZE);

  TEST_Z(conn->send_mr = ibv_reg_mr(
    s_ctx->pd,
    conn->send_region,
    BUFFER_SIZE,
    0));

  TEST_Z(conn->recv_mr = ibv_reg_mr(
    s_ctx->pd,
    conn->recv_region,
    BUFFER_SIZE,
    IBV_ACCESS_LOCAL_WRITE));
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
//   post_receives(conn);
//   post_receives(conn,id);

  memset(&cm_params, 0, sizeof(cm_params));
  TEST_NZ(rdma_accept(id, &cm_params));

  return 0;
}

int on_connection(void *context,struct rdma_cm_id *conn_id)
{
//   struct connection *conn = (struct connection *)context;
//   struct ibv_send_wr wr, *bad_wr = NULL;
//   struct ibv_sge sge;

  

  printf("connected. posting send...\n");

     // 接收文件路径和请求信息
     post_receives(conn_id->context,conn_id);
    //  post_receives(conn_id->context,conn_id);


  return 0;
}

int on_disconnect(struct rdma_cm_id *id)
{
  struct connection *conn = (struct connection *)id->context;

  printf("peer disconnected.\n");

  rdma_destroy_qp(id);

  ibv_dereg_mr(conn->send_mr);
  ibv_dereg_mr(conn->recv_mr);

  free(conn->send_region);
  free(conn->recv_region);

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
    r = on_connection(event->id->context,event->id);
  else if (event->event == RDMA_CM_EVENT_DISCONNECTED)
    r = on_disconnect(event->id);
  else
    die("on_event: unknown event.");

  return r;
}

int main(int argc, char *argv[]) {
    struct rdma_event_channel *channel;
    struct rdma_cm_id *listener, *conn_id;
    struct rdma_cm_event *event;
    struct ibv_wc wc;
    struct ibv_send_wr send_wr = {};
    struct ibv_sge sge = {};
    struct ibv_recv_wr recv_wr = {};


    // 创建事件通道
    channel = rdma_create_event_channel();
    if (!channel) {
        perror("Failed to create event channel");
        exit(1);
    }

    // 创建监听的CM ID
    rdma_create_id(channel, &listener, NULL, RDMA_PS_TCP);
    if (!listener) {
        perror("Failed to create listener ID");
        exit(1);
    }

    // 绑定地址
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(TARGET_PORT);  // 服务端端口
    inet_pton(AF_INET, TARGET_IP, &addr.sin_addr); // 设置目标IPv4地址

    if (rdma_bind_addr(listener, (struct sockaddr *)&addr)) {
        perror("Failed to bind address");
        exit(1);
    }

    // 开始监听
    if (rdma_listen(listener, 10)) {
        perror("Failed to listen");
        exit(1);
    }

    uint16_t port = ntohs(rdma_get_src_port(listener));

    printf("Server listening on port %d.\n", port);

    // 接受连接
    // if (rdma_get_cm_event(channel,  &conn_id)) {
    //     perror("Failed to get connection event");
    //     exit(1);
    // }

    // printf("Client connected.\n");

    // 初始化RDMA资源
    // init_rdma(conn_id);
    while (rdma_get_cm_event(channel, &event) == 0) {
        struct rdma_cm_event event_copy;
    
        memcpy(&event_copy, event, sizeof(*event));
        rdma_ack_cm_event(event);
    
        if (on_event(&event_copy))
          break;
      }
    // // 接收文件路径和请求信息
    // post_receives(conn_id->context,conn_id);

    // 清理资源

    // cleanup_rdma(conn_id);
    rdma_destroy_event_channel(channel);
    rdma_destroy_id(listener);
    // rdma_destroy_id(conn_id);

    return 0;
}