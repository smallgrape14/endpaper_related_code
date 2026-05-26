import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

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
plt.rcParams['font.sans-serif'] = ['SimHei']  # 用来正常显示中文标签
plt.rcParams['axes.unicode_minus'] = False  # 用来正常显示负号

# 设置全局字体
plt.rcParams.update({
    'font.size': FONT_SIZE_TICK,
    'axes.labelsize': FONT_SIZE_LABEL,
    'legend.fontsize': FONT_SIZE_LEGEND,
    'xtick.labelsize': FONT_SIZE_TICK,
    'ytick.labelsize': FONT_SIZE_TICK
})

# 创建新数据
data = {
    '线程数': [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024],
    'EXT4_IOPS': [6509.36, 20772.9, 33747.6, 56064.5, 102235, 162938, 179793, 336387, 289667, 221872, 146873],
    'BeeGFS_IOPS': [1361.54, 2461.18, 4277.28, 7840.6, 12905.9, 24887.8, 23972.4, 10554.7, 5555.67, 2788.24, 1086.59],
    'BeeGFS/EXT4': [1361.54/6509.36 * 100, 2461.18/20772.9 * 100, 4277.28/33747.6 * 100, 7840.6/56064.5 * 100, 12905.9/102235 * 100, 24887.8/162938 * 100, 23972.4/179793 * 100, 10554.7/336387 * 100, 5555.67/289667 * 100, 2788.24/221872 * 100, 1086.59/146873 * 100]
}

df = pd.DataFrame(data)

# 创建图表
fig, ax1 = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT), dpi=100)

# 绘制折线图
x = np.arange(len(df['线程数']))

# 绘制BeeGFS折线
line1, = ax1.plot(x, df['BeeGFS_IOPS']/1000, 
                  marker='o', linewidth=1.5, markersize=4, 
                  label='BeeGFS_IOPS', color='#ff7f0e')

# 绘制EXT4折线
line2, = ax1.plot(x, df['EXT4_IOPS']/1000, 
                  marker='s', linewidth=1.5, markersize=4,linestyle='--', 
                  label='EXT4_IOPS', color='#2ca02c')

# 设置左侧y轴
ax1.set_xlabel('线程数', fontsize=FONT_SIZE_LABEL)
ax1.set_ylabel('KIOPS (千次操作/秒)', fontsize=FONT_SIZE_LABEL)
ax1.set_xticks(x)
ax1.set_xticklabels(df['线程数'], fontsize=FONT_SIZE_TICK)
ax1.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)

# # 创建第二个y轴用于显示BeeGFS/EXT4百分比
# ax2 = ax1.twinx()
# line3, = ax2.plot(x, df['BeeGFS/EXT4'], 
#                   marker='^', linewidth=1.5, markersize=4, 
#                   label='BeeGFS/EXT4', color='#2ca02c', linestyle=':')
# ax2.set_ylabel('BeeGFS/EXT4 (%)', fontsize=FONT_SIZE_LABEL, color='#2ca02c')
# ax2.tick_params(axis='y', labelcolor='#2ca02c', labelsize=FONT_SIZE_TICK)

# 添加组合图例
handles = [line1, line2]
labels = ['BeeGFS_IOPS', 'EXT4_IOPS']
ax1.legend(handles, labels, loc='upper left', fontsize=FONT_SIZE_LEGEND, frameon=False)

# 在数据点上添加数值标签
for i, (x_pos, y1, y2) in enumerate(zip(x, df['BeeGFS_IOPS']/1000, df['EXT4_IOPS']/1000)):
    # 只在关键点添加标签，避免图表过于拥挤
    if i % 2 == 0 or i == len(x) - 1:  # 每隔一个点或最后一个点添加标签
        ax1.annotate(f'{y1:.0f}K',
                    xy=(x_pos, y1),
                    xytext=(0, 5),  # 5 points vertical offset
                    textcoords="offset points",
                    ha='center', va='bottom', fontsize=FONT_SIZE_ANNOT-2, color='#ff7f0e')
        ax1.annotate(f'{y2:.0f}K',
                    xy=(x_pos, y2),
                    xytext=(0, 5),  # 5 points vertical offset
                    textcoords="offset points",
                    ha='center', va='bottom', fontsize=FONT_SIZE_ANNOT-2, color='#2ca02c')

# 保存图表
performance_comparison_chart = '动机一BeeGFSvsEXT4_本地节点_折线图.png'
plt.tight_layout()
plt.savefig(performance_comparison_chart, dpi=300, bbox_inches='tight', facecolor='white')
print(f"图表已保存为: {performance_comparison_chart}")
plt.show()

# 打印关键性能数据
print("\n关键性能数据:")
print("="*50)
print(f"BeeGFS最大IOPS: {df['BeeGFS_IOPS'].max():.0f} (在{df['线程数'][df['BeeGFS_IOPS'].idxmax()]}线程时)")
print(f"EXT4最大IOPS: {df['EXT4_IOPS'].max():.0f} (在{df['线程数'][df['EXT4_IOPS'].idxmax()]}线程时)")
print(f"EXT4相对于BeeGFS的最大性能优势: {df['BeeGFS/EXT4'].min():.1f}% (在{df['线程数'][df['BeeGFS/EXT4'].idxmin()]}线程时)")
print(f"EXT4相对于BeeGFS的平均性能优势: {(100 - df['BeeGFS/EXT4'].mean()):.1f}%")