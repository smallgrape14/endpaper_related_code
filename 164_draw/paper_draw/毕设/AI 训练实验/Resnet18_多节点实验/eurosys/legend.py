import os
import sys
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import argparse

# ===== 可调参数区 =====
FONT_SIZE_LEGEND = 28  # 图例字体大小
FONT_SIZE_LABEL = 32   # 坐标轴字体大小（虽然后面隐藏了轴，但保持一致性）

# ===== 字体配置 =====
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['DejaVu Sans', 'SimSun']
plt.rcParams['mathtext.fontset'] = 'dejavusans'
plt.rcParams['axes.unicode_minus'] = False

# 设置全局字体
plt.rcParams.update({
    'font.size': FONT_SIZE_LEGEND,
    'axes.labelsize': FONT_SIZE_LABEL,
    'legend.fontsize': FONT_SIZE_LEGEND,
})

# ===== 全局颜色定义 (直接从您的原代码复制) =====
batchsize_colors = sns.color_palette("husl", 6)

def create_throughput_legend():
    """
    创建与 plot_throughput_by_nodes 函数完全一致的图例
    """
    # 1. 定义与原代码完全相同的 Marker 和 Linestyle 列表
    marker_list = ['o', 's', 'D', '^', 'v', 'P', 'X', '*'] 
    linestyle_list = ['-', '--', '-.', ':', '-', '--', '-.', ':'] 
    
    # 2. 模拟一些Batchsize数值，用于生成图例文本
    # 假设我们有 5 个 batchsize，对应原代码中的循环
    sample_batchsizes = [16, 32, 64, 128, 256]
    
    # 3. 创建图形和轴
    fig, ax = plt.subplots(figsize=(8.8, 2.5)) # 调整高度，让图例看起来像个横条
    
    # 4. 隐藏坐标轴（我们只需要纯净的图例）
    ax.set_axis_off()
    
    # 5. 手动创建图例句柄 (Handles) 和标签 (Labels)
    handles = []
    labels = []
    
    # 限制数量，防止超出颜色列表
    num_items = min(len(sample_batchsizes), len(batchsize_colors), len(marker_list))
    
    for i in range(num_items):
        bs = sample_batchsizes[i]
        color = batchsize_colors[i % len(batchsize_colors)]
        marker = marker_list[i % len(marker_list)]
        linestyle = linestyle_list[i % len(linestyle_list)]
        
        # 创建一个虚拟的线对象 (Line2D) 作为图例句柄
        # 参数：x, y, 颜色, 线型, 标记, 标记大小, 线宽
        line_handle = plt.Line2D([], [], 
                                color=color, 
                                linestyle=linestyle, 
                                marker=marker, 
                                markersize=10, 
                                linewidth=2.5,
                                label=f'Batchsize={bs}')
        
        handles.append(line_handle)
        labels.append(f'Batchsize={bs}')

    # 6. 生成图例
    # loc='center' 表示居中，frameon=False 表示去掉边框，ncol 可以设置列数
    legend = ax.legend(handles=handles, labels=labels,
                        fontsize=13, 
                        loc='center', 
                        # frameon=False, 
                        ncol=3, # 根据需要可以改成 ncol=5 变成横向一排
                        handlelength=2.0, # 控制线段长度
                        handletextpad=0.5, # 控制图例句柄和文本之间的距离
                        columnspacing=0.5, # 控制列之间的间距
                        frameon=True,  # ✅ 开启边框
                        fancybox=True,  # ✅ 使用圆角边框
                        # shadow=False,  # 无阴影
                        markerscale=0.8,  # 标记大小缩放
                        edgecolor="#cccccc",  # ✅ 边框颜色
                        facecolor='white') # 控制文字间距

    # 7. 保存为 PNG 和 PDF
    plt.tight_layout()
    plt.savefig('throughput_legend.png', dpi=300, bbox_inches='tight', pad_inches=0.1)
    plt.savefig('throughput_legend.pdf', dpi=300, bbox_inches='tight', pad_inches=0.1)
    
    print("图例已保存为 throughput_legend.png 和 throughput_legend.pdf")
    plt.close(fig)

def main():
    parser = argparse.ArgumentParser(description='Generate only the throughput legend.')
    # 即使不需要数据，也保留原参数接口以防万一，或者可以直接调用
    parser.add_argument('data_dir', type=str, nargs='?', default='.', help='Dummy argument for compatibility.')
    
    args = parser.parse_args()
    
    # 直接调用生成图例的函数
    create_throughput_legend()

if __name__ == '__main__':
    main()