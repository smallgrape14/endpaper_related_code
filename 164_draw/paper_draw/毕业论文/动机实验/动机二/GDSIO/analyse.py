import re
import csv
import os

# 读取文件
with open('output_BeeGFS_0407IO.txt', 'r') as file:
    data = file.read()

# 修改正则表达式以同时提取吞吐量和平均延迟
# 注意：Avg_Latency 可能以 "usecs" 结尾
pattern = r'Threads:\s*(\d+)[^*]*?IOSize:\s*(\d+)\s*\(?(KiB|MiB)\)?[^*]*?Throughput:\s*([\d.]+)\s*GiB/sec[,]?.*?Avg_Latency:\s*([\d.]+)\s*usecs'

# 查找所有匹配项
matches = re.findall(pattern, data, re.DOTALL)

csv_file_path = "gdsio_results.csv"

# 检查文件是否存在并且是否有表头
write_header = not os.path.exists(csv_file_path)
if not write_header:
    with open(csv_file_path, mode='r', newline='') as csv_file:
        reader = csv.reader(csv_file)
        try:
            header = next(reader)
            write_header = len(header) == 0 or header[0] != 'IOSize (KB)'
        except StopIteration:
            write_header = True

# 准备写入CSV
with open(csv_file_path, 'a+', newline='') as csvfile:
    # 更新字段名，增加 'Avg Latency (us)'
    fieldnames = ['IOSize (KB)', 'Threads', 'Throughput (GiB/sec)', 'IOPS', 'Avg Latency (us)']
    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
    if write_header:
        writer.writeheader()
    for match in matches:
        threads = match[0]
        iosize = match[1]
        unit = match[2]
        throughput_float = float(match[3])
        avg_latency = float(match[4])  # 提取平均延迟

        # 如果单位是MiB，转换为KiB
        if unit == 'MiB':
            iosize_kb = int(iosize) * 1024
        else:
            iosize_kb = int(iosize) 
        
        # 计算IOPS
        if iosize_kb == 0:
            iosize_kb = 0.5
            iops = (throughput_float * (1024 * 1024 * 1024)) / (iosize_kb * 1024)
        else:
            iops = (throughput_float * (1024 * 1024)) / iosize_kb

        writer.writerow({
            'Threads': threads,
            'IOSize (KB)': iosize_kb,
            'Throughput (GiB/sec)': throughput_float,
            'IOPS': iops,
            'Avg Latency (us)': avg_latency  # 添加平均延迟列
        })

print("数据已提取到 gdsio_results.csv")