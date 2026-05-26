import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

# ===== 参数 =====
FONT_SIZE_LABEL = 22
FONT_SIZE_TICK = 20
FONT_SIZE_LEGEND = 18
FIG_WIDTH =8
FIG_HEIGHT = 1.8

# ===== 字体配置 =====
# plt.rcParams['font.family'] = 'serif'
# plt.rcParams['font.serif'] = ['Times New Roman', 'SimSun']
# plt.rcParams['mathtext.fontset'] = 'stix'
# plt.rcParams['axes.unicode_minus'] = False
# ===== FIXED FONT CONFIGURATION =====
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['SimSun','Times New Roman' ]  # Times + 宋体
plt.rcParams['mathtext.fontset'] = 'stix'  # Math symbols matching Times
plt.rcParams['axes.unicode_minus'] = False
plt.rcParams['axes.formatter.use_mathtext'] = False
# ===== 颜色 =====
COLOR_BASELINE_1 = '#1f77b4'
COLOR_BASELINE_8 = "#ff0e22"
COLOR_BASELINE_16 = '#2ca02c'
COLOR_OURS = '#ff7f0e'

# ===== 数据 =====
data = {
    'batch_size': [32, 64, 128, 256, 512],
    'workernum=1': [0.260131656, 0.566154257, 1.116352894, 2.182704826, 4.203628536],
    'workernum=8': [0.003676687, 0.009498583, 0.035223025, 0.074247143, 0.14355598],
    'workernum=16': [0.002159237, 0.00379106, 0.007155798, 0.014230262, 0.027837908],
    'Ours': [0.001709579, 0.002756197, 0.004531301, 0.010189609, 0.018498671]
}

df = pd.DataFrame(data)

batch_sizes = df['batch_size'].values
baseline_worker1 = df['workernum=1'].values
baseline_worker8 = df['workernum=8'].values
baseline_worker16 = df['workernum=16'].values
ours_times = df['Ours'].values

# ===== 1️⃣ 先画图（只是为了拿 legend 句柄）=====
fig, ax = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))

line1, = ax.plot(batch_sizes, baseline_worker1, '--o', color=COLOR_BASELINE_1,
                 linewidth=2, markersize=8, label='Baseline (Worker=1)')
line2, = ax.plot(batch_sizes, baseline_worker8, '-.D', color=COLOR_BASELINE_8,
                 linewidth=2, markersize=8, label='Baseline (Worker=8)')
line3, = ax.plot(batch_sizes, baseline_worker16, ':^', color=COLOR_BASELINE_16,
                 linewidth=2, markersize=8, label='Baseline (Worker=16)')
line4, = ax.plot(batch_sizes, ours_times, '-s', color=COLOR_OURS,
                 linewidth=2, markersize=8, label='OURS')

lines = [line1, line2, line3, line4]
labels = [l.get_label() for l in lines]

plt.close(fig)  # ❗ 关键：关闭主图（不显示不保存）

# ===== 2️⃣ 单独创建 legend 图 =====
fig_legend = plt.figure(figsize=(10, 1.5))
ax_legend = fig_legend.add_subplot(111)

ax_legend.axis('off')  # 去掉坐标轴

legends = ax_legend.legend(
    lines, labels,
    loc='center',
    ncol=4,
    fontsize=32,
    prop={'family': 'SimSun','size': 30, 'weight': 'bold'},
    handletextpad=0.1,      # 🔥 marker 和文字间距
    borderpad=2,          # 🔥 legend 内边距
    markerscale=3,
    columnspacing=1.2,  # 列之间的间距
    # handlelength=5.0,      # 🔥 线段拉长
    labelspacing=-5,
    # borderpad=1.0,
    frameon=False  # 不要边框（论文更干净）
)
# legend = ax_legend.legend(
#     lines, labels,
#     loc='center',
#     ncol=4,
#     prop={'size': (FONT_SIZE_LEGEND + 10) * 2, 'weight': 'bold'},  # 🔥 字体放大一倍
#     markerscale=2.0,        # 🔥 marker 放大一倍
#     handlelength=4.0,       # 🔥 线段变长（默认 ~2）
#     handleheight=2.0,       # 🔥 提高整体高度（可选）
#     handletextpad=1.0,      # 🔥 marker 和文字间距
#     borderpad=0.8,          # 🔥 legend 内边距
#     frameon=False
# )
plt.tight_layout(pad=0)

# ===== 3️⃣ 保存 =====
plt.savefig("legend.png", dpi=300, bbox_inches='tight')

print("legend 图已保存：legend_only.png")