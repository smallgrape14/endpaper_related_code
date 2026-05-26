import sys
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import argparse
import os

# ===== 可调参数区 =====
FONT_SIZE_LABEL = 10         # 坐标轴标签字体大小
FONT_SIZE_TICK = 10          # 坐标轴刻度字体大小
FONT_SIZE_ANNOT = 10         # 数据标注字体大小
FONT_SIZE_LEGEND = 9         # 图例字体大小
FIG_WIDTH = 8                # 单个图的宽度
FIG_HEIGHT = 4               # 单个图的高度
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

def save_individual_throughput_plots(baseline_df, ours_df, output_dir="individual_plots"):
    """
    为每个模型生成并保存吞吐量独立图形文件
    
    参数:
    baseline_df: baseline数据
    ours_df: ours数据
    output_dir: 输出目录
    """
    # 创建输出目录
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    # 根据指定的顺序筛选和排序模型
    models = [m for m in MODEL_ORDER if m in baseline_df['model'].unique()]
    
    # 获取所有唯一的 batchsize 并排序
    all_batchsizes = sorted(baseline_df['batchsize'].unique())
    x_pos = np.arange(len(all_batchsizes))
    
    # 设置图表样式
    sns.set_style("whitegrid")
    
    for model in models:
        print(f"正在处理模型 {model} 的吞吐量图...")
        
        # 创建独立图形
        fig, ax = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))
        
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
                    marker='o', linestyle='--', label=f'Baseline (worker={thread})', 
                    color=baseline_colors[thread])

        # 绘制 ours 的数据
        x_vals = [batchsize_to_xpos[bs] for bs in ours_model_df['batchsize']]
        ax.plot(x_vals, ours_model_df['throughput'], 
                marker='s', linestyle='-', linewidth=2.5, label='Ours', 
                color=ours_color)

        # 设置图表属性
        ax.set_title(f'{model} - 吞吐量', fontsize=FONT_SIZE_LABEL+2, pad=15)
        ax.set_xlabel('批大小(Batch Size)', fontsize=FONT_SIZE_LABEL)
        ax.set_ylabel('吞吐量 (每秒处理的样本数)', fontsize=FONT_SIZE_LABEL)
        ax.set_xticks(x_pos)
        ax.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
        ax.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
        ax.legend(loc='best', fontsize=FONT_SIZE_LEGEND, frameon=False)
        ax.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
        ax.set_ylim(bottom=0)
        
        plt.tight_layout()
        
        # 保存图形
        filename = f"{output_dir}/throughput_{model}.png"
        plt.savefig(filename, dpi=300, bbox_inches='tight')
        plt.close(fig)
        print(f"  已保存: {filename}")

def save_individual_improvement_plots(baseline_df, ours_df, output_dir="individual_plots"):
    """
    为每个模型生成并保存吞吐量提升百分比独立图形文件
    
    参数:
    baseline_df: baseline数据
    ours_df: ours数据
    output_dir: 输出目录
    """
    # 创建输出目录
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    # 根据指定的顺序筛选和排序模型
    models = [m for m in MODEL_ORDER if m in baseline_df['model'].unique()]
    
    # 获取所有唯一的 batchsize 并排序
    all_batchsizes = sorted(baseline_df['batchsize'].unique())
    x_pos = np.arange(len(all_batchsizes))
    
    # 设置图表样式
    sns.set_style("whitegrid")
    
    for model in models:
        print(f"正在处理模型 {model} 的吞吐量提升图...")
        
        # 创建独立图形
        fig, ax = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))
        
        ours_model_df = ours_df[ours_df['model'] == model]
        if ours_model_df.empty:
            continue
        
        batchsize_to_xpos = {bs: pos for pos, bs in enumerate(all_batchsizes)}
        
        # 获取 baseline 的所有线程配置
        baseline_threads = sorted(baseline_df[baseline_df['model'] == model]['threadnum'].unique())
        
        # 绘制不同 baseline 配置的吞吐量提升百分比
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
                    linestyle='-', label=f'Baseline (worker={thread})', 
                    color=baseline_colors[thread])
            
        # 设置图表属性
        ax.set_title(f'{model} - 吞吐量提升百分比', fontsize=FONT_SIZE_LABEL+2, pad=15)
        ax.set_xlabel('批大小(Batch Size)', fontsize=FONT_SIZE_LABEL)
        ax.set_ylabel('吞吐量提升百分比(%)', fontsize=FONT_SIZE_LABEL)
        ax.set_xticks(x_pos)
        ax.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
        ax.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
        ax.axhline(y=20, color='r', linestyle='--', linewidth=1, label='目标 (20%)')
        ax.legend(loc='best', fontsize=FONT_SIZE_LEGEND, frameon=False)
        ax.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
        
        plt.tight_layout()
        
        # 保存图形
        filename = f"{output_dir}/throughput_improvement_{model}.png"
        plt.savefig(filename, dpi=300, bbox_inches='tight')
        plt.close(fig)
        print(f"  已保存: {filename}")

def save_all_models_throughput_plot(baseline_df, ours_df, output_dir="individual_plots"):
    """
    生成并保存所有模型的吞吐量组合图（每个模型一个子图）
    
    参数:
    baseline_df: baseline数据
    ours_df: ours数据
    output_dir: 输出目录
    """
    print("正在生成所有模型的吞吐量组合图...")
    
    # 根据指定的顺序筛选和排序模型
    models = [m for m in MODEL_ORDER if m in baseline_df['model'].unique()]
    
    # 获取所有唯一的 batchsize 并排序
    all_batchsizes = sorted(baseline_df['batchsize'].unique())
    x_pos = np.arange(len(all_batchsizes))
    
    # 设置图表样式
    sns.set_style("whitegrid")
    
    # 计算图形尺寸
    num_models = len(models)
    num_cols = 2
    num_rows = (num_models + num_cols - 1) // num_cols
    fig, axes = plt.subplots(num_rows, num_cols, figsize=(num_cols * FIG_WIDTH, num_rows * FIG_HEIGHT))
    
    # 如果只有一个子图，axes 不是一个数组，需要处理
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
                    marker='o', linestyle='--', label=f'Baseline (worker={thread})', 
                    color=baseline_colors[thread])

        # 绘制 ours 的数据
        x_vals = [batchsize_to_xpos[bs] for bs in ours_model_df['batchsize']]
        ax.plot(x_vals, ours_model_df['throughput'], 
                marker='s', linestyle='-', linewidth=2.5, label='Ours', 
                color=ours_color)

        # 设置图表属性
        ax.set_title(f'{model} - 吞吐量', fontsize=FONT_SIZE_LABEL)
        ax.set_xlabel('批大小(Batch Size)', fontsize=FONT_SIZE_LABEL)
        ax.set_ylabel('吞吐量 (每秒处理的样本数)', fontsize=FONT_SIZE_LABEL)
        ax.set_xticks(x_pos)
        ax.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
        ax.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
        ax.legend(loc='best', fontsize=FONT_SIZE_LEGEND, frameon=False)
        ax.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
        ax.set_ylim(bottom=0)
    
    # 隐藏多余的子图
    for j in range(i + 1, len(axes)):
        fig.delaxes(axes[j])
    
    plt.tight_layout()
    
    # 保存图形
    filename = f"{output_dir}/throughput_all_models.png"
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f"  已保存: {filename}")

def save_all_models_improvement_plot(baseline_df, ours_df, output_dir="individual_plots"):
    """
    生成并保存所有模型的吞吐量提升百分比组合图（每个模型一个子图）
    
    参数:
    baseline_df: baseline数据
    ours_df: ours数据
    output_dir: 输出目录
    """
    print("正在生成所有模型的吞吐量提升组合图...")
    
    # 根据指定的顺序筛选和排序模型
    models = [m for m in MODEL_ORDER if m in baseline_df['model'].unique()]
    
    # 获取所有唯一的 batchsize 并排序
    all_batchsizes = sorted(baseline_df['batchsize'].unique())
    x_pos = np.arange(len(all_batchsizes))
    
    # 设置图表样式
    sns.set_style("whitegrid")
    
    # 计算图形尺寸
    num_models = len(models)
    num_cols = 2
    num_rows = (num_models + num_cols - 1) // num_cols
    fig, axes = plt.subplots(num_rows, num_cols, figsize=(num_cols * FIG_WIDTH, num_rows * FIG_HEIGHT))
    
    # 如果只有一个子图，axes 不是一个数组，需要处理
    if num_models == 1:
        axes = np.array([axes])
    axes = axes.flatten()
    
    for i, model in enumerate(models):
        ax = axes[i]
        
        ours_model_df = ours_df[ours_df['model'] == model]
        if ours_model_df.empty:
            continue
        
        batchsize_to_xpos = {bs: pos for pos, bs in enumerate(all_batchsizes)}
        
        # 获取 baseline 的所有线程配置
        baseline_threads = sorted(baseline_df[baseline_df['model'] == model]['threadnum'].unique())
        
        # 绘制不同 baseline 配置的吞吐量提升百分比
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
                    linestyle='-', label=f'Baseline (worker={thread})', 
                    color=baseline_colors[thread])
            
        # 设置图表属性
        ax.set_title(f'{model} - 吞吐量提升百分比', fontsize=FONT_SIZE_LABEL)
        ax.set_xlabel('批大小(Batch Size)', fontsize=FONT_SIZE_LABEL)
        ax.set_ylabel('吞吐量提升百分比(%)', fontsize=FONT_SIZE_LABEL)
        ax.set_xticks(x_pos)
        ax.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
        ax.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
        ax.axhline(y=20, color='r', linestyle='--', linewidth=1, label='目标 (20%)')
        ax.legend(loc='best', fontsize=FONT_SIZE_LEGEND, frameon=False)
        ax.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
    
    # 隐藏多余的子图
    for j in range(i + 1, len(axes)):
        fig.delaxes(axes[j])
    
    plt.tight_layout()
    
    # 保存图形
    filename = f"{output_dir}/throughput_improvement_all_models.png"
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f"  已保存: {filename}")

def main():
    """
    主函数：读取文件，处理数据，并调用绘图函数。
    """
    global baseline_colors, ours_color
    
    parser = argparse.ArgumentParser(description='Compare baseline and optimized performance data.')
    parser.add_argument('baseline_file_path', type=str, help='Path to the baseline CSV file.')
    parser.add_argument('ours_file_path', type=str, help='Path to the ours CSV file.')
    parser.add_argument('--draw_threads', type=str, help='Comma-separated list of thread numbers to draw (e.g., "1,4,8"). If not specified, all threads will be drawn.', default=None)
    parser.add_argument('--output_dir', type=str, help='Directory to save individual plot files', default='individual_plots')
    
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
        baseline_threads = sorted(baseline_df['threadnum'].unique())
        
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
        
        print(f"开始生成图表，输出目录: {args.output_dir}")
        print("=" * 50)
        
        # 1. 为每个模型生成吞吐量独立图
        save_individual_throughput_plots(baseline_df, ours_df, args.output_dir)
        
        print("-" * 50)
        
        # 2. 为每个模型生成吞吐量提升独立图
        save_individual_improvement_plots(baseline_df, ours_df, args.output_dir)
        
        print("-" * 50)
        
        # 3. 生成所有模型的吞吐量组合图
        save_all_models_throughput_plot(baseline_df, ours_df, args.output_dir)
        
        print("-" * 50)
        
        # 4. 生成所有模型的吞吐量提升组合图
        save_all_models_improvement_plot(baseline_df, ours_df, args.output_dir)
        
        print("=" * 50)
        print("所有图表生成完成!")
        print(f"总计生成了 {len(MODEL_ORDER)*2 + 2} 张图:")
        print(f"  - 每个模型的独立吞吐量图: {len(MODEL_ORDER)} 张")
        print(f"  - 每个模型的独立吞吐量提升图: {len(MODEL_ORDER)} 张")
        print(f"  - 所有模型的吞吐量组合图: 1 张")
        print(f"  - 所有模型的吞吐量提升组合图: 1 张")
        print(f"图表已保存到目录: {args.output_dir}/")

    except KeyError as e:
        print(f"Error: Missing column in one of the CSV files. Missing column: {e}")
        print("Please ensure both files have the required columns: 'data_avg_filter', 'train_avg_filter', 'model', 'batchsize', 'blocknum', 'threadnum'.")
        sys.exit(1)
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == '__main__':
    main()