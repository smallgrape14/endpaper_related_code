import numpy as np
import matplotlib.pyplot as plt

# ===== 中文字体设置 =====
plt.rcParams['font.sans-serif'] = ['SimHei']  # 支持中文
plt.rcParams['axes.unicode_minus'] = False    # 支持负号

# ===== 可调参数区 =====
FONT_SIZE_LABEL = 10         # 坐标轴标签字体大小
FONT_SIZE_TICK = 10          # 坐标轴刻度字体大小
FONT_SIZE_ANNOT = 10         # 数据标注字体大小
FONT_SIZE_LEGEND = 10        # 图例字体大小
FIG_WIDTH = 4                # 图宽
FIG_HEIGHT = 3               # 图高
GRID_ALPHA = 0.5             # 网格透明度
GRID_LINESTYLE = '--'        # 网格线型

# ===== 颜色区（高对比度） =====
COLOR_BASELINE_LINE = '#0072B2'      # Baseline线和点（蓝色）
COLOR_BASELINE_LABEL = '#0072B2'     # Baseline标注字体
COLOR_OURS_LINE = '#D55E00'          # Ours线和点（橙红色）
COLOR_OURS_LABEL = '#D55E00'         # Ours标注字体
COLOR_BAR = '#009E73'                # 柱状图（高对比度绿色）
COLOR_BAR_LABEL = '#D55E00'          # 柱状标注字体（橙红色）
COLOR_GRID = '#CCCCCC'               # 网格线颜色
COLOR_YLABEL = 'black'               # 坐标轴标签颜色
COLOR_Y2LABEL = 'black'              # 坐标轴标签颜色
COLOR_Y2SPINE = 'black'              # 坐标轴边框颜色
# ======================

# 从 CSV 文件读取数据
import pandas as pd
csv_path = "csv_file/multiGPU.csv"  # 替换为您的实际文件路径
df = pd.read_csv(csv_path)

# 确保列名正确（根据您的 CSV 格式）
# 如果列名不同，请修改以下列名匹配
df.columns = ['BatchSize', 'BaselineDataTime(s)', 'OursDataTime(s)', 'BaselineTrainTime(s)', 'OursTrainTime(s)']


# 数据准备
batch_sizes = np.array(df['BatchSize'])

baseline_data_time = np.array(df['BaselineDataTime(s)'])
ours_data_time     = np.array(df['OursDataTime(s)'])
baseline_train_time = np.array(df['BaselineTrainTime(s)'])
ours_train_time     = np.array(df['OursTrainTime(s)'])

baseline_ratios = baseline_data_time / (baseline_data_time + baseline_train_time)
ours_ratios = ours_data_time / (ours_data_time + ours_train_time)

# ====== 折线图：数据加载时间占比 ======
BASELINE_LABEL_OFFSETS = [12, 12, 8, 8, 8, 8]
OURS_LABEL_OFFSETS     = [2, 8,  6, 6, 6, 6]

fig1, ax1 = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))

line1, = ax1.plot(batch_sizes, baseline_ratios, '-o', color=COLOR_BASELINE_LINE,
                  linewidth=2, markersize=8, label='Baseline')
line2, = ax1.plot(batch_sizes, ours_ratios, '-s', color=COLOR_OURS_LINE,
                  linewidth=2, markersize=8, label='Ours')

ax1.set_xlabel('Batch Size', fontsize=FONT_SIZE_LABEL, color=COLOR_YLABEL)
ax1.set_ylabel('数据加载时间占比', fontsize=FONT_SIZE_LABEL, color=COLOR_YLABEL)
ax1.set_xscale('log', base=2)  # 对数坐标，底数为2
ax1.set_xticks(batch_sizes)
ax1.set_xticklabels(batch_sizes, fontsize=FONT_SIZE_TICK)
ax1.tick_params(axis='y', labelsize=FONT_SIZE_TICK, colors=COLOR_YLABEL)
ax1.tick_params(axis='x', labelsize=FONT_SIZE_TICK, colors=COLOR_YLABEL)
ax1.spines['bottom'].set_color(COLOR_Y2SPINE)
ax1.spines['left'].set_color(COLOR_Y2SPINE)
ax1.grid(True, linestyle=GRID_LINESTYLE, alpha=GRID_ALPHA, color=COLOR_GRID)

for i, (x, y) in enumerate(zip(batch_sizes, baseline_ratios)):
    ax1.annotate(f'{y*100:.2f}%', (x, y),
                xytext=(0, BASELINE_LABEL_OFFSETS[i]),
                textcoords='offset points',
                ha='center', va='bottom', color=COLOR_BASELINE_LABEL,
                fontsize=FONT_SIZE_ANNOT)

for i, (x, y) in enumerate(zip(batch_sizes, ours_ratios)):
    ax1.annotate(f'{y*100:.2f}%', (x, y),
                xytext=(0, OURS_LABEL_OFFSETS[i]),
                textcoords='offset points',
                ha='center', va='bottom', color=COLOR_OURS_LABEL,
                fontsize=FONT_SIZE_ANNOT)

lines = [line1, line2]
labels = [l.get_label() for l in lines]
ax1.legend(lines, labels, loc='upper left', fontsize=FONT_SIZE_LEGEND)

plt.tight_layout()
# plt.show()
plt.savefig("2.2.1.png", dpi=300, bbox_inches='tight')  # 保存图表为PNG文件


# ====== 柱状图：比例相减 Baseline占比 - Ours占比 ======
reduce_percent = baseline_ratios - ours_ratios

log_batch_sizes = np.log2(batch_sizes)
log_gaps = np.diff(log_batch_sizes, append=log_batch_sizes[-1]+(log_batch_sizes[-1]-log_batch_sizes[-2]))
bar_widths = log_gaps * 0.7

fig2, ax2 = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))
bars = ax2.bar(log_batch_sizes, reduce_percent, width=bar_widths, color=COLOR_BAR, alpha=0.85, align='center', label='占比差值')

ax2.set_xlabel('Batch Size', fontsize=FONT_SIZE_LABEL, color=COLOR_Y2LABEL)
ax2.set_ylabel('数据加载占端到端时间减少百分比', fontsize=FONT_SIZE_LABEL, color=COLOR_Y2LABEL)
ax2.spines['bottom'].set_color(COLOR_Y2SPINE)
ax2.spines['left'].set_color(COLOR_Y2SPINE)
ax2.tick_params(axis='y', colors=COLOR_Y2LABEL, labelsize=FONT_SIZE_TICK)
ax2.tick_params(axis='x', colors=COLOR_Y2LABEL, labelsize=FONT_SIZE_TICK)
ax2.set_xticks(log_batch_sizes)
ax2.set_xticklabels(batch_sizes, fontsize=FONT_SIZE_TICK)
ax2.grid(True, linestyle=GRID_LINESTYLE, alpha=GRID_ALPHA, color=COLOR_GRID)

RATIO_LABEL_OFFSETS = [5, 5, 5, 5, 5, 5]
for i, bar in enumerate(bars):
    height = bar.get_height()
    ax2.annotate(f'{height*100:.2f}%',  # 百分号和两位小数
                (bar.get_x() + bar.get_width() / 2, height),
                xytext=(0, RATIO_LABEL_OFFSETS[i]),
                textcoords='offset points',
                ha='center', va='bottom', color=COLOR_BAR_LABEL,
                fontsize=FONT_SIZE_ANNOT)

ax2.legend([bars], ['占比差值'], loc='upper left', fontsize=FONT_SIZE_LEGEND)

plt.tight_layout()
# plt.show()
plt.savefig("2.2.2.png", dpi=300, bbox_inches='tight')  # 保存图表为PNG文件
