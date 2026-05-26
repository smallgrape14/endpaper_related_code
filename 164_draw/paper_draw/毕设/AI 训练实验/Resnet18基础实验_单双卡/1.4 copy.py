# GDS
import numpy as np
import matplotlib.pyplot as plt

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

# ===== 颜色区 =====
COLOR_OURS_LINE = '#D55E00'      # Ours线和点（橙红色）
COLOR_OURS_LABEL = '#D55E00'     # Ours标注字体
COLOR_GDS_LINE = '#CC79A7'       # GDS线和点（紫色高对比）
COLOR_GDS_LABEL = '#CC79A7'      # GDS标注字体
COLOR_BAR = '#009E73'            # 柱状图（绿色）
COLOR_BAR_LABEL = '#D55E00'      # 柱状标注字体（橙红色）
COLOR_GRID = '#CCCCCC'           # 网格线颜色
COLOR_YLABEL = 'black'           # 坐标轴标签颜色
COLOR_Y2LABEL = 'black'          # 坐标轴标签颜色
COLOR_Y2SPINE = 'black'          # 坐标轴边框颜色
# ======================
# 从 CSV 文件读取数据
import pandas as pd
csv_path = "csv_file/singleGPU_GDS.csv"  # 替换为您的实际文件路径
df = pd.read_csv(csv_path)

# 确保列名正确（根据您的 CSV 格式）
# 如果列名不同，请修改以下列名匹配
df.columns = ['BatchSize', 'GDSDataTime(s)', 'OursDataTime(s)', 'GDSTrainTime(s)', 'OursTrainTime(s)']

# ==== 数据准备 ====
batch_sizes = np.array(df['BatchSize'])
ours_data_time     = np.array(df['OursDataTime(s)'])
ours_train_time    = np.array(df['OursTrainTime(s)'])
ours_ratios = ours_data_time / (ours_data_time + ours_train_time)

# GDS只有前5组
gds_data_time      = np.array(df['GDSDataTime(s)'].append(np.nan))
gds_train_time     = np.array(df['GDSTrainTime(s)'].append(np.nan))
gds_ratios = gds_data_time / (gds_data_time + gds_train_time)

# ====== 折线图：数据加载时间占比 ======
GDS_LABEL_OFFSETS = [2, 8, 6, 6, 6，6]  # GDS标注偏移
OURS_LABEL_OFFSETS = [2, 8, 6, 6, 6, 6]

fig1, ax1 = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))

line1, = ax1.plot(batch_sizes, ours_ratios, '-o', color=COLOR_OURS_LINE,
                  linewidth=2, markersize=8, label='Ours')
line2, = ax1.plot(batch_sizes[:5], gds_ratios, '-s', color=COLOR_GDS_LINE,
                  linewidth=2, markersize=8, label='GDS')

ax1.set_xlabel('Batch Size', fontsize=FONT_SIZE_LABEL, color=COLOR_YLABEL)
ax1.set_ylabel('数据加载时间占比', fontsize=FONT_SIZE_LABEL, color=COLOR_YLABEL)
ax1.set_xscale('log', base=2)
ax1.set_xticks(batch_sizes)
ax1.set_xticklabels(batch_sizes, fontsize=FONT_SIZE_TICK)
ax1.tick_params(axis='y', labelsize=FONT_SIZE_TICK, colors=COLOR_YLABEL)
ax1.tick_params(axis='x', labelsize=FONT_SIZE_TICK, colors=COLOR_YLABEL)
ax1.spines['bottom'].set_color(COLOR_Y2SPINE)
ax1.spines['left'].set_color(COLOR_Y2SPINE)
ax1.grid(True, linestyle=GRID_LINESTYLE, alpha=GRID_ALPHA, color=COLOR_GRID)

# ours标注
for i, (x, y) in enumerate(zip(batch_sizes, ours_ratios)):
    ax1.annotate(f'{y*100:.2f}%', (x, y),
                xytext=(0, OURS_LABEL_OFFSETS[i]),
                textcoords='offset points',
                ha='center', va='bottom', color=COLOR_OURS_LABEL,
                fontsize=FONT_SIZE_ANNOT)
# gds标注
for i, (x, y) in enumerate(zip(batch_sizes[:5], gds_ratios)):
    ax1.annotate(f'{y*100:.2f}%', (x, y),
                xytext=(0, GDS_LABEL_OFFSETS[i]),
                textcoords='offset points',
                ha='center', va='bottom', color=COLOR_GDS_LABEL,
                fontsize=FONT_SIZE_ANNOT)

lines = [line1, line2]
labels = [l.get_label() for l in lines]
ax1.legend(lines, labels, loc='upper left', fontsize=FONT_SIZE_LEGEND)

plt.tight_layout()
# plt.show()
plt.savefig("1.4.1.png", dpi=300, bbox_inches='tight')  # 保存图表为PNG文件

# ====== 柱状图：Ours与GDS占比差值 ======
reduce_percent = gds_ratios - ours_ratios[:5]
log_batch_sizes = np.log2(batch_sizes[:5])
log_gaps = np.diff(log_batch_sizes, append=log_batch_sizes[-1]+(log_batch_sizes[-1]-log_batch_sizes[-2]))
bar_widths = log_gaps * 0.7

fig2, ax2 = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))
bars = ax2.bar(log_batch_sizes, reduce_percent, width=bar_widths, color=COLOR_BAR, alpha=0.85, align='center', label='Ours-GDS')

ax2.set_xlabel('Batch Size', fontsize=FONT_SIZE_LABEL, color=COLOR_Y2LABEL)
ax2.set_ylabel('数据加载占端到端时间差值', fontsize=FONT_SIZE_LABEL, color=COLOR_Y2LABEL)
ax2.spines['bottom'].set_color(COLOR_Y2SPINE)
ax2.spines['left'].set_color(COLOR_Y2SPINE)
ax2.tick_params(axis='y', colors=COLOR_Y2LABEL, labelsize=FONT_SIZE_TICK)
ax2.tick_params(axis='x', colors=COLOR_Y2LABEL, labelsize=FONT_SIZE_TICK)
ax2.set_xticks(log_batch_sizes)
ax2.set_xticklabels(batch_sizes[:5], fontsize=FONT_SIZE_TICK)
ax2.grid(True, linestyle=GRID_LINESTYLE, alpha=GRID_ALPHA, color=COLOR_GRID)

RATIO_LABEL_OFFSETS = [5, 5, 5, 5, 5]
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
plt.show()
plt.savefig("1.4.2.png", dpi=300, bbox_inches='tight')  # 保存图表为PNG文件