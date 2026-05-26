
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import matplotlib.ticker as ticker
import os

# 从CSV文件读取数据
# IO_SIZE='1KB'
# IO_SIZE='4KB'
# IO_SIZE='16KB'
# IO_SIZE='1MB'
IOSize_List={'1KB','4KB','16KB','1MB'}
# IOSize_List={'1KB'} #,'4KB','16KB','1MB'}

for IO_SIZE in IOSize_List:
    TestName='test1_thread_vs_IOPS_'

    csv_filename = TestName+IO_SIZE+'.csv'
    print(csv_filename)
    # csv_filename = 'test1_thread_vs_IOPS_1KB.csv'
    data = pd.read_csv(csv_filename)

    # 从文件名提取数据大小信息(取最后一个下划线后的部分)
    # IO_SIZE = os.path.splitext(csv_filename)[0].split('_')[-1]

    # print("IO_SIZE= ",IO_SIZE)
    # 提取数据列
    thread_num = data['thread_num'].tolist()
    baseline = data['Baseline'].tolist()
    ours = data['Ours(Block=2)'].tolist()

    # 设置柱状图参数
    # x = np.arange(len(thread_num))
    width = 0.35  # 柱宽

    # 创建图表
    plt.figure(figsize=(14, 7))
    ax = plt.subplot()

    # 使用浅色配色方案
    # 绘制折线图
    line1, = ax.plot(thread_num, baseline, 
                    label='Baseline', 
                    color='#7FB3D5', 
                    marker='o', 
                    linestyle='--',
                    linewidth=2.5,
                    markersize=8)

    line2, = ax.plot(thread_num, ours, 
                    label='Ours(Block=2)', 
                    color='#F8C471', 
                    marker='s', 
                    linestyle='-',
                    linewidth=2.5,
                    markersize=8)


    ax.set_xlabel('Thread Number', fontsize=13, labelpad=12)
    ax.set_ylabel('Read IOPS (s)', fontsize=13, labelpad=12)
    ax.set_title(f'{IO_SIZE} Read IOPS for various Thread Num', fontsize=15, pad=15)
    # 添加图表标题到下方
    # plt.figtext(0.5, 0.01, 
    #            f'{IO_SIZE} Read IOPS for various Thread Num', 
    #            ha='center', fontsize=15, bbox=dict(facecolor='white', alpha=0.8))


    # ax.set_xticks(x)
    # 设置x轴为对数刻度
    ax.set_xscale('log', base=2)
    ax.set_xticks(thread_num)
    ax.set_xticklabels(thread_num)
    ax.legend(frameon=False, fontsize=12)

    # 设置log2刻度
    # plt.yscale('log', base=2)
    # ax.yaxis.set_major_formatter(ticker.FuncFormatter(lambda y, _: f'{y:.3f}'))
    ax.grid(axis='y', linestyle='--', alpha=0.4)

    ylist=[]
    for i in range(0,8):
        ylist.append(10000*i)
    ax.set_yticks(ylist)
    ax.set_yticklabels(ylist)
    print(ylist)


    # # 为所有柱子添加数值标签（水平显示）
    # def add_labels(rects):
    #     for rect in rects:
    #         height = rect.get_height()
    #         ax.annotate(f'{height:.3f}',
    #                     xy=(rect.get_x() + rect.get_width() / 2, height),
    #                     xytext=(0, 3),
    #                     textcoords="offset points",
    #                     ha='center', va='bottom', fontsize=9,
    #                     rotation=0)

    # add_labels(rects1)
    # add_labels(rects2)

    # 设置y轴范围
    # ax.set_ylim(bottom=0.1) # 这强制y轴从0.1开始

    # 自动调整布局
    plt.tight_layout(pad=2.0)

    # 显示图表
    TestName
    output_filename = f'figure/{TestName}{IO_SIZE}.png'

    # output_filename = f'performance_comparison_{IO_SIZE}.png'
    plt.savefig(output_filename, dpi=300, bbox_inches='tight')
    plt.show()
