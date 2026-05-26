import os
import time
import concurrent.futures
import csv
import sys
import re

# 尝试导入cufile库，这是GDS的核心API
try:
    import cufile
    GDS_AVAILABLE = True
except ImportError:
    print("Warning: cufile not available, falling back to regular file I/O")
    GDS_AVAILABLE = False
    import builtins

def open_file(file_path):
    """使用cufile打开文件并返回文件对象"""
    try:
        if GDS_AVAILABLE:
            # 使用cufile.CuFile直接打开文件，支持GDS加速
            file_obj = cufile.CuFile(file_path, "r")
        else:
            # 回退到常规文件打开方式
            file_obj = builtins.open(file_path, 'rb')
        return file_obj
    except Exception as e:
        print(f"Error opening {file_path}: {e}")
        return None

def read_file(file_obj, file_path):
    """使用cufile API读取文件内容"""
    try:
        if GDS_AVAILABLE:
            # 使用cufile读取，支持直接传输到GPU内存
            # 读取512字节，与原始代码保持一致
            content = file_obj.read(512)
        else:
            # 回退到常规读取方式
            content = file_obj.read(512)
        
        file_obj.close()  # 读取完成后关闭文件
        return content
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
        return None

def main(directory, max_workers=256):
    print(f"directory:{directory}, max_workers: {max_workers}")
    print(f"GDS available: {GDS_AVAILABLE}")
    
    # 获取目录下所有文件的路径
    file_paths = [os.path.join(directory, file) for file in os.listdir(directory)]
    
    # 先打开所有文件，获取文件对象
    file_objects = []
    for file_path in file_paths:
        file_obj = open_file(file_path)
        if file_obj is not None:
            file_objects.append((file_obj, file_path))
    
    # 记录程序开始时间
    start_time = time.time()
    
    # 创建线程池并启动多线程读取文件
    with concurrent.futures.ThreadPoolExecutor(max_workers=max_workers) as executor:
        # 提交任务到线程池
        future_to_file = {executor.submit(read_file, file_obj, file_path): (file_obj, file_path) 
                         for file_obj, file_path in file_objects}
        
        # 处理线程池的返回结果
        for future in concurrent.futures.as_completed(future_to_file):
            file_obj, file_path = future_to_file[future]
            # 可以取消注释以下代码来查看读取内容
            # try:
            #     content = future.result()
            #     if content is not None:
            #         print(f"Successfully read {file_path}")
            # except Exception as e:
            #     print(f"File {file_path} generated an exception: {e}")
    
    # 记录程序结束时间
    end_time = time.time()
    total_time = end_time - start_time
    
    pattern = r'_(\d+[KMGTP]?B)_'
    match = re.search(pattern, directory)

    if match:
        desired_part = match.group(1)
        print(desired_part)
    else:
        print("未找到匹配的模式")
        desired_part = "unknown"

    # 写入总时间、目录名称和线程池大小到CSV文件
    csv_file = f"{desired_part}_read_file_time_gds.csv" if GDS_AVAILABLE else f"{desired_part}_read_file_time_regular.csv"
    print("csv_file:", csv_file)
    
    file_exists = os.path.exists(csv_file)
    with open(csv_file, 'a+', newline='') as csvfile:
        writer = csv.writer(csvfile)
        # 如果文件不存在或为空，写入表头
        if not file_exists or os.path.getsize(csv_file) == 0:
            writer.writerow(['Directory', 'Max Workers', 'Total Time(s)', 'GDS Enabled'])
        writer.writerow([directory, max_workers, f'{total_time:.9f}', GDS_AVAILABLE])
    
    print(f"Total time: {total_time:.9f} seconds")
    print(f"GDS acceleration: {GDS_AVAILABLE}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python script.py <directory> [max_workers]")
        sys.exit(1)
    
    directory = sys.argv[1]
    max_workers = int(sys.argv[2]) if len(sys.argv) > 2 else 8

    main(directory, max_workers=max_workers)