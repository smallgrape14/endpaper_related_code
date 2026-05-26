import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# 创建数据框
data = {
    'batch_size': [16, 16, 16, 16, 16, 16, 32, 32, 32, 32, 32, 32, 64, 64, 64, 64, 64, 64, 
                   128, 128, 128, 128, 128, 128, 256, 256, 256, 256, 256, 256, 512, 512, 512, 512, 512, 512],
    'worker_num': [1, 2, 4, 8, 16, 32, 1, 2, 4, 8, 16, 32, 1, 2, 4, 8, 16, 32, 
                   1, 2, 4, 8, 16, 32, 1, 2, 4, 8, 16, 32, 1, 2, 4, 8, 16, 32],
    'prefetch_factor': [8]*36,
    'data_per': [0.774418855, 0.531585851, 0.206396156, 0.223178274, 0.204130497, 0.242198243,
                 0.873018613, 0.735692064, 0.404842075, 0.244723556, 0.239760518, 0.341737741,
                 0.923604259, 0.844994704, 0.618517893, 0.310043049, 0.320969114, 0.312703714,
                 0.947897747, 0.891651998, 0.735117821, 0.414981797, 0.322165035, 0.427104113,
                 0.956701489, 0.905256073, 0.778446494, 0.379488299, 0.315525113, 0.603337436,
                 0.962621411, 0.919887124, 0.80319949, 0.412311654, 0.34553612, 0.662657074],
    'AVG_data': [0.107440093, 0.037391971, 0.009609606, 0.012150185, 0.013849366, 0.018898412,
                 0.221037692, 0.094965364, 0.02396894, 0.015771398, 0.01908946, 0.033003728,
                 0.458901144, 0.214804603, 0.066942126, 0.025026378, 0.033209871, 0.03223911,
                 0.981370566, 0.442816809, 0.155274396, 0.043424354, 0.047440192, 0.068371576,
                 1.932144062, 0.861647971, 0.326720556, 0.064813796, 0.062394857, 0.185745221,
                 4.022042948, 1.794958746, 0.691110265, 0.128155073, 0.127151054, 0.383147548],
    'AVG_train': [0.031296319, 0.032948447, 0.036949429, 0.042291426, 0.053996282, 0.059130281,
                  0.032150142, 0.034117671, 0.035236714, 0.048674373, 0.060529405, 0.063572458,
                  0.037957916, 0.039403621, 0.041287768, 0.055692663, 0.070257626, 0.070858834,
                  0.053942124, 0.053808343, 0.055949426, 0.061217233, 0.099814125, 0.091710179,
                  0.087445208, 0.090179911, 0.092987874, 0.105978811, 0.135354402, 0.122117692,
                  0.156175927, 0.156322774, 0.169336328, 0.182665811, 0.240830894, 0.195051287]
}

df = pd.DataFrame(data)

# 获取唯一的batch size值
batch_sizes = sorted(df['batch_size'].unique())

# 创建图表
fig, ax = plt.subplots(figsize=(12, 8))

# 为每个batch size绘制一条线
colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b']
markers = ['o', 's', '^', 'D', 'v', 'p']

# 绘制data_per的折线图
for i, batch_size in enumerate(batch_sizes):
    batch_data = df[df['batch_size'] == batch_size]
    
    # 绘制线条
    line = ax.plot(batch_data['worker_num'], batch_data['data_per'], 
                   marker=markers[i % len(markers)], linestyle='-', linewidth=2, markersize=8,
                   color=colors[i % len(colors)], label=f'Batch Size {batch_size}', zorder=3)

# 设置X轴为对数刻度（因为worker数量跨度很大）
ax.set_xscale('log', base=2)
ax.set_xticks([1, 2, 4, 8, 16, 32])
ax.set_xticklabels([1, 2, 4, 8, 16, 32])

# 设置Y轴范围，稍微留出顶部空间
y_min = df['data_per'].min() * 0.9
y_max = df['data_per'].max() * 1.1
ax.set_ylim(y_min, y_max)

# 设置坐标轴标签
ax.set_xlabel('Worker Number', fontsize=12, fontweight='bold')
ax.set_ylabel('Data Percentage (data_per)', fontsize=12, fontweight='bold', color='black')

# 设置刻度标签大小
ax.tick_params(axis='y', labelcolor='black', labelsize=10)
ax.tick_params(axis='x', labelsize=10)

# 添加网格线
ax.grid(True, alpha=0.3, linestyle='--')

# 添加图例，放在图表右侧外部
ax.legend(loc='center left', bbox_to_anchor=(1.02, 0.5),
          fontsize=10, frameon=True, framealpha=0.9, edgecolor='gray')

# 设置标题
plt.title('Data Percentage vs Worker Number for Different Batch Sizes', 
          fontsize=14, fontweight='bold', pad=20)

# 调整布局，为图例留出空间
plt.tight_layout(rect=[0, 0, 0.85, 1])

# 保存图表
plt.savefig('data_per_vs_worker_num.png', dpi=300, bbox_inches='tight')
print("data_per_vs_worker_num.png")
plt.show()