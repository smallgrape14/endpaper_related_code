import matplotlib.pyplot as plt

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

# ================== 图一：单机单 GPU ==================
plt.figure(figsize=(FIG_WIDTH, FIG_HEIGHT))
bars1 = plt.bar(range(len(workers)), single_gpu_percent, color='skyblue', edgecolor='black', alpha=0.8)

# 在每个柱子顶部标注数值
for i, (bar, percent) in enumerate(zip(bars1, single_gpu_percent)):
    height = bar.get_height()
    plt.text(bar.get_x() + bar.get_width()/2., height+0.03,
             f'{percent}%', ha='center', va='bottom', fontsize=FONT_SIZE_ANNOT)

# plt.title('单机单 GPU 数据加载时间占比', fontsize=FONT_SIZE_LABEL+2)
plt.xlabel('Worker 数量', fontsize=FONT_SIZE_LABEL)
plt.ylabel('数据加载时间占比 (%)', fontsize=FONT_SIZE_LABEL)
plt.xticks(range(len(workers)), [str(w) for w in workers])
plt.grid(True, linestyle=GRID_LINESTYLE, alpha=GRID_ALPHA)
plt.ylim(0, max(single_gpu_percent)*1.12)  # 留出空间显示标注

# 可选：保存第一个图
plt.savefig('single_gpu_data_loading_time.png', dpi=300, bbox_inches='tight')

plt.show()

# ================== 图二：单机双 GPU ==================
plt.figure(figsize=(FIG_WIDTH, FIG_HEIGHT))
bars2 = plt.bar(range(len(workers)), dual_gpu_percent, color='lightcoral', edgecolor='black', alpha=0.8)

# 在每个柱子顶部标注数值
for i, (bar, percent) in enumerate(zip(bars2, dual_gpu_percent)):
    height = bar.get_height()
    plt.text(bar.get_x() + bar.get_width()/2., height+0.08,
             f'{percent}%', ha='center', va='bottom', fontsize=FONT_SIZE_ANNOT)

# plt.title('单机双 GPU 数据加载时间占比', fontsize=FONT_SIZE_LABEL+2)
plt.xlabel('数据加载工作进程数量', fontsize=FONT_SIZE_LABEL)
plt.ylabel('数据加载时间占比 (%)', fontsize=FONT_SIZE_LABEL)
plt.xticks(range(len(workers)), [str(w) for w in workers])
plt.grid(True, linestyle=GRID_LINESTYLE, alpha=GRID_ALPHA)
plt.ylim(0, max(dual_gpu_percent)*1.09)

# 可选：保存第二个图
plt.savefig('dual_gpu_data_loading_time.png', dpi=300, bbox_inches='tight')

plt.show()