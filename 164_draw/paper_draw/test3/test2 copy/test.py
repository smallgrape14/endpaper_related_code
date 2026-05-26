import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# 更新后的数据集
data = {
    "batch_size": [16, 32, 64, 128, 256, 512],
    "Baseline worker=0": [0.130329745, 0.260987053, 0.542040969, 1.074424398, 2.169599543, 4.255721262],
    "Baseline worker=1": [0.130394034, 0.271011119, 0.525898178, 1.099640511, 2.421618227, 4.323619921],
    "Baseline worker=2": [0.051295414, 0.106432336, 0.22211631, 0.480746943, 0.978216287, 1.958114392],
    "Baseline worker=4": [0.015879627, 0.039689157, 0.086237754, 0.179231171, 0.365362481, 0.813993355],
    "Baseline worker=8": [0.001356369, 0.00496969, 0.01492066, 0.028603378, 0.076399502, 0.157854138],
    # "Baseline worker=16": [0.001336731, 0.002014723, 0.003644474, 0.006998049, 0.0135518, 0.024252935],
    "Ours prefetch=1": [0.002936787, 0.003732909, 0.006691542, 0.013216008, 0.023752612, 0.051714178],
    "Ours prefetch=2": [0.001782213, 0.002713001, 0.004117876, 0.007672081, 0.017764, np.nan],  # 512处无数据
    "Ours prefetch=3": [0.002025039, 0.003035537, 0.003951914, 0.008269395, 0.015095616, np.nan]  # 512处无数据
}

df = pd.DataFrame(data)

# 创建图表（增大画布尺寸以适应更多图例）
plt.figure(figsize=(14, 8))

# 扩展颜色和标记样式
colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd',  '#e377c2', '#7f7f7f', '#bcbd22']
markers = ['o', 's', '^', 'v', 'D',  '*', 'X', 'd']

# 绘制折线
for i, col in enumerate(df.columns[1:]):
    # 区分基准线和优化方案的线型
    linestyle = '--' if 'Baseline' in col else '-'
    # 绘制折线（自动处理缺失值）
    plt.plot(df['batch_size'], df[col], 
             label=col, 
             color=colors[i],
             marker=markers[i],
             linestyle=linestyle,
             linewidth=2.5,
             markersize=9,
             markeredgewidth=1.5)

# 图表属性设置
plt.title('Data Load Time Comparison', fontsize=20, pad=20)
plt.xlabel('Batch Size', fontsize=16)
plt.ylabel('Data Time (s) - Lower is Better', fontsize=16)

# 设置双对数坐标轴
plt.xscale('log', base=2)
plt.yscale('log')

# 自定义坐标轴范围
plt.ylim(1e-3, 10)

# 网格和刻度设置
plt.grid(True, which='both', linestyle='--', alpha=0.6)
plt.tick_params(axis='both', which='major', labelsize=18)

# 图例设置（右侧外部）
plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left', fontsize=12, 
           framealpha=0.9, shadow=True)

# 优化布局并保存
plt.tight_layout()
plt.savefig('batch_time_comparison.png', dpi=300, bbox_inches='tight')
plt.savefig('batch_time_comparison.pdf', format='pdf', bbox_inches='tight')
plt.show()