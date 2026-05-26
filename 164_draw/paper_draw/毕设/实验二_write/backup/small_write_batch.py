import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import os

# ===== 可调参数区 =====
FONT_SIZE_LABEL = 10         # 坐标轴标签字体大小
FONT_SIZE_TICK = 10          # 坐标轴刻度字体大小
FONT_SIZE_ANNOT = 10         # 数据标注字体大小
FONT_SIZE_LEGEND = 10        # 图例字体大小
FIG_WIDTH = 4               # 单个子图的图宽
FIG_HEIGHT = 3              # 单个子图的图高
GRID_ALPHA = 0.5            # 网格透明度
GRID_LINESTYLE = '--'       # 网格线型

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

def plot_io_performance_comparison(csv_file_path, io_size_kb=None, output_filename=None):
    """
    绘制IO性能比较图
    
    参数:
    csv_file_path: CSV文件路径
    io_size_kb: I/O大小（KB），用于图表标题
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
    line_our, = ax.plot(threads_our, df_our['IOPS_avg']/1000, 
             marker=markers['OUR'], color=colors['OUR'], 
             linewidth=linewidths['OUR'], markersize=6, label='OUR', 
             linestyle=linestyles['OUR'], zorder=3)
    
    # 绘制GDS折线图
    if not df_gds.empty:
        threads_gds = df_gds['THREADS']
        line_gds, = ax.plot(threads_gds, df_gds['IOPS_avg']/1000, 
                 marker=markers['GDS'], color=colors['GDS'], 
                 linewidth=linewidths['GDS'], markersize=6, label='GDS', 
                 linestyle=linestyles['GDS'], zorder=2)
    
    # 绘制POSIX折线图
    threads_posix = df_posix['Thread_Num']
    line_posix, = ax.plot(threads_posix, df_posix['IOPS_avg']/1000, 
             marker=markers['POSIX'], color=colors['POSIX'], 
             linewidth=linewidths['POSIX'], markersize=5, label='POSIX', 
             linestyle=linestyles['POSIX'], zorder=1)
    
    # 设置图表属性
    ax.set_xlabel('线程数量', fontsize=FONT_SIZE_LABEL)
    ax.set_ylabel('KIOPS (千次/秒)', fontsize=FONT_SIZE_LABEL)  # 将IOPS单位转换为千
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
    max_iops = max(
        df_posix['IOPS_avg'].max(),
        df_our['IOPS_avg'].max(),
        df_gds['IOPS_avg'].max() if not df_gds.empty else 0
    )
    
    # 动态设置y轴范围
    y_max = max_iops / 1000 * 1.1
    y_ticks = np.arange(0, y_max, 5) if y_max <= 50 else np.arange(0, y_max, 10)
    ax.set_yticks(y_ticks)
    ax.set_yticklabels([f"{int(tick)}" for tick in y_ticks], fontsize=FONT_SIZE_TICK)
    ax.set_ylim(0, y_max)
    
    # 添加图例
    ax.legend(loc='best', fontsize=FONT_SIZE_LEGEND, frameon=False)
    
    # 添加标题
    # if io_size_kb is not None:
    #     title = f"{io_size_kb}KB写入IOPS性能对比"
    #     ax.set_title(title, fontsize=FONT_SIZE_LABEL+2, pad=15)
    
    # 调整布局
    plt.tight_layout()
    
    # 确定输出文件名
    if output_filename is None:
        if io_size_kb is not None:
            output_filename = f'Write_iops_{io_size_kb}KB_comparison.png'
        else:
            # 从文件名推断
            basename = os.path.basename(csv_file_path)
            if '512B'in basename:
                output_filename = 'Write_iops_512B_comparison.png'
            if '1KB' in basename:
                output_filename = 'Write_iops_1KB_comparison.png'
            elif '2KB' in basename:
                output_filename = 'Write_iops_2KB_comparison.png'
            elif '4KB' in basename:
                output_filename = 'Write_iops_4KB_comparison.png'
            elif '8KB' in basename:
                output_filename = 'Write_iops_8KB_comparison.png'
            elif '16KB' in basename:
                output_filename = 'Write_iops_16KB_comparison.png'
            else:
                output_filename = 'Write_iops_comparison.png'
    
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


def process_all_io_sizes(base_dir='csv', file_pattern='write_iops_{}KB.csv'):
    """
    处理所有I/O大小的数据
    
    参数:
    base_dir: 基础目录
    file_pattern: 文件名模式，{}将被替换为I/O大小
    """
    io_sizes = [1, 2, 4, 8, 16]
    
    for size in io_sizes:
        csv_file = os.path.join(base_dir, file_pattern.format(size))
        if os.path.exists(csv_file):
            print(f"\n处理 {size}KB 数据...")
            plot_io_performance_comparison(csv_file, io_size_kb=size)
        else:
            print(f"警告: 文件 {csv_file} 不存在，跳过")


# ===== 使用示例 =====
if __name__ == "__main__":
    # 方法1: 处理单个文件
    print("方法1: 处理单个文件")
    csv_file_path = 'csv/write_iops_4KB.csv'
    if os.path.exists(csv_file_path):
        plot_io_performance_comparison(csv_file_path, io_size_kb=4)
    else:
        print(f"文件 {csv_file_path} 不存在")
    
    # 方法2: 批量处理所有I/O大小
    print("\n" + "="*50)
    print("方法2: 批量处理所有I/O大小")
    process_all_io_sizes(base_dir='csv', file_pattern='write_iops_{}KB.csv')
    
    # 方法3: 手动指定不同大小的文件
    print("\n" + "="*50)
    print("方法3: 手动处理不同大小的文件")
    
    # 处理1KB数据
    csv_512b = 'csv/write_iops_512B.csv'
    if os.path.exists(csv_512b):
        plot_io_performance_comparison(csv_512b, io_size_kb=0.5)

    # 处理1KB数据
    csv_1kb = 'csv/write_iops_1KB.csv'
    if os.path.exists(csv_1kb):
        plot_io_performance_comparison(csv_1kb, io_size_kb=1)
    
    # 处理2KB数据
    csv_2kb = 'csv/write_iops_2KB.csv'
    if os.path.exists(csv_2kb):
        plot_io_performance_comparison(csv_2kb, io_size_kb=2)
    
    # 处理8KB数据
    csv_8kb = 'csv/write_iops_8KB.csv'
    if os.path.exists(csv_8kb):
        plot_io_performance_comparison(csv_8kb, io_size_kb=8)
    
    # 处理16KB数据
    csv_16kb = 'csv/write_iops_16KB.csv'
    if os.path.exists(csv_16kb):
        plot_io_performance_comparison(csv_16kb, io_size_kb=16)