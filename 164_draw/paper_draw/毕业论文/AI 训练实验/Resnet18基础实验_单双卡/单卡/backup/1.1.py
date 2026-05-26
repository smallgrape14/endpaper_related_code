import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import ScalarFormatter

# ===== 中文字体设置 =====
plt.rcParams['font.sans-serif'] = ['SimHei']  # 支持中文
plt.rcParams['axes.unicode_minus'] = False    # 支持负号

# ===== 可调参数区 =====
FONT_SIZE_LABEL = 10
FONT_SIZE_TICK = 10
FONT_SIZE_ANNOT = 10
FONT_SIZE_LEGEND = 10
FIG_WIDTH = 4
FIG_HEIGHT = 3
GRID_ALPHA = 0.5
GRID_LINESTYLE = '--'

# ===== 高对比度颜色区 =====
COLOR_BASELINE_LINE = '#0072B2'
COLOR_BASELINE_LABEL = '#0072B2'
COLOR_OURS_LINE = '#D55E00'
COLOR_OURS_LABEL = '#D55E00'
COLOR_BAR = '#009E73'
COLOR_BAR_LABEL = '#D55E00'
COLOR_GRID = '#CCCCCC'
COLOR_YLABEL = 'black'
COLOR_Y2LABEL = 'black'
COLOR_Y2SPINE = 'black'
# ======================
# 从 CSV 文件读取数据
import pandas as pd
csv_path = "../csv_file/singleGPU.csv"  # 替换为您的实际文件路径
df = pd.read_csv(csv_path)

# 确保列名正确（根据您的 CSV 格式）
# 如果列名不同，请修改以下列名匹配
df.columns = ['BatchSize', 'BaselineDataTime(s)', 'OursDataTime(s)', 'BaselineTrainTime(s)', 'OursTrainTime(s)']



# ===== 数据区 =====
#这里是数据加载时间
batch_sizes = np.array(df['BatchSize'])
baseline_times = np.array(df['BaselineDataTime(s)'])
ours_times     = np.array(df['OursDataTime(s)'])

# ====== 折线图：数据加载时间 ======
BASELINE_LABEL_OFFSETS = [12, 12, 8, 8, 8, 8]
OURS_LABEL_OFFSETS     = [2, 8,  6, 6, 6, 6]

fig1, ax1 = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))
line1, = ax1.plot(batch_sizes, baseline_times, '-o', color=COLOR_BASELINE_LINE,
                  linewidth=2, markersize=8, label='Baseline')
line2, = ax1.plot(batch_sizes, ours_times, '-s', color=COLOR_OURS_LINE,
                  linewidth=2, markersize=8, label='Ours')

ax1.set_xlabel('批次大小（Batch Size）', fontsize=FONT_SIZE_LABEL, color=COLOR_YLABEL)
ax1.set_ylabel('单个Batch的平均数据加载时间(s)', fontsize=FONT_SIZE_LABEL, color=COLOR_YLABEL)
ax1.set_yscale('log')
ax1.set_xscale('log', base=2)
ax1.set_xticks(batch_sizes)
ax1.set_xticklabels(batch_sizes, fontsize=FONT_SIZE_TICK)
ax1.tick_params(axis='y', labelsize=FONT_SIZE_TICK, colors=COLOR_YLABEL)
ax1.tick_params(axis='x', labelsize=FONT_SIZE_TICK, colors=COLOR_YLABEL)
ax1.spines['bottom'].set_color(COLOR_Y2SPINE)
ax1.spines['left'].set_color(COLOR_Y2SPINE)
ax1.grid(True, linestyle=GRID_LINESTYLE, alpha=GRID_ALPHA, color=COLOR_GRID)

# 修正y轴科学计数法为普通小数
ax1.yaxis.set_major_formatter(ScalarFormatter())
ax1.ticklabel_format(style='plain', axis='y')

for i, (x, y) in enumerate(zip(batch_sizes, baseline_times)):
    ax1.annotate(f'{y:.6f}', (x, y),
                xytext=(0, BASELINE_LABEL_OFFSETS[i]),
                textcoords='offset points',
                ha='center', va='bottom', color=COLOR_BASELINE_LABEL,
                fontsize=FONT_SIZE_ANNOT)
for i, (x, y) in enumerate(zip(batch_sizes, ours_times)):
    ax1.annotate(f'{y:.6f}', (x, y),
                xytext=(0, OURS_LABEL_OFFSETS[i]),
                textcoords='offset points',
                ha='center', va='bottom', color=COLOR_OURS_LABEL,
                fontsize=FONT_SIZE_ANNOT)

lines = [line1, line2]
labels = [l.get_label() for l in lines]
ax1.legend(lines, labels, loc='upper left', fontsize=FONT_SIZE_LEGEND)

plt.tight_layout()
# plt.show()
plt.savefig("1.1.1.png", dpi=300, bbox_inches='tight')  # 保存图表为PNG文件

# ====== 柱状图：提升比例（1-加速比） ======
# ratios = 1 - (ours_times / baseline_times)
# log_batch_sizes = np.log2(batch_sizes)
# log_gaps = np.diff(log_batch_sizes, append=log_batch_sizes[-1]+(log_batch_sizes[-1]-log_batch_sizes[-2]))
# bar_widths = log_gaps * 0.7

# fig2, ax2 = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))
# bars = ax2.bar(log_batch_sizes, ratios, width=bar_widths, color=COLOR_BAR, alpha=0.85, align='center', label='提升比例')
# ax2.set_ylim(0, 1.15)
# ax2.set_xlabel('批次大小（Batch Size）', fontsize=FONT_SIZE_LABEL, color=COLOR_Y2LABEL)
# ax2.set_ylabel('数据加载时间降低比例', fontsize=FONT_SIZE_LABEL, color=COLOR_Y2LABEL)
# ax2.spines['bottom'].set_color(COLOR_Y2SPINE)
# ax2.spines['left'].set_color(COLOR_Y2SPINE)
# ax2.tick_params(axis='y', colors=COLOR_Y2LABEL, labelsize=FONT_SIZE_TICK)
# ax2.tick_params(axis='x', colors=COLOR_Y2LABEL, labelsize=FONT_SIZE_TICK)
# ax2.set_xticks(log_batch_sizes)
# ax2.set_xticklabels(batch_sizes, fontsize=FONT_SIZE_TICK)
# ax2.grid(True, linestyle=GRID_LINESTYLE, alpha=GRID_ALPHA, color=COLOR_GRID)

# RATIO_LABEL_OFFSETS = [5, 5, 5, 5, 5, 5]
# for i, bar in enumerate(bars):
#     height = bar.get_height()
#     ax2.annotate(f'{height:.2f}',  # 精度2位小数
#                 (bar.get_x() + bar.get_width() / 2, height),
#                 xytext=(0, RATIO_LABEL_OFFSETS[i]),
#                 textcoords='offset points',
#                 ha='center', va='bottom', color=COLOR_BAR_LABEL,
#                 fontsize=FONT_SIZE_ANNOT)

# ax2.legend([bars], ['降低比例'], loc='upper left', fontsize=FONT_SIZE_LEGEND)

# plt.tight_layout()
# # plt.show()
# plt.savefig("1.1.2.png", dpi=300, bbox_inches='tight')  # 保存图表为PNG文件