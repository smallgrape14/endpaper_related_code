import csv
import numpy as np
import math
from matplotlib import pyplot as plt
import matplotlib.font_manager
import sys
from matplotlib.pyplot import MultipleLocator
import matplotlib.ticker as ticker
import pandas as pd
import os


matplotlib.rcParams['pdf.fonttype'] = 42
matplotlib.rcParams['ps.fonttype'] = 42
plt.figure(num=1, figsize=(8, 6), facecolor='w', edgecolor='k')# 容器
ax = plt.subplot(111)

import matplotlib.font_manager
print([f.name for f in matplotlib.font_manager.fontManager.ttflist if 'Times' in f.name])
# from matplotlib.font_manager import FontManager
# fm = FontManager()
# mat_fonts = set(f.name for f in fm.ttflist)
# print(mat_fonts)
# import matplotlib
# print(matplotlib.matplotlib_fname())

# # 创建图表
# plt.figure(figsize=(14, 7))
# ax = plt.subplot()

font = {'family': 'Times New Roman',
        'weight': 'normal',
        'size': 26 #26,
        }
font1 = {'family': 'Times New Roman',
        'weight': 'normal',
        'size': 24 #24 #12,
        }
# 自定义标签生成器
def log2_formatter(val, pos):
    exponent = int(np.log2(val))
    return r"$2^{{{}}}$".format(exponent)
def log4_formatter(val, pos):
    if pos == 4:
        return f'{val:.2f}'
    else:
        return f'{val:.3f}'
    # exponent = int(np.log2(val))
    # return r"$2^{{{}}}$".format(exponent)
def log2_formatter_y(val, pos):
    if val <= 0:  # 避免非正数报错
        return "0"
    exponent = math.log2(val)
    # 若指数为整数则省略小数位（如 2^3 而非 2^{3.0}）
    if exponent.is_integer():
        return r"$2^{{{}}}$".format(int(exponent))
    return r"$2^{{{:.1f}}}$".format(exponent)  # 非整数保留1位小数
# 从CSV文件读取数据
# IO_SIZE='1KB'
# IO_SIZE='4KB'
# IO_SIZE='16KB'
# IO_SIZE='1MB'
# IOSize_List={'1KB','4KB','16KB','1MB'}
IOSize_List={'1KB','4KB','16KB'}

for IO_SIZE in IOSize_List:
    TestName='test1_thread_vs_latancy_'

    csv_filename = TestName+IO_SIZE+'.csv'
    print(csv_filename)
    # csv_filename = 'test1_thread_vs_latancy_1KB.csv'
    data = pd.read_csv(csv_filename)

    # 从文件名提取数据大小信息(取最后一个下划线后的部分)
    # IO_SIZE = os.path.splitext(csv_filename)[0].split('_')[-1]

    # print("IO_SIZE= ",IO_SIZE)
    # 提取数据列
    thread_num = data['thread_num'].tolist()
    baseline = data['Baseline'].tolist()
    ours = data['Ours(Block=2)'].tolist()

    # 设置柱状图参数
    x = np.arange(len(thread_num))
    width = 0.35  # 柱宽



    # 使用浅色配色方案
    rects1 = ax.bar(x - width/2, baseline, width, label='Baseline', color='#7FB3D5', alpha=0.85)
    rects2 = ax.bar(x + width/2, ours, width, label='Ours(Block=2)', color='#F8C471', alpha=0.85)

    # 添加标签和标题(使用提取的IO_SIZE)
    # ax.set_xlabel('Thread Number', fontsize=13, labelpad=12)
    # ax.set_ylabel('Read Latancy (s)', fontsize=13, labelpad=12)
    # ax.set_title(f'{IO_SIZE} Read Latancy for various Thread Num', fontsize=15, pad=15)
    
    ax.set_xlabel('Read Concurrency',  fontdict = font)
    ax.set_ylabel('Read Latancy (s)',  fontdict = font)
    # ax.set_title(f'{IO_SIZE} Read Latancy for various Thread Num', fontsize=15, pad=15)

    # plt.xscale('log', base=4)
    
    xlist=[]
    for i in range(10):
        xlist.append(pow(2,i))
    ax.set_xticks(x)
    # print(x)
    x_tick_labels = [r"$2^{{{}}}$".format(int(math.log2(n))) for n in thread_num]
    ax.set_xticklabels(x_tick_labels,fontdict=font1)
    # ax.set_xticklabels(thread_num,fontdict=font1)

    # ax.yaxis.set_major_locator(xlist)
    # ax.xaxis.set_major_formatter(ticker.FormatStrFormatter('%.1f'))
    # ax.set_xticks(x)
    # ax.set_xticklabels(thread_num,fontdict=font1)

    plt.yscale('log', base=2)
    yticklist=[]
    for i in range(-3,4):
        yticklist.append(pow(2,i))
    # y_tick_labels = [r"$2^{{{}}}$".format(n) for n in range(-3,4)]
    
    # plt.yscale('log', base=4)
    # yticklist=[]
    # for i in range(-2,3):
    #     yticklist.append(pow(4,i))
    
    ax.set_yticks(yticklist)
    ax.set_yticklabels(yticklist,fontdict=font1)
    # ax.set_yticks(ax.get_yticks())
    # ax.set_yticklabels(ax.get_yticks(),fontdict=font1)

    #图例
    # ax.legend(frameon=False, fontsize=12)

    # 设置log2刻度
    

    # ax.yaxis.set_major_formatter(ticker.FuncFormatter(lambda y, _: f'{y:.3f}'))
    
    # ax.yaxis.set_major_formatter(ticker.FuncFormatter(log4_formatter))

    # ax.xaxis.set_major_formatter(ticker.FuncFormatter(log2_formatter))
    ax.yaxis.set_major_formatter(ticker.FuncFormatter(log2_formatter_y))
    # ax.yaxis.set_major_locator(ticker.LogLocator(base=2.0, numticks=4))
    ax.yaxis.set_major_locator(ticker.LogLocator(base=2.0, numticks=8))

    # 设置格式化器（强制保留两位小数）
    # ax.yaxis.set_major_formatter(ticker.FormatStrFormatter('%.3f'))  # 格式化为两位小数
    # plt.xscale('log', base=2)

    # ax.xaxis.set_major_locator(ticker.LogLocator(base=2.0, numticks=1))

    ax.grid(axis='y', linestyle='--', alpha=0.4)

    # 为所有柱子添加数值标签（水平显示）
    def add_labels(rects):
        for rect in rects:
            height = rect.get_height()
            ax.annotate(f'{height:.3f}',
                        xy=(rect.get_x() + rect.get_width() / 2, height),
                        xytext=(0, 3),
                        textcoords="offset points",
                        ha='center', va='bottom', fontsize=9,
                        rotation=0)

    # add_labels(rects1)
    # add_labels(rects2)

    # 设置y轴范围
    ax.set_ylim(bottom=-0.1)

    # ax.yaxis.set_major_locator(ticker.LogLocator(base=10.0,numticks=5))
    # 自动调整布局
    plt.tight_layout(pad=2.0)

    # 显示图表
    # TestName
    output_filename = f'figure/{TestName}{IO_SIZE}.png'
    output_filename_pdf = f'figure/{TestName}{IO_SIZE}.pdf'


    # output_filename = f'performance_comparison_{IO_SIZE}.png'
    plt.savefig(output_filename, dpi=300, bbox_inches='tight')
    # plt.show()
    plt.savefig(output_filename_pdf, format='pdf', bbox_inches='tight')
    
