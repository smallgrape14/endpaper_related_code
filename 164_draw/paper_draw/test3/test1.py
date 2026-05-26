import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# 准备数据
data = {
    "batch_size": [16, 32, 64, 128, 256, 512],
    "Baseline worker=0": [0.1301434, 0.260131656, 0.566154257, 1.116352894, 2.182704826, 4.203628536],
    "Baseline worker=1": [0.126651781, 0.258835478, 0.530332372, 1.094989829, 2.218892777, 4.373704498],
    "Baseline worker=2": [0.049098072, 0.108924731, 0.238039304, 0.485833261, 0.961595947, 1.935681916],
    "Baseline worker=4": [0.012369253, 0.036215889, 0.085624278, 0.176307683, 0.3617269, 0.720987492],
    "Baseline worker=8": [0.001338715, 0.003701183, 0.009822644, 0.027659984, 0.063210017, 0.126348266],
    "Baseline worker=16": [0.001316628, 0.001835479, 0.003524393, 0.006309885, 0.012285153, 0.02413445],
    "Ours": [0.007796359, 0.013968873, 0.0136283767223358, 0.011500806, 0.023347676, 0.049223447]
}

df = pd.DataFrame(data)

# 创建图表
plt.figure(figsize=(12, 7))  # 增加高度以适应图例
colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b', '#e377c2']
markers = ['o', 's', '^', 'v', 'D', 'P', '*']

# 绘制每条折线
for i, col in enumerate(df.columns[1:]):
    plt.plot(df['batch_size'], df[col], 
             label=col, 
             color=colors[i], 
             marker=markers[i],
             linestyle='--' if 'Baseline' in col else '-',
             linewidth=2,
             markersize=8)

# 设置图表属性
plt.title('Data Time  Comparison by Batch Size and Configuration', fontsize=20, pad=20)
plt.xlabel('Batch Size', fontsize=20)
plt.ylabel('Data Time(s) (Lower is Better)', fontsize=20)
plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')

# 设置双对数刻度
plt.xscale('log', base=2)
plt.yscale('log')  # 默认以10为底的对数刻度

# 自定义y轴刻度标签
y_min = df.iloc[:, 1:].min().min() * 0.8  # 最小值乘以0.8作为下限
y_max = df.iloc[:, 1:].max().max() * 1.2  # 最大值乘以1.2作为上限
# plt.ylim(y_min, y_max)
plt.ylim(1e-3, 10)  # 强制y轴范围从10^-3到10

ax = plt.gca()
ax.tick_params(axis='both', which='major', labelsize=20)  # 主刻度标签大小
ax.tick_params(axis='both', which='minor', labelsize=16)  # 次刻度标签大小
# 设置网格线
plt.grid(True, which='both', linestyle='--', alpha=0.4)

# 调整布局
plt.tight_layout()
# plt.show()
output_filename = f'batch_time.png'
output_filename_pdf = f'batch_time..pdf'


# output_filename = f'performance_comparison_{IO_SIZE}.png'
plt.savefig(output_filename, dpi=300, bbox_inches='tight')
# plt.show()
plt.savefig(output_filename_pdf, format='pdf', bbox_inches='tight')
