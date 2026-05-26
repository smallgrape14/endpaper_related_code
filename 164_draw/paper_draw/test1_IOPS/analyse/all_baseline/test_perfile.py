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

def extract_file_count_from_dirname(dir_name):
    """
    从目录名称中提取文件数量（假设末尾的数字是文件数）
    
    参数:
    dir_name (str): 目录名称
    
    返回:
    int: 提取到的文件数量
    """
    # 从目录名中提取文件数量（例如从"/mnt/beegfs/test_c"中提取数字）
    pattern = r'(\d+)$'  # 匹配末尾的数字
    match = re.search(pattern, dir_name)
    if match:
        return int(match.group())
    else:
        # 如果无法提取，尝试其他模式或使用默认值
        print(f"警告: 无法从目录名 '{dir_name}' 中提取文件数量，使用默认值10000")
        return 10000  # 默认值，您可能需要根据实际情况调整

def calculate_and_plot_avg_time_per_file(csv_file_path):
    """
    读取CSV文件，计算平均每个文件读取时间，并绘制折线图
    
    参数:
    csv_file_path (str): CSV文件路径
    """
    # 读取CSV文件
    df = pd.read_csv(csv_file_path)
    
    # 检查必要的列是否存在
    required_columns = ['dir_name', 'thread_num', 'total_read_time(ms)']
    for col in required_columns:
        if col not in df.columns:
            print(f"错误: CSV文件中缺少必要的列 '{col}'")
            print(f"CSV文件中的列: {list(df.columns)}")
            return
    
    # 从目录名中提取文件数量
    df['file_count'] = df['dir_name'].apply(extract_file_count_from_dirname)
    
    # 计算平均每个文件的读取时间（毫秒）
    df['read_time_per_file(ms)'] = df['total_read_time(ms)'] / df['file_count']
    
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
        plt.plot(range(1, len(subset) + 1), subset['read_time_per_file(ms)'].values, 
                marker='o', linewidth=2, markersize=6, color='#F8C471')
        
        # 计算统计信息
        mean_value = subset['read_time_per_file(ms)'].mean()
        min_value = subset['read_time_per_file(ms)'].min()
        max_value = subset['read_time_per_file(ms)'].max()
        std_value = subset['read_time_per_file(ms)'].std()
        
        # 添加统计线
        plt.axhline(y=mean_value, color='red', linestyle='--', linewidth=1.5, 
                   label=f'avg: {mean_value:.6f}ms')
        plt.axhline(y=min_value, color='green', linestyle=':', linewidth=1.5, 
                   label=f'min: {min_value:.6f}ms')
        plt.axhline(y=max_value, color='blue', linestyle=':', linewidth=1.5, 
                   label=f'max: {max_value:.6f}ms')
        
        # 设置图表标题和标签
        file_count = subset['file_count'].iloc[0]
        plt.title(f'Avg Time per File - ThreadNum: {thread_num}, Files: {file_count}\nDirectory: {dir_name}', fontsize=14)
        plt.xlabel('Measurement Number', fontsize=12)
        plt.ylabel('Average Time per File (ms)', fontsize=12)

        # 设置x轴刻度
        plt.xticks(range(1, len(subset) + 1))
        
        # 添加网格
        plt.grid(True, alpha=0.3)
        
        # 添加图例
        plt.legend()
        
        # 调整布局
        plt.tight_layout()
        
        # 创建输出目录
        output_dir = 'avg_time_per_file_plots'
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        
        # 生成安全的文件名
        safe_dir_name = re.sub(r'[\\/*?:"<>|]', "_", dir_name)
        filename = f'{output_dir}/{safe_dir_name}_thread_{thread_num}_avg_time.png'
        plt.savefig(filename, dpi=300, bbox_inches='tight')
        print(f"图表已保存为: {filename}")
        
        # 显示图表
        plt.show()
        
        # 打印统计摘要
        print(f"\nConfiguration (Dir: {safe_dir_name}, ThreadNum: {thread_num}) Statistics:")
        print(f"Number of measurements: {len(subset)}")
        print(f"File count: {file_count}")
        print(f"Mean time per file: {mean_value:.6f}ms")
        print(f"Min time per file: {min_value:.6f}ms")
        print(f"Max time per file: {max_value:.6f}ms")
        print(f"Standard deviation: {std_value:.6f}ms")
        print(f"Relative standard deviation: {(std_value/mean_value*100):.2f}%")
        print("-" * 50)
    
    # 保存包含新列的数据到新CSV文件
    output_csv_path = csv_file_path.replace('.csv', '_with_avg_time.csv')
    df.to_csv(output_csv_path, index=False)
    print(f"\n包含平均每文件时间的数据已保存到: {output_csv_path}")
    
    return df

# 使用示例
if __name__ == "__main__":
    # 替换为您的CSV文件路径
    csv_file_path = "/home/oem/xyp/paper_draw/test1_IOPS/analyse/all_baseline/0904_read_results_1KB_10428_s.csv"  # 请修改为实际文件路径
    
    try:
        result_df = calculate_and_plot_avg_time_per_file(csv_file_path)
        print("\n数据处理完成!")
    except Exception as e:
        print(f"处理过程中发生错误: {e}")
        import traceback
        traceback.print_exc()