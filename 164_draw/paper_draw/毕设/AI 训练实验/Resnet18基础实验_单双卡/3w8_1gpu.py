import matplotlib.pyplot as plt
import numpy as np
import matplotlib.pyplot as plt

plt.rcParams['font.sans-serif'] = ['WenQuanYi Micro Hei', 'SimHei', 'Microsoft YaHei']
plt.rcParams['axes.unicode_minus'] = False
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

    plt.tick_params(axis='both', which='major', labelsize=20)  # 设置主刻度标签字号

    # 标签与标题
    plt.xlabel("Batch Size", fontsize=18, fontweight='bold')
    plt.ylabel("时间减少百分比 (%)", fontsize=18, fontweight='bold')
    # plt.title("Ours vs Baseline端到端时间减少百分比", fontsize=18, fontweight='bold')
    plt.grid(True, linestyle='--', alpha=0.5)

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
    #######################################Ours vs Baseline###############################################
    ##################### 单机单卡
    # 原始数据
    
    csv_path = "csv_file/singleGPU_w8.csv"  # 替换为您的实际文件路径
    df = pd.read_csv(csv_path)

    # 确保列名正确（根据您的 CSV 格式）
    # 如果列名不同，请修改以下列名匹配
    df.columns = ['BatchSize', 'BaselineDataTime(s)', 'OursDataTime(s)', 'BaselineTrainTime(s)', 'OursTrainTime(s)']

    batch_sizes = np.array(df['BatchSize'])
    baseline_data_times = np.array(df['BaselineDataTime(s)'])
    ours_data_times     = np.array(df['OursDataTime(s)'])
    baseline_train_times = np.array(df['BaselineTrainTime(s)'])
    ours_train_times     = np.array(df['OursTrainTime(s)'])

    res_name = '3.1_w8_1gpu.png'
    # 调用绘图函数
    draw_comparison_chart(batch_sizes, baseline_data_times, ours_data_times, baseline_train_times, ours_train_times, res_name)
    print(f"图像已保存为 {res_name}")
    
    '''
    ##################### 单机双卡

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

    #######################################Ours vs GDS###############################################
    ##################### 单机单卡
        # 原始数据
    batch_sizes = [16, 32, 64, 128, 256]
    gds_data_times = [0.001822, 0.002238, 0.016774, 0.063283, 0.126923]
    ours_data_times = [0.00222, 0.00376, 0.00628, 0.01126, 0.02395]
    gds_train_times = [0.067837, 0.074451, 0.084959, 0.109804, 0.158008]
    ours_train_times = [0.015308, 0.020147, 0.028963, 0.047799, 0.085151]
    res_name = '3.3.png'
    # 调用绘图函数
    draw_comparison_chart(batch_sizes, gds_data_times, ours_data_times, gds_train_times, ours_train_times, res_name)
    print(f"图像已保存为 {res_name}")
    '''