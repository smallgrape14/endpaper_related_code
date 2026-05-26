#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <rdma/rdma_cma.h>//注意

#define SERVER_PORT 54321

void usage(const char *progname) {
    printf("Usage: %s <server_ip>\n", progname);
    printf("Example: %s 192.168.3.212\n", progname);
}

int main(int argc, char *argv[]) {
    struct rdma_cm_id *listen_id = NULL, *conn_id = NULL;
    struct rdma_addrinfo hints, *res;
    struct sockaddr_in server_addr;
    int ret;

    // 检查参数
    if (argc != 2) {
        usage(argv[0]);
        return -1;
    }

    // 初始化RDMA地址信息
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = RAI_PASSIVE; // 被动模式（监听）
    hints.ai_family = AF_INET;    // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP风格的RDMA连接

    // 获取RDMA地址信息
    ret = rdma_getaddrinfo(argv[1], NULL, &hints, &res);
    if (ret) {
        fprintf(stderr, "rdma_getaddrinfo failed: %s\n", rdma_strerror(ret));
        return -1;
    }

    // 创建监听ID
    listen_id = rdma_create_id(NULL, NULL, NULL);
    if (!listen_id) {
        fprintf(stderr, "rdma_create_id failed: %s\n", rdma_strerror(ret));
        return -1;
    }

    // 绑定地址到监听ID
    server_addr = *(struct sockaddr_in *)res->ai_addr;
    server_addr.sin_port = htons(SERVER_PORT);
    ret = rdma_bind_addr(listen_id, (struct sockaddr *)&server_addr);
    if (ret) {
        fprintf(stderr, "rdma_bind_addr failed: %s\n", rdma_strerror(ret));
        return -1;
    }

    // 开始监听连接
    ret = rdma_listen(listen_id, 5);
    if (ret) {
        fprintf(stderr, "rdma_listen failed: %s\n", rdma_strerror(ret));
        return -1;
    }

    printf("Server is listening on port %d...\n", SERVER_PORT);

    // 等待客户端连接
    conn_id = rdma_accept(listen_id, NULL);
    if (!conn_id) {
        fprintf(stderr, "rdma_accept failed: %s\n", rdma_strerror(ret));
        return -1;
    }

    printf("Client connected successfully!\n");

    // 这里可以进行RDMA通信操作（如数据传输等）

    // 清理资源
    rdma_destroy_id(conn_id);
    rdma_destroy_id(listen_id);
    rdma_freeaddrinfo(res);

    return 0;
}