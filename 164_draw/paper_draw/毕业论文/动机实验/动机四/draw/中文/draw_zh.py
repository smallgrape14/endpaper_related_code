import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

# ===== 可调参数区 =====
FONT_SIZE_LABEL = 21         # 坐标轴标签字体大小
FONT_SIZE_TICK = 20          # 坐标轴刻度字体大小
FONT_SIZE_ANNOT = 18         # 数据标注字体大小
FONT_SIZE_LEGEND = 18        # 图例字体大小
FIG_WIDTH = 5.8              # 单个子图的图宽
FIG_HEIGHT = 4.8             # 单个子图的图高
GRID_ALPHA = 0.5             # 网格透明度
GRID_LINESTYLE = '--'        # 网格线型

# 设置中文字体（如果需要）
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['SimSun','Times New Roman' ]  # Times + 宋体
plt.rcParams['mathtext.fontset'] = 'stix'  # Math symbols matching Times
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

# 计算子图布局
n_batches = len(batch_sizes)
fig, axes = plt.subplots(1, n_batches, figsize=(FIG_WIDTH * n_batches, FIG_HEIGHT))

# 如果只有一个batch_size，axes不是数组，需要处理
if n_batches == 1:
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
        annot=True, 
        fmt='.4f', 
        cmap='YlOrRd', 
        cbar=True if i == n_batches-1 else False,  # 只在最后一个子图显示colorbar
        cbar_kws={'label': '数据加载时间占比' if i == n_batches-1 else ''},
        # cbar_kws= {
        #     'label': '数据加载时间占比',
        #     'labelsize': FONT_SIZE_TICK,  # 设置色条标签字体大小
        #     'ticklabelsize': FONT_SIZE_TICK,  # 设置色条刻度字体大小
        #     'shrink': 0.8,  # 调整色条高度
        #     'aspect': 20,  # 控制色条宽度
        # },
        # {'label': '数据加载时间占比' if i == n_batches-1 else ''},
        vmin=vmin,  # 使用统一的最小值
        vmax=vmax,  # 使用统一的最大值
        linewidths=0.5, 
        linecolor='gray',
        square=False,  # 使热力图的单元格为正方形
        ax=axes[i],
        annot_kws={"size": FONT_SIZE_ANNOT+2}
    )
    
    axes[i].set_title(f'batch_size = {batch_size}', fontsize=FONT_SIZE_LABEL + 2, fontweight='bold')
    axes[i].set_xlabel('工作进程数', fontsize=FONT_SIZE_LABEL)
    axes[i].set_ylabel('预取因子', fontsize=FONT_SIZE_LABEL)
    
    # 设置x轴和y轴标签
    axes[i].tick_params(axis='both', which='major', labelsize=FONT_SIZE_TICK)
    axes[i].tick_params(axis='both', which='minor', labelsize=FONT_SIZE_TICK - 2)
    
    # 添加网格
    axes[i].grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)

# 调整布局
plt.suptitle('各批次大小的数据加载时间占比热力图', fontsize=FONT_SIZE_LABEL + 4, fontweight='bold', y=1.05)
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
    plt.figure(figsize=(FIG_WIDTH * 2, FIG_HEIGHT * 1.5))
    ax=sns.heatmap(
        pivot_table, 
        annot=True, 
        fmt='.4f', 
        cmap='YlOrRd', 
        # cbar_kws={'label': '数据加载时间占比'},
        linewidths=0.5, 
        linecolor='gray',
        square=False,
        annot_kws={"size": FONT_SIZE_ANNOT+2}
    )
    # 3. 获取 colorbar 对象
    cbar = ax.collections[0].colorbar

    # 4. 设置 colorbar 标题 (Title) 大小
    cbar.ax.set_title('数据加载时间占比', fontsize=20) # 设置标题内容和字号

    # 5. 设置 colorbar 刻度标签 (Tick Labels) 大小
    cbar.ax.tick_params(labelsize=20) # 设置刻度数字大小
    
    # plt.title(f'数据加载时间占比热力图 (batch_size = {batch_size})', fontsize=FONT_SIZE_LABEL + 4, fontweight='bold')
    plt.xlabel('工作进程数', fontsize=FONT_SIZE_LABEL + 2)
    plt.ylabel('预取因子', fontsize=FONT_SIZE_LABEL + 2)
    plt.tick_params(axis='both', labelsize=FONT_SIZE_TICK)
    plt.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
    plt.tight_layout()
    
    # 保存单独的热力图
    single_file = f'performance_heatmap_batchsize_{batch_size}.png'
    plt.savefig(single_file, dpi=300, bbox_inches='tight')
    plt.close()  # 关闭图形，避免重叠
    print(f"单独热力图已保存为: {single_file}")

# 输出每个batch_size的数据摘要
print("\n=== 每个batch_size的数据摘要 ===")
for batch_size in batch_sizes:
    df_batch = df[df['batch_size'] == batch_size]
    print(f"\nbatch_size={batch_size}:")
    print(f"  数据点数: {len(df_batch)}")
    print(f"  工作进程数取值范围: {sorted(df_batch['worker_num'].unique())}")
    print(f"  预取因子取值范围: {sorted(df_batch['prefetch_factor'].unique())}")
    print(f"  数据加载时间占比范围: [{df_batch['data_per'].min():.6f}, {df_batch['data_per'].max():.6f}]")