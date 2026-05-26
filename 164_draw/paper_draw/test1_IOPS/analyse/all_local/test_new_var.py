import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
import os
import re

# 设置中文字体和样式
plt.rcParams['font.sans-serif'] = ['SimHei', 'Arial Unicode MS', 'DejaVu Sans']
plt.rcParams['axes.unicode_minus'] = False
sns.set_style("whitegrid")

def extract_size_from_dirname(dir_name):
    """
    从目录名称中提取大小信息（如1KB）
    
    参数:
    dir_name (str): 目录名称
    
    返回:
    str: 提取到的大小字符串，如果未找到则返回"Unknown"
    """
    pattern = r'\d+[KMGTP]?B'
    match = re.search(pattern, dir_name)
    return match.group() if match else "Unknown"

def calculate_volatility_percentage(mean_value, min_value, max_value):
    """
    计算最大值和最小值相对于平均值的波动百分比
    
    参数:
    mean_value (float): 平均值
    min_value (float): 最小值
    max_value (float): 最大值
    
    返回:
    tuple: (min_volatility_pct, max_volatility_pct)
    """
    # 计算最小值相对于平均值的波动百分比
    min_volatility_pct = ((mean_value - min_value) / mean_value) * 100
    
    # 计算最大值相对于平均值的波动百分比
    max_volatility_pct = ((max_value - mean_value) / mean_value) * 100
    
    return min_volatility_pct, max_volatility_pct

def plot_individual_configurations(csv_file_path):
    """
    读取CSV文件并为每个配置绘制单独的读取时间折线图
    
    参数:
    csv_file_path (str): CSV文件路径
    """
    # 读取CSV文件
    df = pd.read_csv(csv_file_path)
    
    # 检查必要的列是否存在
    required_columns = ['dir_name', 'thread_num', 'total_read_time(s)']
    for col in required_columns:
        if col not in df.columns:
            print(f"错误: CSV文件中缺少必要的列 '{col}'")
            print(f"CSV文件中的列: {list(df.columns)}")
            return
    
    # 提取所有不同的配置组合 (同时考虑dir_name和thread_num)
    config_combinations = df[['dir_name', 'thread_num']].drop_duplicates()
    
    print(f"找到 {len(config_combinations)} 种不同的配置组合")
    
    # 为每个配置组合创建单独的图表
    for idx, config in config_combinations.iterrows():
        dir_name = config['dir_name']
        thread_num = config['thread_num']
        
        # 筛选当前配置的数据
        subset = df[(df['dir_name'] == dir_name) & (df['thread_num'] == thread_num)]
        
        # 确保有足够的数据点
        if len(subset) < 1:
            print(f"警告: 配置 (Dir={dir_name}, ThreadNum={thread_num}) 没有数据点")
            continue
        
        # 创建图表
        plt.figure(figsize=(12, 6))
        
        # 绘制折线图
        plt.plot(range(1, len(subset) + 1), subset['total_read_time(s)'].values, 
                marker='o', linewidth=2, markersize=6, color='#F8C471')
        
        # 计算统计信息
        mean_value = subset['total_read_time(s)'].mean()
        min_value = subset['total_read_time(s)'].min()
        max_value = subset['total_read_time(s)'].max()
        std_value = subset['total_read_time(s)'].std()
        
        # 计算波动百分比
        min_vol_pct, max_vol_pct = calculate_volatility_percentage(mean_value, min_value, max_value)
        
        # 添加统计线
        plt.axhline(y=mean_value, color='red', linestyle='--', linewidth=1.5)
        plt.axhline(y=min_value, color='green', linestyle=':', linewidth=1.5)
        plt.axhline(y=max_value, color='blue', linestyle=':', linewidth=1.5)
        
        # 创建统一的信息框文本
        info_text = (f'avg: {mean_value:.6f}s\n'
                    f'min: {min_value:.6f}s ({min_vol_pct:.1f}% below)\n'
                    f'max: {max_value:.6f}s ({max_vol_pct:.1f}% above)')
        
        # 在图表左上角添加统一的信息框
        plt.text(0.02, 0.98, info_text, 
                transform=plt.gca().transAxes,
                fontsize=10,
                verticalalignment='top',
                horizontalalignment='left',
                bbox=dict(boxstyle="round,pad=0.5", facecolor="wheat", alpha=0.8),
                color='black')
        
        # 设置图表标题和标签
        size_info = extract_size_from_dirname(dir_name)
        plt.title(f'Read Time Performance - ThreadNum: {thread_num}, Size: {size_info}\nDirectory: {dir_name}', fontsize=14)
        plt.xlabel('Run Number', fontsize=12)
        plt.ylabel('Read Time (seconds)', fontsize=12)

        # 设置x轴刻度
        plt.xticks(range(1, len(subset) + 1))
        
        # 添加网格
        plt.grid(True, alpha=0.3)
        
        # 调整布局
        plt.tight_layout()
        
        # 创建输出目录
        output_dir = 'performance_plots_var'
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        
        # 生成安全的文件名
        safe_dir_name = re.sub(r'[\\/*?:"<>|]', "_", dir_name)
        filename = f'{output_dir}/{safe_dir_name}_thread_{thread_num}_performance.png'
        plt.savefig(filename, dpi=300, bbox_inches='tight')
        print(f"图表已保存为: {filename}")
        
        # 显示图表
        plt.show()
        
        # 打印统计摘要
        print(f"\nConfiguration (Dir: {safe_dir_name}, ThreadNum: {thread_num}) Statistics:")
        print(f"Number of data points: {len(subset)}")
        print(f"Mean: {mean_value:.6f}s")
        print(f"Min: {min_value:.6f}s")
        print(f"Max: {max_value:.6f}s")
        print(f"Standard deviation: {std_value:.6f}s")
        print(f"Min volatility: {min_vol_pct:.2f}% below mean")
        print(f"Max volatility: {max_vol_pct:.2f}% above mean")
        print("-" * 50)

# 使用示例
if __name__ == "__main__":
    # 替换为您的CSV文件路径
    csv_file_path = "/home/oem/xyp/paper_draw/test1_IOPS/analyse/all_local/nvme0_read_result.csv"  # 请修改为实际文件路径
    
    try:
        plot_individual_configurations(csv_file_path)
    except Exception as e:
        print(f"处理过程中发生错误: {e}")
        import traceback
        traceback.print_exc()