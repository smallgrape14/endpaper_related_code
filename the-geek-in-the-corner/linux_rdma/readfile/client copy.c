#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <infiniband/verbs.h>
#include <rdma/rdma_cma.h>

#define FILE_PATH_SIZE 256
#define BUFFER_SIZE (1024 * 1024)  // 1MB buffer
const int TIMEOUT_IN_MS = 500; /* ms */

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
  static struct context *s_ctx = NULL;
struct connection {
    struct rdma_cm_id *id;
    struct ibv_qp *qp;
  
    struct ibv_mr *recv_mr;
    struct ibv_mr *send_mr;
  
    char *recv_region;
    char *send_region;
  
    int num_completions;
  };
  
  void build_context(struct ibv_context *verbs)
  {
    printf("verbs: %p\n", verbs);
    printf("------Z1------\n");
    if (s_ctx) {
      if (s_ctx->ctx != verbs)
        die("cannot handle events in more than one context.");
  
      return;
    }
    printf("------Z2------\n");
  
    s_ctx = (struct context *)malloc(sizeof(struct context));
  
    s_ctx->ctx = verbs;
    printf("------Z3------\n");
  
    TEST_Z(s_ctx->pd = ibv_alloc_pd(s_ctx->ctx));
    printf("------Z4------\n");

    TEST_Z(s_ctx->comp_channel = ibv_create_comp_channel(s_ctx->ctx));
    printf("------Z5------\n");

    TEST_Z(s_ctx->cq = ibv_create_cq(s_ctx->ctx, 10, NULL, s_ctx->comp_channel, 0)); /* cqe=10 is arbitrary */
    printf("------Z6------\n");
    
    TEST_NZ(ibv_req_notify_cq(s_ctx->cq, 0));
  
    // TEST_NZ(pthread_create(&s_ctx->cq_poller_thread, NULL, poll_cq, NULL));
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
  int on_addr_resolved(struct rdma_cm_id *id)
{
  struct ibv_qp_init_attr qp_attr;
  struct connection *conn;

  printf("address resolved.\n");
  printf("------Q0------\n");

  build_context(id->verbs);
printf("------Q2------\n");

  build_qp_attr(&qp_attr);
  printf("------Q3------\n");

  TEST_NZ(rdma_create_qp(id, s_ctx->pd, &qp_attr));
  printf("------Q4------\n");

  id->context = conn = (struct connection *)malloc(sizeof(struct connection));

  conn->id = id;
  conn->qp = id->qp;
  conn->num_completions = 0;

//   register_memory(conn);
//   post_receives(conn);
printf("------Q5------\n");
  TEST_NZ(rdma_resolve_route(id, TIMEOUT_IN_MS));
printf("------Q6------\n");


  return 0;
}
// 初始化RDMA资源
void init_rdma(struct rdma_cm_id *id) {
    on_addr_resolved(id);
}

// 清理RDMA资源
void cleanup_rdma(struct rdma_cm_id *id) {
    rdma_destroy_qp(id);
    // ibv_destroy_cq(id->verbs);
    // ibv_dealloc_pd(id->verbs);
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

#define TARGET_PORT 54321           // 替换为特定端口号

int send_message(void *context, struct rdma_cm_id *conn_id)
{
  struct connection *conn = (struct connection *)context;
  struct ibv_send_wr wr, *bad_wr = NULL;
  struct ibv_sge sge;
  RequestInfo request;
  strcpy(request.file_path, "/mnt/beegfs/test_250220/image_data.csv");
  char buffer[BUFFER_SIZE];
    uint64_t remote_addr = (uint64_t)buffer;  // 客户端的接收缓冲区地址
    uint32_t rkey = ibv_reg_mr(conn_id->pd, buffer, BUFFER_SIZE, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE|IBV_ACCESS_REMOTE_READ)->rkey;
    request.remote_addr = (uint64_t)buffer;
    request.rkey = rkey;
    request.file_size = 1024;  // 假设请求的文件大小为1KB
 
    memset(&wr, 0, sizeof(wr));

  wr.wr_id = (uintptr_t)conn;
  wr.opcode = IBV_WR_SEND;
  wr.sg_list = &sge;
  wr.num_sge = 1;
  wr.send_flags = IBV_SEND_SIGNALED;

  sge.addr = (uint64_t)&request;
  sge.length = sizeof(request);
    
  struct ibv_mr * send_mr = ibv_reg_mr(conn_id->pd, &request, sizeof(request), IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE);
  sge.lkey = send_mr->lkey;

  TEST_NZ(ibv_post_send(conn->qp, &wr, &bad_wr));
 // 等待发送完成
 struct ibv_wc wc;
 while (ibv_poll_cq(conn_id->send_cq, 1, &wc) == 0) {
    // 等待完成事件
}

    printf("Request sent to server.\n");
    ibv_dereg_mr(send_mr);
  return 0;
}

void post_receives(struct connection *conn, struct rdma_cm_id *conn_id)
{
  struct ibv_recv_wr wr, *bad_wr = NULL;
  struct ibv_sge sge;
  char *recv_region=(char*)malloc(BUFFER_SIZE);

  wr.wr_id = (uintptr_t)conn;
  wr.next = NULL;
  wr.sg_list = &sge;
  wr.num_sge = 1;

  sge.addr = (uintptr_t)recv_region;
  sge.length = BUFFER_SIZE;
  struct ibv_mr * recv_mr =ibv_reg_mr(conn_id->pd, recv_region, BUFFER_SIZE, IBV_ACCESS_LOCAL_WRITE|IBV_ACCESS_REMOTE_WRITE|IBV_ACCESS_REMOTE_READ);
  sge.lkey = recv_mr->lkey;

  TEST_NZ(ibv_post_recv(conn->qp, &wr, &bad_wr));
  struct ibv_wc wc;

  while (ibv_poll_cq(conn_id->recv_cq, 1, &wc) == 0) {
    // 等待接收完成
}

  printf("Received file content:\n%s\n", recv_region);
  ibv_dereg_mr(recv_mr);
  free(recv_region);
}
int main() {


    struct rdma_event_channel *channel;
    struct rdma_cm_id *conn_id;
    struct rdma_cm_event *event;
    struct ibv_wc wc;
    struct ibv_send_wr send_wr = {};
    struct ibv_sge sge = {};
    struct ibv_recv_wr recv_wr = {};
    char file_path[FILE_PATH_SIZE];
    uint64_t remote_addr;
    uint32_t rkey;
    uint32_t file_size;
    char buffer[BUFFER_SIZE];
    printf("------00------\n");

    // 初始化RDMA通道
    channel = rdma_create_event_channel();
    if (!channel) {
        perror("Failed to create event channel");
        return 1;
    }
    printf("------01------\n");

    // 创建CM ID
    rdma_create_id(channel, &conn_id, NULL, RDMA_PS_TCP);
    if (!conn_id) {
        perror("Failed to create CM ID");
        return 1;
    }
    printf("------02------\n");

    // 设置目标地址
    struct addrinfo *addr;
    TEST_NZ(getaddrinfo("192.168.3.212","54321", NULL, &addr));


    // 解析地址
    printf("------1------\n");
    if (rdma_resolve_addr(conn_id, NULL, addr->ai_addr, TIMEOUT_IN_MS)) {
        perror("Failed to resolve address");
        return 1;
    }
    printf("------2------\n");

    // freeaddrinfo(addr);
    // 初始化RDMA资源
    init_rdma(conn_id);
    printf("------3------\n");

    // 解析路由
    // if (rdma_resolve_route(conn_id, 2000)) {
    //     perror("Failed to resolve route");
    //     return 1;
    // }



    printf("------4------\n");

    // 连接到服务端
    if (rdma_connect(conn_id, NULL)) {
        perror("Failed to connect");
        return 1;
    }
    printf("-----5------\n");

    printf("Connecting to server...\n");

    // 等待连接建立
    while (rdma_get_cm_event(channel, &event)) {
        handle_rdma_event(event);
        rdma_ack_cm_event(event);
        if (event->event == RDMA_CM_EVENT_ESTABLISHED) {
            break;
        }
    }

    printf("Connection established.\n");
    send_message(conn_id->context,conn_id);
    post_receives(conn_id->context,conn_id);

    // 清理资源
    cleanup_rdma(conn_id);
    rdma_destroy_event_channel(channel);
    rdma_destroy_id(conn_id);

    return 0;
}
/*

    // 准备文件路径和请求信息
    strncpy(file_path, "/mnt/beegfs/test_250220/image_data.csv", FILE_PATH_SIZE);
    remote_addr = (uint64_t)buffer;  // 客户端的接收缓冲区地址
    rkey = ibv_reg_mr(conn_id->pd, buffer, BUFFER_SIZE, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE)->rkey;
    file_size = BUFFER_SIZE;  // 假设请求的文件大小为1MB

    // 发送文件路径和请求信息
    sge.addr = (uint64_t)file_path;
    sge.length = FILE_PATH_SIZE;
    sge.lkey = conn_id->context;
    send_wr.wr_id = 1;
    send_wr.opcode = IBV_WR_SEND;
    send_wr.sg_list = &sge;
    send_wr.num_sge = 1;
    send_wr.send_flags = IBV_SEND_SIGNALED;
    rdma_post_send(conn_id, &send_wr, NULL);

    sge.addr = (uint64_t)&remote_addr;
    sge.length = sizeof(remote_addr);
    rdma_post_send(conn_id, &send_wr, NULL);

    sge.addr = (uint64_t)&rkey;
    sge.length = sizeof(rkey);
    rdma_post_send(conn_id, &send_wr, NULL);

    sge.addr = (uint64_t)&file_size;
    sge.length = sizeof(file_size);
    rdma_post_send(conn_id, &send_wr, NULL);

    // 等待发送完成
    while (ibv_poll_cq(conn_id->send_cq, 1, &wc) == 0) {
        // 等待完成事件
    }

    printf("Request sent to server.\n");

    // 接收文件内容
    sge.addr = (uint64_t)buffer;
    sge.length = BUFFER_SIZE;
    sge.lkey = ibv_reg_mr(conn_id->pd, buffer, BUFFER_SIZE, IBV_ACCESS_LOCAL_WRITE)->lkey;
    recv_wr.wr_id = 2;
    recv_wr.sg_list = &sge;
    recv_wr.num_sge = 1;
    rdma_post_recv(conn_id, &recv_wr);

    while (ibv_poll_cq(conn_id->recv_cq, 1, &wc) == 0) {
        // 等待接收完成
    }

    printf("Received file content:\n%s\n", buffer);
*/