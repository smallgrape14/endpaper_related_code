import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import os

# # ===== 可调参数区 =====
# FONT_SIZE_LABEL = 10         # 坐标轴标签字体大小
# FONT_SIZE_TICK = 10          # 坐标轴刻度字体大小
# FONT_SIZE_ANNOT = 10         # 数据标注字体大小
# FONT_SIZE_LEGEND = 10        # 图例字体大小
# FIG_WIDTH = 4               # 单个子图的图宽
# FIG_HEIGHT = 3              # 单个子图的图高
# GRID_ALPHA = 0.5            # 网格透明度
# GRID_LINESTYLE = '--'       # 网格线型

# # 设置中文字体支持
# plt.rcParams['font.sans-serif'] = ['SimHei']  # 用来正常显示中文标签
# plt.rcParams['axes.unicode_minus'] = False  # 用来正常显示负号
# ===== 可调参数区 =====
FONT_SIZE_LABEL = 20         # 坐标轴标签字体大小
FONT_SIZE_TICK = 20          # 坐标轴刻度字体大小
FONT_SIZE_ANNOT = 18         # 数据标注字体大小
FONT_SIZE_LEGEND = 18        # 图例字体大小
FIG_WIDTH = 5.8              # 单个子图的图宽
FIG_HEIGHT = 4.8             # 单个子图的图高
GRID_ALPHA = 0.5             # 网格透明度
GRID_LINESTYLE = '--'        # 网格线型
# ===== FIXED FONT CONFIGURATION =====
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['SimSun','Times New Roman' ]  # Times + 宋体
plt.rcParams['mathtext.fontset'] = 'stix'  # Math symbols matching Times
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

def plot_throughput_performance_comparison(csv_file_path, io_size_mb=None, output_filename=None):
    """
    绘制吞吐量性能比较图
    
    参数:
    csv_file_path: CSV文件路径
    io_size_mb: I/O大小（MB），用于图表标题
    output_filename: 输出文件名，如果为None则自动生成
    """
    
    # 读取CSV文件
    try:
        df = pd.read_csv(csv_file_path)
        print(f"成功读取文件: {csv_file_path}")
    except FileNotFoundError:
        print(f"错误: 找不到文件 {csv_file_path}")
        return
    except Exception as e:
        print(f"读取CSV文件时出错: {e}")
        return
    
    # 显示数据信息
    print(f"读取的数据形状: {df.shape}")
    print("列名:", df.columns.tolist())
    print("\n前5行数据:")
    print(df.head())
    
    # 处理列名，去除空格
    df.columns = df.columns.str.strip()
    
    # 检查必要的列是否存在
    required_columns = ['ThreadNum', 'OUR', 'GDS', 'POSIX']
    missing_columns = [col for col in required_columns if col not in df.columns]
    
    if missing_columns:
        print(f"警告: 缺少以下列: {missing_columns}")
        print(f"实际列名: {df.columns.tolist()}")
        
        # 尝试查找相似的列名
        for missing_col in missing_columns:
            for actual_col in df.columns:
                if missing_col.lower() in actual_col.lower():
                    print(f"  建议: 将 '{actual_col}' 重命名为 '{missing_col}'")
                    df.rename(columns={actual_col: missing_col}, inplace=True)
                    break
        
        # 重新检查
        missing_columns = [col for col in required_columns if col not in df.columns]
        if missing_columns:
            print(f"错误: 仍然缺少以下列: {missing_columns}")
            return
    
    # 处理缺失值（将空值转换为NaN）
    df['GDS'] = pd.to_numeric(df['GDS'], errors='coerce')
    df['OUR'] = pd.to_numeric(df['OUR'], errors='coerce')
    df['POSIX'] = pd.to_numeric(df['POSIX'], errors='coerce')
    
    # 创建数据字典
    posix_data = {
        'Thread_Num': df['ThreadNum'].tolist(),
        'Throughput': df['POSIX'].tolist()
    }
    
    our_data = {
        'ThreadNum': df['ThreadNum'].tolist(),
        'Throughput': df['OUR'].tolist()
    }
    
    # GDS数据可能有缺失，只提取非空值
    gds_mask = df['GDS'].notna()
    gds_data = {
        'THREADS': df.loc[gds_mask, 'ThreadNum'].tolist(),
        'Throughput': df.loc[gds_mask, 'GDS'].tolist()
    }
    
    # 转换为DataFrame
    df_posix = pd.DataFrame(posix_data)
    df_our = pd.DataFrame(our_data)
    df_gds = pd.DataFrame(gds_data)
    
    # 创建图表
    fig, ax = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT), dpi=100)
    
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
    
    # 线宽
    linewidths = {
        'POSIX': 1.5,
        'OUR': 2.0,
        'GDS': 1.5
    }
    
    # 绘制OUR折线图
    threads_our = df_our['ThreadNum']
    line_our, = ax.plot(threads_our, df_our['Throughput'], 
             marker=markers['OUR'], color=colors['OUR'], 
             linewidth=linewidths['OUR'], markersize=8, label='OUR', 
             linestyle=linestyles['OUR'], zorder=3)
    
    # 绘制GDS折线图
    if not df_gds.empty:
        threads_gds = df_gds['THREADS']
        line_gds, = ax.plot(threads_gds, df_gds['Throughput'], 
                 marker=markers['GDS'], color=colors['GDS'], 
                 linewidth=linewidths['GDS'], markersize=8, label='GDS', 
                 linestyle=linestyles['GDS'], zorder=2)
    
    # 绘制POSIX折线图
    threads_posix = df_posix['Thread_Num']
    line_posix, = ax.plot(threads_posix, df_posix['Throughput'], 
             marker=markers['POSIX'], color=colors['POSIX'], 
             linewidth=linewidths['POSIX'], markersize=8, label='POSIX', 
             linestyle=linestyles['POSIX'], zorder=1)
    
    # 设置图表属性
    ax.set_xlabel('线程数量', fontsize=FONT_SIZE_LABEL)
    ax.set_ylabel('吞吐量 (GB/s)', fontsize=FONT_SIZE_LABEL)
    ax.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE, zorder=0)
    
    # 设置x轴为对数刻度，并设置刻度标签
    ax.set_xscale('log')
    thread_ticks = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024]
    # 只显示实际存在的线程数
    thread_ticks = [t for t in thread_ticks if t <= df['ThreadNum'].max()]
    ax.set_xticks(thread_ticks)
    ax.set_xticklabels([str(t) for t in thread_ticks], rotation=45, fontsize=FONT_SIZE_TICK)
    
    # 设置y轴刻度
    # 计算最大值，忽略NaN
    max_throughput = max(
        df_posix['Throughput'].max(),
        df_our['Throughput'].max(),
        df_gds['Throughput'].max() if not df_gds.empty else 0
    )
    
    # 动态设置y轴范围
    y_max = max_throughput * 1.1
    # 根据最大值确定刻度间隔
    if y_max <= 5:
        y_step = 0.5
    elif y_max <= 10:
        y_step = 1
    elif y_max <= 20:
        y_step = 2
    elif y_max <= 50:
        y_step = 5
    else:
        y_step = 10
    
    y_ticks = np.arange(0, y_max, y_step)
    ax.set_yticks(y_ticks)
    ax.set_yticklabels([f"{tick:.1f}" if y_step < 1 else f"{int(tick)}" for tick in y_ticks], fontsize=FONT_SIZE_TICK)
    ax.set_ylim(0, y_max)
    
    # 添加图例
    ax.legend(loc='best', fontsize=FONT_SIZE_LEGEND, frameon=False)
    
    # 添加标题
    # if io_size_mb is not None:
    #     title = f"{io_size_mb}MB文件读取吞吐量对比"
    #     ax.set_title(title, fontsize=FONT_SIZE_LABEL+2, pad=15)
    
    # 调整布局
    plt.tight_layout()
    
    # 确定输出文件名
    if output_filename is None:
        if io_size_mb is not None:
            output_filename = f'Throughput_{io_size_mb}MB_comparison.png'
        else:
            # 从文件名推断
            basename = os.path.basename(csv_file_path)
            if '1MB' in basename:
                output_filename = 'Throughput_1MB_comparison.png'
            elif '8MB' in basename:
                output_filename = 'Throughput_8MB_comparison.png'
            else:
                output_filename = 'Throughput_comparison.png'
    
    # 保存图表
    plt.savefig(output_filename, dpi=300, bbox_inches='tight', facecolor='white')
    plt.show()
    
    print(f"图表已保存为 '{output_filename}'")
    print(f"从CSV文件读取了{len(df)}行数据")
    print(f"OUR数据点: {len(df_our)}个")
    print(f"GDS数据点: {len(df_gds)}个")
    print(f"POSIX数据点: {len(df_posix)}个")
    print("-" * 50)
    
    return fig, ax


def process_all_throughput_files(base_dir='csv'):
    """
    处理所有吞吐量数据文件
    
    参数:
    base_dir: 基础目录
    """
    # 查找所有包含吞吐量数据的CSV文件
    import glob
    throughput_files = glob.glob(os.path.join(base_dir, '*throughput*.csv')) + \
                      glob.glob(os.path.join(base_dir, '*Throughput*.csv')) + \
                      glob.glob(os.path.join(base_dir, '*1MB*.csv')) + \
                      glob.glob(os.path.join(base_dir, '*8MB*.csv'))
    
    for csv_file in throughput_files:
        # 从文件名中推断文件大小
        basename = os.path.basename(csv_file)
        io_size_mb = None
        
        if '1MB' in basename:
            io_size_mb = 1
        elif '8MB' in basename:
            io_size_mb = 8
        
        print(f"\n处理文件: {basename}")
        plot_throughput_performance_comparison(csv_file, io_size_mb=io_size_mb)


# ===== 使用示例 =====
if __name__ == "__main__":
    # 方法1: 处理单个文件
    print("方法1: 处理单个文件")
    
    # 处理1MB吞吐量数据
    csv_1mb = 'csv/write_iops_1MB.csv'
    if os.path.exists(csv_1mb):
        plot_throughput_performance_comparison(csv_1mb, io_size_mb=1)
    else:
        print(f"文件 {csv_1mb} 不存在")
        # 尝试其他可能的文件名
        csv_1mb_alt = 'csv/write_iops_1mb.csv'
        if os.path.exists(csv_1mb_alt):
            plot_throughput_performance_comparison(csv_1mb_alt, io_size_mb=1)

    
    # 处理2MB吞吐量数据
    csv_2mb = 'csv/write_iops_2MB.csv'
    if os.path.exists(csv_2mb):
        plot_throughput_performance_comparison(csv_2mb, io_size_mb=2)
    else:
        print(f"文件 {csv_2mb} 不存在")
        # 尝试其他可能的文件名
        csv_2mb_alt = 'csv/write_iops_2mb.csv'
        if os.path.exists(csv_2mb_alt):
            plot_throughput_performance_comparison(csv_2mb_alt, io_size_mb=2)      


    # 处理4MB吞吐量数据
    csv_4mb = 'csv/write_iops_4MB.csv'
    if os.path.exists(csv_4mb):
        plot_throughput_performance_comparison(csv_4mb, io_size_mb=4)
    else:
        print(f"文件 {csv_4mb} 不存在")
        # 尝试其他可能的文件名
        csv_4mb_alt = 'csv/write_iops_4mb.csv'
        if os.path.exists(csv_4mb_alt):
            plot_throughput_performance_comparison(csv_4mb_alt, io_size_mb=4)
    
      
    # 处理8MB吞吐量数据
    csv_8mb = 'csv/write_iops_8MB.csv'
    if os.path.exists(csv_8mb):
        plot_throughput_performance_comparison(csv_8mb, io_size_mb=8)
    else:
        print(f"文件 {csv_8mb} 不存在")
        # 尝试其他可能的文件名
        csv_8mb_alt = 'csv/write_iops_8mb.csv'
        if os.path.exists(csv_8mb_alt):
            plot_throughput_performance_comparison(csv_8mb_alt, io_size_mb=8)
    
    # 方法2: 批量处理所有吞吐量文件
    print("\n" + "="*50)
    print("方法2: 批量处理所有吞吐量文件")
    process_all_throughput_files(base_dir='csv')