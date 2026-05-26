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

# CSV文件路径
csv_file_path = 'csv/write_iops_4KB.csv'  # 请确保文件在当前目录

# 读取CSV文件
df = pd.read_csv(csv_file_path)

# 显示数据信息
print("读取的数据:")
print(df)
print("\n数据类型:")
print(df.dtypes)
print("\n列名:", df.columns.tolist())

# 处理缺失值（将空值转换为NaN）
df['GDS'] = pd.to_numeric(df['GDS'], errors='coerce')

# 创建数据字典
posix_data = {
    'Thread_Num': df['ThreadNum'].tolist(),
    'IOPS_avg': df['POSIX'].tolist()
}

our_data = {
    'ThreadNum': df['ThreadNum'].tolist(),
    'IOPS_avg': df['OUR'].tolist()
}

# GDS数据可能有缺失，只提取非空值
gds_mask = df['GDS'].notna()
gds_data = {
    'THREADS': df.loc[gds_mask, 'ThreadNum'].tolist(),
    'IOPS_avg': df.loc[gds_mask, 'GDS'].tolist()
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
         linewidth=1.5, markersize=4, label='OUR', 
         linestyle=linestyles['OUR'])

# 绘制GDS折线图
threads_gds = df_gds['THREADS']
plt.plot(threads_gds, df_gds['IOPS_avg']/1000, 
         marker=markers['GDS'], color=colors['GDS'], 
         linewidth=1.5, markersize=4, label='GDS', 
         linestyle=linestyles['GDS'])

# 绘制POSIX折线图
threads_posix = df_posix['Thread_Num']
plt.plot(threads_posix, df_posix['IOPS_avg']/1000, 
         marker=markers['POSIX'], color=colors['POSIX'], 
         linewidth=1.5, markersize=4, label='POSIX', 
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
y_ticks = np.arange(0, max_iops/1000 + 5, 5)
plt.yticks(y_ticks, fontsize=FONT_SIZE_TICK)
plt.ylim(0, max_iops/1000 * 1.1)  # 留10%的顶部空间

# 添加图例
plt.legend(loc='best', fontsize=FONT_SIZE_LEGEND, frameon=False)

# 调整布局
plt.tight_layout()

# 保存图表
plt.savefig('Write_iops_4kb_comparison_cn.png', dpi=300, bbox_inches='tight', facecolor='white')
plt.show()

print("图表已保存为 'Write_iops_4kb_comparison_cn.png'")
print(f"从CSV文件读取了{len(df)}行数据")
print(f"OUR数据点: {len(df_our)}个")
print(f"GDS数据点: {len(df_gds)}个 (有缺失值)")
print(f"POSIX数据点: {len(df_posix)}个")