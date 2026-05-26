
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import matplotlib.ticker as ticker
import os
import csv
import numpy as np
import math
from matplotlib import pyplot as plt
import matplotlib.font_manager
import sys
from matplotlib.pyplot import MultipleLocator
import matplotlib.ticker as ticker
matplotlib.rcParams['pdf.fonttype'] = 42
matplotlib.rcParams['ps.fonttype'] = 42



font = {'family': 'Times New Roman',
        'weight': 'normal',
        'size': 26,
        }
font1 = {'family': 'Times New Roman',
        'weight': 'normal',
        'size': 24 #12,
        }
fig = plt.figure(num=1, figsize=(8, 6), facecolor='w', edgecolor='k')# 容器
ax = fig.add_subplot(111)

# 从CSV文件读取数据
# IO_SIZE='1KB'
# IO_SIZE='4KB'
# IO_SIZE='16KB'
# IO_SIZE='1MB'
# IOSize_List={'1KB','4KB','16KB','1MB'}
# IOSize_List={'1KB','4KB','16KB'}

IOSize_List={'1KB'} #,'4KB','16KB','1MB'}

for IO_SIZE in IOSize_List:
    
    # fig = plt.figure(num=1, figsize=(6, 4), facecolor='w', edgecolor='k')# 容器
    # ax = fig.add_subplot(111)
    ax.clear()  # 清除之前绘制的内容
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
    ours_1 = data['Ours(Block=1)'].tolist()
    ours_2 = data['Ours(Block=2)'].tolist()

    ours_4 = data['Ours(Block=4)'].tolist()
    ours_5 = data['Ours(Block=5)'].tolist()
    ours_6 = data['Ours(Block=6)'].tolist()
    ours_7 = data['Ours(Block=7)'].tolist()
    ours_8 = data['Ours(Block=8)'].tolist()




    # 设置柱状图参数
    

    # 创建图表
    # plt.figure(figsize=(14, 7))
    # ax = plt.subplot()

    # 使用浅色配色方案
    # 绘制折线图
    line1, = ax.plot(thread_num, baseline, 
                    label='Baseline', 
                    color='#7FB3D5', 
                    marker='o', 
                    linestyle='--',
                    linewidth=2.5,
                    markersize=8)

    line2, = ax.plot(thread_num, ours_1, 
                    label='Ours(Block=1)', 
                    color='#F8C471', 
                    marker='s', 
                    linestyle='-',
                    linewidth=2.5,
                    markersize=8)

    line3, = ax.plot(thread_num, ours_2, 
                    label='Ours(Block=2)', 
                    color='blue', 
                    marker='d', 
                    linestyle='-',
                    linewidth=2.5,
                    markersize=8)
    line4, = ax.plot(thread_num, ours_4, 
                    label='Ours(Block=4)', 
                    color='green', 
                    marker='x', 
                    linestyle='-',
                    linewidth=2.5,
                    markersize=8)
    line5, = ax.plot(thread_num, ours_5, 
                    label='Ours(Block=5)', 
                    color='purple', 
                    marker='<', 
                    linestyle='-',
                    linewidth=2.5,
                    markersize=8)
    line6, = ax.plot(thread_num, ours_6, 
                    label='Ours(Block=6)', 
                    color='cyan', 
                    marker='^', 
                    linestyle='-',
                    linewidth=2.5,
                    markersize=8)
    line7, = ax.plot(thread_num, ours_7, 
                    label='Ours(Block=7)', 
                    color='pink', 
                    marker='p', 
                    linestyle='-',
                    linewidth=2.5,
                    markersize=8)
    line8, = ax.plot(thread_num, ours_8, 
                    label='Ours(Block=8)', 
                    color='red', 
                    marker='s', 
                    linestyle='-',
                    linewidth=2.5,
                    markersize=8)


    ax.set_xlabel('Thread Number', fontdict = font)
    ax.set_ylabel('Read IOPS (s)', fontdict = font)
    # ax.set_title(f'{IO_SIZE} Read IOPS for various Thread Num', fontsize=15, pad=15)
    # 添加图表标题到下方
    # plt.figtext(0.5, 0.01, 
    #            f'{IO_SIZE} Read IOPS for various Thread Num', 
    #            ha='center', fontsize=15, bbox=dict(facecolor='white', alpha=0.8))


    # ax.set_xticks(x)
    # 设置x轴为对数刻度
    ax.set_xscale('log', base=2)
    ax.set_xticks(thread_num)
    x_tick_labels = [r"$2^{{{}}}$".format(int(math.log2(n))) for n in thread_num]
    # ax.set_xticklabels(thread_num,fontdict=font1)
    ax.set_xticklabels(x_tick_labels,fontdict=font1)

    ax.legend(frameon=False, fontsize=12)

    # 设置log2刻度
    # plt.yscale('log', base=2)
    # ax.yaxis.set_major_formatter(ticker.FuncFormatter(lambda y, _: f'{y:.3f}'))
    ax.grid(axis='y', linestyle='--', alpha=0.4)

    ylist=[]
    for i in range(0,8):
        ylist.append(10000*i)
    ax.set_yticks(ylist)
    ax.set_yticklabels(ylist,fontdict=font1)
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
    # TestName
    output_filename = f'figure/{TestName}{IO_SIZE}.png'
    output_filename_pdf = f'figure/{TestName}{IO_SIZE}.pdf'

    # output_filename = f'performance_comparison_{IO_SIZE}.png'
    plt.savefig(output_filename, dpi=300, bbox_inches='tight')
    # plt.show()
    fig.savefig(output_filename_pdf, format='pdf', bbox_inches='tight')

