import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# 设置中文字体和图形参数
plt.rcParams['font.sans-serif'] = ['Noto Sans CJK JP', 'DejaVu Sans', 'Arial']
plt.rcParams['axes.unicode_minus'] = False
plt.rcParams['font.size'] = 12
plt.rcParams['figure.figsize'] = [14, 10]

# 创建数据框（完整数据）
data = {
    'Threads': [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024],
    'IOPS(BeeGFS)Cufile-1KB': [1385.23, 2310.66, 4035.6, 5266.67, 5482.65, 5152.17, 5071.98, 5001.44, 4942.18, 4693.07, 4597.88],
    'IOPS(BeeGFS)Cufile-4KB': [1299.12, 2334.45, 4365.01, 6225.67, 5227.07, 5059.68, 4968.08, 4932.83, 4638.79, 4796.69, 4473.62],
    'IOPS(BeeGFS)Cufile-16KB': [1194.09, 2068.23, 4174.54, 5491.31, 5366.96, 5114.27, 4987.09, 4893.48, 4868.35, 4832.25, 4387.04],
   
    'IOPS(localNVMeSSD)-512B': [18294.74, 14463.25, 13904, 14383.45, 13018.73, 9091.54, 7545.59, 7162.09, 7261.84, 6448.98, 4428.03],
    'IOPS(localNVMeSSD)-1KB': [4553.71, 15047.62, 17555.56, 15113.04, 9277.58, 8596.87, 7984.69, 7606.13, 8071.21, 7443.25, 7098.71],
    'IOPS(localNVMeSSD)-2KB': [15610.78, 14897.14, 15752.27, 14982.76, 11863.48, 8027.71, 7846.5, 7307.64, 7036.44, 7437.95, 6533.83],
    'IOPS(localNVMeSSD)-4KB': [4914.23, 8033.9, 14284.93, 12326.24, 8058.73, 6350.79, 6723.4, 6933.51, 6591.66, 8108.86, 7395.74],
    'IOPS(localNVMeSSD)-8KB': [3978.63, 6024.26, 15357.88, 15896.34, 9505.93, 6629.37, 6608.37, 6421.18, 6370.19, 5891.53, 3882.35],
    'IOPS(localNVMeSSD)-16KB': [1692.03, 3492.3, 6676.06, 10828.66, 6112.54, 6055.75, 6066.32, 6000, 5780.49, 5799.78, 6112.54]

}

df = pd.DataFrame(data)

# 创建图形和坐标轴
fig, ax = plt.subplots(figsize=(16, 10))

# 定义颜色和标记样式
beeGFS_colors = ['#1f77b4', '#ff7f0e', '#2ca02c']  # 蓝色，橙色，绿色
nvmeSSD_colors = ['#d62728', '#9467bd', '#8c564b', '#e377c2', '#7f7f7f', '#bcbd22']  # 红色，紫色，棕色等

# 绘制BeeGFS Cufile系列数据（实线）
ax.plot(df['Threads'], df['IOPS(BeeGFS)Cufile-1KB'], marker='o', label='BeeGFS Cufile-1KB', 
         linewidth=2.5, color=beeGFS_colors[0], markersize=6)
ax.plot(df['Threads'], df['IOPS(BeeGFS)Cufile-4KB'], marker='s', label='BeeGFS Cufile-4KB', 
         linewidth=2.5, color=beeGFS_colors[1], markersize=6)
ax.plot(df['Threads'], df['IOPS(BeeGFS)Cufile-16KB'], marker='^', label='BeeGFS Cufile-16KB', 
         linewidth=2.5, color=beeGFS_colors[2], markersize=6)

# 绘制localNVMeSSD系列数据（虚线）[6,8](@ref)
ax.plot(df['Threads'], df['IOPS(localNVMeSSD)-512B'], marker='o', label='localNVMeSSD-512B', 
         linewidth=2.5, linestyle='--', color=nvmeSSD_colors[0], markersize=6)
ax.plot(df['Threads'], df['IOPS(localNVMeSSD)-1KB'], marker='s', label='localNVMeSSD-1KB', 
         linewidth=2.5, linestyle='--', color=nvmeSSD_colors[1], markersize=6)
ax.plot(df['Threads'], df['IOPS(localNVMeSSD)-2KB'], marker='^', label='localNVMeSSD-2KB', 
         linewidth=2.5, linestyle='--', color=nvmeSSD_colors[2], markersize=6)
ax.plot(df['Threads'], df['IOPS(localNVMeSSD)-4KB'], marker='d', label='localNVMeSSD-4KB', 
         linewidth=2.5, linestyle='--', color=nvmeSSD_colors[3], markersize=6)
ax.plot(df['Threads'], df['IOPS(localNVMeSSD)-8KB'], marker='*', label='localNVMeSSD-8KB', 
         linewidth=2.5, linestyle='--', color=nvmeSSD_colors[4], markersize=8)
ax.plot(df['Threads'], df['IOPS(localNVMeSSD)-16KB'], marker='x', label='localNVMeSSD-16KB', 
         linewidth=2.5, linestyle='--', color=nvmeSSD_colors[5], markersize=7)

# 设置标题和轴标签[1](@ref)
ax.set_title('IOPS Performance Comparison Across Different IO Sizes', fontsize=20, pad=20)
ax.set_xlabel('Threads', fontsize=16, labelpad=10)
ax.set_ylabel('IOPS', fontsize=16, labelpad=10)

# 设置坐标轴刻度[7](@ref)
ax.set_xscale('log', base=2)
ax.set_yscale('log')

# 设置密集的刻度[7](@ref)
x_ticks = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024]
ax.set_xticks(x_ticks)
ax.set_xticklabels([str(x) for x in x_ticks], fontsize=14)

# 设置Y轴刻度（对数坐标下需要特殊处理）
y_ticks = [1000, 2000, 5000, 10000, 20000, 50000]
ax.set_yticks(y_ticks)
ax.set_yticklabels([str(y) for y in y_ticks], fontsize=14)
ax.set_ylim(500, 30000)  # 设置适当的Y轴范围

# 设置网格[1](@ref)
ax.grid(True, which="both", linestyle='-', alpha=0.2, linewidth=0.5)

# 设置图例[7](@ref)
ax.legend(bbox_to_anchor=(1.05, 1), loc='upper left', fontsize=12, 
          frameon=True, fancybox=True, shadow=True, ncol=1)

# 调整布局以防止标签被截断
plt.tight_layout()

# 保存高分辨率图片[1](@ref)
img_path = 'iops_comparison_complete.png'
plt.savefig(img_path, dpi=300, bbox_inches='tight', facecolor='white', edgecolor='none')
print(f"完整IOPS对比图已保存为 {img_path}")

# 显示图表
plt.show()

# 打印数据统计摘要
print("\n数据统计摘要:")
print("="*50)
for column in df.columns[1:]:  # 跳过Threads列
    if 'BeeGFS' in column:
        storage_type = 'BeeGFS Cufile'
    else:
        storage_type = 'localNVMeSSD'
    
    io_size = column.split('-')[-1]
    max_iops = df[column].max()
    max_threads = df.loc[df[column] == max_iops, 'Threads'].values[0]
    
    print(f"{storage_type:15} {io_size:6}: 最大IOPS = {max_iops:8.2f} (线程数 = {max_threads})")