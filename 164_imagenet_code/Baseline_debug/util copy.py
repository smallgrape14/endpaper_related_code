import GPUtil
import time

def get_average_gpu_usage(duration=30, interval=1):
    usage_values = []
    start_time = time.time()
    
    while time.time() - start_time < duration:
        gpus = GPUtil.getGPUs()
        for gpu in gpus:
            usage = gpu.load * 100  # 转换为百分比
            usage_values.append(usage)
            print(f"GPU {gpu.id}: {usage}%")
        time.sleep(interval)  # 等待指定间隔

    average_usage = sum(usage_values) / len(usage_values)
    print(f"{duration} 秒内的平均GPU利用率: {average_usage:.2f}%")
    return average_usage

# 例如，统计训练开始后300秒内的平均利用率，每2秒采样一次
if __name__ == '__main__':
    get_average_gpu_usage(duration=60, interval=1)