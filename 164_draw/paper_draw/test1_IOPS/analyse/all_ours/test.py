import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
import os

# 设置中文字体和样式
plt.rcParams['font.sans-serif'] = ['SimHei', 'Arial Unicode MS', 'DejaVu Sans']
plt.rcParams['axes.unicode_minus'] = False
sns.set_style("whitegrid")

def plot_individual_configurations(csv_file_path):
    """
    读取CSV文件并为每个配置绘制单独的读取时间折线图
    
    参数:
    csv_file_path (str): CSV文件路径
    """
    # 读取CSV文件
    df = pd.read_csv(csv_file_path)
    
    # 检查必要的列是否存在
    required_columns = ['MSG_type', 'ReadDirName', 'BlockNum', 'ThreadNum', 'Read Time(s)']
    for col in required_columns:
        if col not in df.columns:
            print(f"错误: CSV文件中缺少必要的列 '{col}'")
            return
    
    # 提取所有不同的配置组合
    config_combinations = df[['BlockNum', 'ThreadNum']].drop_duplicates()
    
    # 为每个配置组合创建单独的图表
    for _, config in config_combinations.iterrows():
        block_num = config['BlockNum']
        thread_num = config['ThreadNum']
        
        # 筛选当前配置的数据
        subset = df[(df['BlockNum'] == block_num) & (df['ThreadNum'] == thread_num)]
        
        # 确保有足够的数据点
        if len(subset) < 1:
            print(f"警告: 配置 (BlockNum={block_num}, ThreadNum={thread_num}) 没有数据点")
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
        
        # 添加统计线
        plt.axhline(y=mean_value, color='red', linestyle='--', linewidth=1.5, 
                   label=f'avg: {mean_value:.6f}s')
        plt.axhline(y=min_value, color='green', linestyle=':', linewidth=1.5, 
                   label=f'min: {min_value:.6f}s')
        plt.axhline(y=max_value, color='blue', linestyle=':', linewidth=1.5, 
                   label=f'max: {max_value:.6f}s')
        
        # 设置图表标题和标签
        read_dir_name = subset['ReadDirName'].iloc[0] if 'ReadDirName' in subset.columns else 'Unknown'
        # plt.title(f'读取时间性能 - BlockNum: {block_num}, ThreadNum: {thread_num}\n目录: {read_dir_name}', fontsize=14)
        # plt.xlabel('运行次数', fontsize=12)
        # plt.ylabel('读取时间 (秒)', fontsize=12)
        plt.title(f'Read Time Performance - BlockNum: {block_num}, ThreadNum: {thread_num}\nDirectory: {read_dir_name}', fontsize=14)
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
        output_dir = 'performance_plots'
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        
        # 保存图表
        filename = f'{output_dir}/block_{block_num}_thread_{thread_num}_performance.png'
        plt.savefig(filename, dpi=300, bbox_inches='tight')
        print(f"图表已保存为: {filename}")
        
        # 显示图表
        plt.show()
        
        # # 打印统计摘要
        # print(f"\n配置 (BlockNum={block_num}, ThreadNum={thread_num}) 统计摘要:")
        # print(f"数据点数量: {len(subset)}")
        # print(f"平均值: {mean_value:.6f}s")
        # print(f"最小值: {min_value:.6f}s")
        # print(f"最大值: {max_value:.6f}s")
        # print(f"标准差: {subset['Read Time(s)'].std():.6f}s")
        # print("-" * 50)
        # 打印统计摘要
        print(f"\nConfiguration (BlockNum={block_num}, ThreadNum={thread_num}) Statistics:")
        print(f"Number of data points: {len(subset)}")
        print(f"Mean: {mean_value:.6f}s")
        print(f"Min: {min_value:.6f}s")
        print(f"Max: {max_value:.6f}s")
        print(f"Standard deviation: {subset['Read Time(s)'].std():.6f}s")
        print("-" * 50)
# 使用示例
if __name__ == "__main__":
    # 替换为您的CSV文件路径
    csv_file_path = "your_data.csv"  # 请修改为实际文件路径
    
    try:
        plot_individual_configurations(csv_file_path)
    except Exception as e:
        print(f"处理过程中发生错误: {e}")