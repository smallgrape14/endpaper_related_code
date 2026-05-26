import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# ===== 可调参数区 =====
FONT_SIZE_LABEL = 16         # 坐标轴标签字体大小
FONT_SIZE_TICK = 14          # 坐标轴刻度字体大小
FONT_SIZE_ANNOT = 14         # 数据标注字体大小
FONT_SIZE_LEGEND = 14        # 图例字体大小
FIG_WIDTH = 5.4                # 单个子图的图宽
FIG_HEIGHT = 4.4               # 单个子图的图高
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

# 创建数据
data = {
    '线程数': [1,2,4, 8, 16, 32, 64, 128, 256,512, 1024],
    # '客户端_IOPS': [1223.94, 7343.66, 16526.10, 26670.10, 27156.20, 14265.40, 6874.09, 1387.99],
    # 164BeeGFS(跨节点) 数据
    '客户端_IOPS': [992.9537231, 1738.289715, 3297.912713, 6377.981651, 12670.71689, 20939.75904, 23700, 13472.86822, 6529.743269, 3014.744146, 1238.774056],

    # '后端_SSD_IOPS': [11800, 131000, 238000, 350000, 350000, 359000, 371000, 361000],
    '后端_SSD_IOPS': [6509.36, 20772.9, 33747.6, 56064.5, 102235, 162938, 179793, 336387, 289667, 221872, 146873],
    'SSD_利用率': [992.9537231/10484*100, 1738.289715/30036*100, 3297.912713/68476*100, 6377.981651/132862*100, 12670.71689/234880*100, 20939.75904/372224*100, 23700/374784*100, 13472.86822/342528*100, 6529.743269/326400*100, 3014.744146/361472*100, 1238.774056/374528*100]
}

df = pd.DataFrame(data)

# 创建图表
fig, ax1 = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT), dpi=100)

# 绘制柱状图
x = np.arange(len(df['线程数']))
width = 0.35

bars1 = ax1.bar([i - width/2 for i in x], [i/1000 for i in df['客户端_IOPS']], width, 
                label='客户端 IOPS', color='#ff7f0e', alpha=0.8)
bars2 = ax1.bar([i + width/2 for i in x], [i/1000 for i in df['后端_SSD_IOPS']], width, 
                label='后端SSD_IOPS', color='#1f77b4', alpha=0.8)

# 设置左侧y轴
ax1.set_xlabel('线程数', fontsize=FONT_SIZE_LABEL)
ax1.set_ylabel('KIOPS (千次操作/秒)', fontsize=FONT_SIZE_LABEL)
# ax1.set_title('客户端IOPS与后端SSD_IOPS对比', fontsize=FONT_SIZE_LABEL+2, fontweight='bold')
ax1.set_xticks(x)
ax1.set_xticklabels(df['线程数'], fontsize=FONT_SIZE_TICK)
ax1.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
ax1.legend(loc='upper left', fontsize=FONT_SIZE_LEGEND, frameon=False)

# 创建第二个y轴用于显示SSD利用率
# ax2 = ax1.twinx()
# line, = ax2.plot(x, df['SSD_利用率'], color='#2ca02c', marker='o', linewidth=1.5, markersize=4, label='SSD利用率')
# ax2.set_ylabel('SSD利用率 (%)', fontsize=FONT_SIZE_LABEL, color='#2ca02c')
# ax2.tick_params(axis='y', labelcolor='#2ca02c', labelsize=FONT_SIZE_TICK)

# 添加组合图例
handles = [bars1[0], bars2[0]]
labels = ['客户端IOPS', '后端SSD_IOPS']
ax1.legend(handles, labels, loc='upper left', fontsize=FONT_SIZE_LEGEND, frameon=False)

# 在柱状图上添加数值标签
for i, bar in enumerate(bars1):
    height = bar.get_height()
    ax1.annotate(f'{height:.0f}K',
                xy=(bar.get_x() + bar.get_width() / 2, height),
                xytext=(0, 3),  # 3 points vertical offset
                textcoords="offset points",
                ha='center', va='bottom', fontsize=FONT_SIZE_ANNOT-2,color='#ff7f0e')

for i, bar in enumerate(bars2):
    height = bar.get_height()
    ax1.annotate(f'{height:.0f}K',
                xy=(bar.get_x() + bar.get_width() / 2, height),
                xytext=(0, 1.5),
                textcoords="offset points",
                ha='center', va='bottom', fontsize=FONT_SIZE_ANNOT-2,color='#1f77b4')

# 保存图表
performance_comparison_chart = '动机一BeeGFSvsSSD_跨节点.png'
plt.tight_layout()
plt.savefig(performance_comparison_chart, dpi=300, bbox_inches='tight', facecolor='white')
print(f"图表已保存为: {performance_comparison_chart}")
plt.show()