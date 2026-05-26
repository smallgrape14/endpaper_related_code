import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# ===== 可调参数区 =====
FONT_SIZE_LABEL = 16         # 坐标轴标签字体大小
FONT_SIZE_TICK = 16          # 坐标轴刻度字体大小
FONT_SIZE_ANNOT = 15         # 数据标注字体大小
FONT_SIZE_LEGEND = 15        # 图例字体大小
FIG_WIDTH = 5.8              # 单个子图的图宽
FIG_HEIGHT = 4.8             # 单个子图的图高
GRID_ALPHA = 0.5             # 网格透明度
GRID_LINESTYLE = '--'        # 网格线型

# 设置中文字体与数学公式支持
# plt.rcParams['font.sans-serif'] = ['SimSun']  # 用来正常显示中文标签
# # plt.rcParams['font.sans-serif'] = ['SimHei']  # 用来正常显示中文标签
# # plt.rcParams['font.serif'] = ['Times New Roman']
# plt.rcParams['font.serif'] = ['Times New Roman']
# plt.rcParams['axes.unicode_minus'] = False    # 用来正常显示负号
# # plt.rcParams['mathtext.fontset'] = 'stix'     # 设置数学公式字体（可选，使 2^n 更好看）
# 方法：设置西文字体为 Times New Roman，中文字体为宋体（SimSun）
# plt.rcParams['font.family'] = 'serif'
# plt.rcParams['font.serif'] = ['Times New Roman']  # 西文用 Times New Roman，中文用 SimSun
# plt.rcParams['axes.unicode_minus'] = False  # 解决负号显示问题

# ===== FIXED FONT CONFIGURATION =====
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['SimSun','Times New Roman' ]  # Times + 宋体
plt.rcParams['mathtext.fontset'] = 'stix'  # Math symbols matching Times
plt.rcParams['axes.unicode_minus'] = False
plt.rcParams['axes.formatter.use_mathtext'] = False


# 设置全局字体
plt.rcParams.update({
    # 'font.family': 'serif',
    # 'font.serif': ['Times New Roman', 'SimSun'],  # 英文用Times，中文用宋体
    # 'font.sans-serif': ['Times New Roman', 'SimSun'],       # 备用配置
    # 'mathtext.fontset': 'stix',                            # 数学字体
    # 'axes.unicode_minus': False,                          # 负号显示
    'font.size': FONT_SIZE_TICK,
    'axes.labelsize': FONT_SIZE_LABEL,
    'legend.fontsize': FONT_SIZE_LEGEND,
    'xtick.labelsize': FONT_SIZE_TICK,
    'ytick.labelsize': FONT_SIZE_TICK
})

# 创建数据
data = {
    '线程数': [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024],
    '客户端_IOPS': [992.95, 1738.29, 3297.91, 6377.98, 12670.72, 20939.76, 23700, 13472.87, 6529.74, 3014.74, 1238.77],
    # '后端_SSD_IOPS': [6509.36, 20772.9, 33747.6, 56064.5, 102235, 162938, 179793, 336387, 289667, 221872, 146873]
    '后端_SSD_IOPS': [10484, 30036, 68476, 132862, 234880, 372224, 374784, 342528, 326400, 361472, 374528]
}

df = pd.DataFrame(data)

# 创建图表
fig, ax1 = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT), dpi=100)

# 绘制柱状图
x = np.arange(len(df['线程数']))  # 生成 0, 1, 2, ... 作为x坐标
width = 0.35

bars1 = ax1.bar([i - width/2 for i in x], [i/1000 for i in df['客户端_IOPS']], width, 
                label='客户端 IOPS', color='#ff7f0e', alpha=0.8)
bars2 = ax1.bar([i + width/2 for i in x], [i/1000 for i in df['后端_SSD_IOPS']], width, 
                label='后端SSD_IOPS', color='#1f77b4', alpha=0.8)

# 设置左侧y轴
ax1.set_xlabel('线程数', fontsize=FONT_SIZE_LABEL)
ax1.set_ylabel('KIOPS (千次操作/秒)', fontsize=FONT_SIZE_LABEL)
ax1.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
ax1.legend(loc='upper left', fontsize=FONT_SIZE_LEGEND, frameon=False)

# --- 核心修改：将x轴刻度标签改为 2^n 格式 ---
# 1. 设置x轴刻度的位置（在每一个条形图的中心）
ax1.set_xticks(x) 
# 2. 计算每个线程数的对数，并生成 LaTeX 标签
#    注意：线程数1 = 2^0, 线程数2 = 2^1, ..., 线程数1024 = 2^10
power_labels = [rf'$2^{{{int(np.log2(t))}}}$' for t in df['线程数']]
ax1.set_xticklabels(power_labels, fontsize=FONT_SIZE_TICK)

# yticklist=[]
# for i in range(0,8):
#     yticklist.append(100*i)
# ax1.set_yticklabels(yticklist,fontsize=FONT_SIZE_TICK)

# 设置 Y 轴刻度，从 0 到 400，步长为 50
y_ticks = np.arange(0, 401, 50)  # 0, 50, 100, ..., 450
ax1.set_yticks(y_ticks)
ax1.set_yticklabels(y_ticks, fontsize=FONT_SIZE_TICK)

# 添加图例
handles = [bars1[0], bars2[0]]
# labels = ['客户端IOPS', '后端SSD_IOPS']
labels = ['BeeGFS', '后端SSD']

ax1.legend(handles, labels, loc='upper left', fontsize=FONT_SIZE_LEGEND, frameon=False)

# 在柱状图上添加数值标签
# for i, bar in enumerate(bars1):
#     height = bar.get_height()
#     ax1.annotate(f'{height:.0f}K',
#                 xy=(bar.get_x() + bar.get_width() / 2, height),
#                 xytext=(0, 3),
#                 textcoords="offset points",
#                 ha='center', va='bottom', fontsize=FONT_SIZE_ANNOT-2, color='#ff7f0e')

# for i, bar in enumerate(bars2):
#     height = bar.get_height()
#     ax1.annotate(f'{height:.0f}K',
#                 xy=(bar.get_x() + bar.get_width() / 2, height),
#                 xytext=(0, 0),
#                 textcoords="offset points",
#                 ha='center', va='bottom', fontsize=FONT_SIZE_ANNOT-2, color='#1f77b4')

# 保存图表
performance_comparison_chart = '动机一BeeGFSvsSSD_跨节点.png'
plt.tight_layout()
plt.savefig(performance_comparison_chart, dpi=300, bbox_inches='tight', facecolor='white')
print(f"图表已保存为: {performance_comparison_chart}")
plt.show()