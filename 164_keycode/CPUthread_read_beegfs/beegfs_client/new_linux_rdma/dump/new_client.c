#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <rdma/rdma_cma.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#define NUM_THREADS 256
#define FILE_SIZE 4096
#define REQUEST_SIZE 256  // 文件请求内容的大小

struct thread_data {
    int thread_id;
    struct rdma_cm_id *id;
    char file_request[REQUEST_SIZE];  // 存放文件请求的内容
};

void *client_thread(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    struct ibv_mr *mr;
    char *buffer = malloc(FILE_SIZE);

    // 初始化 RDMA 连接
    struct rdma_cm_event *event;
    rdma_create_event_channel();
    rdma_get_cm_event(data->id->channel, &event);
    rdma_ack_cm_event(event);

    // 注册内存区域
    mr = rdma_reg_msgs(data->id, buffer, FILE_SIZE);

    // 发送 RDMA Send 请求
    struct ibv_send_wr wr, *bad_wr = NULL;
    struct ibv_sge sge;
    struct ibv_send_wr wr_list = {
        .wr_id = data->thread_id,
        .opcode = IBV_WR_SEND,
        .sg_list = &sge,
        .num_sge = 1,
        .next = NULL
    };
    sge.addr = (uintptr_t)buffer;
    sge.length = FILE_SIZE;
    sge.lkey = mr->lkey;
    rdma_post_send(data->id, &wr_list, &bad_wr);

    // 等待完成
    struct ibv_wc wc;
    rdma_get_send_comp(data->id, &wc);
    rdma_ack_send_comp(data->id, &wc);

    // 接收文件数据
    struct ibv_recv_wr recv_wr, *bad_recv_wr = NULL;
    struct ibv_sge recv_sge;
    recv_sge.addr = (uintptr_t)buffer;
    recv_sge.length = FILE_SIZE;
    recv_sge.lkey = mr->lkey;
    recv_wr.wr_id = data->thread_id;
    recv_wr.sg_list = &recv_sge;
    recv_wr.num_sge = 1;
    recv_wr.next = NULL;
    rdma_post_recv(data->id, &recv_wr, &bad_recv_wr);

    // 等待接收完成
    rdma_get_recv_comp(data->id, &wc);
    rdma_ack_recv_comp(data->id, &wc);

    // 处理接收到的文件数据
    // 这里可以添加文件存储逻辑

    // 清理资源
    rdma_dereg_mr(mr);
    free(buffer);
    return NULL;
}

int main() {
    struct rdma_cm_id *id;
    struct rdma_addrinfo *res;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    rdma_getaddrinfo("server_ip", "5000", NULL, &res); // 使用服务端的 IP 地址
    rdma_create_id(NULL, &id, NULL, RDMA_PS_TCP);
    rdma_resolve_addr(id, NULL, res->ai_dst_addr, 2000); // 解析服务端地址
    rdma_resolve_wait(id, 2000); // 等待地址解析完成
    rdma_connect(id, NULL); // 连接到服务端
    rdma_connect_wait(id, 2000); // 等待连接完成

    pthread_t threads[NUM_THREADS];
    struct thread_data data[NUM_THREADS];

    // 初始化每个线程的文件请求内容
    for (int i = 0; i < NUM_THREADS; i++) {
        snprintf(data[i].file_request, REQUEST_SIZE, "file_request_%d", i);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        data[i].thread_id = i;
        data[i].id = id;
        pthread_create(&threads[i], NULL, client_thread, &data[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    rdma_disconnect(id); // 断开连接
    rdma_freeaddrinfo(res);
    rdma_destroy_id(id);
    return 0;
}