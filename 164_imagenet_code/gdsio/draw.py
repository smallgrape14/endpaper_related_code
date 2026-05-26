import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# 创建数据
data = {
    'IOSize (KB)': [32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 
                    0.5, 1, 2, 4, 8, 16],
    'Threads': [1]*18,
    'Throughput (GiB/sec)': [0.061404, 0.124598, 0.235798, 0.39612, 0.541117, 0.567067, 
                             0.651317, 0.589013, 0.579501, 1.072432, 1.955229, 2.053076,
                             0.000938, 0.001742, 0.003568, 0.008271, 0.016281, 0.033939
                             ],
    'IOPS': [2012.086272, 2041.413632, 1931.657216, 1622.50752, 1108.207616, 580.676608,
             333.474304, 150.787328, 74.176128, 68.635648, 62.567328, 32.849216,
             1967.128576, 1826.619392, 1870.659584, 2168.193024, 2133.983232, 2224.226304
             ]
}

df = pd.DataFrame(data)

# 设置中文字体和图表样式
plt.rcParams['font.sans-serif'] = ['SimHei', 'Arial Unicode MS', 'DejaVu Sans']
plt.rcParams['axes.unicode_minus'] = False

# 创建图表
fig, ax1 = plt.subplots(figsize=(14, 8))

# 按IOSize排序
df_sorted = df.sort_values('IOSize (KB)')

# 绘制吞吐量折线图
color = 'tab:blue'
ax1.set_xlabel('IO大小 (KB)', fontsize=14)
ax1.set_ylabel('吞吐量 (GiB/sec)', color=color, fontsize=14)
line1 = ax1.plot(df_sorted['IOSize (KB)'], df_sorted['Throughput (GiB/sec)'], 
                 marker='o', linewidth=2, markersize=8, color=color, label='吞吐量')
ax1.tick_params(axis='y', labelcolor=color)
ax1.set_xscale('log', base=2)  # 使用log scale更好地展示跨度很大的IO大小
ax1.grid(True, alpha=0.3)

# 创建第二个y轴用于IOPS
ax2 = ax1.twinx()
color = 'tab:red'
ax2.set_ylabel('IOPS', color=color, fontsize=14)
line2 = ax2.plot(df_sorted['IOSize (KB)'], df_sorted['IOPS'], 
                 marker='s', linewidth=2, markersize=8, color=color, label='IOPS')
ax2.tick_params(axis='y', labelcolor=color)

# 添加图例
lines1, labels1 = ax1.get_legend_handles_labels()
lines2, labels2 = ax2.get_legend_handles_labels()
ax1.legend(lines1 + lines2, labels1 + labels2, loc='center right', fontsize=12)

# 设置标题和美化
plt.title('不同IO大小下的存储性能表现\n(线程数固定为1)', fontsize=16, pad=20)
ax1.set_xlim(0.25, 131072)  # 设置x轴范围以适应所有数据点

# 设置x轴刻度标签
x_ticks = [0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536]
x_labels = ['0.5', '1', '2', '4', '8', '16', '32', '64', '128', '256', '512', '1K', '2K', '4K', '8K', '16K', '32K', '64K']
ax1.set_xticks(x_ticks)
ax1.set_xticklabels(x_labels, rotation=45)

# 调整布局
plt.tight_layout()

# 保存图表
plt.savefig('io_performance_comparison.png', dpi=300, bbox_inches='tight')
plt.show()

# 打印一些关键统计数据
print("性能统计摘要:")
print("="*50)
max_throughput_idx = df['Throughput (GiB/sec)'].idxmax()
max_iops_idx = df['IOPS'].idxmax()

print(f"最高吞吐量: {df.loc[max_throughput_idx, 'Throughput (GiB/sec)']:.3f} GiB/sec "
      f"(IO大小: {df.loc[max_throughput_idx, 'IOSize (KB)']} KB)")
print(f"最高IOPS: {df.loc[max_iops_idx, 'IOPS']:.0f} IOPS "
      f"(IO大小: {df.loc[max_iops_idx, 'IOSize (KB)']} KB)")

# 分析趋势
small_io = df[df['IOSize (KB)'] <= 16]  # 小IO
large_io = df[df['IOSize (KB)'] >= 1024]  # 大IO

print(f"\n小IO平均吞吐量 (<16KB): {small_io['Throughput (GiB/sec)'].mean():.4f} GiB/sec")
print(f"大IO平均吞吐量 (>1MB): {large_io['Throughput (GiB/sec)'].mean():.4f} GiB/sec")
print(f"小IO平均IOPS (<16KB): {small_io['IOPS'].mean():.0f} IOPS")
print(f"大IO平均IOPS (>1MB): {large_io['IOPS'].mean():.0f} IOPS")