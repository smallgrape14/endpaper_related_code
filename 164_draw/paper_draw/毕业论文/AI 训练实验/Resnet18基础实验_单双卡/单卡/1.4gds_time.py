# GDS 测试

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import ScalarFormatter

# ===== 中文字体设置 =====
plt.rcParams['font.sans-serif'] = ['SimHei']
# plt.rcParams['font.sans-serif'] = ['Noto Sans CJK JP']

plt.rcParams['axes.unicode_minus'] = False

import matplotlib.font_manager as fm

# # 检查系统可用中文字体
# chinese_fonts = [f.name for f in fm.fontManager.ttflist if 'CJK' in f.name or 'SC' in f.name]
# print("可用中文字体:", chinese_fonts)  # 输出类似 ['SimHei', 'Microsoft YaHei']

# # ===== 可调参数区 =====
# FONT_SIZE_LABEL = 10
# FONT_SIZE_TICK = 10
# FONT_SIZE_ANNOT = 10
# FONT_SIZE_LEGEND = 10
# FIG_WIDTH = 4
# FIG_HEIGHT = 3
# GRID_ALPHA = 0.5
# GRID_LINESTYLE = '--'
# ===== 可调参数区 =====
FONT_SIZE_LABEL = 24         # 坐标轴标签字体大小
FONT_SIZE_TICK = 22          # 坐标轴刻度字体大小
FONT_SIZE_ANNOT = 20         # 数据标注字体大小
FONT_SIZE_LEGEND = 20        # 图例字体大小
FIG_WIDTH =7              # 单个子图的图宽
FIG_HEIGHT = 4.8             # 单个子图的图高
GRID_ALPHA = 0.5             # 网格透明度
GRID_LINESTYLE = '--'        # 网格线型
# ===== FIXED FONT CONFIGURATION =====
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['SimSun','Times New Roman' ]  # Times + 宋体
plt.rcParams['mathtext.fontset'] = 'stix'  # Math symbols matching Times
plt.rcParams['axes.unicode_minus'] = False
plt.rcParams['axes.formatter.use_mathtext'] = False

# ===== 高对比度颜色区 =====
COLOR_OURS_LINE = '#ff7f0e'       # Ours线和点（橙红色）
COLOR_OURS_LABEL = '#ff7f0e' 
COLOR_GDS_LINE = '#CC79A7'       # GDS线和点（紫色）
COLOR_GDS_LABEL = '#CC79A7'
COLOR_BAR = '#009E73'
COLOR_BAR_LABEL = '#D55E00'
COLOR_GRID = '#CCCCCC'
COLOR_YLABEL = 'black'
COLOR_Y2LABEL = 'black'
COLOR_Y2SPINE = 'black'
# ======================
# 从 CSV 文件读取数据
import pandas as pd
csv_path = "csv_file/singleGPU_gds.csv"  # 替换为您的实际文件路径
df = pd.read_csv(csv_path)

# 确保列名正确（根据您的 CSV 格式）
# 如果列名不同，请修改以下列名匹配
df.columns = ['BatchSize', 'GDSDataTime(s)', 'OursDataTime(s)', 'GDSTrainTime(s)', 'OursTrainTime(s)']


# batch size
batch_sizes = np.array(df['BatchSize'])

# ours 数据
ours_times = np.array(df['OursDataTime(s)'])

list_gds_data=df['GDSDataTime(s)'].tolist()
# list_gds_data.append(np.nan)
print(list_gds_data)
# GDS 数据（batch_size=512缺失，前5个点，最后一个用nan）
gds_times_full = np.array(df['GDSDataTime(s)'])

# ====== 折线图：数据加载时间（到512） ======
BASELINE_LABEL_OFFSETS = [12, 12, 8, 8, 8, 8]
OURS_LABEL_OFFSETS     = [2, 8,  6, 6, 6, 6]
GDS_LABEL_OFFSETS      = [2, 8,  6, 6, 6, 6]

fig1, ax1 = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))

line1, = ax1.plot(batch_sizes, ours_times, '-s', color=COLOR_OURS_LINE,
                  linewidth=2, markersize=10, label='OURS')
# GDS只画前5个点
line2, = ax1.plot(batch_sizes, gds_times_full, '--o', color=COLOR_GDS_LINE,
                  linewidth=2, markersize=10, label='GDS')

ax1.set_xlabel('批次大小(Batch Size)', fontsize=FONT_SIZE_LABEL, color=COLOR_YLABEL)
ax1.set_ylabel('单个批次的数据加载时间(s)', fontsize=FONT_SIZE_LABEL, color=COLOR_YLABEL)
ax1.set_yscale('log')
ax1.set_xscale('log', base=2)
ax1.set_xticks(batch_sizes)
ax1.set_xticklabels(batch_sizes, fontsize=FONT_SIZE_TICK)
ylist=[]
for i in range(-3,1):
    ylist.append(10**i)
ax1.set_yticks(ylist)
ax1.set_yticklabels(ylist,fontsize=FONT_SIZE_TICK)

ax1.tick_params(axis='y', labelsize=FONT_SIZE_TICK, colors=COLOR_YLABEL)
ax1.tick_params(axis='x', labelsize=FONT_SIZE_TICK, colors=COLOR_YLABEL)
ax1.spines['bottom'].set_color(COLOR_Y2SPINE)
ax1.spines['left'].set_color(COLOR_Y2SPINE)
ax1.grid(True, linestyle=GRID_LINESTYLE, alpha=GRID_ALPHA, color=COLOR_GRID)
ax1.yaxis.set_major_formatter(ScalarFormatter())
ax1.ticklabel_format(style='plain', axis='y')

# ours 标注
# for i, (x, y) in enumerate(zip(batch_sizes, ours_times)):
#     ax1.annotate(f'{y:.5f}', (x, y),
#                 xytext=(0, OURS_LABEL_OFFSETS[i]),
#                 textcoords='offset points',
#                 ha='center', va='bottom', color=COLOR_OURS_LABEL,
#                 fontsize=FONT_SIZE_ANNOT)
# # GDS 标注（只前5个点）
# for i, (x, y) in enumerate(zip(batch_sizes, gds_times_full)):
#     ax1.annotate(f'{y:.5f}', (x, y),
#                 xytext=(0, GDS_LABEL_OFFSETS[i]),
#                 textcoords='offset points',
#                 ha='center', va='bottom', color=COLOR_GDS_LABEL,
#                 fontsize=FONT_SIZE_ANNOT)

lines = [line1, line2]
labels = [l.get_label() for l in lines]
ax1.legend(lines, labels, loc='upper left', fontsize=FONT_SIZE_LEGEND)

plt.tight_layout()
# plt.show()
plt.savefig("gds_datatime.png", dpi=300, bbox_inches='tight')  # 保存图表为PNG文件

# ====== 柱状图：提升比例（到256） ======
batch_sizes_bar = batch_sizes
ours_times_bar = ours_times
gds_times_bar = gds_times_full

# 提升比例=1-ours/GDS
ratios_ours_vs_gds = 1 - (ours_times_bar / gds_times_bar)

log_batch_sizes_bar = np.log2(batch_sizes_bar)
log_gaps = np.diff(log_batch_sizes_bar, append=log_batch_sizes_bar[-1]+(log_batch_sizes_bar[-1]-log_batch_sizes_bar[-2]))
bar_widths = log_gaps * 0.7

fig2, ax2 = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))
bars = ax2.bar(log_batch_sizes_bar, ratios_ours_vs_gds, width=bar_widths, color=COLOR_BAR, alpha=0.85, align='center', label='提升比例')

ax2.set_xlabel('Batch Size', fontsize=FONT_SIZE_LABEL, color=COLOR_Y2LABEL)
ax2.set_ylabel('数据加载时间降低比例 (Ours vs GDS)', fontsize=FONT_SIZE_LABEL, color=COLOR_Y2LABEL)
ax2.spines['bottom'].set_color(COLOR_Y2SPINE)
ax2.spines['left'].set_color(COLOR_Y2SPINE)
ax2.tick_params(axis='y', colors=COLOR_Y2LABEL, labelsize=FONT_SIZE_TICK)
ax2.tick_params(axis='x', colors=COLOR_Y2LABEL, labelsize=FONT_SIZE_TICK)
ax2.set_xticks(log_batch_sizes_bar)
ax2.set_xticklabels(batch_sizes_bar, fontsize=FONT_SIZE_TICK)
ax2.grid(True, linestyle=GRID_LINESTYLE, alpha=GRID_ALPHA, color=COLOR_GRID)

RATIO_LABEL_OFFSETS = [5, 5, 5, 5, 5,5]
for i, bar in enumerate(bars):
    height = bar.get_height()
    ax2.annotate(f'{height:.2f}',  # 精度2位小数
                (bar.get_x() + bar.get_width() / 2, height),
                xytext=(0, RATIO_LABEL_OFFSETS[i]),
                textcoords='offset points',
                ha='center', va='bottom', color=COLOR_BAR_LABEL,
                fontsize=FONT_SIZE_ANNOT)

ax2.legend([bars], ['降低比例'], loc='upper left', fontsize=FONT_SIZE_LEGEND)

plt.tight_layout()
# plt.show()
plt.savefig("1.3.2.png", dpi=300, bbox_inches='tight')  # 保存图表为PNG文件