#include <iostream>
#include <cstring>
#include <mqueue.h>
#include <cuda_runtime.h>
#include <csignal>
#include <unistd.h>

#define QUEUE_NAME_DATA "/data_queue"
#define QUEUE_NAME_TRAIN "/train_queue"
#define MSG_START_TRAINING "START_TRAINING"
#define MSG_DATA_LOADED "DATA_LOADED"
#define MSG_BATCH_DONE "BATCH_DONE"
#define MSG_EXIT "EXIT"

volatile std::sig_atomic_t running = 1;

void signal_handler(int signum) {
    running = 0;
    std::cout << "C++: Received exit signal. Exiting..." << std::endl;
}

int main() {
    signal(SIGINT, signal_handler); // 注册 Ctrl+C 信号处理

    // 删除已存在的消息队列
    mq_unlink(QUEUE_NAME);
    // 新增代码：定义消息队列属性
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;      // 队列最大消息数
    attr.mq_msgsize = 100;    // 每条消息最大长度（必须 ≥ 接收缓冲区大小）
    attr.mq_curmsgs = 0;

    // 修改 mq_open 调用
    mqd_t mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0666, &attr);  // 传入自定义属性
    // 创建消息队列，设置权限为0666（所有用户可读写）
    // mqd_t mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0666, nullptr);
    if (mq == -1) {
        perror("mq_open");
        return 1;
    }

    while (running) {
        // 阻塞等待Python程序的通知
        char buffer[100];
        ssize_t bytes_read = mq_receive(mq, buffer, sizeof(buffer), nullptr);
        if (bytes_read == -1) {
            perror("mq_receive");
            return 1;
        }
        buffer[bytes_read] = '\0';

        if (strcmp(buffer, MSG_START_TRAINING) == 0) {
            std::cout << "C++: Received START_TRAINING command." << std::endl;

            // 模拟数据加载到GPU内存
            void* data;
            cudaMallocManaged(&data, sizeof(float) * 1000); // 统一内存
            // 在这里进行实际的数据加载逻辑
            std::cout << "C++: Data loaded to GPU memory." << std::endl;

            // 通知Python程序数据已加载完成
            mq_send(mq, MSG_DATA_LOADED, strlen(MSG_DATA_LOADED) + 1, 0);
        } else if (strcmp(buffer, MSG_BATCH_DONE) == 0) {
            std::cout << "C++: Received BATCH_DONE command." << std::endl;

            // 模拟加载下一批数据
            // 在这里进行实际的数据加载逻辑
            std::cout << "C++: Loaded next batch of data." << std::endl;

            // 通知Python程序数据已加载完成
            mq_send(mq, MSG_DATA_LOADED, strlen(MSG_DATA_LOADED) + 1, 0);
        } else if (strcmp(buffer, MSG_EXIT) == 0) {
            std::cout << "C++: Received EXIT command. Exiting..." << std::endl;
            running = 0;
        } else {
            std::cout << "C++: Received unexpected message: " << buffer << std::endl;
        }
    }

    // 清理
    mq_close(mq);
    mq_unlink(QUEUE_NAME);

    return 0;
}