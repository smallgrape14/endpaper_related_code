import matplotlib.pyplot as plt
import numpy as np

# ===== 可调参数区 =====
FONT_SIZE_LABEL = 14         # 坐标轴标签字体大小
FONT_SIZE_TICK = 14          # 坐标轴刻度字体大小
FONT_SIZE_ANNOT = 14         # 数据标注字体大小
FONT_SIZE_LEGEND = 14        # 图例字体大小
FIG_WIDTH = 5.4              # 单个子图的图宽
FIG_HEIGHT = 4.4             # 单个子图的图高
GRID_ALPHA = 0.5             # 网格透明度
GRID_LINESTYLE = '--'        # 网格线型

# 设置中文字体支持
plt.rcParams['font.sans-serif'] = ['SimHei']  # 用来正常显示中文标签
plt.rcParams['axes.unicode_minus'] = False    # 用来正常显示负号

# 设置全局字体
plt.rcParams.update({
    'font.size': FONT_SIZE_TICK,
    'axes.labelsize': FONT_SIZE_LABEL,
    'legend.fontsize': FONT_SIZE_LEGEND,
    'xtick.labelsize': FONT_SIZE_TICK,
    'ytick.labelsize': FONT_SIZE_TICK
})

# 数据
workers = [32, 64, 128, 256, 512]
single_gpu_percent = [19.23, 34.33, 38.91, 49.97, 53.02]
dual_gpu_percent = [28.04, 45.53, 54.78, 61.77, 66.28]

# 创建画布，包含两个子图
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(FIG_WIDTH * 2, FIG_HEIGHT))

# ================== 第一个子图：单机单 GPU ==================
bars1 = ax1.bar(range(len(workers)), single_gpu_percent, color='skyblue', edgecolor='black', alpha=0.8)

# 在每个柱子顶部标注数值
for bar, percent in zip(bars1, single_gpu_percent):
    height = bar.get_height()
    ax1.text(bar.get_x() + bar.get_width()/2., height+0.05,
             f'{percent}%', ha='center', va='bottom', fontsize=FONT_SIZE_ANNOT)

ax1.set_title('单机单 GPU 数据加载时间占比', fontsize=FONT_SIZE_LABEL+2)
ax1.set_xlabel('Worker 数量', fontsize=FONT_SIZE_LABEL)
ax1.set_ylabel('数据加载时间占比 (%)', fontsize=FONT_SIZE_LABEL)
ax1.set_xticks(range(len(workers)))
ax1.set_xticklabels([str(w) for w in workers])
ax1.grid(True, linestyle=GRID_LINESTYLE, alpha=GRID_ALPHA)
ax1.set_ylim(0, max(single_gpu_percent)*1.15)  # 留出空间显示标注

# ================== 第二个子图：单机双 GPU ==================
bars2 = ax2.bar(range(len(workers)), dual_gpu_percent, color='lightcoral', edgecolor='black', alpha=0.8)

# 在每个柱子顶部标注数值
for bar, percent in zip(bars2, dual_gpu_percent):
    height = bar.get_height()
    ax2.text(bar.get_x() + bar.get_width()/2., height+0.25,
             f'{percent}%', ha='center', va='bottom', fontsize=FONT_SIZE_ANNOT)

ax2.set_title('单机双 GPU 数据加载时间占比', fontsize=FONT_SIZE_LABEL+2)
ax2.set_xlabel('Worker 数量', fontsize=FONT_SIZE_LABEL)
ax2.set_ylabel('数据加载时间占比 (%)', fontsize=FONT_SIZE_LABEL)
ax2.set_xticks(range(len(workers)))
ax2.set_xticklabels([str(w) for w in workers])
ax2.grid(True, linestyle=GRID_LINESTYLE, alpha=GRID_ALPHA)
ax2.set_ylim(0, max(dual_gpu_percent)*1.10) 

# 调整布局并显示
plt.tight_layout()
# plt.show()
performance_comparison_chart = '动机三单机单双GPU的数据加载时间占比.png'
plt.savefig(performance_comparison_chart, dpi=300, bbox_inches='tight', facecolor='white')