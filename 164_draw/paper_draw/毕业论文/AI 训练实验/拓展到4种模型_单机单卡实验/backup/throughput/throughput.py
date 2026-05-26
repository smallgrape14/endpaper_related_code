import sys
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import argparse

# ===== 可调参数区 =====
FONT_SIZE_LABEL = 10         # 坐标轴标签字体大小
FONT_SIZE_TICK = 10          # 坐标轴刻度字体大小
FONT_SIZE_ANNOT = 10         # 数据标注字体大小
FONT_SIZE_LEGEND = 10        # 图例字体大小
FIG_WIDTH = 4                # 单个子图的图宽
FIG_HEIGHT = 3               # 单个子图的图高
GRID_ALPHA = 0.5             # 网格透明度
GRID_LINESTYLE = '--'        # 网格线型

# ===== 模型顺序定义 =====
MODEL_ORDER = ['mobilenet_v2', 'shufflenet_v2_x1_0', 'resnet18', 'resnet50']

# 设置中文字体支持
plt.rcParams['font.sans-serif'] = ['SimHei']  # 用来正常显示中文标签
plt.rcParams['axes.unicode_minus'] = False  # 用来正常显示负号

# 设置全局字体
plt.rcParams.update({
    'font.size': FONT_SIZE_TICK,
    'axes.labelsize': FONT_SIZE_LABEL,
    'legend.fontsize': FONT_SIZE_LEGEND,
    'xtick.labelsize': FONT_SIZE_TICK,
    'ytick.labelsize': FONT_SIZE_TICK
})


# ===== 全局颜色定义 =====
# 在 main 函数中根据数据实际 unique 值初始化
baseline_colors = {} 
ours_color = ''

def calculate_metrics(df):
    """
    计算 total_time 和 throughput。
    """
    df['total_time'] = df['data_avg_filter'] + df['train_avg_filter']
    # 计算吞吐量 (throughput)
    df['throughput'] = df['batchsize'] / df['total_time']
    return df

def plot_throughput(baseline_df, ours_df):
    """
    生成第一组图：比较不同 model 下，baseline 和 ours 的 throughput 随 batchsize 的变化。
    """
    # 根据指定的顺序筛选和排序模型
    models = [m for m in MODEL_ORDER if m in baseline_df['model'].unique()]

    # 设置图表样式
    sns.set_style("whitegrid")
    
    # 获取所有唯一的 batchsize 并排序，用于等距绘制和刻度标注
    all_batchsizes = sorted(baseline_df['batchsize'].unique())
    x_pos = np.arange(len(all_batchsizes)) # 生成等距的 x 坐标值
    
    # 为每个模型生成一个子图
    num_models = len(models)
    num_cols = 2
    num_rows = (num_models + num_cols - 1) // num_cols # 向上取整
    fig, axes = plt.subplots(num_rows, num_cols, figsize=(num_cols * FIG_WIDTH, num_rows * FIG_HEIGHT))
    if num_models == 1:
        axes = np.array([axes])
    axes = axes.flatten()

    for i, model in enumerate(models):
        ax = axes[i]
        
        # 筛选 baseline 和 ours 的数据
        baseline_model_df = baseline_df[baseline_df['model'] == model].sort_values('batchsize')
        ours_model_df = ours_df[ours_df['model'] == model].sort_values('batchsize')
        
        # 将 batchsize 映射到等距的 x 坐标
        batchsize_to_xpos = {bs: pos for pos, bs in enumerate(all_batchsizes)}
        
        # 绘制 baseline 的数据
        baseline_threads = sorted(baseline_model_df['threadnum'].unique())
        for thread in baseline_threads:
            thread_df = baseline_model_df[baseline_model_df['threadnum'] == thread]
            x_vals = [batchsize_to_xpos[bs] for bs in thread_df['batchsize']]
            ax.plot(x_vals, thread_df['throughput'], 
                    marker='o', linestyle='--', label=f'Baseline (worker={thread})', color=baseline_colors[thread])

        # 绘制 ours 的数据
        x_vals = [batchsize_to_xpos[bs] for bs in ours_model_df['batchsize']]
        ax.plot(x_vals, ours_model_df['throughput'], 
                marker='s', linestyle='-', linewidth=2.5, label='Ours', color=ours_color)

        # 使用可调参数设置字体和样式
        ax.set_title(f'{model} - 吞吐量', fontsize=FONT_SIZE_LABEL)
        ax.set_xlabel('批大小(Batch Size)', fontsize=FONT_SIZE_LABEL)
        ax.set_ylabel('吞吐量 (每秒处理的样本数)', fontsize=FONT_SIZE_LABEL)
        ax.set_xticks(x_pos)
        ax.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
        ax.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
        ax.legend(fontsize=FONT_SIZE_LEGEND)
        ax.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
        ax.set_ylim(bottom=0)

    # 隐藏多余的子图
    for j in range(i + 1, len(axes)):
        fig.delaxes(axes[j])
        
    plt.tight_layout()
    plt.savefig('throughput_comparison.png', dpi=300)
    plt.show()

def plot_throughput_improve_percent(baseline_df, ours_df):
    """
    生成第二组图：比较不同 model 下，ours 相对不同 baseline 配置的吞吐量提升百分比。
    """
    models = [m for m in MODEL_ORDER if m in baseline_df['model'].unique()]

    sns.set_style("whitegrid")
    
    all_batchsizes = sorted(baseline_df['batchsize'].unique())
    x_pos = np.arange(len(all_batchsizes))

    baseline_threads = sorted(baseline_df['threadnum'].unique())
    
    num_models = len(models)
    num_cols = 2
    num_rows = (num_models + num_cols - 1) // num_cols
    fig, axes = plt.subplots(num_rows, num_cols, figsize=(num_cols * FIG_WIDTH, num_rows * FIG_HEIGHT))
    if num_models == 1:
        axes = np.array([axes])
    axes = axes.flatten()

    for i, model in enumerate(models):
        ax = axes[i]
        
        ours_model_df = ours_df[ours_df['model'] == model]
        if ours_model_df.empty:
            continue
        
        batchsize_to_xpos = {bs: pos for pos, bs in enumerate(all_batchsizes)}

        for thread in baseline_threads:
            baseline_config_df = baseline_df[(baseline_df['model'] == model) & (baseline_df['threadnum'] == thread)].sort_values('batchsize')
            
            throughput_improve_percent = []
            x_vals_plot = []
            for bs in all_batchsizes:
                ours_throughput = ours_model_df[ours_model_df['batchsize'] == bs]['throughput'].values
                baseline_throughput = baseline_config_df[baseline_config_df['batchsize'] == bs]['throughput'].values
                
                if ours_throughput.size > 0 and baseline_throughput.size > 0 and baseline_throughput[0] != 0:
                    improve_percent = ((ours_throughput[0] - baseline_throughput[0]) / baseline_throughput[0]) * 100
                    throughput_improve_percent.append(improve_percent)
                    x_vals_plot.append(batchsize_to_xpos[bs])

            ax.plot(x_vals_plot, throughput_improve_percent, marker='o', 
                    linestyle='-', label=f'Baseline (worker={thread})', color=baseline_colors[thread])
            
        ax.set_title(f'{model} - 吞吐量提升百分比', fontsize=FONT_SIZE_LABEL)
        ax.set_xlabel('批大小（Batch Size）', fontsize=FONT_SIZE_LABEL)
        ax.set_ylabel('吞吐量提升百分比(%)', fontsize=FONT_SIZE_LABEL)
        ax.set_xticks(x_pos)
        ax.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
        ax.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
        ax.axhline(y=20, color='r', linestyle='--', linewidth=1, label='target (20%)')
        ax.legend(fontsize=FONT_SIZE_LEGEND)
        ax.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)

    for j in range(len(models), len(axes)):
        fig.delaxes(axes[j])
        
    plt.tight_layout()
    plt.savefig('throughput_improve_percent_comparison.png', dpi=300)
    plt.show()

def plot_combined_results(baseline_df, ours_df):
    """
    生成第三组图：
    1. 一张总的组合图：每行展示一个模型，左侧为吞吐量，右侧为吞吐量提升百分比。
    2. 每个模型单独拆分成一张图片，包含该模型的左右两个子图。
    """
    models = [m for m in MODEL_ORDER if m in baseline_df['model'].unique()]

    sns.set_style("whitegrid")
    
    all_batchsizes = sorted(baseline_df['batchsize'].unique())
    x_pos = np.arange(len(all_batchsizes))
    
    baseline_threads = sorted(baseline_df['threadnum'].unique())

    # --- 绘制总的组合图 ---
    num_models = len(models)
    fig_combined, axes_combined = plt.subplots(num_models, 2, figsize=(2 * FIG_WIDTH, num_models * FIG_HEIGHT))
    if num_models == 1:
        axes_combined = np.array([axes_combined])

    for i, model in enumerate(models):
        ax_left_combined = axes_combined[i, 0]
        ax_right_combined = axes_combined[i, 1]
        
        baseline_model_df = baseline_df[baseline_df['model'] == model].sort_values('batchsize')
        ours_model_df = ours_df[ours_df['model'] == model].sort_values('batchsize')
        
        batchsize_to_xpos = {bs: pos for pos, bs in enumerate(all_batchsizes)}
        
        # --- 绘制左侧子图 (吞吐量) ---
        for thread in baseline_threads:
            thread_df = baseline_model_df[baseline_model_df['threadnum'] == thread]
            x_vals = [batchsize_to_xpos[bs] for bs in thread_df['batchsize']]
            ax_left_combined.plot(x_vals, thread_df['throughput'], 
                                 marker='o', linestyle='--', label=f'Baseline (worker={thread})', color=baseline_colors[thread])

        x_vals = [batchsize_to_xpos[bs] for bs in ours_model_df['batchsize']]
        ax_left_combined.plot(x_vals, ours_model_df['throughput'], 
                             marker='s', linestyle='-', linewidth=2.5, label='Ours', color=ours_color)
        
        ax_left_combined.set_title(f'{model} - Throughput', fontsize=FONT_SIZE_LABEL)
        ax_left_combined.set_xlabel('Batch Size', fontsize=FONT_SIZE_LABEL)
        ax_left_combined.set_ylabel('Throughput (samples/s)', fontsize=FONT_SIZE_LABEL)
        ax_left_combined.set_xticks(x_pos)
        ax_left_combined.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
        ax_left_combined.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
        ax_left_combined.legend(fontsize=FONT_SIZE_LEGEND)
        ax_left_combined.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
        ax_left_combined.set_ylim(bottom=0)

        # --- 绘制右侧子图 (吞吐量提升百分比) ---
        for thread in baseline_threads:
            baseline_config_df = baseline_df[(baseline_df['model'] == model) & (baseline_df['threadnum'] == thread)].sort_values('batchsize')
            
            throughput_improve_percent = []
            x_vals_plot = []
            for bs in all_batchsizes:
                ours_throughput = ours_model_df[ours_model_df['batchsize'] == bs]['throughput'].values
                baseline_throughput = baseline_config_df[baseline_config_df['batchsize'] == bs]['throughput'].values
                
                if ours_throughput.size > 0 and baseline_throughput.size > 0 and baseline_throughput[0] != 0:
                    improve_percent = ((ours_throughput[0] - baseline_throughput[0]) / baseline_throughput[0]) * 100
                    throughput_improve_percent.append(improve_percent)
                    x_vals_plot.append(batchsize_to_xpos[bs])

            ax_right_combined.plot(x_vals_plot, throughput_improve_percent, marker='o', 
                                  linestyle='-', label=f'Baseline (worker={thread})', color=baseline_colors[thread])
            
        ax_right_combined.set_title(f'{model} - Throughput Improvement', fontsize=FONT_SIZE_LABEL)
        ax_right_combined.set_xlabel('Batch Size', fontsize=FONT_SIZE_LABEL)
        ax_right_combined.set_ylabel('Throughput Improvement (%)', fontsize=FONT_SIZE_LABEL)
        ax_right_combined.set_xticks(x_pos)
        ax_right_combined.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
        ax_right_combined.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
        ax_right_combined.axhline(y=20, color='r', linestyle='--', linewidth=1, label='target (20%)')
        ax_right_combined.legend(fontsize=FONT_SIZE_LEGEND)
        ax_right_combined.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
        
    plt.tight_layout()
    plt.savefig('throughput_combined_comparison_all_models.png', dpi=300)
    plt.close(fig_combined)

    # --- 额外单独拆分成不同的图片 ---
    for model in models:
        fig_single, (ax_left_single, ax_right_single) = plt.subplots(1, 2, figsize=(2 * FIG_WIDTH, FIG_HEIGHT))

        baseline_model_df = baseline_df[baseline_df['model'] == model].sort_values('batchsize')
        ours_model_df = ours_df[ours_df['model'] == model].sort_values('batchsize')
        
        batchsize_to_xpos = {bs: pos for pos, bs in enumerate(all_batchsizes)}
        
        # --- 绘制左侧子图 (吞吐量) ---
        for thread in baseline_threads:
            thread_df = baseline_model_df[baseline_model_df['threadnum'] == thread]
            x_vals = [batchsize_to_xpos[bs] for bs in thread_df['batchsize']]
            ax_left_single.plot(x_vals, thread_df['throughput'], 
                                marker='o', linestyle='--', label=f'Baseline (worker={thread})', color=baseline_colors[thread])

        x_vals = [batchsize_to_xpos[bs] for bs in ours_model_df['batchsize']]
        ax_left_single.plot(x_vals, ours_model_df['throughput'], 
                            marker='s', linestyle='-', linewidth=2.5, label='Ours', color=ours_color)
        
        ax_left_single.set_title(f'{model} - Throughput', fontsize=FONT_SIZE_LABEL)
        ax_left_single.set_xlabel('Batch Size', fontsize=FONT_SIZE_LABEL)
        ax_left_single.set_ylabel('Throughput (samples/s)', fontsize=FONT_SIZE_LABEL)
        ax_left_single.set_xticks(x_pos)
        ax_left_single.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
        ax_left_single.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
        ax_left_single.legend(fontsize=FONT_SIZE_LEGEND)
        ax_left_single.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
        ax_left_single.set_ylim(bottom=0)

        # --- 绘制右侧子图 (吞吐量提升百分比) ---
        for thread in baseline_threads:
            baseline_config_df = baseline_df[(baseline_df['model'] == model) & (baseline_df['threadnum'] == thread)].sort_values('batchsize')
            
            throughput_improve_percent = []
            x_vals_plot = []
            for bs in all_batchsizes:
                ours_throughput = ours_model_df[ours_model_df['batchsize'] == bs]['throughput'].values
                baseline_throughput = baseline_config_df[baseline_config_df['batchsize'] == bs]['throughput'].values
                
                if ours_throughput.size > 0 and baseline_throughput.size > 0 and baseline_throughput[0] != 0:
                    improve_percent = ((ours_throughput[0] - baseline_throughput[0]) / baseline_throughput[0]) * 100
                    throughput_improve_percent.append(improve_percent)
                    x_vals_plot.append(batchsize_to_xpos[bs])

            ax_right_single.plot(x_vals_plot, throughput_improve_percent, marker='o', 
                                 linestyle='-', label=f'Baseline (worker={thread})', color=baseline_colors[thread])
            
        ax_right_single.set_title(f'{model} - Throughput Improvement', fontsize=FONT_SIZE_LABEL)
        ax_right_single.set_xlabel('Batch Size', fontsize=FONT_SIZE_LABEL)
        ax_right_single.set_ylabel('Throughput Improvement (%)', fontsize=FONT_SIZE_LABEL)
        ax_right_single.set_xticks(x_pos)
        ax_right_single.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
        ax_right_single.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
        ax_right_single.axhline(y=20, color='r', linestyle='--', linewidth=1, label='target (20%)')
        ax_right_single.legend(fontsize=FONT_SIZE_LEGEND)
        ax_right_single.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)

        plt.tight_layout()
        plt.savefig(f'throughput_combined_comparison_{model}.png', dpi=300)
        plt.close(fig_single)

def main():
    """
    主函数：读取文件，处理数据，并调用绘图函数。
    """
    global baseline_colors, ours_color
    
    parser = argparse.ArgumentParser(description='Compare baseline and optimized performance data.')
    parser.add_argument('baseline_file_path', type=str, help='Path to the baseline CSV file.')
    parser.add_argument('ours_file_path', type=str, help='Path to the ours CSV file.')
    parser.add_argument('--draw_threads', type=str, help='Comma-separated list of thread numbers to draw (e.g., "1,4,8"). If not specified, all threads will be drawn.', default=None)
    
    args = parser.parse_args()
    
    try:
        baseline_df = pd.read_csv(args.baseline_file_path)
    except FileNotFoundError:
        print(f"Error: Baseline file not found at '{args.baseline_file_path}'")
        sys.exit(1)
        
    try:
        ours_df = pd.read_csv(args.ours_file_path)
    except FileNotFoundError:
        print(f"Error: Ours file not found at '{args.ours_file_path}'")
        sys.exit(1)
    
    try:
        # 过滤掉 blocknum 不为 0 的 baseline 数据
        baseline_df = baseline_df[baseline_df['blocknum'] == 0].copy()
        
        # 根据命令行参数过滤线程
        if args.draw_threads:
            threads_to_draw = [int(t) for t in args.draw_threads.split(',')]
            baseline_df = baseline_df[baseline_df['threadnum'].isin(threads_to_draw)].copy()
            if baseline_df.empty:
                print(f"Warning: No data found for specified threads: {threads_to_draw}. Skipping plotting.")
                sys.exit(0)
        
        # 统一计算 total_time 和 throughput
        baseline_df = calculate_metrics(baseline_df)
        ours_df = calculate_metrics(ours_df)
        
        # ===== 初始化全局颜色映射 =====
        # baseline_threads = sorted(baseline_df['threadnum'].unique())
        # base_palette = sns.color_palette("viridis", n_colors=len(baseline_threads))
        # baseline_colors = {thread: base_palette[i] for i, thread in enumerate(baseline_threads)}
        # ours_color = sns.color_palette("plasma")[3]
        # 定义线程配置列表
        baseline_threads = [1, 4] # 您的线程配置

        # 自定义颜色列表（与 baseline_threads 长度对应）
        custom_colors = [
            '#1f77b4', 
            '#2ca02c',
            '#ff0e22', 
            '#2ca02c',
            "#1f77b4",  # 蓝色 (matplotlib默认颜色1)
            "#ff7f0e",  # 橙色
            "#2ca02c",  # 绿色
            "#d62728",  # 红色
            "#9467bd",  # 紫色
            "#8c564b"   # 棕色
        ]

        # 创建颜色映射字典
        baseline_colors = {thread: color for thread, color in zip(baseline_threads, custom_colors)}

        # 为Ours设置单独颜色
        ours_color ='#ff7f0e'# "#e377c2"  # 粉色
        # ============================
        
        # 绘制第一组图 (吞吐量)
        plot_throughput(baseline_df, ours_df)

        # 绘制第二组图 (吞吐量提升百分比)
        plot_throughput_improve_percent(baseline_df, ours_df)
        
        # 绘制第三组图（组合图和单独拆分图）
        plot_combined_results(baseline_df, ours_df)
        
        print("Plots generated successfully!")
        print("Check 'throughput_comparison.png', 'throughput_improve_percent_comparison.png'.")
        print("Check 'throughput_combined_comparison_all_models.png' and individual model files (e.g., 'throughput_combined_comparison_mobilenet_v2.png').")

    except KeyError as e:
        print(f"Error: Missing column in one of the CSV files. Missing column: {e}")
        print("Please ensure both files have the required columns: 'data_avg_filter', 'train_avg_filter', 'model', 'batchsize', 'blocknum', 'threadnum'.")
        sys.exit(1)
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()