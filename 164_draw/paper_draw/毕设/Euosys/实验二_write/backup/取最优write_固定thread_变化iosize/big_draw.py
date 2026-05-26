import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# ===== 可调参数区 =====
FONT_SIZE_LABEL = 10         # 坐标轴标签字体大小
FONT_SIZE_TICK = 10          # 坐标轴刻度字体大小
FONT_SIZE_ANNOT = 10         # 数据标注字体大小
FONT_SIZE_LEGEND = 10        # 图例字体大小
FIG_WIDTH = 4                # 单个子图的图宽
FIG_HEIGHT = 3               # 单个子图的图高
GRID_ALPHA = 0.5             # 网格透明度
GRID_LINESTYLE = '--'        # 网格线型

# 设置中文字体支持
plt.rcParams['font.sans-serif'] = ['SimHei']
plt.rcParams['axes.unicode_minus'] = False

# 设置全局字体
plt.rcParams.update({
    'font.size': FONT_SIZE_TICK,
    'axes.labelsize': FONT_SIZE_LABEL,
    'legend.fontsize': FONT_SIZE_LEGEND,
    'xtick.labelsize': FONT_SIZE_TICK,
    'ytick.labelsize': FONT_SIZE_TICK
})

# IO大小数据（单位：KB）
io_sizes = [0.5, 1, 2, 4, 8, 16]

# OUR方案的数据
our_data_point = [6.54830685, 6.655389088, 6.833962859, 6.492783275, 6.793019559]

# GDS方案的数据
gds_data_point = [3.773309333, 3.702722333, 3.532633667, 4.103557, 3.33619]

# POSIX方案的数据
posix_data_point = [1.406074219, 1.068261719, 0.527998047, 0.709824219, 1.002783203]

# 创建数据
gds_data = {
    'IOSize_KB': [0.5, 1, 2, 4, 8],
    'IOPS':gds_data_point #[31214.80729, 32177.483939840004, 31645.54134, 30971.41182, 27896.95816, 25563.1609]
}

our_data = {
    'IOSize_KB': [0.5, 1, 2, 4, 8],
    'IOPS': our_data_point #[54489.02173, 51912.62271, 58182.54858, 50076.35347, 54669.27396, 53140.36742]
}

posix_data = {
    'IOSize_KB': [0.5, 1, 2, 4, 8],
    'IOPS': posix_data_point #[26106.67989, 26438.95563, 24204.7235, 25670.69238, 23798.00451, 24512.01625]
}

# 转换为DataFrame
df_gds = pd.DataFrame(gds_data)
df_our = pd.DataFrame(our_data)
df_posix = pd.DataFrame(posix_data)

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
io_sizes_our = df_our['IOSize_KB']
plt.plot(io_sizes_our, df_our['IOPS'], 
         marker=markers['OUR'], color=colors['OUR'], 
         linewidth=1.5, markersize=4, label='OUR', 
         linestyle=linestyles['OUR'])

# 绘制GDS折线图
io_sizes_gds = df_gds['IOSize_KB']
plt.plot(io_sizes_gds, df_gds['IOPS'], 
         marker=markers['GDS'], color=colors['GDS'], 
         linewidth=1.5, markersize=4, label='GDS', 
         linestyle=linestyles['GDS'])



# 绘制POSIX折线图
io_sizes_posix = df_posix['IOSize_KB']
plt.plot(io_sizes_posix, df_posix['IOPS'], 
         marker=markers['POSIX'], color=colors['POSIX'], 
         linewidth=1.5, markersize=4, label='POSIX', 
         linestyle=linestyles['POSIX'])

# 设置图表属性
plt.xlabel('I/O大小 (MB)', fontsize=FONT_SIZE_LABEL)
plt.ylabel('吞吐量（GB/s）', fontsize=FONT_SIZE_LABEL)
plt.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)

# 设置x轴为对数刻度
plt.xscale('log')
io_size_ticks = [0.5, 1, 2, 4, 8]
plt.xticks(io_size_ticks, [str(size) if size >= 1 else str(int(size*2))+'/2' for size in io_size_ticks], 
           fontsize=FONT_SIZE_TICK)

# 设置y轴刻度
all_iops = list(df_gds['IOPS']) + list(df_our['IOPS']) + list(df_posix['IOPS'])
max_iops = max(all_iops)
y_interval = 1
y_max = ((max_iops) // y_interval + 1) * y_interval
y_ticks = np.arange(0, y_max + y_interval, y_interval)
plt.yticks(y_ticks, fontsize=FONT_SIZE_TICK)
plt.ylim(0, y_max)

# 添加图例
plt.legend(loc='upper right', fontsize=FONT_SIZE_LEGEND,bbox_to_anchor=(1,0.9), frameon=False)

# 添加中文标题
# plt.title('不同IO大小下的IOPS性能对比', fontsize=FONT_SIZE_LABEL+2, fontweight='bold')

# 调整布局
plt.tight_layout()

# 保存图表
output_filename = 'big_write_peek_iops_io_size_comparison.png'
plt.savefig(output_filename, dpi=300, bbox_inches='tight', facecolor='white')
plt.show()

print(f"图表已保存为 '{output_filename}'")