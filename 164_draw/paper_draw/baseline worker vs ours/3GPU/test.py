import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# 更新后的数据集（基于您提供的新数据）
data = {
    "batch_size": [8, 16, 32, 64, 128, 256, 512],
    "Baseline workernum=0": [0.066556369, 0.125743095, 0.243648804, 0.583092101, 1.064253323, 2.199586451, 4.306008353],
    "Baseline workernum=1": [0.054388039, 0.118148706, 0.22634335, 0.48963513, 0.974016898, 1.968698809, 3.811758958],
    "Baseline workernum=2": [0.011663082, 0.051205092, 0.118091175, 0.243152627, 0.482497083, 0.897650835, 1.989103606],
    "Baseline workernum=4": [0.016137802, 0.012123762, 0.045560306, 0.102440225, 0.207412845, 0.412409656, 0.816832749],
    "Baseline workernum=8": [0.013242363, 0.013338591, 0.026251997, 0.041915353, 0.081999793, 0.161717464, 0.323926228],
    "Baseline workernum=16": [0.023794325, 0.025576666, 0.032339884, 0.050851909, 0.091884316, 0.168890217, 0.335975658],
    "Baseline workernum=32": [0.038502813, 0.042162248, 0.04535277, 0.065608026, 0.108449426, 0.183167073, 0.359200848],
    "Ours": [0.006963302, 0.011659976, 0.014195685, 0.012451172, 0.014573526, 0.026356608, 0.053415152]
}

df = pd.DataFrame(data)

# 创建图表（增大画布尺寸以适应更多图例）
plt.figure(figsize=(14, 8))

# 扩展颜色和标记样式（10种不同样式）
colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', 
          '#8c564b',
        '#e377c2',
        #   '#7f7f7f', 
        '#bcbd22',
          '#17becf']
markers = ['o', 's', '^', 'v', 'D',
            'P', '*',
            # 'X',
            'd', 'h']

# 绘制折线
for i, col in enumerate(df.columns[1:]):
    # 区分基准线和优化方案的线型
    linestyle = '--' if 'Baseline' in col else '-'
    # 绘制折线
    plt.plot(df['batch_size'], df[col], 
             label=col, 
             color=colors[i],
             marker=markers[i],
             linestyle=linestyle,
             linewidth=2.5,
             markersize=9,
             markeredgewidth=1.5,
             markeredgecolor='white')

# 图表属性设置
plt.title('Data Loading Performance Comparison by Worker Count', fontsize=20, pad=20)
plt.xlabel('Batch Size', fontsize=16)
plt.ylabel('Data Loading Time (s) - Lower is Better', fontsize=16)

# 设置双对数坐标轴
plt.xscale('log', base=2)
plt.yscale('log')

# 自定义坐标轴范围
plt.ylim(1e-3, 10)

# 网格和刻度设置
plt.grid(True, which='both', linestyle='--', alpha=0.6)
plt.tick_params(axis='both', which='major', labelsize=18)

# 添加关键性能对比标注
plt.annotate('Ours outperforms all baselines',
             xy=(32, 0.007), 
             xytext=(50, 0.02),
             arrowprops=dict(arrowstyle="->", color='black', lw=1.5),
             fontsize=12,
             bbox=dict(boxstyle="round,pad=0.3", fc="white", ec="gray", alpha=0.8))

# 图例设置（右侧外部）
plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left', fontsize=12, 
           framealpha=0.9, shadow=True, title='Configuration')

# 优化布局并保存
plt.tight_layout()
plt.savefig('worker_comparison_until_w32.png', dpi=300, bbox_inches='tight')
plt.savefig('worker_comparison_until_w32.pdf', format='pdf', bbox_inches='tight')
plt.show()