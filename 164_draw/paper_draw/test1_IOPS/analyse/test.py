import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns

# 设置中文字体和样式
plt.rcParams['font.sans-serif'] = ['SimHei']#, 'Arial Unicode MS', 'DejaVu Sans']
plt.rcParams['axes.unicode_minus'] = False
sns.set_style("whitegrid")

def plot_same_blocknum(csv_file_path):
    """
    读取CSV文件并绘制相同BlockNum的读取时间折线图
    
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
    
    # 提取所有不同的BlockNum值
    blocknums = df['BlockNum'].unique()
    
    # 为每个BlockNum创建图表
    for block_num in blocknums:
        # 筛选当前BlockNum的数据
        subset = df[df['BlockNum'] == block_num]
        
        # 提取当前BlockNum下的不同ThreadNum值
        threadnums = subset['ThreadNum'].unique()
        
        # 创建图表
        plt.figure(figsize=(14, 8))
        
        # 定义颜色列表，确保每条线有不同的颜色
        colors = ['#F8C471', '#4E79A7', '#59A14F', '#E15759', '#79706E', '#B07AA1', 
                  '#F28E2B', '#FFBE7D', '#86BCB6', '#8CD17D']
        
        # 为每个ThreadNum绘制折线
        for idx, thread_num in enumerate(threadnums):
            # 筛选当前ThreadNum的数据
            thread_subset = subset[subset['ThreadNum'] == thread_num]
            
            # 确保有足够的数据点
            if len(thread_subset) < 30:
                print(f"警告: 参数组合 (BlockNum={block_num}, ThreadNum={thread_num}) 只有 {len(thread_subset)} 个数据点，不足30个")
                continue
            
            # 取前30次运行的数据
            thread_subset = thread_subset.head(30)
            
            # 计算平均值
            mean_value = thread_subset['Read Time(s)'].mean()
            
            # 绘制折线图
            color = colors[idx % len(colors)]
            plt.plot(range(1, 31), thread_subset['Read Time(s)'].values[:30], 
                    marker='o', linewidth=2, markersize=4, color=color,
                    label=f'Thread{thread_num} (Avg: {mean_value:.4f}s)')
        
        # 设置图表标题和标签
        plt.title(f'BlockNum={block_num} 的读取时间变化', fontsize=16)
        plt.xlabel('运行次数', fontsize=12)
        plt.ylabel('读取时间 (秒)', fontsize=12)
        
        # 设置x轴刻度
        plt.xticks(range(1, 31))
        
        # 添加网格
        plt.grid(True, alpha=0.3)
        
        # 添加图例
        plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
        
        # 调整布局
        plt.tight_layout()
        
        # 保存图表
        filename = f'block_{block_num}_read_time.png'
        plt.savefig(filename, dpi=300, bbox_inches='tight')
        print(f"图表已保存为: {filename}")
        
        # 显示图表
        plt.show()

# 使用示例
if __name__ == "__main__":
    # 替换为您的CSV文件路径
    csv_file_path = "/home/oem/xyp/paper_draw/test1_IOPS/analyse/1KB_read_kernel_time_0901.csv"  # 请修改为实际文件路径
    
    try:
        plot_same_blocknum(csv_file_path)
    except Exception as e:
        print(f"处理过程中发生错误: {e}")