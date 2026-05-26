#!/bin/bash python

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
fig = plt.figure(num=1, figsize=(6, 4), facecolor='w', edgecolor='k')# 容器
bx = fig.add_subplot(111)
# col_types = [float, float, float, float, float,float]
col_types = [str, int, float,float,float,float,float,float,float,float,float,float,float,float,float,float,float,float,float]#, float,float,float,float,float]
font = {'family': 'Times New Roman',
        'weight': 'normal',
        'size': 26,
        }
font1 = {'family': 'Times New Roman',
        'weight': 'normal',
        'size': 24 #12,
        }

with open('Accuracy_comparision.csv') as f:
    trace = int(sys.argv[1])
    output = sys.argv[2]
    f_csv = csv.reader(f)

    headers = next(f_csv)

    # x_axis_data=['2:1','2:2','4:1','4:2','4:4','8:1','8:2','8:4','8:8','16:1','16:2','16:4','16:8']
    alg = [ 'CM', 'CU', 'AS', 'CF', 'Ladder', 'Elastic','Our']
    
    alg_length = len(alg)

    x_axis_data =[]
    
    data = [[] for _ in range(len(alg))]

    index = 0

    for row in f_csv:
        row = tuple(convert(value) for convert, value in zip(col_types, row))
        for i in range(len(alg)):
            if row[0] == alg[i] and row[2] == trace and row[1] != 1536:
                if row[0] == 'Our':
                    x_axis_data.append(row[1])
                data[i].append(row[10]) #ARE_M
        index += 1
    
    print(data[0])
    x_len = index * 0.1
    t = np.arange(0, x_len, 0.01)  # 0.1


    line4, = bx.plot(x_axis_data, data[3], color='orange', marker='^', linestyle='--',markerfacecolor='white',markersize=10,markeredgewidth=1.5, alpha=1, linewidth=1.5, label='CF')
    line1, = bx.plot(x_axis_data, data[0], color='seagreen', marker='x', linestyle='-.',markerfacecolor='white',markersize=10,markeredgewidth=1.5, alpha=1.0, linewidth=1.5, label='CM')
    line2, = bx.plot(x_axis_data, data[1], color='steelblue', marker='+', linestyle=':',markerfacecolor='white',markersize=10,markeredgewidth=1.5, alpha=1, linewidth=1.5, label='CU')
    line3, = bx.plot(x_axis_data, data[2], color='palevioletred', marker='v', linestyle='--',markerfacecolor='white',markersize=10,markeredgewidth=1.5, alpha=1, linewidth=1.5, label='AS')
    line5, = bx.plot(x_axis_data, data[4], color='indigo', marker='>', linestyle='-.',markerfacecolor='white',markersize=10,markeredgewidth=1.5, alpha=1, linewidth=1.5, label='Ladder')
    line6, = bx.plot(x_axis_data, data[5], color='#CE0505', marker='s', linestyle=':',markerfacecolor='white',markersize=10,markeredgewidth=1.5, alpha=1.0, linewidth=1.5, label='Elastic')
    line7, = bx.plot(x_axis_data, data[6], color='k', marker='o', linestyle='-', markerfacecolor='white',markersize=10,markeredgewidth=1.5, alpha=1,linewidth=2, label='Ours')


    #plt.xlabel("Memory (KiB)", font)
    #plt.ylabel("ARE", font)#@
    bx.set_xlabel("Memory (KiB)", fontdict = font)
    bx.set_ylabel("ARE_M", fontdict = font)

    bx.set_xticks(bx.get_xticks())
    bx.set_xticklabels(bx.get_xticks(), fontdict=font1)
    
    bx.set_yticks(bx.get_yticks())
    bx.set_yticklabels(bx.get_yticks(), fontdict=font1)

    
    
    bx.set_ylim(10**(-5),10**3)
    #bx.set_xlim(128,4096)
    bx.set_xscale('log', base=2)
    bx.set_yscale('log', base=10)
    bx.yaxis.set_major_locator(ticker.LogLocator(base=10.0,numticks=5))
    #bx.set_aspect(0.618)
    plt.tight_layout()
    #plt.show()
    fig.savefig(output, format='pdf', bbox_inches='tight')#@

