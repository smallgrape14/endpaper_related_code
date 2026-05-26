import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import PercentFormatter
import pandas as pd

# # ===== 可调参数区 =====
# FONT_SIZE_LABEL = 10
# FONT_SIZE_TICK = 10
# FONT_SIZE_ANNOT = 10
# FONT_SIZE_LEGEND = 10
# FIG_WIDTH = 6
# FIG_HEIGHT = 4
# GRID_ALPHA = 0.5
# GRID_LINESTYLE = '--'

# # 设置中文字体支持
# # ===== 字体设置 =====
# # 设置英文字体为 Times New Roman
# # plt.rcParams['font.family'] = 'Times New Roman'
# # 设置中文字体支持
# plt.rcParams['font.sans-serif'] = ['SimHei']  # 用来正常显示中文标签
# plt.rcParams['axes.unicode_minus'] = False  # 用来正常显示负号
# ===== 可调参数区 =====
FONT_SIZE_LABEL = 22         # 坐标轴标签字体大小
FONT_SIZE_TICK = 20          # 坐标轴刻度字体大小
FONT_SIZE_ANNOT = 18         # 数据标注字体大小
FONT_SIZE_LEGEND = 18        # 图例字体大小
FIG_WIDTH = 7              # 单个子图的图宽
FIG_HEIGHT = 4.8             # 单个子图的图高
GRID_ALPHA = 0.5             # 网格透明度
GRID_LINESTYLE = '--'        # 网格线型
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

# ===== 高对比度颜色区 =====
COLOR_BASELINE_1 = '#1f77b4'  # 蓝色
COLOR_BASELINE_8 = "#ff0e22"  # 红色
COLOR_BASELINE_16 = '#2ca02c'  # 绿色
COLOR_OURS = '#ff7f0e'  # 橙色
COLOR_GRID = '#CCCCCC'
COLOR_YLABEL = 'black'
COLOR_Y2SPINE = 'black'

# ===== 数据区 =====
# 从新数据构建DataFrame（数据加载时间百分比）
data_percentage = {
    'batch_size': [32, 64, 128, 256, 512],
    'workernum=1': [92.51, 94.88, 96.16, 96.58, 96.78],
    'workernum=8': [14.74, 25.91, 43.88, 49.14, 50.63],
    'workernum=16': [9.07, 11.68, 13.69, 15.67, 16.59],
    'Ours': [6.43, 7.29, 7.52, 9.59, 9.57]
}

df_percentage = pd.DataFrame(data_percentage)

# 数据提取
batch_sizes = df_percentage['batch_size'].values
baseline_worker1_pct = df_percentage['workernum=1'].values
baseline_worker8_pct = df_percentage['workernum=8'].values
baseline_worker16_pct = df_percentage['workernum=16'].values
ours_pct = df_percentage['Ours'].values

print(baseline_worker1_pct)
# ====== 折线图：不同配置的数据加载时间百分比对比 ======
# 调整标签偏移量，避免重叠
BASELINE_1_OFFSETS = [10, 10, 10, 10, 10]
BASELINE_8_OFFSETS = [8, 8, 8, 8, 8]
BASELINE_16_OFFSETS = [6, 6, 6, 6, 6]
OURS_OFFSETS = [-12, -12, -12, -12, -12]

fig2, ax2 = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))

# 绘制四条折线
line1, = ax2.plot(batch_sizes, baseline_worker1_pct, '--o', color=COLOR_BASELINE_1,
                  linewidth=2, markersize=8, label='Baseline (Workernum=1)')
line2, = ax2.plot(batch_sizes, baseline_worker8_pct, '-.D', color=COLOR_BASELINE_8,
                  linewidth=2, markersize=8, label='Baseline (Workernum=8)')
line3, = ax2.plot(batch_sizes, baseline_worker16_pct, ':^', color=COLOR_BASELINE_16,
                  linewidth=2, markersize=8, label='Baseline (Workernum=16)')
line4, = ax2.plot(batch_sizes, ours_pct, '-s', color=COLOR_OURS,
                  linewidth=2, markersize=8, label='OURS')

# 设置图表属性
# ax2.set_xlabel('批次大小 (Batch Size)', fontsize=FONT_SIZE_LABEL, color=COLOR_YLABEL, **chinese_font)
# ax2.set_ylabel('单个批次（Batch）数据加载时间占比(%)', fontsize=FONT_SIZE_LABEL, color=COLOR_YLABEL, **chinese_font)
ax2.set_xlabel('批次大小 (Batch Size)', fontsize=FONT_SIZE_LABEL, color=COLOR_YLABEL)
ax2.set_ylabel('单个批次数据加载时间占比(%)', fontsize=FONT_SIZE_LABEL, color=COLOR_YLABEL)
ax2.set_xscale('log', base=2)
ax2.set_xticks(batch_sizes)
ax2.set_xticklabels(batch_sizes, fontsize=FONT_SIZE_TICK)
ax2.tick_params(axis='y', labelsize=FONT_SIZE_TICK, colors=COLOR_YLABEL)
ax2.tick_params(axis='x', labelsize=FONT_SIZE_TICK, colors=COLOR_YLABEL)
ax2.spines['bottom'].set_color(COLOR_Y2SPINE)
ax2.spines['left'].set_color(COLOR_Y2SPINE)
ax2.grid(True, linestyle=GRID_LINESTYLE, alpha=GRID_ALPHA, color=COLOR_GRID)

# 设置y轴为百分比格式
ax2.yaxis.set_major_formatter(PercentFormatter())

# 设置y轴范围，留出一些空间给标签
y_min = 0
y_max = 105
ax2.set_ylim(y_min, y_max)

# 为每条线添加数据标签
# for i, (x, y) in enumerate(zip(batch_sizes, baseline_worker1_pct)):
#     ax2.annotate(f'{y:.1f}%', (x, y),
#                 xytext=(0, BASELINE_1_OFFSETS[i]),
#                 textcoords='offset points',
#                 ha='center', va='bottom', color=COLOR_BASELINE_1,
#                 fontsize=FONT_SIZE_ANNOT, rotation=0)

# for i, (x, y) in enumerate(zip(batch_sizes, baseline_worker8_pct)):
#     ax2.annotate(f'{y:.1f}%', (x, y),
#                 xytext=(0, BASELINE_8_OFFSETS[i]),
#                 textcoords='offset points',
#                 ha='center', va='bottom', color=COLOR_BASELINE_8,
#                 fontsize=FONT_SIZE_ANNOT, rotation=0)

# for i, (x, y) in enumerate(zip(batch_sizes, baseline_worker16_pct)):
#     ax2.annotate(f'{y:.1f}%', (x, y),
#                 xytext=(0, BASELINE_16_OFFSETS[i]),
#                 textcoords='offset points',
#                 ha='center', va='bottom', color=COLOR_BASELINE_16,
#                 fontsize=FONT_SIZE_ANNOT, rotation=0)

# for i, (x, y) in enumerate(zip(batch_sizes, ours_pct)):
#     ax2.annotate(f'{y:.1f}%', (x, y),
#                 xytext=(0, OURS_OFFSETS[i]),
#                 textcoords='offset points',
#                 ha='center', va='bottom', color=COLOR_OURS,
#                 fontsize=FONT_SIZE_ANNOT, rotation=0)

# 添加图例
lines = [line1, line2, line3, line4]
labels = [l.get_label() for l in lines]
# ax2.legend(lines, labels, loc='upper left',bbox_to_anchor=(0, 0.8), fontsize=FONT_SIZE_LEGEND)
# ax2.legend(lines, labels, loc='upper left',bbox_to_anchor=(0, 1.2),  # (x, y) 坐标，y>1 表示在图外上方
#            ncol=4,
#            fontsize=FONT_SIZE_LEGEND)
plt.tight_layout()
plt.savefig("data_loading_percentage_comparison.png", dpi=300, bbox_inches='tight')
print("数据加载时间占比图表已保存为 'data_loading_percentage_comparison.png'")