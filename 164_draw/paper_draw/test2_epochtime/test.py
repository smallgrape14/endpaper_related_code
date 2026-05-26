import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# 数据准备
data = {
    'batch_size': [16, 32, 64, 128, 256, 512],
    'Baseline_data': [31.89977044, 23.67463697, 21.31556531, 19.69679349, 19.26731176, 18.29807149],
    'Baseline_train': [42.50207023, 20.55564996, 13.03348876, 9.512496761, 7.590755775, 6.945064187],
    'Ours_data': [10.40462287, 9.321072472, 3.021972316, 1.918548519, 1.947413526, 2.052846866],
    'Ours_train': [38.34435055, 21.03050501, 13.47332209, 9.659481118, 7.764076805, 6.781370981]
}

df = pd.DataFrame(data)

# 创建图表
plt.figure(figsize=(14, 8))
ax = plt.subplot()

# 设置柱状图参数
bar_width = 0.35
x = np.arange(len(df['batch_size']))

# 计算总时间
df['Baseline_total'] = df['Baseline_train'] + df['Baseline_data']
df['Ours_total'] = df['Ours_train'] + df['Ours_data']

# 绘制Baseline柱状图
baseline_bars = ax.bar(x - bar_width/2, df['Baseline_total'], bar_width,
                      color='none', edgecolor='black', linewidth=1)

# 绘制Baseline堆叠部分（训练时间在下，数据时间在上）
ax.bar(x - bar_width/2, df['Baseline_train'], bar_width,
       color='#7FB3D5', edgecolor='black', label='Baseline Train Time')
ax.bar(x - bar_width/2, df['Baseline_data'], bar_width,
       bottom=df['Baseline_train'], color='#7FB3D5', edgecolor='black',
       label='Baseline Data Time')

# 绘制Ours柱状图
ours_bars = ax.bar(x + bar_width/2, df['Ours_total'], bar_width,
                  color='none', edgecolor='black', linewidth=1)

# 绘制Ours堆叠部分（训练时间在下，数据时间在上）
ax.bar(x + bar_width/2, df['Ours_train'], bar_width,
       color='#F8C471', edgecolor='black', label='Ours Train Time')
ax.bar(x + bar_width/2, df['Ours_data'], bar_width,
       bottom=df['Ours_train'], color='#F8C471', edgecolor='black',
       label='Ours Data Time')

# 添加标签和标题
ax.set_xlabel('Batch Size', fontsize=12)
ax.set_ylabel('Time (minutes)', fontsize=12)
ax.set_title('Data Processing and Training Time Comparison', fontsize=14, pad=20)
ax.set_xticks(x)
ax.set_xticklabels(df['batch_size'])
ax.legend(loc='upper right', bbox_to_anchor=(1.15, 1))

# 添加数据标签
def add_labels(bars, bottom_values=None, is_total=False):
    for idx, bar in enumerate(bars):
        height = bar.get_height()
        if is_total:
            plt.text(bar.get_x() + bar.get_width()/2, height + 2,
                    f'{height:.1f}', ha='center', va='bottom', fontsize=9)
        elif bottom_values is not None:
            mid_height = bottom_values[idx] + (height - bottom_values[idx])/2
            plt.text(bar.get_x() + bar.get_width()/2, mid_height,
                    f'{height-bottom_values[idx]:.1f}', ha='center', va='center', fontsize=8)

# 添加分段标签
for i in range(len(df)):
    # Baseline训练时间
    plt.text(x[i] - bar_width/2, df['Baseline_train'][i]/2,
            f"{df['Baseline_train'][i]:.1f}", ha='center', va='center', color='black', fontsize=8)
    # Baseline数据时间
    plt.text(x[i] - bar_width/2, df['Baseline_train'][i] + df['Baseline_data'][i]/2,
            f"{df['Baseline_data'][i]:.1f}", ha='center', va='center', color='black', fontsize=8)
    # Ours训练时间
    plt.text(x[i] + bar_width/2, df['Ours_train'][i]/2,
            f"{df['Ours_train'][i]:.1f}", ha='center', va='center', color='black', fontsize=8)
    # Ours数据时间
    plt.text(x[i] + bar_width/2, df['Ours_train'][i] + df['Ours_data'][i]/2,
            f"{df['Ours_data'][i]:.1f}", ha='center', va='center', color='black', fontsize=8)

# 添加总时间标签
add_labels(baseline_bars, is_total=True)
add_labels(ours_bars, is_total=True)

# 添加网格线
ax.grid(axis='y', linestyle='--', alpha=0.4)

# 调整布局
plt.tight_layout()

# 保存和显示图表
plt.savefig('batch_size_comparison_reversed.png', dpi=300, bbox_inches='tight')
plt.show()
