#include <iostream>
#include <cstring>
#include <mqueue.h>
#include <cuda_runtime.h>
#include <csignal>
#include <unistd.h>
#include <cerrno>

#define PY_TO_CPP_QUEUE "/py_to_cpp"
#define CPP_TO_PY_QUEUE "/cpp_to_py"
#define MSG_START_TRAINING "START_TRAINING"
#define MSG_DATA_LOADED "DATA_LOADED"
#define MSG_BATCH_DONE "BATCH_DONE"
#define MSG_EXIT "EXIT"

volatile sig_atomic_t running = 1;
void* data = nullptr;

void signal_handler(int signum) {
    running = 0;
    std::cout << "C++: 正在清理资源..." << std::endl;
    if(data) cudaFree(data);
}

int main() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);

    // 定义队列属性[2,6](@ref)
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 100;
    attr.mq_curmsgs = 0;

    // 创建双向队列[6,7](@ref)
    mqd_t mq_recv = mq_open(PY_TO_CPP_QUEUE, O_CREAT | O_RDWR, 0666, &attr);
    mqd_t send_mq = mq_open(CPP_TO_PY_QUEUE, O_CREAT | O_RDWR, 0666, &attr);
    
    while(running) {
        char buffer[100];
        ssize_t bytes_read = mq_receive(mq_recv, buffer, sizeof(buffer), nullptr);
        
        if(bytes_read == -1) {
            if(errno == EINTR) {  // 处理信号中断[2](@ref)
                std::cout << "C++: 系统调用中断" << std::endl;
                continue;
            }
            perror("mq_receive");
            break;
        }
        buffer[bytes_read] = '\0';

        if(strcmp(buffer, MSG_START_TRAINING) == 0) {
            std::cout << "C++: 收到训练启动命令" << std::endl;
            cudaMallocManaged(&data, sizeof(float)*1000);
            mq_send(send_mq, MSG_DATA_LOADED, strlen(MSG_DATA_LOADED)+1, 0);
        }
        else if(strcmp(buffer, MSG_BATCH_DONE) == 0) {
            std::cout << "C++: 收到批次完成通知" << std::endl;
            mq_send(send_mq, MSG_DATA_LOADED, strlen(MSG_DATA_LOADED)+1, 0);
        }
        else if(strcmp(buffer, MSG_EXIT) == 0) {
            running = 0;
        }
    }

    // 清理资源[6](@ref)
    mq_close(mq_recv);
    mq_close(send_mq);
    mq_unlink(PY_TO_CPP_QUEUE);
    mq_unlink(CPP_TO_PY_QUEUE);
    return 0;
}