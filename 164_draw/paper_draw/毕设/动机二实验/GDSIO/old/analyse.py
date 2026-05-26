import re
import csv
import os

# 示例数据（如果从文件读取，可以替换为文件读取逻辑）
# data = """
# IoType: READ XferType: GPUD Threads: 1 DataSetSize: 16/16(KiB) IOSize: 8(KiB) Throughput: 0.001513 GiB/sec, Avg_Latency: 351.000000 usecs ops: 2 total_time 0.010084 secs
# IoType: READ XferType: GPUD Threads: 12 DataSetSize: 104/256(KiB) IOSize: 8(KiB) Throughput: 0.009509 GiB/sec, Avg_Latency: 977.750000 usecs ops: 13 total_time 0.010430 secs
# """

with open('output_BeeGFS_0407IO.txt', 'r') as file:
    data = file.read()
# 定义正则表达式模式来提取关键信息
# pattern = r'Threads: (\d+).*?IOSize: (\d+)(KiB|MiB).*?Throughput: ([\d.]+) GiB/sec'
# pattern = r'Threads: (\d+).*?IOSize: (\d+)(KiB|MiB).*?Throughput: ([\d.]+) GiB/sec'
pattern = r'Threads:\s*(\d+)[^*]*?IOSize:\s*(\d+)\s*\(?(KiB|MiB)\)?[^*]*?Throughput:\s*([\d.]+)\s*GiB/sec[,]?'
# 查找所有匹配项
matches = re.findall(pattern, data, re.DOTALL)

csv_file_path="gdsio_results.csv"

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
    fieldnames = ['IOSize (KB)','Threads',  'Throughput (GiB/sec)','IOPS']
    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
    if write_header:
        writer.writeheader()
    for match in matches:
        threads = match[0]
        iosize = match[1]
        print(f"threads {threads} iosize {iosize}")
        # 如果单位是MiB，转换为KiB
        unit = match[2]
        if unit == 'MiB':
            iosize_kb = int(iosize) * 1024
        else:
            iosize_kb = int(iosize) 
        throughput_float = float(match[3])  # 先将吞吐量的字符串转换为浮点数

        # 计算IOPS
        # 注意：吞吐量单位是 GiB/s, 1 GiB = 1024 MiB = 1024 * 1024 KiB
        # 因此计算公式为：IOPS = (Throughput_in_GiB/sec * 1024 * 1024) / IOSize_in_KB
        if iosize_kb == 0:
            iosize_kb = 0.5  # 表示512B，但注意单位是KB，所以这里可能是0.5KB？需要根据你的实际逻辑调整
            iops = (throughput_float * (1024 * 1024 * 1024)) / (iosize_kb * 1024)  # 注意单位统一换算为字节
        else:
            iops = (throughput_float * (1024 * 1024)) / iosize_kb  # Throughput(GiB/s) -> 转换为 KiB/s, 然后除以 IOSize(KB) 得到 IOPS

        writer.writerow({
            'Threads': threads,
            'IOSize (KB)': iosize_kb,
            'Throughput (GiB/sec)': throughput_float,  # 这里直接使用浮点数，或者你可以格式化为字符串控制精度
            'IOPS': iops
        })

print("数据已提取到 gdsio_results.csv")