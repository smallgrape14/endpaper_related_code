import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# ===== 可调参数区 =====
# FONT_SIZE_LABEL = 10         # 坐标轴标签字体大小
# FONT_SIZE_TICK = 10          # 坐标轴刻度字体大小
# FONT_SIZE_ANNOT = 9          # 数据标注字体大小
# FONT_SIZE_LEGEND = 9         # 图例字体大小
# FIG_WIDTH = 6                # 单个子图的图宽
# FIG_HEIGHT = 4               # 单个子图的图高
# GRID_ALPHA = 0.5             # 网格透明度
# GRID_LINESTYLE = '--'        # 网格线型

# # 设置中文字体支持
# plt.rcParams['font.sans-serif'] = ['SimHei']  # 用来正常显示中文标签
# plt.rcParams['axes.unicode_minus'] = False  # 用来正常显示负号

FONT_SIZE_LABEL = 20         # 坐标轴标签字体大小
FONT_SIZE_TICK = 20          # 坐标轴刻度字体大小
FONT_SIZE_ANNOT = 18         # 数据标注字体大小
FONT_SIZE_LEGEND = 18        # 图例字体大小
FIG_WIDTH = 5.8              # 单个子图的图宽
FIG_HEIGHT = 4.8             # 单个子图的图高
GRID_ALPHA = 0.5             # 网格透明度
GRID_LINESTYLE = '--'        # 网格线型

plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['SimSun','Times New Roman' ]  # Times + 宋体
plt.rcParams['mathtext.fontset'] = 'stix'  # Math symbols matching Times
plt.rcParams['axes.unicode_minus'] = False
plt.rcParams['axes.formatter.use_mathtext'] = False

# 设置全局字体
plt.rcParams.update({
    'font.size': FONT_SIZE_TICK,
    'axes.labelsize': FONT_SIZE_LABEL,
    'legend.fontsize': FONT_SIZE_LEGEND,
    'xtick.labelsize': FONT_SIZE_TICK,
    'ytick.labelsize': FONT_SIZE_TICK
})
# T1: SSD->CPU->GPU 路径延迟
t1_latency = [2478.83, 2521.75, 2342.66, 2716.99, 3387.92]

# T2: SSD->GPU 路径延迟
t2_latency = [519.799011, 527.956604, 530.9736325, 525.3432615, 581.611328]
# 创建数据
data = {
    'IOsize(KB)': [4, 8, 16, 32, 64],
    'T1:SSD->CPU->GPU': t1_latency,
    'T2: SSD->GPU': t2_latency
}

df = pd.DataFrame(data)

# 创建图表
fig, ax1 = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT), dpi=100)

# 绘制柱状图
x = np.arange(len(df['IOsize(KB)']))
width = 0.35

# bars1 = ax1.bar([i - width/2 for i in x], df['T1:SSD->CPU->GPU'], width, 
#                 label='T1: SSD→CPU→GPU', color='#1f77b4', alpha=0.8)
# bars2 = ax1.bar([i + width/2 for i in x], df['T2: SSD->GPU'], width, 
#                 label='T2: SSD→GPU', color='#ff7f0e', alpha=0.8)

bars1 = ax1.bar([i - width/2 for i in x], df['T1:SSD->CPU->GPU'], width, 
                label='T1: SSD→CPU→GPU', color="#7f7f7f", alpha=0.8)
bars2 = ax1.bar([i + width/2 for i in x], df['T2: SSD->GPU'], width, 
                label='T2: SSD→GPU', color="#17becf", alpha=0.8)
# 设置坐标轴
ax1.set_xlabel('I/O 大小 (KB)', fontsize=FONT_SIZE_LABEL)
ax1.set_ylabel('延迟 (微秒)', fontsize=FONT_SIZE_LABEL)
ax1.set_xticks(x)
ax1.set_xticklabels(df['IOsize(KB)'], fontsize=FONT_SIZE_TICK)
ax1.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE, axis='y')
ax1.legend(loc='upper left', bbox_to_anchor=(0, 1), fontsize=FONT_SIZE_LEGEND, frameon=False)

# 在柱状图上添加数值标签
# for i, bar in enumerate(bars1):
#     height = bar.get_height()
#     ax1.annotate(f'{height:.1f}',
#                 xy=(bar.get_x() + bar.get_width() / 2, height),
#                 xytext=(0, 3),  # 3 points vertical offset
#                 textcoords="offset points",
#                 ha='center', va='bottom', fontsize=FONT_SIZE_ANNOT, color='#1f77b4')

# for i, bar in enumerate(bars2):
#     height = bar.get_height()
#     ax1.annotate(f'{height:.1f}',
#                 xy=(bar.get_x() + bar.get_width() / 2, height),
#                 xytext=(0, 3),
#                 textcoords="offset points",
#                 ha='center', va='bottom', fontsize=FONT_SIZE_ANNOT, color='#ff7f0e')

# 添加标题
# plt.title('不同IO大小下SSD到GPU的数据传输延迟对比', fontsize=FONT_SIZE_LABEL+2, pad=20)

# 保存图表
performance_comparison_chart = 'small_IO_remote_SSD_to_GPU_latency_comparison.png'
plt.tight_layout()
plt.savefig(performance_comparison_chart, dpi=300, bbox_inches='tight', facecolor='white')
plt.savefig('small_IO_remote_SSD_to_GPU_latency_comparison.pdf', dpi=300, bbox_inches='tight', facecolor='white')
print(f"图表已保存为: {performance_comparison_chart}")
plt.show()