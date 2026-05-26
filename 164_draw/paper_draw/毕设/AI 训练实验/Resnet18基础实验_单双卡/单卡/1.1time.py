import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import ScalarFormatter
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
baseline_colors=['#1f77b4', '#ff0e22', '#2ca02c']  # 蓝色、红色、绿色
ours_color='#ff7f0e'  # 橙色
# ===== 高对比度颜色区 =====
COLOR_BASELINE_1 = '#1f77b4'  # 蓝色
COLOR_BASELINE_8 = "#ff0e22"  # 橙色
COLOR_BASELINE_16 = '#2ca02c'  # 绿色
COLOR_OURS ='#ff7f0e'# '#d62728'  # 红色
COLOR_GRID = '#CCCCCC'
COLOR_YLABEL = 'black'
COLOR_Y2SPINE = 'black'

# ===== 数据区 =====
# 从您的数据构建DataFrame
data = {
    # 'batch_size': [16, 32, 64, 128, 256, 512],
    # 'workernum=1': [0.1301434, 0.260131656, 0.566154257, 1.116352894, 2.182704826, 4.203628536],
    # 'workernum=8': [0.00165695, 0.003676687, 0.009498583, 0.035223025, 0.074247143, 0.14355598],
    # 'workernum=16': [0.00168587, 0.002159237, 0.00379106, 0.007155798, 0.014230262, 0.027837908],
    # 'Ours': [0.002664236, 0.001709579, 0.002756197, 0.004531301, 0.010189609, 0.018498671]
    'batch_size': [ 32, 64, 128, 256, 512],
    'workernum=1': [ 0.260131656, 0.566154257, 1.116352894, 2.182704826, 4.203628536],
    'workernum=8': [0.003676687, 0.009498583, 0.035223025, 0.074247143, 0.14355598],
    'workernum=16': [ 0.002159237, 0.00379106, 0.007155798, 0.014230262, 0.027837908],
    'Ours': [0.001709579, 0.002756197, 0.004531301, 0.010189609, 0.018498671]
}

df = pd.DataFrame(data)

# 数据提取
batch_sizes = df['batch_size'].values
baseline_worker1 = df['workernum=1'].values
baseline_worker8 = df['workernum=8'].values
baseline_worker16 = df['workernum=16'].values
ours_times = df['Ours'].values

# ====== 折线图：不同配置的数据加载时间对比 ======
BASELINE_1_OFFSETS = [10,10, 10, 10, 5, -10]
BASELINE_8_OFFSETS = [8, 8, 8, 8, 8, 8]
BASELINE_16_OFFSETS = [4, 4, 4, 4, 4, 4]
OURS_OFFSETS = [-14, -14, -14, -14, -14, -14]

fig1, ax1 = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))

# 绘制四条折线
line1, = ax1.plot(batch_sizes, baseline_worker1, '--o', color=COLOR_BASELINE_1,
                  linewidth=2, markersize=8, label='Baseline (workernum=1)')
line2, = ax1.plot(batch_sizes, baseline_worker8, '-.D', color=COLOR_BASELINE_8,
                  linewidth=2, markersize=8, label='Baseline (workernum=8)')
line3, = ax1.plot(batch_sizes, baseline_worker16, ':^', color=COLOR_BASELINE_16,
                  linewidth=2, markersize=8, label='Baseline (workernum=16)')
line4, = ax1.plot(batch_sizes, ours_times, '-s', color=COLOR_OURS,
                  linewidth=2, markersize=8, label='OURS')

# 设置图表属性
# 如果坐标轴标签需要显示中文，单独设置字体
# ax1.set_xlabel('批次大小 (Batch Size)', fontsize=FONT_SIZE_LABEL, color=COLOR_YLABEL, **chinese_font)
# ax1.set_ylabel('单个批次（Batch）数据加载时间 (秒)', fontsize=FONT_SIZE_LABEL, color=COLOR_YLABEL, **chinese_font)
ax1.set_xlabel('批次大小 (Batch Size)', fontsize=FONT_SIZE_LABEL, color=COLOR_YLABEL)
ax1.set_ylabel('单个批次的数据加载时间 (秒)', fontsize=FONT_SIZE_LABEL, color=COLOR_YLABEL)
ax1.set_yscale('log')
ylist=[]
for i in range(-3,2):
    ylist.append(10**i)
ax1.set_yticks(ylist)
print(ylist)
ax1.set_yticklabels(ylist,fontsize=FONT_SIZE_TICK)
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

# 为每条线添加数据标签
# for i, (x, y) in enumerate(zip(batch_sizes, baseline_worker1)):
#     ax1.annotate(f'{y:.6f}', (x, y),
#                 xytext=(0, BASELINE_1_OFFSETS[i]),
#                 textcoords='offset points',
#                 ha='center', va='bottom', color=COLOR_BASELINE_1,
#                 fontsize=FONT_SIZE_ANNOT, rotation=0)

# for i, (x, y) in enumerate(zip(batch_sizes, baseline_worker8)):
#     ax1.annotate(f'{y:.6f}', (x, y),
#                 xytext=(0, BASELINE_8_OFFSETS[i]),
#                 textcoords='offset points',
#                 ha='center', va='bottom', color=COLOR_BASELINE_8,
#                 fontsize=FONT_SIZE_ANNOT, rotation=0)

# for i, (x, y) in enumerate(zip(batch_sizes, baseline_worker16)):
#     ax1.annotate(f'{y:.6f}', (x, y),
#                 xytext=(0, BASELINE_16_OFFSETS[i]),
#                 textcoords='offset points',
#                 ha='center', va='bottom', color=COLOR_BASELINE_16,
#                 fontsize=FONT_SIZE_ANNOT, rotation=0)

# for i, (x, y) in enumerate(zip(batch_sizes, ours_times)):
#     ax1.annotate(f'{y:.6f}', (x, y),
#                 xytext=(0, OURS_OFFSETS[i]),
#                 textcoords='offset points',
#                 ha='center', va='bottom', color=COLOR_OURS,
#                 fontsize=FONT_SIZE_ANNOT, rotation=0)

# 添加图例
lines = [line1, line2, line3, line4]
labels = [l.get_label() for l in lines]
# ax1.legend(lines, labels, loc='upper left',bbox_to_anchor=(0, 1.2),  # (x, y) 坐标，y>1 表示在图外上方
#            ncol=4,
#            fontsize=FONT_SIZE_LEGEND)

plt.tight_layout()
plt.savefig("data_loading_comparison.png", dpi=300, bbox_inches='tight')
print("图表已保存为 'data_loading_comparison.png'")