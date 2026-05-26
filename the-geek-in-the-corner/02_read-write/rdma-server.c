#include "rdma-common.h"
#define BUFFER_SIZE 4096  // 定义缓冲区大小

static int on_connect_request(struct rdma_cm_id *id);
static int on_connection(struct rdma_cm_id *id);
static int on_disconnect(struct rdma_cm_id *id);
static int on_event(struct rdma_cm_event *event);
static void usage(const char *argv0);

int main(int argc, char **argv)
{
  struct sockaddr_in6 addr;
  struct rdma_cm_event *event = NULL;
  struct rdma_cm_id *listener = NULL;
  struct rdma_event_channel *ec = NULL;
  uint16_t port = 0;

  if (argc != 2)
    usage(argv[0]);

  if (strcmp(argv[1], "write") == 0)
    set_mode(M_WRITE);
  else if (strcmp(argv[1], "read") == 0)
    set_mode(M_READ);
  else
    usage(argv[0]);

  // 初始化IPv6地址结构
  memset(&addr, 0, sizeof(addr));
  addr.sin6_family = AF_INET6;

  // port = 12345;
  // addr.sin6_port = htons(port); // 将端口号转换为网络字节序并设置

 // 创建事件通道
  TEST_Z(ec = rdma_create_event_channel());
 // 创建监听器ID
  TEST_NZ(rdma_create_id(ec, &listener, NULL, RDMA_PS_TCP));
 // 绑定监听地址
  TEST_NZ(rdma_bind_addr(listener, (struct sockaddr *)&addr));
// 开始监听连接请求
  TEST_NZ(rdma_listen(listener, 10)); /* backlog=10 is arbitrary 监听端口随机 */

  port = ntohs(rdma_get_src_port(listener));

  printf("listening on port %d.\n", port);
 // 循环处理RDMA CM事件
  while (rdma_get_cm_event(ec, &event) == 0) {
    struct rdma_cm_event event_copy;

    // 复制事件信息，以便处理完成后可以确认事件
    memcpy(&event_copy, event, sizeof(*event));
    rdma_ack_cm_event(event);
    // 处理事件
    if (on_event(&event_copy))
      break;
  }

  rdma_destroy_id(listener);
  rdma_destroy_event_channel(ec);

  return 0;
}
// 处理连接请求事件
int on_connect_request(struct rdma_cm_id *id)//传入的事件ID
{
  struct rdma_conn_param cm_params;

  printf("received connection request.\n");
  build_connection(id);
  build_params(&cm_params);
  //TODO： 感觉可以在这里 open/read 本地文件，然后将文件内容写入到 conn->rdma_local_region

  // sprintf(get_local_message_region(id->context), "message from server side with pid %d", getpid());
  enum mode m;
    get_s_mode(&m);
  if (m== M_WRITE)//本地
    sprintf(get_local_message_region(id->context), "[Server] Write message to Local Memory with pid %d", getpid());
  else //远端
  {
    const char *filename = "1.txt";
    FILE *file = fopen(filename, "r");  // 打开文件
    if (!file) {
        perror("Failed to open file");
        return EXIT_FAILURE;
    }

    char buffer[BUFFER_SIZE];  // 定义缓冲区
    size_t bytesRead;

    // 读取整个文件内容到缓冲区
    bytesRead = fread(buffer, 1, BUFFER_SIZE, file);
    if (ferror(file)) {
        perror("Error reading file");
        fclose(file);
        return EXIT_FAILURE;
    }

    // sprintf(get_local_message_region(id->context), "[Server] Read message from remote Memory with pid %d", getpid());
    sprintf(get_local_message_region(id->context), "%.*s", (int)bytesRead, buffer);  // 将读取的内容格式化到目标缓冲区
    printf("Copied to buffer: %s\n", (char *)get_local_message_region(id->context));  // 打印目标缓冲区内容
  }
  TEST_NZ(rdma_accept(id, &cm_params));

  return 0;
}

int on_connection(struct rdma_cm_id *id)
{
  on_connect(id->context);

  return 0;
}

int on_disconnect(struct rdma_cm_id *id)
{
  printf("peer disconnected.\n");

  destroy_connection(id->context);
  return 0;
}

int on_event(struct rdma_cm_event *event)
{
  int r = 0;
  printf("[server] event: %s, status: %d\n", rdma_event_str(event->event), event->status);
  if (event->event == RDMA_CM_EVENT_CONNECT_REQUEST)
    r = on_connect_request(event->id);
  else if (event->event == RDMA_CM_EVENT_ESTABLISHED)
    r = on_connection(event->id);
  else if (event->event == RDMA_CM_EVENT_DISCONNECTED)
    r = on_disconnect(event->id);
  else
    die("on_event: unknown event.");// 如果事件未知，终止程序

  return r;
}

void usage(const char *argv0)
{
  fprintf(stderr, "usage: %s <mode>\n  mode = \"read\", \"write\"\n", argv0);
  exit(1);
}
