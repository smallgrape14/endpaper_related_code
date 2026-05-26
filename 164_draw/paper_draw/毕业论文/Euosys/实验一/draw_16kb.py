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
# FIG_WIDTH = 6.6
# FIG_HEIGHT = 4.8
# GRID_ALPHA = 0.3
# GRID_LINESTYLE = "--"
# FONT_SIZE_LABEL = 14
# FONT_SIZE_TICK = 12
# FONT_SIZE_LEGEND = 14
# FONT_SIZE_TITLE = 14
FIG_WIDTH = 8.8
FIG_HEIGHT = 6.8
GRID_ALPHA = 0.6
GRID_LINESTYLE = "--"
FONT_SIZE_LABEL = 30
FONT_SIZE_TICK = 26
FONT_SIZE_LEGEND = 28
FONT_SIZE_TITLE = 28
# # 设置中文字体支持
# # Windows系统可以使用SimHei
# plt.rcParams['font.sans-serif'] = ['SimHei']  # 用来正常显示中文标签
# # Mac系统可以使用PingFang SC
# # plt.rcParams['font.sans-serif'] = ['PingFang SC']  # Mac系统
# # 或者使用Arial Unicode MS
# # plt.rcParams['font.sans-serif'] = ['Arial Unicode MS']  # 跨平台

# # 解决负号显示问题
# plt.rcParams['axes.unicode_minus'] = False
# # ===== 可调参数区 =====
# FONT_SIZE_LABEL = 20         # 坐标轴标签字体大小
# FONT_SIZE_TICK = 20          # 坐标轴刻度字体大小
# FONT_SIZE_ANNOT = 18         # 数据标注字体大小
# FONT_SIZE_LEGEND = 18        # 图例字体大小
# FIG_WIDTH = 5.8              # 单个子图的图宽
# FIG_HEIGHT = 4.8             # 单个子图的图高
# GRID_ALPHA = 0.5             # 网格透明度
# GRID_LINESTYLE = '--'        # 网格线型
# ===== FIXED FONT CONFIGURATION =====
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['DejaVu Sans']  # Times + 宋体
plt.rcParams['mathtext.fontset'] = 'dejavusans'  # Math symbols matching Times
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

# 创建数据 - 16KB块大小
posix_data = {
    'Thread_Num': [1, 2, 4, 8, 16, 32, 64, 128, 256],#, 512, 1024],
    'IOPS_avg': [1132.33325, 1798.147122, 3494.176707, 6749.781059, 13772.1182, 22645.68878, 24512.01625, 14072.30476, 6838.714416]#, 3290.627958, 1399.572988]
}

our_data = {
    'ThreadNum': [1, 2, 4, 8, 16, 32, 64, 128, 256],# 512, 1024],
    'IOPS_avg': [12123.62188, 12762.1141, 13367.29423, 25669.04945, 31296.70646, 34931.63075, 39299.03901, 41887.76105, 43077.55861]#, 44879.02289, 46819.22875]
}

gds_data = {
    'THREADS': [1, 2, 4, 8, 16, 32, 64, 128, 256],
    'IOPS_avg': [2018.5308149087118, 3643.6178675651963, 6968.729701089921, 13519.15132296192, 23049.710195790067, 25563.162458823314, 19472.63023153433, 10264.129774263165, 4340.732683188605]
}

# 转换为DataFrame
df_posix = pd.DataFrame(posix_data)
df_our = pd.DataFrame(our_data)
df_gds = pd.DataFrame(gds_data)

# 创建图表
plt.figure(figsize=(FIG_WIDTH, FIG_HEIGHT), dpi=100)

# 定义颜色
colors = {
    'POSIX':"#1D4F91", # '#1f77b4',  # 蓝色
    'OUR':"#F28B82",# '#ff7f0e',    # 橙色
    'GDS': "#B388EB" #'#2ca02c'     # 绿色
}

# 线型
linestyles = {
    'POSIX': '--',
    'OUR': '-',
    'GDS': '-'
}

# 标记
markers = {
    'POSIX': 'o',
    'OUR': 'P',
    'GDS': 'X'
}
# 绘制OUR折线图
threads_our = df_our['ThreadNum']
plt.plot(threads_our, df_our['IOPS_avg']/1000, 
         marker=markers['OUR'], color=colors['OUR'], 
         linewidth=2.5, markersize=12, label='OURS(GPU)', 
         linestyle=linestyles['OUR'])


# 绘制GDS折线图
threads_gds = df_gds['THREADS']
plt.plot(threads_gds, df_gds['IOPS_avg']/1000, 
         marker=markers['GDS'], color=colors['GDS'], 
         linewidth=2.5, markersize=12, label='GDS', 
         linestyle=linestyles['GDS'])

# 绘制POSIX折线图
threads_posix = df_posix['Thread_Num']
plt.plot(threads_posix, df_posix['IOPS_avg']/1000, 
         marker=markers['POSIX'], color=colors['POSIX'], 
         linewidth=2.5, markersize=12, label='POSIX(CPU)', 
         linestyle=linestyles['POSIX'],
         fillstyle='none')





# 设置图表属性 - 使用中文标签
plt.xlabel('Host Concurrent Threads', fontsize=FONT_SIZE_LABEL)
plt.ylabel('Throughput (KIOPS)', fontsize=FONT_SIZE_LABEL)  # 将IOPS单位转换为千
plt.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)

# 设置x轴为对数刻度，并设置刻度标签
plt.xscale('log')
thread_ticks = [1, 2, 4, 8, 16, 32, 64, 128, 256]#, 512, 1024]
plt.xticks(thread_ticks, [str(t) for t in thread_ticks], fontsize=FONT_SIZE_TICK)
# 👇 在这里添加这一行，关闭次要刻度线
plt.minorticks_off()
# 设置y轴刻度
max_iops = max(df_posix['IOPS_avg'].max(), df_our['IOPS_avg'].max(), df_gds['IOPS_avg'].max())
# 自动计算合适的y轴刻度间隔
y_interval = 5 if max_iops/1000 < 25 else 10
y_max = ((max_iops/1000) // y_interval + 1) * y_interval
y_ticks = np.arange(0, y_max + y_interval, y_interval)
plt.yticks(y_ticks, fontsize=FONT_SIZE_TICK)
plt.ylim(0, y_max)  # 设置y轴上限

# 添加图例
# plt.legend(loc='best', fontsize=FONT_SIZE_LEGEND, frameon=False)

# 添加中文标题
# plt.title('16KB块大小 - IOPS性能对比', fontsize=FONT_SIZE_LABEL+2, fontweight='bold')

# 调整布局
plt.tight_layout()

# 保存图表
output_filename = 'Read_iops_16kb_comparison.png'
# plt.savefig(output_filename, dpi=300, bbox_inches='tight', facecolor='white')
plt.savefig('Read_iops_16kb_comparison.png', dpi=300, bbox_inches='tight', facecolor='white')
plt.savefig('Read_iops_16kb_comparison.pdf', dpi=300, bbox_inches='tight', facecolor='white')
plt.show()

print(f"图表已保存为 '{output_filename}'")