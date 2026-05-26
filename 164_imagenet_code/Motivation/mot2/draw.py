import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# 创建数据
single_gpu_data = {
    'batch_size': [32, 64, 128, 256, 512],
    'cpu_loader': [19.23, 34.33, 38.91, 49.97, 53.02]
}

dual_gpu_data = {
    'batch_size': [8, 16, 32, 64, 128, 256],
    'cpu_loader': [29.46, 28.04, 45.53, 54.78, 61.77, 66.28]
}

# 转换为DataFrame
df_single = pd.DataFrame(single_gpu_data)
df_dual = pd.DataFrame(dual_gpu_data)

# 设置中文字体
plt.rcParams['font.sans-serif'] = ['Noto Sans CJK JP']
plt.rcParams['axes.unicode_minus'] = False

# 创建图表
fig, ax = plt.subplots(figsize=(12, 7))

# 绘制单机单卡和单机双卡的折线图
line1 = ax.plot(df_single['batch_size'], df_single['cpu_loader'], marker='o', 
                label='单机单卡', color='#1f77b4', linewidth=2, markersize=8)
line2 = ax.plot(df_dual['batch_size'], df_dual['cpu_loader'], marker='s', 
                label='单机双卡', color='#ff7f0e', linewidth=2, markersize=8)

# 为单机单卡数据点添加标签
for i, (x, y) in enumerate(zip(df_single['batch_size'], df_single['cpu_loader'])):
    ax.text(x, y+2, f'{y}%', ha='center', va='bottom', fontsize=10, 
            fontweight='bold', color='#1f77b4',
            bbox=dict(boxstyle='round,pad=0.2', facecolor='white', edgecolor='#1f77b4', alpha=0.8))

# 为单机双卡数据点添加标签
for i, (x, y) in enumerate(zip(df_dual['batch_size'], df_dual['cpu_loader'])):
    # 调整标签位置，避免重叠
    va_pos = 'bottom' if y != 28.04 else 'top'
    y_offset = 2 if y != 28.04 else -2
    
    ax.text(x, y+y_offset, f'{y}%', ha='center', va=va_pos, fontsize=12, 
            fontweight='bold', color='#ff7f0e',
            bbox=dict(boxstyle='round,pad=0.2', facecolor='white', edgecolor='#ff7f0e', alpha=0.8))

# 设置坐标轴标签
ax.set_xlabel('数据读取的批次大小', fontsize=15)
ax.set_ylabel('传统CPU数据加载器占端到端时间的百分比 (%)', fontsize=15)

# 设置网格
ax.grid(True, alpha=0.3, linestyle='--')

# 设置X轴为对数刻度
ax.set_xscale('log')
ax.set_xticks([8, 16, 32, 64, 128, 256, 512])
ax.set_xticklabels(['8', '16', '32', '64', '128', '256', '512'], fontsize=14)

# 设置Y轴范围，为标签留出空间
ax.set_ylim(0, 80)

# 调整Y轴刻度字体大小
ax.tick_params(axis='y', labelsize=14)

# 添加图例
ax.legend(fontsize=12, loc='lower right')

# 设置标题
plt.title('单机单卡/双卡的训练过程中数据加载时间占比分析', fontsize=16, fontweight='bold', pad=20)

# 添加网格线
ax.yaxis.grid(True, alpha=0.2)
ax.xaxis.grid(True, alpha=0.2)

# 调整布局
plt.tight_layout()

# 保存图表
plt.savefig('performance_comparison_with_labels.png', dpi=300, bbox_inches='tight')
print('图表已保存为: performance_comparison_with_labels.png')

# 显示图表
plt.show()