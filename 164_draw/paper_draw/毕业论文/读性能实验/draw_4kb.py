import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# # ===== 可调参数区 =====
# FONT_SIZE_LABEL = 10         # 坐标轴标签字体大小
# FONT_SIZE_TICK = 10          # 坐标轴刻度字体大小
# FONT_SIZE_ANNOT = 10         # 数据标注字体大小
# FONT_SIZE_LEGEND = 10        # 图例字体大小
# FIG_WIDTH = 4                # 单个子图的图宽
# FIG_HEIGHT = 3               # 单个子图的图高
# GRID_ALPHA = 0.5             # 网格透明度
# GRID_LINESTYLE = '--'        # 网格线型
# ===== 可调参数区 =====
FONT_SIZE_LABEL = 20         # 坐标轴标签字体大小
FONT_SIZE_TICK = 20          # 坐标轴刻度字体大小
FONT_SIZE_ANNOT = 18         # 数据标注字体大小
FONT_SIZE_LEGEND = 18        # 图例字体大小
FIG_WIDTH = 5.8              # 单个子图的图宽
FIG_HEIGHT = 4.8             # 单个子图的图高
GRID_ALPHA = 0.5             # 网格透明度
GRID_LINESTYLE = '--'        # 网格线型
# # 设置中文字体支持
# # Windows系统可以使用SimHei
# plt.rcParams['font.sans-serif'] = ['SimHei']  # 用来正常显示中文标签
# # Mac系统可以使用PingFang SC
# # plt.rcParams['font.sans-serif'] = ['PingFang SC']  # Mac系统
# # 或者使用Arial Unicode MS
# # plt.rcParams['font.sans-serif'] = ['Arial Unicode MS']  # 跨平台

# # 解决负号显示问题
# plt.rcParams['axes.unicode_minus'] = False
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

# 创建数据
posix_data = {
    'Thread_Num': [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024],
    'IOPS_avg': [1289.552203, 1845.445856, 3444.742407, 6744.420234, 13965.56019, 23567.59298, 25670.69238, 14109.89154, 6918.560292, 3393.931346, 1461.071236]
}

our_data = {
    'ThreadNum': [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024],
    'IOPS_avg': [10835.97444, 13212.20875, 14003.04823, 27542.85412, 32722.27487, 36399.5574, 39749.94282, 42124.31277, 43348.13189, 44244.74625, 46256.41526]
}

gds_data = {
    'THREADS': [1, 2, 4, 8, 16, 32, 64, 128, 256],
    'IOPS_avg': [3196.0909807032267, 5874.945598464766, 10453.479039402859, 16421.01241290704, 28267.124585133082, 30971.41873609555, 23938.818801653833, 12571.224031401904, 5896.61798092979]
}

# 转换为DataFrame
df_posix = pd.DataFrame(posix_data)
df_our = pd.DataFrame(our_data)
df_gds = pd.DataFrame(gds_data)

# 创建图表
plt.figure(figsize=(FIG_WIDTH, FIG_HEIGHT), dpi=100)

# 定义颜色
colors = {
    'POSIX': '#1f77b4',  # 蓝色
    'OUR': '#ff7f0e',    # 橙色
    'GDS': '#2ca02c'     # 绿色
}

# 线型
linestyles = {
    'POSIX': '--',
    'OUR': '-',
    'GDS': ':'
}

# 标记
markers = {
    'POSIX': 'o',
    'OUR': 's',
    'GDS': '^'
}
# 绘制OUR折线图
threads_our = df_our['ThreadNum']
plt.plot(threads_our, df_our['IOPS_avg']/1000, 
         marker=markers['OUR'], color=colors['OUR'], 
         linewidth=1.5, markersize=6, label='OUR', 
         linestyle=linestyles['OUR'])





# 绘制GDS折线图
threads_gds = df_gds['THREADS']
plt.plot(threads_gds, df_gds['IOPS_avg']/1000, 
         marker=markers['GDS'], color=colors['GDS'], 
         linewidth=1.5, markersize=6, label='GDS', 
         linestyle=linestyles['GDS'])
# 绘制POSIX折线图
threads_posix = df_posix['Thread_Num']
plt.plot(threads_posix, df_posix['IOPS_avg']/1000, 
         marker=markers['POSIX'], color=colors['POSIX'], 
         linewidth=1.5, markersize=6, label='POSIX', 
         linestyle=linestyles['POSIX'])


# 设置图表属性
plt.xlabel('线程数量', fontsize=FONT_SIZE_LABEL)
plt.ylabel('KIOPS (千次/秒)', fontsize=FONT_SIZE_LABEL)  # 将IOPS单位转换为千
plt.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)

# 设置x轴为对数刻度，并设置刻度标签
plt.xscale('log')
thread_ticks = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024]
plt.xticks(thread_ticks, [str(t) for t in thread_ticks], rotation=45, fontsize=FONT_SIZE_TICK)

# 设置y轴刻度
max_iops = max(df_posix['IOPS_avg'].max(), df_our['IOPS_avg'].max(), df_gds['IOPS_avg'].max())
y_ticks = np.arange(0, max_iops/1000 + 5, 10)
plt.yticks(y_ticks, fontsize=FONT_SIZE_TICK)
plt.ylim(0, max_iops/1000 * 1.1)  # 留10%的顶部空间

# 添加图例
plt.legend(loc='best', fontsize=FONT_SIZE_LEGEND, frameon=False)

# 调整布局
plt.tight_layout()

# 保存图表
plt.savefig('iops_4kb_comparison_cn.png', dpi=300, bbox_inches='tight', facecolor='white')
plt.show()

print("图表已保存为 'iops_4kb_comparison_cn.png'")