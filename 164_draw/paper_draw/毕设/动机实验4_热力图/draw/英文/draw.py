import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

# ===== FIXED FONT CONFIGURATION =====
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['DejaVu Sans','SimSun' ]  # Times + 宋体
plt.rcParams['mathtext.fontset'] = 'dejavusans'  # Math symbols matching Times
plt.rcParams['axes.unicode_minus'] = False
plt.rcParams['axes.formatter.use_mathtext'] = False

# 如果从CSV文件读取，可以这样写：
df = pd.read_csv('your_data.csv')

# 获取所有不同的batch_size
batch_sizes = df['batch_size'].unique()
print(f"数据中包含的batch_size: {batch_sizes}")

# 设置统一的颜色映射范围，便于比较
vmin = df['data_per'].min()
vmax = df['data_per'].max()

# 创建子图，每个batch_size一个热力图
fig, axes = plt.subplots(1, len(batch_sizes), figsize=(6*len(batch_sizes), 5))

# 如果只有一个batch_size，axes不是数组，需要处理
if len(batch_sizes) == 1:
    axes = [axes]

# 为每个batch_size创建热力图
for i, batch_size in enumerate(batch_sizes):
    # 筛选当前batch_size的数据
    df_batch = df[df['batch_size'] == batch_size]
    
    # 创建透视表
    pivot_table = df_batch.pivot(
        index='prefetch_factor', 
        columns='worker_num', 
        values='data_per'
    )
    
    # 对行和列进行排序
    pivot_table = pivot_table.sort_index(ascending=False)  # prefetch_factor从大到小
    pivot_table = pivot_table.reindex(sorted(pivot_table.columns), axis=1)  # worker_num从小到大
    
    # 绘制热力图
    im = sns.heatmap(
        pivot_table, 
        annot=True,#True, 
        fmt='.3f', 
        cmap='YlOrRd', 
        cbar=True if i == len(batch_sizes)-1 else False,  # 只在最后一个子图显示colorbar
        cbar_kws={'label': 'data_per' if i == len(batch_sizes)-1 else ''},
        vmin=vmin,  # 使用统一的最小值
        vmax=vmax,  # 使用统一的最大值
        linewidths=0.5, 
        linecolor='gray',
        square=False,  # 使热力图的单元格为正方形
        ax=axes[i]
    )
    
    axes[i].set_title(f'batch_size = {batch_size}', fontsize=14, fontweight='bold')
    axes[i].set_xlabel('Worker Number', fontsize=24)
    axes[i].set_ylabel('Prefetch Factor', fontsize=24)
    
    # 设置x轴和y轴标签
    axes[i].tick_params(axis='both', which='major', labelsize=22)
    axes[i].tick_params(axis='both', which='minor', labelsize=22)

# 调整布局
plt.suptitle('Data Percentage Heatmaps by Batch Size', fontsize=16, fontweight='bold', y=1.05)
plt.tight_layout()

# 保存图像
output_file = f'performance_heatmap_batch_sizes_{"_".join(map(str, batch_sizes))}.png'
plt.savefig(output_file, dpi=300, bbox_inches='tight')
print(f"热力图已保存为: {output_file}")

# 显示图像
plt.show()

# 同时，为每个batch_size单独保存一个热力图
for batch_size in batch_sizes:
    # 筛选当前batch_size的数据
    df_batch = df[df['batch_size'] == batch_size]
    
    # 创建透视表
    pivot_table = df_batch.pivot(
        index='prefetch_factor', 
        columns='worker_num', 
        values='data_per'
    )
    
    # 对行和列进行排序
    pivot_table = pivot_table.sort_index(ascending=False)  # prefetch_factor从大到小
    pivot_table = pivot_table.reindex(sorted(pivot_table.columns), axis=1)  # worker_num从小到大
    
    # 创建单独的热力图
    plt.figure(figsize=(10, 8))
    im=sns.heatmap(
        pivot_table, 
        annot=True, 
        fmt='.3f', 
        cmap='YlOrRd', 
        # cbar_kws={'label': 'data_per'},
        cbar_kws={'label': 'Data Loading Time Proportion'},  # ✅ 修改colorbar标签和字体大小
        annot_kws={"size": 14},  # ✅ 新增参数，控制字体大小,'color': 'black'
        linewidths=0.5, 
        linecolor='gray',
        square=False
    )
    # 🔧 获取colorbar并设置字体
    cbar = im.collections[0].colorbar
    cbar.ax.tick_params(labelsize=16)  # 刻度字体
    cbar.set_label('Data Loading Time Proportion', size=18)  # 标签字体
    # plt.title(f'Data Percentage Heatmap (batch_size = {batch_size})', fontsize=16, fontweight='bold')
    plt.xlabel('Worker Number', fontsize=20)
    plt.ylabel('Prefetch Factor', fontsize=20)
    plt.tick_params(axis='both', which='major', labelsize=18)
    plt.tick_params(axis='both', which='minor', labelsize=18)
    plt.tight_layout()
    
    # 保存单独的热力图
    single_file = f'performance_heatmap_batchsize_{batch_size}.png'
    single_file_pdf = f'performance_heatmap_batchsize_{batch_size}.pdf'

    plt.savefig(single_file, dpi=300, bbox_inches='tight')
    plt.savefig(single_file_pdf, dpi=300, bbox_inches='tight')

    plt.close()  # 关闭图形，避免重叠
    print(f"单独热力图已保存为: {single_file}")

# 输出每个batch_size的数据摘要
# print("\n=== 每个batch_size的数据摘要 ===")
# for batch_size in batch_sizes:
#     df_batch = df[df['batch_size'] == batch_size]
#     print(f"\nbatch_size={batch_size}:")
#     print(f"  数据点数: {len(df_batch)}")
#     print(f"  worker_num取值范围: {sorted(df_batch['worker_num'].unique())}")
#     print(f"  prefetch_factor取值范围: {sorted(df_batch['prefetch_factor'].unique())}")
#     print(f"  data_per范围: [{df_batch['data_per'].min():.6f}, {df_batch['data_per'].max():.6f}]")