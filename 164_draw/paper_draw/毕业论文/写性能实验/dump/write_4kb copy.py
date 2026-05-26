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
# Windows系统可以使用SimHei
plt.rcParams['font.sans-serif'] = ['SimHei']  # 用来正常显示中文标签
# Mac系统可以使用PingFang SC
# plt.rcParams['font.sans-serif'] = ['PingFang SC']  # Mac系统
# 或者使用Arial Unicode MS
# plt.rcParams['font.sans-serif'] = ['Arial Unicode MS']  # 跨平台

# 解决负号显示问题
plt.rcParams['axes.unicode_minus'] = False

# 设置全局字体
plt.rcParams.update({
    'font.size': FONT_SIZE_TICK,
    'axes.labelsize': FONT_SIZE_LABEL,
    'legend.fontsize': FONT_SIZE_LEGEND,
    'xtick.labelsize': FONT_SIZE_TICK,
    'ytick.labelsize': FONT_SIZE_TICK
})

# 从CSV文件读取数据
# 假设CSV文件名为 'write_iops_data.csv'
csv_filename = 'write_iops_4KB.csv'  # 请根据实际文件名修改

try:
    # 读取CSV文件，指定制表符为分隔符
    df = pd.read_csv(csv_filename, sep='\t')
    
    # 如果制表符分隔不起作用，尝试使用空格分隔
    if len(df.columns) == 1:
        df = pd.read_csv(csv_filename, sep='\s+')
    
    # 检查列名
    print("CSV文件列名:", df.columns.tolist())
    print("数据预览:")
    print(df.head())
    
    # 重命名列以确保一致性
    df = df.rename(columns={
        'ThreadNum': 'ThreadNum',
        'OUR': 'OUR',
        'GDS': 'GDS',
        'POSIX': 'POSIX'
    })
    print('----------1-----------')
    # 处理缺失值，将空字符串转换为NaN
    df.replace('', np.nan, inplace=True)
    print('----------2-----------')
    
    # 将数值列转换为浮点数类型
    df['ThreadNum'] = df['ThreadNum'].astype(int)
    df['OUR'] = pd.to_numeric(df['OUR'], errors='coerce')
    df['GDS'] = pd.to_numeric(df['GDS'], errors='coerce')
    df['POSIX'] = pd.to_numeric(df['POSIX'], errors='coerce')
    print('----------3-----------')
    
    # 创建数据字典
    posix_data = {
        'Thread_Num': df['ThreadNum'].tolist(),
        'IOPS_avg': df['POSIX'].tolist()
    }
    
    our_data = {
        'ThreadNum': df['ThreadNum'].tolist(),
        'IOPS_avg': df['OUR'].tolist()
    }
    print('----------4-----------')
    
    # GDS数据可能有缺失，只提取非空值
    gds_mask = df['GDS'].notna()
    gds_data = {
        'THREADS': df.loc[gds_mask, 'ThreadNum'].tolist(),
        'IOPS_avg': df.loc[gds_mask, 'GDS'].tolist()
    }
    print('----------5-----------')
    
    # 转换为DataFrame
    df_posix = pd.DataFrame(posix_data)
    df_our = pd.DataFrame(our_data)
    df_gds = pd.DataFrame(gds_data)
    print('----------6-----------')
    
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
    # 计算最大值，忽略NaN
    max_iops = max(
        df_posix['IOPS_avg'].max(),
        df_our['IOPS_avg'].max(),
        df_gds['IOPS_avg'].max() if not df_gds.empty else 0
    )
    
    y_ticks = np.arange(0, max_iops/1000 + 5, 5)
    plt.yticks(y_ticks, fontsize=FONT_SIZE_TICK)
    plt.ylim(0, max_iops/1000 * 1.1)  # 留10%的顶部空间
    
    # 添加图例
    plt.legend(loc='best', fontsize=FONT_SIZE_LEGEND, frameon=False)
    
    # 调整布局
    plt.tight_layout()
    
    # 保存图表
    output_filename = 'Write_iops_4kb_comparison_cn.png'
    plt.savefig(output_filename, dpi=300, bbox_inches='tight', facecolor='white')
    plt.show()
    
    print(f"图表已保存为 '{output_filename}'")
    print(f"数据读取成功，共{len(df)}行数据")
    
except FileNotFoundError:
    print(f"错误：找不到文件 '{csv_filename}'")
    print("请确保CSV文件在当前目录下，或提供正确的文件路径")
except pd.errors.EmptyDataError:
    print(f"错误：文件 '{csv_filename}' 为空")
except Exception as e:
    print(f"读取CSV文件时出错: {e}")
    print("建议的解决方案：")
    print("1. 确保CSV文件存在且格式正确")
    print("2. 检查CSV文件的分隔符（制表符、逗号或空格）")
    print("3. 可以尝试手动指定分隔符，如：pd.read_csv('filename.csv', sep='\\t')")