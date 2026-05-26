import pandas as pd
import numpy as np

# 如果您有实际的CSV文件，请使用以下代码（替换文件路径）
df = pd.read_csv('gdsio_results.csv')

# 手动创建DataFrame（基于图片数据）
# data = {
#     'IOSize (KB)': [0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 1, 1, 1, 1],
#     'Threads': [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1, 2, 4, 8, 16],
#     'Throughput (GiB/sec)': [0.000935, 0.001786, 0.003737, 0.007708, 0.014158, 0.015614, 0.011983, 0.0086, 0.000382, 0.0002, 0.002116, 0.003537, 0.006932, 0.013708, 0.028061],
#     'IOPS': [1960.83712, 3745.513472, 7837.057024, 16164.847616, 29691.478016, 32744.931328, 25130.172416, 18035.5072, 801.112064, 419.4304, 2218.786816, 3708.813312, 7268.728832, 14373.879808, 29424.091136]
# }

# df = pd.DataFrame(data)

# 计算平均值（虽然每个组合只有一条数据）
averages = df.groupby(['IOSize (KB)', 'Threads']).agg({
    'Throughput (GiB/sec)': 'mean',
    'IOPS': 'mean',
    'IOSize (KB)': 'count' , # 计算每个分组的数据点数量
    'Avg Latency (us)': 'mean'  # 计算平均延迟的平均值
}).rename(columns={'IOSize (KB)': 'Data_Points'}).reset_index()

# 保存结果
output_file = "io_performance_statistics.csv"
averages.to_csv(output_file, index=False)

print("数据处理完成！")
print(f"结果已保存到: {output_file}")
print("\n统计结果:")
print(averages.to_string(index=False))