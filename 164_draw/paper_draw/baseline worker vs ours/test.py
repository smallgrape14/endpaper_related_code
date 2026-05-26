import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# 更新后的数据集（基于您提供的新数据）
data = {
    "batch_size": [8, 16, 32, 64, 128, 256],
    "Baseline worker=0": [0.064871135, 0.13336372, 0.255006324, 0.575134936, 1.049032248, 2.099788369],
    "Baseline worker=1": [0.055442449, 0.127164439, 0.246786561, 0.52907212, 1.120404425, 1.906278987],
    "Baseline worker=2": [0.011477937, 0.046225445, 0.11227575, 0.248237926, 0.509329412, 0.883794576],
    "Baseline worker=4": [0.007963315, 0.014296235, 0.044608158, 0.118922359, 0.216662756, 0.437187306],
    "Baseline worker=8": [0.011139786, 0.011072473, 0.021733313, 0.04180397, 0.081569892, 0.16110957],
    # "Baseline worker=16": [np.nan, 0.019038463, 0.031569131, 0.050057706, 0.089572679, 0.167922949],
    # "Baseline worker=32": [np.nan, np.nan, 0.044348766, 0.065579332, 0.104801212, 0.183960277],
    # "Baseline worker=40": [np.nan, np.nan, np.nan, 0.073192045, 0.112235274, 0.193847568],
    "Baseline worker=16": [0.016212221, 0.019038463, 0.031569131, 0.050057706, 0.089572679, 0.167922949],
    "Baseline worker=32": [0.034264285, 0.0336896, 0.044348766, 0.065579332, 0.104801212, 0.183960277],
    "Baseline worker=40": [0.040037986, 0.046036309, 0.053359386, 0.073192045, 0.112235274, 0.193847568],
    "Ours_old": [0.003588945, 0.006138337, 0.007042786, 0.007193807, 0.011435493, 0.033179436],
    # "Ours": [0.0034193650655064923,0.003975897416777756,0.007533340558953311,0.01461046162633405,0.03197773655076911,0.03197773655076911*2]
    "Ours":[0.00114938,0.00134879,0.002438405,0.004659966,0.01172559,0.031704362]

}

df = pd.DataFrame(data)

# 创建图表（增大画布尺寸以适应更多图例）
plt.figure(figsize=(14, 8))

# 扩展颜色和标记样式（10种不同样式）
colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', 
          '#8c564b',
        '#e377c2', '#7f7f7f', 
        '#bcbd22',
        '#000000',
          '#17becf']
markers = ['o', 's', '^', 'v', 'D',
            'P', '*', 'X',
            'o',
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
plt.savefig('worker_comparison_new1013.png', dpi=300, bbox_inches='tight')
plt.savefig('worker_comparison_new1013.pdf', format='pdf', bbox_inches='tight')
plt.show()