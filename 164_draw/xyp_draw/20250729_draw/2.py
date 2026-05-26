"""_summary_
python 画图，横坐标Batch Size(要求展示时等距排布)，纵轴展示堆叠，分为左侧Baseline和右侧Ours，每个堆叠从上到下是Data Time和Train Time，并且展示的时候时间转换为在(Data Time+Train Time)的占比

|Batch Size|①Baseline Data Time(s)|②Ours Data Time(s)|Train Time(s)|
|---|---|---|---|
|16|0.00237|0.00222|0.02115|
|32|0.00540|0.00376|0.02007|
|64|0.01818|0.00628|0.03065|
|128|0.06015|0.01126|0.04788|
|256|0.23704|0.02395|0.08411|
|512|0.82261|0.04329|0.16089|
"""
import matplotlib.pyplot as plt
import numpy as np

# 计算总时间和占比
def calculate_percentages(data_times, train_times):
    totals = [d + t for d, t in zip(data_times, train_times)]
    data_percent = [d / total * 100 for d, total in zip(data_times, totals)]
    train_percent = [t / total * 100 for t, total in zip(train_times, totals)]
    return data_percent, train_percent

def draw_comparison_chart(batch_sizes, baseline_data_times, ours_data_times, baseline_train_times, ours_train_times, res_name):

    baseline_data_percent, baseline_train_percent = calculate_percentages(baseline_data_times, baseline_train_times)
    ours_data_percent, ours_train_percent = calculate_percentages(ours_data_times, ours_train_times)

    # 绘图设置
    fig, ax = plt.subplots(figsize=(12, 6))
    x = np.arange(len(batch_sizes))  # 等距横坐标位置
    width = 0.45  # 柱状图宽度 0.35

    # 绘制Baseline堆叠柱状图 (左侧)
    ax.bar(
        x - width/2, 
        baseline_train_percent, 
        width, 
        label='Baseline Train Time', 
        color='#1f77b4',  # 深蓝色
        # color='black',  # 黑色
        edgecolor='black'
    )
    ax.bar(
        x - width/2, 
        baseline_data_percent, 
        width, 
        bottom=baseline_train_percent, 
        label='Baseline Data Time', 
        color='#a1c9f1',  # 浅蓝色
        edgecolor='black'
    )

    # 绘制Ours堆叠柱状图 (右侧)
    ax.bar(
        x + width/2, 
        ours_train_percent, 
        width, 
        label='Ours Train Time', 
        color='#ff7f0e',  # 深橙色
        # color='black',  # 黑色
        edgecolor='black'
    )
    ax.bar(
        x + width/2, 
        ours_data_percent, 
        width, 
        bottom=ours_train_percent, 
        label='Ours Data Time', 
        color='#ffc27d',  # 浅橙色
        edgecolor='black'
    )

    # 添加标签和标题
    ax.set_xlabel('Batch Size', fontsize=18)
    ax.set_ylabel('时间占比 (%)', fontsize=18)
    ax.set_title('数据加载时间占比对比', fontsize=18, pad=20)
    ax.set_xticks(x)
    ax.set_xticklabels(batch_sizes)
    ax.legend(loc='upper center', ncol=4, fontsize=14)
    ax.grid(axis='y', linestyle='--', alpha=0.5)

    ax.set_ylim(0, 112)  # 设置y轴范围为0到115%

    # 添加数据标签
    fontsize_text = 16
    for i in range(len(batch_sizes)):
        # Baseline标签
        ax.text(
            x[i] - width/2, 
            baseline_train_percent[i]/2, 
            # f"{baseline_data_times[i] + train_times[i]:.3f}s\n({baseline_train_percent[i]:.1f}%)",
            f"train:\n({baseline_train_percent[i]:.1f}%)",
            ha='center', 
            va='center',
            fontsize=fontsize_text,
            color='white'
        )
        ax.text(
            x[i] - width/2, 
            baseline_train_percent[i] + baseline_data_percent[i]/2, 
            f"data:\n{baseline_data_times[i]*1000:.1f}ms\n({baseline_data_percent[i]:.1f}%)",
            ha='center', 
            va='center',
            fontsize=fontsize_text,
            color='black'
        )
        
        # Ours标签
        ax.text(
            x[i] + width/2, 
            ours_train_percent[i]/2, 
            # f"{ours_data_times[i] + train_times[i]:.3f}s\n({ours_data_percent[i]:.1f}%)",
            f"train:\n({ours_train_percent[i]:.1f}%)",
            ha='center', 
            va='center',
            fontsize=fontsize_text,
            color='white'
        )
        ax.text(
            x[i] + width/2, 
            ours_train_percent[i] + ours_data_percent[i]/2, 
            f"data:\n{ours_data_times[i]*1000:.1f}ms\n({ours_data_percent[i]:.1f}%)",
            ha='center', 
            va='center',
            fontsize=fontsize_text,
            color='black'
        )

    plt.tight_layout()
    plt.savefig(res_name, dpi=300)  # 保存高清图


if __name__ == '__main__':
    mode = 2
    if mode == 1:#单GPU
        # 原始数据
        batch_sizes = [16, 32, 64, 128, 256, 512]
        baseline_data_times = [0.00237, 0.00540, 0.01818, 0.06015, 0.23704, 0.82261]
        ours_data_times = [0.00222, 0.00376, 0.00628, 0.01126, 0.02395, 0.04329]
        baseline_train_times = [0.02115, 0.02007, 0.03065, 0.04788, 0.08411, 0.16089]
        ours_train_times = [0.02115, 0.02007, 0.03065, 0.04788, 0.08411, 0.16089]
        res_name = '2.1.png'
    else:#多GPU
        # 原始数据
        batch_sizes = [16, 32, 64, 128, 256, 512]
        baseline_data_times = [0.02178, 0.04543, 0.10038, 0.23121, 0.56707, 1.31692]
        ours_data_times = [0.00726, 0.00758, 0.00624, 0.01276, 0.02406, 0.04589]
        baseline_train_times = [0.025301712, 0.026498436, 0.037202237, 0.054799401, 0.096635524, 0.159944416]
        ours_train_times = [0.021990604, 0.022727877, 0.033632204, 0.050314642, 0.084026253, 0.15494151]
        res_name = '2.2.png'
    
    # 调用绘图函数
    draw_comparison_chart(batch_sizes, baseline_data_times, ours_data_times, baseline_train_times, ours_train_times, res_name)
    print(f"图像已保存为 {res_name}")