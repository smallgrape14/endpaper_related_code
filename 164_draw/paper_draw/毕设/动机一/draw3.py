import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# ===== 可调参数区 =====
# FONT_SIZE_LABEL = 10         # 坐标轴标签字体大小
# FONT_SIZE_TICK = 10          # 坐标轴刻度字体大小
# FONT_SIZE_ANNOT = 10         # 数据标注字体大小
# FONT_SIZE_LEGEND = 10        # 图例字体大小
# FIG_WIDTH = 4                # 单个子图的图宽
# FIG_HEIGHT = 3               # 单个子图的图高
FONT_SIZE_LABEL = 18         # 坐标轴标签字体大小
FONT_SIZE_TICK = 18          # 坐标轴刻度字体大小
FONT_SIZE_ANNOT = 16         # 数据标注字体大小
FONT_SIZE_LEGEND = 16        # 图例字体大小
FIG_WIDTH = 7              # 单个子图的图宽
FIG_HEIGHT = 3             # 单个子图的图高
GRID_ALPHA = 0.5             # 网格透明度
GRID_LINESTYLE = '--'        # 网格线型

# 设置中文字体支持
# plt.rcParams['font.sans-serif'] = ['SimHei']  # 用来正常显示中文标签
# plt.rcParams['axes.unicode_minus'] = False  # 用来正常显示负号
# ===== FIXED FONT CONFIGURATION =====
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

# 创建新数据
data = {
    '线程数': [1,2,4 ,8, 16, 32, 64, 128, 256,512, 1024],
    # '客户端_IOPS': [1462.35, 11200.90, 19750.00, 55468.10, 39203.00, 10284.00, 4577.70, 1171.29],
    # '后端_SSD_IOPS': [11800, 131000, 238000, 350000, 350000, 359000, 371000, 361000],
    'EXT4_IOPS': [6509.36, 20772.9, 33747.6, 56064.5, 102235, 162938, 179793, 336387, 289667, 221872, 146873],
    'BeeGFS_IOPS': [1361.54, 2461.18, 4277.28, 7840.6, 12905.9, 24887.8, 23972.4, 10554.7, 5555.67, 2788.24, 1086.59],
    'BeeGFS/EXT4':[1361.54/6509.36*100, 2461.18/20772.9*100, 4277.28/33747.6*100, 7840.6/56064.5*100, 12905.9/102235*100, 24887.8/162938*100, 23972.4/179793*100, 10554.7/336387*100, 5555.67/289667*100, 2788.24/221872*100, 1086.59/146873*100]
}

df = pd.DataFrame(data)

# 创建图表
fig, ax1 = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT), dpi=100)

# 绘制柱状图
x = np.arange(len(df['线程数']))
width = 0.35

bars1 = ax1.bar([i - width/2 for i in x], [i/1000 for i in df['BeeGFS_IOPS']], width, 
                label='客户端 IOPS', color='#ff7f0e', alpha=0.8)
bars2 = ax1.bar([i + width/2 for i in x], [i/1000 for i in df['EXT4_IOPS']], width, 
                label='后端 SSD IOPS', color="#14a045", alpha=0.8)

# 设置左侧y轴
ax1.set_xlabel('线程数', fontsize=FONT_SIZE_LABEL)
ax1.set_ylabel('KIOPS (千次操作/秒)', fontsize=FONT_SIZE_LABEL)
# ax1.set_title('BeeGFS_IOPS与EXT4_IOPS对比', fontsize=FONT_SIZE_LABEL+2, fontweight='bold')
ax1.set_xticks(x)
power_labels = [rf'$2^{{{int(np.log2(t))}}}$' for t in df['线程数']]
ax1.set_xticklabels(power_labels, fontsize=FONT_SIZE_TICK)

# ax1.set_xticklabels(df['线程数'], fontsize=FONT_SIZE_TICK)
ax1.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
ax1.legend(loc='upper left', fontsize=FONT_SIZE_LEGEND, frameon=False)

# 创建第二个y轴用于显示SSD利用率
# ax2 = ax1.twinx()
# line, = ax2.plot(x, df['SSD_利用率'], color='#2ca02c', marker='o', linewidth=1.5, markersize=4, label='SSD利用率')
# ax2.set_ylabel('SSD利用率 (%)', fontsize=FONT_SIZE_LABEL, color='#2ca02c')
# ax2.tick_params(axis='y', labelcolor='#2ca02c', labelsize=FONT_SIZE_TICK)
# 设置 Y 轴刻度，从 0 到 400，步长为 50
y_ticks = np.arange(0, 351, 50)  # 0, 50, 100, ..., 450
ax1.set_yticks(y_ticks)
ax1.set_yticklabels(y_ticks, fontsize=FONT_SIZE_TICK)

# 添加组合图例
handles = [bars1[0], bars2[0]]
# labels = ['BeeGFS_IOPS', 'EXT4_IOPS']
labels = ['BeeGFS', 'EXT4']

ax1.legend(handles, labels, loc='upper left', fontsize=FONT_SIZE_LEGEND, frameon=False)

# 在柱状图上添加数值标签
# for i, bar in enumerate(bars1):
#     height = bar.get_height()
#     ax1.annotate(f'{height:.0f}K',
#                 xy=(bar.get_x() + bar.get_width() / 2, height),
#                 xytext=(0, 3),  # 3 points vertical offset
#                 textcoords="offset points",
#                 ha='center', va='bottom', fontsize=FONT_SIZE_ANNOT)

# for i, bar in enumerate(bars2):
#     height = bar.get_height()
#     ax1.annotate(f'{height:.0f}K',
#                 xy=(bar.get_x() + bar.get_width() / 2, height),
#                 xytext=(0, 3),
#                 textcoords="offset points",
#                 ha='center', va='bottom', fontsize=FONT_SIZE_ANNOT)

# 保存图表
performance_comparison_chart = '动机一BeeGFSvsEXT4_本地节点.png'
plt.tight_layout()
plt.savefig(performance_comparison_chart, dpi=300, bbox_inches='tight', facecolor='white')
print(f"图表已保存为: {performance_comparison_chart}")
plt.show()