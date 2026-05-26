import GPUtil
import time

def get_average_gpu_usage(duration=30, interval=1, gpu_id=None):
    """
    统计指定GPU在特定时间段内的平均使用率
    
    参数:
        duration: 监控持续时间(秒)，默认为30秒
        interval: 采样间隔时间(秒)，默认为1秒
        gpu_id: 要监控的GPU ID(整数)，如0,1,2...；默认为None表示监控所有GPU
    """
    usage_values = []
    start_time = time.time()
    
    while time.time() - start_time < duration:
        gpus = GPUtil.getGPUs()
        
        for gpu in gpus:
            # 如果指定了GPU ID，只收集该GPU的数据
            if gpu_id is not None and gpu.id != gpu_id:
                continue
                
            usage = gpu.load * 100  # 转换为百分比
            usage_values.append(usage)
            print(f"GPU {gpu.id}: {usage:.2f}%")
        
        time.sleep(interval)  # 等待指定间隔

    if not usage_values:
        print("错误: 没有收集到GPU使用率数据")
        return 0
        
    average_usage = sum(usage_values) / len(usage_values)
    
    if gpu_id is not None:
        print(f"GPU {gpu_id} 在 {duration} 秒内的平均利用率: {average_usage:.2f}%")
    else:
        print(f"所有GPU在 {duration} 秒内的平均利用率: {average_usage:.2f}%")
        
    return average_usage

# 使用示例
if __name__ == '__main__':
    # 示例1: 监控GPU 0在60秒内的使用率，每秒采样一次
    get_average_gpu_usage(duration=10, interval=0.001, gpu_id=1)
    
    # 示例2: 监控所有GPU
    # get_average_gpu_usage(duration=30, interval=2)
    
    # 示例3: 监控GPU 1，时长300秒，每5秒采样一次
    # get_average_gpu_usage(duration=300, interval=5, gpu_id=1)