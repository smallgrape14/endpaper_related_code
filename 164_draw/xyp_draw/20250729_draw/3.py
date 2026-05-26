import matplotlib.pyplot as plt
import numpy as np

def draw_comparison_chart(batch_sizes, baseline_data_times, ours_data_times, baseline_train_times, ours_train_times, res_name):
    baseline_total = [d + t for d, t in zip(baseline_data_times, baseline_train_times)]
    ours_total = [d + t for d, t in zip(ours_data_times, ours_train_times)]
    
    # 计算加速百分比
    reduction = [100 * (1 - ours / base) for base, ours in zip(baseline_total, ours_total)]

    # 创建等距索引（0~5）
    x_index = np.arange(len(batch_sizes))  # [0,1,2,3,4,5]

    # 绘图设置
    plt.figure(figsize=(12, 6))
    plt.plot(x_index, reduction, 'o-', color='#1f77b4', linewidth=2, markersize=8)

    # ⭐ 关键修正：将索引位置绑定到Batch Size标签
    plt.xticks(ticks=x_index,           # 等距位置 [0,1,2,3,4,5]
            labels=batch_sizes)      # 显示实际Batch Size值[5,7](@ref)

    # 标签与标题
    plt.xlabel("Batch Size", fontsize=18, fontweight='bold')
    plt.ylabel("时间减少百分比 (%)", fontsize=18, fontweight='bold')
    # plt.title("Ours vs Baseline端到端时间减少百分比", fontsize=18, fontweight='bold')
    plt.grid(True, linestyle='--', alpha=0.7)

    # 添加数据标签
    for i, pct in enumerate(reduction):
        plt.text(x_index[i], pct + 2, f'{pct:.2f}%', 
                ha='center', va='bottom', fontsize=20)

    # 纵轴范围优化
    plt.ylim(0, max(reduction) + 10)  
    plt.tight_layout()
    # plt.show()
    plt.savefig(res_name, dpi=300)  # 保存高清图

import pandas as pd
if __name__ == '__main__':
    mode = 1
    if mode == 1:#单GPU
        # 原始数据
        csv_path = "csv_file/singleGPU.csv"  # 替换为您的实际文件路径
        df = pd.read_csv(csv_path)

        # 确保列名正确（根据您的 CSV 格式）
        # 如果列名不同，请修改以下列名匹配
        df.columns = ['BatchSize', 'BaselineDataTime(s)', 'OursDataTime(s)', 'BaselineTrainTime(s)', 'OursTrainTime(s)']

        batch_sizes = np.array(df['BatchSize'])
        baseline_data_times = np.array(df['BaselineDataTime(s)'])
        ours_data_times     = np.array(df['OursDataTime(s)'])
        baseline_train_times = np.array(df['BaselineTrainTime(s)'])
        ours_train_times     = np.array(df['OursTrainTime(s)'])
        res_name = '3.1.png'
    else:#多GPU
        # 原始数据
        csv_path = "csv_file/multiGPU.csv"  # 替换为您的实际文件路径
        df = pd.read_csv(csv_path)

        # 确保列名正确（根据您的 CSV 格式）
        # 如果列名不同，请修改以下列名匹配
        df.columns = ['BatchSize', 'BaselineDataTime(s)', 'OursDataTime(s)', 'BaselineTrainTime(s)', 'OursTrainTime(s)']

        batch_sizes = np.array(df['BatchSize'])
        baseline_data_times = np.array(df['BaselineDataTime(s)'])
        ours_data_times     = np.array(df['OursDataTime(s)'])
        baseline_train_times = np.array(df['BaselineTrainTime(s)'])
        ours_train_times     = np.array(df['OursTrainTime(s)'])
    
        res_name = '3.2.png'
    
    # 调用绘图函数
    draw_comparison_chart(batch_sizes, baseline_data_times, ours_data_times, baseline_train_times, ours_train_times, res_name)
    print(f"图像已保存为 {res_name}")
