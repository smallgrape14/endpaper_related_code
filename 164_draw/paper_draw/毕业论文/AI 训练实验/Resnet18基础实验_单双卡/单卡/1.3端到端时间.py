import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import PercentFormatter
import pandas as pd

# # ===== 可调参数区 =====
# FONT_SIZE_LABEL = 10
# FONT_SIZE_TICK = 10
# FONT_SIZE_ANNOT = 9
# FONT_SIZE_LEGEND = 9
# FIG_WIDTH = 6
# FIG_HEIGHT = 4
# GRID_ALPHA = 0.5
# GRID_LINESTYLE = '--'

# # 设置中文字体支持
# plt.rcParams['font.sans-serif'] = ['SimHei']  # 用来正常显示中文标签
# plt.rcParams['axes.unicode_minus'] = False  # 用来正常显示负号
# ===== 可调参数区 =====
FONT_SIZE_LABEL = 18         # 坐标轴标签字体大小
FONT_SIZE_TICK = 18          # 坐标轴刻度字体大小
FONT_SIZE_ANNOT = 16         # 数据标注字体大小
FONT_SIZE_LEGEND = 16        # 图例字体大小
FIG_WIDTH = 8              # 单个子图的图宽
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
COLOR_GRID = '#CCCCCC'
COLOR_YLABEL = 'black'
COLOR_Y2SPINE = 'black'

# ===== 数据区 =====
# 从新数据构建DataFrame（相对于Baseline的端到端时间降低百分比）
data = {
    'batch_size': [32, 64, 128, 256, 512],
    'workernum=1': ['91.84%', '94.96%', '95.73%', '96.15%', '96.35%'],
    'workernum=8': ['7.89%', '18.39%', '38.24%', '42.40%', '44.11%'],
    'workernum=16': ['1.92%', '3.34%', '5.03%', '4.44%', '5.57%']
}

df = pd.DataFrame(data)

# 将百分比字符串转换为浮点数
for col in ['workernum=1', 'workernum=8', 'workernum=16']:
    df[col] = df[col].str.rstrip('%').astype('float')

# 数据提取
batch_sizes = df['batch_size'].values
baseline_worker1_pct = df['workernum=1'].values
baseline_worker8_pct = df['workernum=8'].values
baseline_worker16_pct = df['workernum=16'].values

# ====== 折线图：OUR相对于Baseline的端到端时间降低百分比 ======
# 调整标签偏移量
BASELINE_1_OFFSETS = [8, 8, 8, 8, 8]
BASELINE_8_OFFSETS = [8, 8, 8, 8, 8]
BASELINE_16_OFFSETS = [8, 8, 8, 8, 8]

fig1, ax1 = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))

# 绘制三条折线
line1, = ax1.plot(batch_sizes, baseline_worker1_pct, '--o', color=COLOR_BASELINE_1,
                  linewidth=2, markersize=8, label='相对于Baseline (Workernum=1)')
line2, = ax1.plot(batch_sizes, baseline_worker8_pct, '-.D', color=COLOR_BASELINE_8,
                  linewidth=2, markersize=8, label='相对于Baseline (Workernum=8)')
line3, = ax1.plot(batch_sizes, baseline_worker16_pct, ':^', color=COLOR_BASELINE_16,
                  linewidth=2, markersize=8, label='相对于Baseline (Workernum=16)')

# 设置图表属性
ax1.set_xlabel('批次大小 (Batch Size)', fontsize=FONT_SIZE_LABEL, color=COLOR_YLABEL)
ax1.set_ylabel('端到端时间降低百分比 (%)', fontsize=FONT_SIZE_LABEL, color=COLOR_YLABEL)
ax1.set_xscale('log', base=2)
ax1.set_xticks(batch_sizes)
ax1.set_xticklabels(batch_sizes, fontsize=FONT_SIZE_TICK)
ax1.tick_params(axis='y', labelsize=FONT_SIZE_TICK, colors=COLOR_YLABEL)
ax1.tick_params(axis='x', labelsize=FONT_SIZE_TICK, colors=COLOR_YLABEL)
ax1.spines['bottom'].set_color(COLOR_Y2SPINE)
ax1.spines['left'].set_color(COLOR_Y2SPINE)
ax1.grid(True, linestyle=GRID_LINESTYLE, alpha=GRID_ALPHA, color=COLOR_GRID)

# 设置y轴为百分比格式
ax1.yaxis.set_major_formatter(PercentFormatter())

# 设置y轴范围
y_min = 0
y_max = 110
ax1.set_ylim(y_min, y_max)

# 添加参考线
ax1.axhline(y=0, color='gray', linestyle='-', linewidth=0.5, alpha=0.5)
ax1.axhline(y=50, color='gray', linestyle='--', linewidth=0.5, alpha=0.3, label='50%参考线')
ax1.axhline(y=100, color='gray', linestyle='-', linewidth=0.5, alpha=0.5, label='100%参考线')

# 为每条线添加数据标签
# for i, (x, y) in enumerate(zip(batch_sizes, baseline_worker1_pct)):
#     ax1.annotate(f'{y:.1f}%', (x, y),
#                 xytext=(0, BASELINE_1_OFFSETS[i]),
#                 textcoords='offset points',
#                 ha='center', va='bottom', color=COLOR_BASELINE_1,
#                 fontsize=FONT_SIZE_ANNOT, rotation=0)

# for i, (x, y) in enumerate(zip(batch_sizes, baseline_worker8_pct)):
#     ax1.annotate(f'{y:.1f}%', (x, y),
#                 xytext=(0, BASELINE_8_OFFSETS[i]),
#                 textcoords='offset points',
#                 ha='center', va='bottom', color=COLOR_BASELINE_8,
#                 fontsize=FONT_SIZE_ANNOT, rotation=0)

# for i, (x, y) in enumerate(zip(batch_sizes, baseline_worker16_pct)):
#     ax1.annotate(f'{y:.1f}%', (x, y),
#                 xytext=(0, BASELINE_16_OFFSETS[i]),
#                 textcoords='offset points',
#                 ha='center', va='bottom', color=COLOR_BASELINE_16,
#                 fontsize=FONT_SIZE_ANNOT, rotation=0)

# 添加图例
lines = [line1, line2, line3]
labels = [l.get_label() for l in lines]
# ax1.legend(lines, labels, loc='upper left', bbox_to_anchor=(0, 1.1), ncol=3, mode="expand",
#             # borderaxespad=0.5,frameon=True,fancybox=True,
#             fontsize=FONT_SIZE_LEGEND)
ax1.legend(lines, labels, loc='upper left', bbox_to_anchor=(0, 0.7), ncol=1, fontsize=FONT_SIZE_LEGEND-4)

# 最有效的方法：使用 mode="expand"
# plt.legend(
#     loc='upper center',
#     bbox_to_anchor=(0.5, 1.15),  # 放在顶部中间，稍微上移
#     ncol=3,  # 3列，每个图例项占一列
#     mode="expand",  # ✅ 关键参数：均匀扩展宽度
#     borderaxespad=0.5,
#     frameon=True,
#     fancybox=True
# )
# 添加标题
# plt.title('OUR相对于Baseline的端到端时间降低百分比', fontsize=12, pad=20)

# plt.tight_layout()
plt.savefig("end_to_end_time_reduction_percentage.png", dpi=300, bbox_inches='tight')
print("图表已保存为 'end_to_end_time_reduction_percentage.png'")

# 显示图表
plt.show()