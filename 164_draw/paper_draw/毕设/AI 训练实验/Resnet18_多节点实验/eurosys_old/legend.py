import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns
from matplotlib.patches import Patch
import os

# ===== 可调参数区 =====
FIG_WIDTH = 8.8
FIG_HEIGHT = 7.6
FONT_SIZE_LABEL = 28
FONT_SIZE_TICK = 26
FONT_SIZE_LEGEND = 20
FONT_SIZE_TITLE = 28
GRID_ALPHA = 0.3
GRID_LINESTYLE = "--"

# 假设的模型顺序和颜色
MODEL_ORDER = ['ResNet50', 'VGG16', 'BERT-base', 'GPT2-small']
batchsize_colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b']

# ===== 字体配置 =====
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['DejaVu Sans', 'SimSun']
plt.rcParams['mathtext.fontset'] = 'dejavusans'
plt.rcParams['axes.unicode_minus'] = False
plt.rcParams['axes.formatter.use_mathtext'] = False

# 设置全局字体
plt.rcParams.update({
    'font.size': FONT_SIZE_TICK,
    'axes.labelsize': FONT_SIZE_LABEL,
    'legend.fontsize': FONT_SIZE_LEGEND,
    'xtick.labelsize': FONT_SIZE_TICK,
    'ytick.labelsize': FONT_SIZE_TICK
})

def create_batchsize_legend():
    """
    创建 batchsize 图例（折线图的线条+标记）
    模拟: Batchsize=32, 64, 128, 256, 512, 1024
    """
    # 创建虚拟图形获取图例句柄
    fig, ax = plt.subplots(figsize=(1, 1), dpi=100)
    
    # batchsize 列表
    batchsizes = [32, 64, 128, 256, 512, 1024]
    
    # 创建虚拟线条
    handles = []
    labels = []
    
    for i, bs in enumerate(batchsizes):
        if i < len(batchsize_colors):
            # 创建带有标记的线条
            line, = ax.plot([0, 1], [0, 1], 
                           color=batchsize_colors[i], 
                           marker='o', 
                           linestyle='-',
                           markersize=8,
                           label=f'Batchsize={bs}')
            handles.append(line)
            labels.append(f'Batchsize={bs}')
    
    plt.close(fig)
    
    return handles, labels

def create_model_legend():
    """
    创建模型图例（如果需要的话）
    """
    # 使用颜色方块表示模型
    handles = []
    labels = []
    
    # 假设的模型颜色
    model_colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728']
    
    for i, model in enumerate(MODEL_ORDER):
        if i < len(model_colors):
            # 创建颜色方块
            patch = Patch(facecolor=model_colors[i], edgecolor='black', label=model)
            handles.append(patch)
            labels.append(model)
    
    return handles, labels

def save_legend_only(handles, labels, output_name="multi_node_legend", ncol=3, frame=True):
    """
    保存单独的图例图片
    
    参数:
    - handles: 图例句柄列表
    - labels: 图例标签列表
    - output_name: 输出文件名（不含后缀）
    - ncol: 图例列数
    - frame: 是否显示边框
    """
    # 创建专门的图例图形
    # 根据图例项数量和图例列数计算合适的图形大小
    legend_height = 0.8
    legend_width = min(ncol * 3, len(handles) * 3)
    
    fig_legend = plt.figure(figsize=(legend_width, legend_height), dpi=100)
    ax_legend = fig_legend.add_subplot(111)
    ax_legend.axis('off')
    
    # 创建图例
    legend = ax_legend.legend(
        handles=handles,
        labels=labels,
        loc='center',
        ncol=ncol,
        fontsize=FONT_SIZE_LEGEND,
        frameon=frame,  # 是否显示边框
        fancybox=True,  # 圆角边框
        shadow=False,   # 无阴影
        edgecolor='#CCCCCC' if frame else 'none',  # 边框颜色
        facecolor='white' if frame else 'none',    # 背景颜色
        framealpha=0.9,  # 透明度
        borderpad=0.8,   # 边框内部填充
        handletextpad=0.5,  # 图例句柄和文本间距
        columnspacing=1.2,  # 列间距
        labelspacing=0.3,  # 标签间距
        markerscale=1.2,  # 标记大小缩放
        handlelength=2.0  # 图例句柄长度
    )
    
    # 调整布局
    plt.tight_layout(pad=0.1)
    
    # 保存PNG
    png_filename = f"{output_name}.png"
    plt.savefig(png_filename, dpi=300, bbox_inches='tight', pad_inches=0.1, facecolor='white')
    
    # 保存PDF
    pdf_filename = f"{output_name}.pdf"
    plt.savefig(pdf_filename, dpi=300, bbox_inches='tight', pad_inches=0.1, facecolor='white')
    
    plt.close()
    
    print(f"✅ 图例PNG已保存为: {png_filename}")
    print(f"✅ 图例PDF已保存为: {pdf_filename}")
    
    return png_filename, pdf_filename

def main():
    """主函数：生成各种图例"""
    print("正在生成图例文件...")
    
    # 1. 生成 Batchsize 图例
    print("\n1. 生成 Batchsize 图例...")
    batchsize_handles, batchsize_labels = create_batchsize_legend()
    save_legend_only(batchsize_handles, batchsize_labels, 
                    output_name="batchsize_legend", ncol=3, frame=True)
    
    # 2. 生成模型图例
    print("\n2. 生成模型图例...")
    model_handles, model_labels = create_model_legend()
    save_legend_only(model_handles, model_labels, 
                    output_name="model_legend", ncol=2, frame=True)
    
    # 3. 生成组合图例示例（如果需要）
    print("\n3. 生成组合图例（示例）...")
    # 组合不同样式的图例项
    fig, ax = plt.subplots(figsize=(1, 1), dpi=100)
    
    # 创建不同类型的图例句柄
    handles = []
    labels = []
    
    # 线条+标记样式
    line1, = ax.plot([0, 1], [0, 1], 'b-o', label='Line with marker', markersize=8)
    handles.append(line1)
    labels.append('Line with marker')
    
    # 虚线样式
    line2, = ax.plot([0, 1], [0, 1], 'r--s', label='Dashed line', markersize=8)
    handles.append(line2)
    labels.append('Dashed line')
    
    # 实线样式
    line3, = ax.plot([0, 1], [0, 1], 'g-^', label='Solid line', markersize=8)
    handles.append(line3)
    labels.append('Solid line')
    
    # 颜色块样式
    patch = Patch(facecolor='orange', edgecolor='black', label='Color patch')
    handles.append(patch)
    labels.append('Color patch')
    
    plt.close(fig)
    
    save_legend_only(handles, labels, output_name="combined_legend", ncol=2, frame=True)
    
    print("\n✅ 所有图例生成完成！")
    
    # 显示生成的文件列表
    print("\n📁 生成的文件:")
    for file in os.listdir('.'):
        if file.endswith(('.png', '.pdf')) and 'legend' in file:
            print(f"  - {file}")

if __name__ == "__main__":
    main()