import posix_ipc
import time
import signal
import sys

QUEUE_NAME = "/py_to_cpp"
MSG_START_TRAINING = "START_TRAINING"
MSG_DATA_LOADED = "DATA_LOADED"
MSG_BATCH_DONE = "BATCH_DONE"
MSG_EXIT = "EXIT"

# 创建消息队列（如果不存在），设置权限为0666
try:
    mq = posix_ipc.MessageQueue(QUEUE_NAME, posix_ipc.O_CREAT, max_messages=10, max_message_size=100, mode=0o666)
except posix_ipc.ExistentialError:
    mq = posix_ipc.MessageQueue(QUEUE_NAME)

# 定义一个标志变量，用于控制循环
running = True

def signal_handler(signum, frame):
    """信号处理函数，用于优雅地退出程序"""
    global running
    print("Python: Received exit signal. Exiting...")
    running = False

# 注册信号处理器，捕获 Ctrl+C 信号
signal.signal(signal.SIGINT, signal_handler)

try:
    while running:
        # 发送START_TRAINING消息，通知C++开始加载数据
        print("Python: Sending START_TRAINING command.")
        mq.send(MSG_START_TRAINING)

        # 阻塞等待C++程序的通知
        msg, _ = mq.receive()
        msg = msg.decode("utf-8")  # 将字节类型解码为字符串类型

        if msg == MSG_DATA_LOADED:
            print("Python: Received DATA_LOADED message.")

            # 模拟训练一个批次
            print("Python: Training batch...")
            time.sleep(2)  # 模拟训练时间

            # 发送BATCH_DONE消息，通知C++加载下一批数据
            print("Python: Sending BATCH_DONE command.")
            mq.send(MSG_BATCH_DONE)
        elif msg == MSG_EXIT:
            print("Python: Received EXIT command. Exiting...")
            running = False
        else:
            print(f"Python: Unexpected message received: {msg}")
except KeyboardInterrupt:
    print("Python: Manually interrupted by user. Exiting...")
except Exception as e:
    print(f"Python: An error occurred: {e}")
finally:
    # 清理
    mq.close()
    try:
        posix_ipc.unlink_message_queue(QUEUE_NAME)
    except posix_ipc.ExistentialError:
        print("Python: Message queue already unlinked.")

print("Python: Program exited gracefully.")