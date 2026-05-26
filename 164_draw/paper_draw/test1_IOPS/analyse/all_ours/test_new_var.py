import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
import os
import re  # 确保导入了re模块

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
    # 确保文件路径正确，使用原始字符串（raw string）或正斜杠以避免转义问题[1](@ref)
    try:
        df = pd.read_csv(csv_file_path)
        print("CSV文件读取成功！")
        print(f"文件中的列：{list(df.columns)}") # 打印列名以进行调试
    except Exception as e:
        print(f"读取CSV文件时出错: {e}")
        print("请检查文件路径是否正确，以及文件格式是否有效[1,5](@ref)")
        return
    
    # 检查必要的列是否存在
    required_columns = ['MSG_type', 'ReadDirName', 'BlockNum', 'ThreadNum', 'Read Time(s)']
    missing_columns = [col for col in required_columns if col not in df.columns]
    if missing_columns:
        print(f"错误: CSV文件中缺少必要的列: {missing_columns}")
        print(f"CSV文件中的列: {list(df.columns)}")
        return
    
    # 提取所有不同的配置组合 (包含ReadDirName, BlockNum, ThreadNum)
    config_combinations = df[['ReadDirName', 'BlockNum', 'ThreadNum']].drop_duplicates()
    
    print(f"找到 {len(config_combinations)} 种不同的配置组合")
    
    # 为每个配置组合创建单独的图表
    for idx, config in config_combinations.iterrows():
        dir_name = config['ReadDirName']
        block_num = config['BlockNum']
        thread_num = config['ThreadNum']
        
        # 筛选当前配置的数据
        subset = df[(df['ReadDirName'] == dir_name) & 
                    (df['BlockNum'] == block_num) & 
                    (df['ThreadNum'] == thread_num)]
        
        # 确保有足够的数据点
        if len(subset) < 1:
            print(f"警告: 配置 (Dir={dir_name}, BlockNum={block_num}, ThreadNum={thread_num}) 没有数据点")
            continue
        
        # 创建图表
        plt.figure(figsize=(12, 6))
        
        # 绘制折线图
        plt.plot(range(1, len(subset) + 1), subset['Read Time(s)'].values, 
                marker='o', linewidth=2, markersize=6, color='#F8C471')
        
        # 计算统计信息
        mean_value = subset['Read Time(s)'].mean()
        min_value = subset['Read Time(s)'].min()
        max_value = subset['Read Time(s)'].max()
        std_value = subset['Read Time(s)'].std()
        # 计算波动百分比
        min_vol_pct, max_vol_pct = calculate_volatility_percentage(mean_value, min_value, max_value)
        
        
        # 添加统计线
        plt.axhline(y=mean_value, color='red', linestyle='--', linewidth=1.5, 
                   label=f'avg: {mean_value:.6f}s')
        plt.axhline(y=min_value, color='green', linestyle=':', linewidth=1.5, 
                   label=f'min: {min_value:.6f}s')
        plt.axhline(y=max_value, color='blue', linestyle=':', linewidth=1.5, 
                   label=f'max: {max_value:.6f}s')
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
        plt.title(f'Read Time Performance - BlockNum: {block_num}, ThreadNum: {thread_num}\nDirectory: {dir_name}', fontsize=14)
        plt.xlabel('Run Number', fontsize=12)
        plt.ylabel('Read Time (seconds)', fontsize=12)

        # 设置x轴刻度
        plt.xticks(range(1, len(subset) + 1))
        
        # 添加网格
        plt.grid(True, alpha=0.3)
        
        # 添加图例
        plt.legend()
        
        # 调整布局
        plt.tight_layout()
        
        # 创建输出目录
        output_dir = 'performance_plots_var'
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        
        # 生成安全的文件名
        safe_dir_name = re.sub(r'[\\/*?:"<>|]', "_", dir_name)
        filename = f"{output_dir}/{safe_dir_name}_block_{block_num}_thread_{thread_num}_performance.png"
        plt.savefig(filename, dpi=300, bbox_inches='tight')
        print(f"图表已保存为: {filename}")
        
        # 显示图表
        plt.show()
        
        # 打印统计摘要
        print(f"\nConfiguration (Dir: {safe_dir_name}, BlockNum: {block_num}, ThreadNum: {thread_num}) Statistics:")
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
    # 请使用正确的路径格式[1](@ref)
    csv_file_path = r"your_data.csv"  # 在路径前加 'r' 表示原始字符串，避免转义问题
    
    try:
        plot_individual_configurations(csv_file_path)
    except Exception as e:
        print(f"处理过程中发生错误: {e}")
        import traceback
        traceback.print_exc() # 打印完整的错误跟踪信息，有助于定位问题