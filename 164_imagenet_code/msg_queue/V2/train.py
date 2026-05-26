import posix_ipc
import signal
import sys

# 配置双向队列[4,7](@ref)
SEND_QUEUE = "/py_to_cpp"
RECV_QUEUE = "/cpp_to_py"

class IPCManager:
    def __init__(self):
        # 创建发送队列（Python→C++）
        self.mq_send = posix_ipc.MessageQueue(
            SEND_QUEUE, 
            posix_ipc.O_CREAT,
            max_messages=10,
            max_message_size=100,
            mode=0o666
        )
        
        # 创建接收队列（C++→Python）
        self.mq_recv = posix_ipc.MessageQueue(
            RECV_QUEUE,
            posix_ipc.O_CREAT,
            max_messages=10,
            max_message_size=100,
            mode=0o666
        )
        
        # 注册信号处理
        signal.signal(signal.SIGINT, self.signal_handler)

    def signal_handler(self, signum, frame):
        print("\nPython: 正在退出...")
        self.cleanup()
        sys.exit(0)

    def cleanup(self):
        self.mq_send.close()
        self.mq_recv.close()
        posix_ipc.unlink_message_queue(SEND_QUEUE)
        posix_ipc.unlink_message_queue(RECV_QUEUE)

    def run(self):
        try:
            while True:
                print("发送启动命令...")
                self.mq_send.send("START_TRAINING")
                
                # 等待C++响应[4](@ref)
                msg, _ = self.mq_recv.receive()
                msg = msg.decode().strip('\x00')
                
                if msg == "DATA_LOADED":
                    print("收到数据加载确认")
                    # 模拟训练过程
                    print("训练中...")
                    self.mq_send.send("BATCH_DONE")
                else:
                    print(f"未知消息: {msg}")

        except KeyboardInterrupt:
            self.cleanup()

if __name__ == "__main__":
    ipc = IPCManager()
    ipc.run()