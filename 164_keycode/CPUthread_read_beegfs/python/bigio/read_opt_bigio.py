import os
import time
import concurrent.futures
import csv
import sys
import re

def open_file(file_path):
    """打开文件并返回文件对象"""
    try:
        file = open(file_path, 'rb')
        return file
    except Exception as e:
        print(f"Error opening {file_path}: {e}")
        return None

def read_file(file_obj, file_path, iosize=512):
    """读取文件内容的函数
    参数:
        file_obj: 文件对象
        file_path: 文件路径
        iosize: 每次读取的字节数，默认512字节
    """
    try:
        content = file_obj.read(iosize)  # 使用iosize参数控制读取大小
        file_obj.close()  # 读取完成后关闭文件
        return content
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
        return None

def main(directory, max_workers=256, iosize=512,iosize_str="512B"):
    print(f"directory: {directory}, max_workers: {max_workers}, iosize: {iosize} bytes, iosize_str: {iosize_str}")
    
    # 获取目录下所有文件的路径
    file_paths = [os.path.join(directory, file) for file in os.listdir(directory)]
    
    # 先打开所有文件，获取文件对象
    file_objects = []
    file_num = 0
    for file_path in file_paths:
        file_obj = open_file(file_path)
        if file_obj is not None:
            file_objects.append((file_obj, file_path))
            file_num =file_num+ 1
    print(f"Total opened files: {file_num}")
    # 记录程序开始时间
    start_time = time.time()
    
    # 创建线程池并启动多线程读取文件
    with concurrent.futures.ThreadPoolExecutor(max_workers=max_workers) as executor:
        # 提交任务到线程池，传递iosize参数
        future_to_file = {
            executor.submit(read_file, file_obj, file_path, iosize): (file_obj, file_path) 
            for file_obj, file_path in file_objects
        }
        
        # 处理线程池的返回结果
        for future in concurrent.futures.as_completed(future_to_file):
            file_obj, file_path = future_to_file[future]
            # 可以取消注释以下代码来查看读取结果
            # try:
            #     content = future.result()
            #     if content is not None:
            #         print(f"Read {len(content)} bytes from {file_path}")
            # except Exception as e:
            #     print(f"File {file_path} generated an exception: {e}")
    
    # 记录程序结束时间
    end_time = time.time()
    total_time = end_time - start_time
    
    # 提取目录中的大小信息
    pattern = r'_(\d+[KMGTP]?B)_'
    match = re.search(pattern, directory)

    if match:
        desired_part = match.group(1)
        print(f"Detected size: {desired_part}")
    else:
        desired_part = "Unknown"
        print("未找到匹配的模式")

    # 写入总时间、目录名称、线程池大小和IOSIZE到CSV文件
    # csv_file = f"{desired_part}_read_file_time.csv"
    csv_file = f"Baseline_read_file_time_1201.csv"

    print(f"CSV file: {csv_file}")
    
    file_exists = os.path.exists(csv_file)
    with open(csv_file, 'a+', newline='') as csvfile:
        writer = csv.writer(csvfile)
        # 如果文件不存在或为空，写入表头（增加IOSIZE列）
        if not file_exists or os.path.getsize(csv_file) == 0:
            writer.writerow(['Method','Directory', 'ThreadNum', 'IOSIZE', 'Total Time(s)','IOPS','Throughput(GBytes/s)','Filenum','Avg_latancy(ms)'])
        writer.writerow(["Baseline",directory, max_workers, iosize_str, f'{total_time:.9f}',f'{file_num/total_time:.2f}',f'{(file_num * iosize)/(total_time * 1024 * 1024 * 1024):.9f}',file_num,f'{(total_time/file_num)*1000:.6f}'])
    
    print(f"Total time: {total_time:.9f} seconds")
    print(f"Total files processed: {len(file_paths)}")
    print(f"IOPS: {file_num/total_time:.2f}")
    print(f"I/O size: {iosize_str} ")



def parse_size(size_str):
    """将大小字符串转换为字节数
    支持格式: 512, 1KB, 2KB, 4KB, 8KB, 16KB, 32KB, 64KB, 128KB, 256KB, 512KB, 1MB等
    """
    size_str = size_str.upper().strip()
    
    # 如果是纯数字，直接返回
    if size_str.isdigit():
        return int(size_str)
    
    # 处理带单位的情况 - 更灵活的单位匹配
    units = {
        'B': 1, 
        'KB': 1024, 
        'K': 1024,      # 支持K作为KB的简写
        'MB': 1024**2, 
        'M': 1024**2,   # 支持M作为MB的简写
        'GB': 1024**3,
        'G': 1024**3
    }
    
    # 使用正则表达式更灵活地匹配数字和单位
    import re
    match = re.match(r'^(\d+(?:\.\d+)?)\s*([A-Za-z]*)$', size_str)
    
    if match:
        num_str = match.group(1)
        unit_str = match.group(2).upper()
        
        try:
            num_value = float(num_str)
            if unit_str in units:
                return int(num_value * units[unit_str])
            elif unit_str == '':  # 没有单位，默认是字节
                return int(num_value)
        except ValueError:
            pass
    
    # 如果上面的解析都失败，尝试一些常见格式的硬编码处理
    common_sizes = {
        '32KB': 32768,
        '64KB': 65536, 
        '128KB': 131072,
        '256KB': 262144,
        '512KB': 524288,
        '1MB': 1048576
    }
    
    if size_str in common_sizes:
        return common_sizes[size_str]
    
    # 默认值
    print(f"Warning: Cannot parse size '{size_str}', using default 512 bytes")
    return 512
if __name__ == "__main__":
    # 传参指定
    if len(sys.argv) < 2:
        print("Usage: python script.py <directory> [max_workers] [iosize]")
        print("Example: python script.py /mnt/beegfs/test_dir_1KB_256 32 4KB")
        print("iosize支持格式: 512, 1KB, 2KB, 4KB, 8KB, 16KB等")
        sys.exit(1)
    
    directory = sys.argv[1]
    max_workers = int(sys.argv[2]) if len(sys.argv) > 2 else 8
    iosize_str = sys.argv[3] if len(sys.argv) > 3 else "512"  # 默认512字节
    
    # 解析IOSIZE参数
    iosize = parse_size(iosize_str)
    
    main(directory, max_workers=max_workers, iosize=iosize,iosize_str=iosize_str)