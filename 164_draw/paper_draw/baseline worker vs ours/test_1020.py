import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# 读取CSV文件
# 假设CSV文件名为 'data_loading_performance.csv'，请根据实际文件名修改
df = pd.read_csv('Baseline_Vs_Ours_Opt_1020.csv')

# 获取唯一的batch_size和worker_num组合
batch_sizes = sorted(df['batch_size'].unique())
worker_nums = sorted(df['worker_num'].unique())

# 创建图表
plt.figure(figsize=(14, 8))

# 定义颜色和标记样式
colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b', '#e377c2', '#7f7f7f', '#bcbd22']
markers = ['o', 's', '^', 'v', 'D', 'P', '*', 'X', 'd']

# 为每个worker_num绘制两条线（AVG_data和AVG_data_Without_FirstBatch）
for i, worker_num in enumerate(worker_nums):
    # 提取对应worker_num的数据并按batch_size排序
    worker_data = df[df['worker_num'] == worker_num].sort_values('batch_size')
    
    if worker_num == 0:
        label_prefix = "Baseline worker=0"
    else:
        label_prefix = f"Baseline worker={worker_num}"
    
    
    # 为相同worker的两条线使用相近但不同的颜色
    base_color = colors[i % len(colors)]
    
    # 生成相近颜色（稍微调整色调）
    # 使用matplotlib颜色操作来生成相近色
    import matplotlib.colors as mcolors
    base_rgb = mcolors.hex2color(colors)
    
    # 第一条线颜色（稍亮一点）
    color1 = tuple(min(1.0, c * 1.1) for c in base_rgb)
    # 第二条线颜色（稍暗一点）
    color2 = tuple(max(0.0, c * 0.9) for c in base_rgb)
    

    # 绘制AVG_data（虚线）
    plt.plot(worker_data['batch_size'], worker_data['AVG_data'], 
             label=f'{label_prefix} ',
            #  color=colors[i % len(colors)],
            color=color1,
             marker=markers[i % len(markers)],
             linestyle='--',  # 虚线
             linewidth=2.5,
             markersize=9,
             markeredgewidth=1.5)
    
    
    # 绘制AVG_data_Without_FirstBatch（实线）
    plt.plot(worker_data['batch_size'], worker_data['AVG_data_Without_FirstBatch'], 
             label=f'{label_prefix} (without first batch)',
            #  color=colors[i % len(colors)],  # 相同颜色
            color=color2,
             marker=markers[i % len(markers)],  # 相同标记
             linestyle='-',  # 实线
             linewidth=2.5,
             markersize=9,
             markeredgewidth=1.5)

# 添加你的优化方案数据（这里需要你提供具体数值）
# 根据你的需求添加"Ours"数据
ours_data = [0.00114938, 0.00134879, 0.002438405, 0.004659966, 0.01172559, 0.031704362]
plt.plot(batch_sizes, ours_data,
         label='Ours',
         color='#17becf',  # 使用一个新的颜色
         marker='h',  # 使用不同的标记
         linestyle='-',
         linewidth=2.5,
         markersize=9,
         markeredgewidth=1.5)

# 图表属性设置
plt.title('Data Load Time Comparison', fontsize=20, pad=20)
plt.xlabel('Batch Size', fontsize=16)
plt.ylabel('Data Time (s) - Lower is Better', fontsize=16)

# 设置双对数坐标轴
plt.xscale('log', base=2)
plt.yscale('log')

# 自定义坐标轴范围
plt.ylim(1e-3, 10)

# 网格和刻度设置
plt.grid(True, which='both', linestyle='--', alpha=0.6)
plt.tick_params(axis='both', which='major', labelsize=18)

# 图例设置（右侧外部）
# plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left', fontsize=12, 
#            framealpha=0.9, shadow=True)
# 改进图例设置 - 使用两列并增加间距
plt.legend(bbox_to_anchor=(1.05, 1), 
           loc='upper left', 
           fontsize=14,
           ncol=1,  # 两列布局
           columnspacing=1.5,  # 增加列间距
           handlelength=2.0,   # 增加图例句柄长度
           handletextpad=0.5,   # 增加句柄与文本间距
           framealpha=0.95,
           shadow=True,
           fancybox=True)
# 优化布局并保存
plt.tight_layout()
plt.savefig('batch_time_comparison_1020.png', dpi=300, bbox_inches='tight')
plt.savefig('batch_time_comparison_1020.pdf', format='pdf', bbox_inches='tight')
plt.show()

# 打印处理后的数据框以供检查
print("原始数据:")
print(df)