#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <rdma/rdma_cma.h>
#include "beegfs/getentryid.hpp"
#define TEST_NZ(x)                                      \
  do                                                    \
  {                                                     \
    if ((x))                                            \
      die("error: " #x " failed (returned non-zero)."); \
  } while (0)
#define TEST_Z(x)                                        \
  do                                                     \
  {                                                      \
    if (!(x))                                            \
      die("error: " #x " failed (returned zero/null)."); \
  } while (0)

const int BUFFER_SIZE = 1024;
const int TIMEOUT_IN_MS = 500; /* ms */

// #define FILE_NUM 10
#define MAX_FILE_SIZE 512 // 512B
#define MALF_SIZE 8
#define D_BUFSIZE (32*1024*FILE_NUM)//340*1024*1024
int total_recv=0;
int total_send=0;
struct context
{
  struct ibv_context *ctx;
  struct ibv_pd *pd;
  struct ibv_cq *cq;
  struct ibv_comp_channel *comp_channel;

  pthread_t cq_poller_thread;
};

struct connection
{
  struct rdma_cm_id *id;
  struct ibv_qp *qp;

  // 1. 预注册内存池
  struct ibv_mr *recv_mr_pool[FILE_NUM];
  char **recv_pool;
  // 2. auth msg
  struct ibv_mr *auth_mr;
  char *auth_region;
  // 3. read_request 缓冲区
  struct ibv_mr *send_mr_pool[FILE_NUM];
  char **send_pool;
  // 4. file data buffer
  struct ibv_mr *data_mr;
  char *data_region;

  /*
    struct ibv_mr *recv_mr;
    struct ibv_mr *recv_mr2;
    char *recv_region;
    char *recv_region2;
  */

  int num_completions;
  char file_path[256];  // 文件路径
  uint64_t remote_addr; // 要写入的客户端缓冲区地址
  uint32_t rkey;        // 客户端缓冲区的 rkey
  size_t file_size;     // 请求文件长度
};

static void die(const char *reason);

static void build_context(struct ibv_context *verbs);
static void build_qp_attr(struct ibv_qp_init_attr *qp_attr);
static void *poll_cq(void *);
static void post_receives(struct connection *conn);
static void register_memory(struct connection *conn);

static int on_addr_resolved(struct rdma_cm_id *id);
static void on_completion(struct ibv_wc *wc);
static int on_connection(void *context);
static int on_disconnect(struct rdma_cm_id *id);
static int on_event(struct rdma_cm_event *event);
static int on_route_resolved(struct rdma_cm_id *id);

static struct context *s_ctx = NULL;
// #define TARGET_IP "192.168.3.212"  // 替换为特定IP地址
#define TARGET_IP "192.168.3.216" // 替换为特定IP地址
// #define TARGET_IP "192.168.5.210"  // 替换为特定IP地址

#define TARGET_PORT "8003" // 替换为特定端口号
// int main(int argc, char **argv)
int main()

{
  struct addrinfo *addr;
  struct rdma_cm_event *event = NULL;
  struct rdma_cm_id *conn = NULL;
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

  while (rdma_get_cm_event(ec, &event) == 0)
  {
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

  if (s_ctx)
  {
    if (s_ctx->ctx != verbs)
      die("cannot handle events in more than one context.");

    return;
  }

  s_ctx = (struct context *)malloc(sizeof(struct context));

  s_ctx->ctx = verbs;

  TEST_Z(s_ctx->pd = ibv_alloc_pd(s_ctx->ctx));
  TEST_Z(s_ctx->comp_channel = ibv_create_comp_channel(s_ctx->ctx));
  int cq_size = 1024;  // 设置CQ深度为1024
  TEST_Z(s_ctx->cq = ibv_create_cq(s_ctx->ctx, cq_size, NULL, s_ctx->comp_channel, 0)); /* cqe=10 is arbitrary */
  TEST_NZ(ibv_req_notify_cq(s_ctx->cq, 0));

  TEST_NZ(pthread_create(&s_ctx->cq_poller_thread, NULL, poll_cq, NULL));
}

void build_qp_attr(struct ibv_qp_init_attr *qp_attr)
{
  memset(qp_attr, 0, sizeof(*qp_attr));

  qp_attr->send_cq = s_ctx->cq;
  qp_attr->recv_cq = s_ctx->cq;
  qp_attr->qp_type = IBV_QPT_RC;

  qp_attr->cap.max_send_wr = FILE_NUM * 2;
  qp_attr->cap.max_recv_wr = FILE_NUM * 4;
  qp_attr->cap.max_send_sge = 32 ; // 支持多SGE操作
  qp_attr->cap.max_recv_sge = 32 ;
  qp_attr->cap.max_inline_data = 512; //// 内联数据传输优化
}

void *poll_cq(void *ctx)
{
  struct ibv_cq *cq;
  struct ibv_wc wc;

  while (1)
  {
    TEST_NZ(ibv_get_cq_event(s_ctx->comp_channel, &cq, &ctx));
    ibv_ack_cq_events(cq, 1);
    TEST_NZ(ibv_req_notify_cq(cq, 0));

    while (ibv_poll_cq(cq, 1, &wc))
    // while (ibv_poll_cq(cq, 32, &wc))// // 每次轮询32个事件[3](@ref)
      on_completion(&wc);
  }

  return NULL;
}
void post_receives_multiple(struct connection *conn)
{

 //方式一：循环提交
  for (int i = 0; i < FILE_NUM; i++)
  {
    struct ibv_recv_wr wr, *bad_wr = NULL;
    struct ibv_sge sge;

    wr.wr_id = (uintptr_t)conn;
    wr.next = NULL;
    wr.sg_list = &sge;
    wr.num_sge = 1;

    sge.addr = (uintptr_t)conn->recv_pool[i];
    sge.length = MAX_FILE_SIZE; //
    sge.lkey = conn->recv_mr_pool[i]->lkey;
    if(i==FILE_NUM-1)
      printf("%d th post recv \n", i);
    TEST_NZ(ibv_post_recv(conn->qp, &wr, &bad_wr));
  }

/*
  //方式二： 构建接收请求链表,批量提交
  struct ibv_recv_wr *recv_wr_list = (struct ibv_recv_wr *)malloc(FILE_NUM * sizeof(struct ibv_recv_wr));
  struct ibv_sge *sge_list = (struct ibv_sge *)malloc(FILE_NUM * sizeof(struct ibv_sge));
    for (int i = 0; i < FILE_NUM; i++)
  {
    struct ibv_recv_wr wr, *bad_wr = NULL;
    struct ibv_sge sge;

    wr.wr_id = (uintptr_t)conn;
    wr.next = (i < FILE_NUM-1) ? &recv_wr_list[i+1] : NULL;
    wr.sg_list = &sge_list[i];
    wr.num_sge = 1;

    sge.addr = (uintptr_t)conn->recv_pool[i];
    sge.length = MAX_FILE_SIZE; //MAX_MSG_SIZE;  // 与发送端MTU匹配[1](@ref)
    sge.lkey = conn->recv_mr_pool[i]->lkey;
    if(i==FILE_NUM-1)
      printf("%d th post recv \n", i);
    
  }
struct ibv_recv_wr *bad_wr;
TEST_NZ(ibv_post_recv(conn->qp, &recv_wr_list[0], &bad_wr));  // 批量提交

*/

}

// void post_receives(struct connection *conn)
// {

//   struct ibv_recv_wr wr, *bad_wr = NULL;
//   struct ibv_sge sge;

//   wr.wr_id = (uintptr_t)conn;
//   wr.next = NULL;
//   wr.sg_list = &sge;
//   wr.num_sge = 1;

//   sge.addr = (uintptr_t)conn->recv_region2;
//   sge.length = MAX_FILE_SIZE; //
//   sge.lkey = conn->recv_mr2->lkey;

//   TEST_NZ(ibv_post_recv(conn->qp, &wr, &bad_wr));
// }

void register_memory(struct connection *conn)
{
  // 注册接收和 发送的内存池
  printf("[register_memory] 注册接收和 发送的内存池 \n");

  // 1. 接收的内存池
  printf("1. 接收的内存池初始化 \n");//用于接收 8B 的Malformed packet 

  conn->recv_pool = (char **)malloc(FILE_NUM * (sizeof(char *)));
  if (conn->recv_pool == NULL)
  {
    perror("recv_pool 内存分配失败");
    return;
  }
  for (int i = 0; i < FILE_NUM; i++)
  {
    conn->recv_pool[i] = (char *)malloc(MALF_SIZE * sizeof(char));
    if (conn->recv_pool[i] == NULL)
    {
      perror("recv_pool[] 内存分配失败");
      return;
    }
  }

  
  for (int i = 0; i < FILE_NUM; i++)
  {
    TEST_Z(conn->recv_mr_pool[i] = ibv_reg_mr(s_ctx->pd, conn->recv_pool[i], MALF_SIZE, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE));
  }

  // 2.发送的缓冲区
  printf("2.发送的缓冲区初始化 \n");
  conn->send_pool = (char **)malloc(FILE_NUM * (sizeof(char *)));
  if (conn->send_pool == NULL)
  {
    perror("send_pool 内存分配失败");
    return ;
  }
  for (int i = 0; i < FILE_NUM; i++)
  {
    conn->send_pool[i] = (char *)malloc(MAX_FILE_SIZE * sizeof(char));
     if (conn->send_pool[i] == NULL)
    {
      perror("send_pool[] 内存分配失败");
      return ;
    }
  }

  for (int i = 0; i < FILE_NUM; i++)
  {
    TEST_Z(conn->send_mr_pool[i] = ibv_reg_mr(s_ctx->pd, conn->send_pool[i], MAX_FILE_SIZE, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE));
  }

  // 3.
  printf("3. auth msg发送缓冲区初始化 \n"); 
  {
    conn->auth_region = (char*) malloc(BUFFER_SIZE);
    TEST_Z(conn->auth_mr = ibv_reg_mr(
               s_ctx->pd,
               conn->auth_region,
               BUFFER_SIZE,
               IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ));
  }
  // 4.
  printf("4. File data Buffer 接收文件数据缓冲区初始化 \n");
  {
    conn->data_region =(char*)  malloc(D_BUFSIZE);
    if(conn->data_region==NULL)
    {
      printf("error data_region malloc \n"); 
      return ;
    }
    TEST_Z(conn->data_mr = ibv_reg_mr(
               s_ctx->pd,
               conn->data_region,
               D_BUFSIZE,
               IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ));
  }
  return ;

  /*
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
  */
}

int on_addr_resolved(struct rdma_cm_id *id)
{
  struct ibv_qp_init_attr qp_attr;
  struct connection *conn;

  printf("1. address resolved.\n");

  build_context(id->verbs);
  build_qp_attr(&qp_attr);

  TEST_NZ(rdma_create_qp(id, s_ctx->pd, &qp_attr));

  id->context = conn = (struct connection *)malloc(sizeof(struct connection));

  conn->id = id;
  conn->qp = id->qp;
  conn->num_completions = 0;

  register_memory(conn);
  // post_receives(conn);
  post_receives_multiple(conn);

  TEST_NZ(rdma_resolve_route(id, TIMEOUT_IN_MS));

  return 0;
}

void on_completion(struct ibv_wc *wc)
{
  struct connection *conn = (struct connection *)(uintptr_t)wc->wr_id;

  // if (wc->status != IBV_WC_SUCCESS)
  //   die("on_completion: status is not IBV_WC_SUCCESS.");

  if (wc->opcode & IBV_WC_RECV)
  {
    total_recv++;
    // 接收到服务端完成通知
    printf("\n[on_completion]已接收到 %d  recv \n",total_recv);
    int print_len =32;
    for(int i=0;i<min(MAX_FILE_SIZE,print_len);i++)
    {
      printf("%c ",(conn->data_region+(total_recv-1)*MAX_FILE_SIZE)[i]);
    }
    printf("\n Malformed Packet :\n");
 for(int i=0;i<MALF_SIZE;i++)
    {
      printf("%02x ",conn->recv_pool[total_recv-1][i]);
    }
  }

  else if (wc->opcode == IBV_WC_SEND)
  {
    total_send++;
      printf("total_send %d , send completed successfully.\n",total_send);

  }  
else if(wc->opcode == IBV_WC_RDMA_WRITE)
    printf("RDMA Write successfully.\n");
  else 
    die("on_completion: completion isn't a send or a receive.");

  // if (++conn->num_completions == 5)
  if(total_recv>=FILE_NUM)
    {
      printf("total_recv >= FILE_NUM \n");
      rdma_disconnect(conn->id);
    }
}

int on_connection(void *context)
{

  struct connection *conn = (struct connection *)context;
  struct ibv_send_wr wr, *bad_wr = NULL;
  struct ibv_sge sge;

  // 1. auth_msg 准备
  printf("auth_msg 准备 \n");

  char *auth_msg = (char *)malloc(1024);
  size_t auth_msg_len = 0;
  AuthenticateChannelMsg_generate(auth_msg, &auth_msg_len);

  // 复制填充缓冲区
  memcpy(conn->auth_region, auth_msg, auth_msg_len);

  printf("auth_msg send \n");

  {
    memset(&wr, 0, sizeof(wr));

    wr.wr_id = (uintptr_t)conn;
    wr.opcode = IBV_WR_SEND;
    wr.sg_list = &sge;
    wr.num_sge = 1;
    wr.send_flags = IBV_SEND_SIGNALED;

    sge.addr = (uintptr_t)conn->auth_region;
    sge.length = auth_msg_len;
    sge.lkey = conn->auth_mr->lkey;

    TEST_NZ(ibv_post_send(conn->qp, &wr, &bad_wr));
  }
  // 2.
  printf("read request 准备 \n");
  // char inputsearchPath[50] = "test_dir"; // 用这个得 uid =1000 所有文件大小=512B 500个
  // char inputsearchPath[50] = "test_dir_1000"; // 用这个得 uid =1000 所有文件大小=512B 1000个
  // char inputsearchPath[50] = "test_dir_10428"; // 用这个得 uid =0 所有文件大小=512B 1000个

  // char inputsearchPath[50] = "test_dir_4KB_500"; // 用这个得 uid =0 所有文件大小=4KB 500个
  char inputsearchPath[50] = "test_dir_512B_500"; // 用这个得 uid =0 所有文件大小=4KB 500个

  

  uint32_t file_num = 0;
  struct file_info *file_info_arr = (struct file_info *)malloc(FILE_NUM * sizeof(struct file_info)); // XYP_TODO 需要释放内存
  char *read_msg = (char *)malloc((size_t)(FILE_NUM * MAX_REQUEST_LEN));
  unsigned int *read_msg_len=(unsigned int *)malloc(FILE_NUM*sizeof(unsigned int));

  if (file_info_arr == NULL || read_msg == NULL || read_msg_len ==NULL)
  {
    perror("file_info_arr 内存分配失败");
    return -1;
  }
  
  uint64_t vaddr=(uint64_t)conn->data_region;//conn->data_mr->addr;
  uint32_t rkey=conn->data_mr->rkey;
  printf("vaddr : 0x%016llx,  rkey : 0x%08x ,conn->data_mr->addr :  0x%016llx, \n",vaddr,rkey,conn->data_mr->addr);
  read_dir(inputsearchPath, vaddr, rkey,
           &file_num, file_info_arr, read_msg, read_msg_len);
 //循环提交
 /*  
  for(int i=0;i<FILE_NUM;i++)
  {
      memcpy(conn->send_pool[i], read_msg+i*MAX_REQUEST_LEN,read_msg_len[i]);
      memset(&wr, 0, sizeof(wr));

      wr.wr_id = (uintptr_t)conn;
      wr.opcode = IBV_WR_SEND;
      wr.sg_list = &sge;
      wr.num_sge = 1;
      wr.send_flags = IBV_SEND_SIGNALED;

      sge.addr = (uintptr_t)conn->send_pool[i];
      sge.length = read_msg_len[i];
      sge.lkey = conn->send_mr_pool[i]->lkey;
      if(i==FILE_NUM-1)
      printf("%d th file post send \n",i);
      TEST_NZ(ibv_post_send(conn->qp, &wr, &bad_wr));
  }
  */

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
          wr_list[i].send_flags = IBV_SEND_SIGNALED; // 每16个请求发一次完成信号[3](@ref)

          // wr_list[i].send_flags = (i%16 == 0) ? IBV_SEND_SIGNALED : 0; // 每16个请求发一次完成信号[3](@ref)
          
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

  return 0;
}

int on_disconnect(struct rdma_cm_id *id)
{
  struct connection *conn = (struct connection *)id->context;

  printf("disconnected.\n");

  rdma_destroy_qp(id);

  
  ibv_dereg_mr(conn->auth_mr);
  ibv_dereg_mr(conn->data_mr);

  free(conn->auth_region);
  free(conn->data_region);
  
  for(int i=0;i<FILE_NUM;i++)
  {
    ibv_dereg_mr(conn->recv_mr_pool[i]);
    ibv_dereg_mr(conn->send_mr_pool[i]);

    free(conn->recv_pool[i]); 
    free(conn->send_pool[i]); 
  }
  free(conn->send_pool);
  free(conn->recv_pool);


  free(conn);

  rdma_destroy_id(id);

  return 1; /* exit event loop */
}

int on_event(struct rdma_cm_event *event)
{
  int r = 0;

  if (event->event == RDMA_CM_EVENT_ADDR_RESOLVED) // 1.
    r = on_addr_resolved(event->id);
  else if (event->event == RDMA_CM_EVENT_ROUTE_RESOLVED) // 2.
    r = on_route_resolved(event->id);
  else if (event->event == RDMA_CM_EVENT_ESTABLISHED) // 3.
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
