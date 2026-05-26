#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <rdma/rdma_cma.h>
#define TEST_NZ(x) do { if ( (x)) die("error: " #x " failed (returned non-zero)." ); } while (0)
#define TEST_Z(x)  do { if (!(x)) die("error: " #x " failed (returned zero/null)."); } while (0)

#define NUM_THREADS 256
#define FILE_SIZE 4096

struct thread_data {
    int thread_id;
    struct rdma_cm_id *id;
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
#define TARGET_IP "192.168.3.212"  // 替换为特定IP地址
#define TARGET_PORT "8003"           // 替换为特定端口号
int main() {
    struct addrinfo *addr;
    struct rdma_cm_id *conn= NULL;

    struct addrinfo hints = {
        .ai_flags = AI_PASSIVE,
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM
    };

    TEST_NZ(getaddrinfo(TARGET_IP, TARGET_PORT, NULL, &addr));
    TEST_Z(ec = rdma_create_event_channel());
    TEST_NZ(rdma_create_id(ec, &conn, NULL, RDMA_PS_TCP));
    
    rdma_create_id(NULL, &id, NULL, RDMA_PS_TCP);
    rdma_bind_addr(id, res->ai_addr);
    rdma_listen(id, 5);

    pthread_t threads[NUM_THREADS];
    struct thread_data data[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        data[i].thread_id = i;
        data[i].id = id;
        pthread_create(&threads[i], NULL, client_thread, &data[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    rdma_freeaddrinfo(res);
    rdma_destroy_id(id);
    return 0;
}