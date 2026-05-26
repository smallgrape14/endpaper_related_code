import sys
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import argparse
import os
import matplotlib

# ===== 可调参数区 =====
FONT_SIZE_LABEL = 10         # 坐标轴标签字体大小
FONT_SIZE_TICK = 10          # 坐标轴刻度字体大小
FONT_SIZE_ANNOT = 10         # 数据标注字体大小
FONT_SIZE_LEGEND = 10         # 图例字体大小
FIG_WIDTH = 5                # 单个图的宽度
FIG_HEIGHT = 4               # 单个图的高度
GRID_ALPHA = 0.5             # 网格透明度
GRID_LINESTYLE = '--'        # 网格线型

# ===== 模型顺序定义 =====
MODEL_ORDER = ['mobilenet_v2', 'shufflenet_v2_x1_0', 'resnet18', 'resnet50']

# ===== 解决中文显示问题的关键设置 =====
# 方法1: 设置matplotlib参数
matplotlib.rcParams['font.sans-serif'] = ['SimHei', 'Microsoft YaHei', 'DejaVu Sans']
matplotlib.rcParams['axes.unicode_minus'] = False
matplotlib.rcParams['font.family'] = 'sans-serif'

# 方法2: 设置seaborn的字体
sns.set(font='SimHei')
sns.set_style("whitegrid", {
    'font.sans-serif': ['SimHei', 'Microsoft YaHei', 'Arial Unicode MS']
})

# 方法3: 设置全局字体参数
plt.rcParams.update({
    'font.sans-serif': ['SimHei', 'Microsoft YaHei'],
    'axes.unicode_minus': False,
    'font.family': 'sans-serif',
    'font.size': FONT_SIZE_TICK,
    'axes.labelsize': FONT_SIZE_LABEL,
    'legend.fontsize': FONT_SIZE_LEGEND,
    'xtick.labelsize': FONT_SIZE_TICK,
    'ytick.labelsize': FONT_SIZE_TICK
})

# ===== 性能指标定义 =====
METRIC_ORDER = ['SMACT', 'SMOCC', 'DRAMA', 'GPU_Util']
METRIC_LABELS_PERCENT = { # 用于第一组图的百分比标签
    'SMACT': 'GPU 流处理器(SM) 活跃度 (%)',
    'SMOCC': 'GPU 流处理器(SM) 占用率 (%)',
    'DRAMA': 'DRAM Activity (%)',
    'GPU_Util': 'GPU 利用率 (%)'
}
METRIC_LABELS_IMPROVEMENT = { # 用于第二组图的绝对提升标签
    'SMACT': 'SM Activity Improvement (Abs. %)',
    'SMOCC': 'SM Occupancy Improvement (Abs. %)',
    'DRAMA': 'DRAM Activity Improvement (Abs. %)',
    'GPU_Util': 'GPU Utilization Improvement (Abs. %)'
}

# ===== 全局颜色定义 =====
baseline_colors = {}
ours_color = ''

def save_individual_performance_plots(baseline_df, ours_df, output_dir="individual_plots"):
    """
    为每个模型、每个指标生成独立的性能指标百分比图
    
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
    
    # 创建性能指标百分比图目录
    performance_dir = os.path.join(output_dir, "performance_percent")
    if not os.path.exists(performance_dir):
        os.makedirs(performance_dir)
    
    print("正在生成性能指标百分比独立图...")
    
    for metric in METRIC_ORDER:
        metric_name = METRIC_LABELS_PERCENT[metric].split(' (')[0]  # 提取指标名称
        for model in models:
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
                if thread_df.empty:
                    continue
                x_vals = [batchsize_to_xpos[bs] for bs in thread_df['batchsize']]
                
                # 根据指标类型获取数据并转换为百分比
                plot_data = thread_df[metric] * 100 if metric in ['SMACT', 'SMOCC', 'DRAMA'] else thread_df[metric]

                ax.plot(x_vals, plot_data, 
                        marker='o', linestyle='--', label=f'Baseline (workernum={thread})', 
                        color=baseline_colors[thread])

            # 绘制 ours 的数据
            x_vals = [batchsize_to_xpos[bs] for bs in ours_model_df['batchsize']]
            ours_plot_data = ours_model_df[metric] * 100 if metric in ['SMACT', 'SMOCC', 'DRAMA'] else ours_model_df[metric]
            ax.plot(x_vals, ours_plot_data, 
                    marker='s', linestyle='-', linewidth=2.5, label='OUR', 
                    color=ours_color)

            # 设置图表属性
            title = f'{model} - {metric_name}'
            # ax.set_title(title, fontsize=FONT_SIZE_LABEL+2, pad=15)
            ax.set_xlabel('批大小（Batch Size）', fontsize=FONT_SIZE_LABEL)
            ax.set_ylabel(METRIC_LABELS_PERCENT[metric], fontsize=FONT_SIZE_LABEL)
            ax.set_xticks(x_pos)
            ax.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
            ax.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
            
            # 设置图例
            ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.2), 
                     ncol=3, fontsize=FONT_SIZE_LEGEND, frameon=False)
            
            ax.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
            
            # 调整布局，为上方图例留出空间
            plt.tight_layout(rect=[0, 0, 1, 0.95])
            
            # 保存图形
            filename = f"{performance_dir}/{model}_{metric}_performance.png"
            plt.savefig(filename, dpi=300, bbox_inches='tight')
            plt.close(fig)
            
    print(f"  性能指标百分比图已保存到目录: {performance_dir}/")

def save_individual_improvement_plots(baseline_df, ours_df, output_dir="individual_plots"):
    """
    为每个模型、每个指标生成独立的绝对提升值图
    
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
    
    # 创建绝对提升图目录
    improvement_dir = os.path.join(output_dir, "improvement_absolute")
    if not os.path.exists(improvement_dir):
        os.makedirs(improvement_dir)
    
    print("正在生成绝对提升值独立图...")
    
    for metric in METRIC_ORDER:
        metric_name = METRIC_LABELS_IMPROVEMENT[metric].split(' (')[0]  # 提取指标名称
        for model in models:
            # 创建独立图形
            fig, ax = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))
            
            baseline_model_df = baseline_df[baseline_df['model'] == model].sort_values('batchsize')
            ours_model_df = ours_df[ours_df['model'] == model].sort_values('batchsize')
            
            batchsize_to_xpos = {bs: pos for pos, bs in enumerate(all_batchsizes)}
            
            # 绘制 Ours 相对 Baseline 的绝对差值
            baseline_threads = sorted(baseline_model_df['threadnum'].unique())
            for thread in baseline_threads:
                baseline_thread_df = baseline_model_df[baseline_model_df['threadnum'] == thread]
                
                improvement_values = []
                x_vals_plot = []
                for bs in all_batchsizes:
                    ours_metric_val = ours_model_df[ours_model_df['batchsize'] == bs][metric].values
                    baseline_metric_val = baseline_thread_df[baseline_thread_df['batchsize'] == bs][metric].values
                    
                    if ours_metric_val.size > 0 and baseline_metric_val.size > 0:
                        # 计算绝对差值
                        diff = ours_metric_val[0] - baseline_metric_val[0]
                        # 对于 'SMACT', 'SMOCC', 'DRAMA'，差值也乘以100转换为百分点
                        if metric in ['SMACT', 'SMOCC', 'DRAMA']:
                            diff *= 100
                        improvement_values.append(diff)
                        x_vals_plot.append(batchsize_to_xpos[bs])

                if improvement_values:  # 只有当有数据时才绘制
                    ax.plot(x_vals_plot, improvement_values, 
                            marker='o', linestyle='-', label=f'Baseline (workernum={thread})', 
                            color=baseline_colors[thread])
            
            # 设置图表属性
            title = f'{model} - {metric_name} Improvement'
            # ax.set_title(title, fontsize=FONT_SIZE_LABEL+2, pad=15)
            ax.set_xlabel('批大小（Batch Size）', fontsize=FONT_SIZE_LABEL)
            ax.set_ylabel(METRIC_LABELS_IMPROVEMENT[metric], fontsize=FONT_SIZE_LABEL)
            ax.set_xticks(x_pos)
            ax.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
            ax.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
            
            # 设置图例
            ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.2), 
                     ncol=3, fontsize=FONT_SIZE_LEGEND, frameon=False)
            
            ax.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
            
            # 调整布局，为上方图例留出空间
            plt.tight_layout(rect=[0, 0, 1, 0.95])
            
            # 保存图形
            filename = f"{improvement_dir}/{model}_{metric}_improvement.png"
            plt.savefig(filename, dpi=300, bbox_inches='tight')
            plt.close(fig)
            
    print(f"  绝对提升值图已保存到目录: {improvement_dir}/")

def save_combined_per_metric_plots(baseline_df, ours_df, output_dir="individual_plots"):
    """
    为每个指标生成组合图（上下两个子图：上为性能指标，下为绝对提升）
    
    参数:
    baseline_df: baseline数据
    ours_df: ours数据
    output_dir: 输出目录
    """
    # 创建输出目录
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    models = [m for m in MODEL_ORDER if m in baseline_df['model'].unique()]
    all_batchsizes = sorted(baseline_df['batchsize'].unique())
    x_pos = np.arange(len(all_batchsizes))
    
    # 创建组合图目录
    combined_dir = os.path.join(output_dir, "combined_per_metric")
    if not os.path.exists(combined_dir):
        os.makedirs(combined_dir)
    
    print("正在生成每个指标的组合图...")
    
    for metric in METRIC_ORDER:
        metric_name = METRIC_LABELS_PERCENT[metric].split(' (')[0]  # 提取指标名称
        
        # 计算图形尺寸
        num_models = len(models)
        fig, axes = plt.subplots(2, num_models, figsize=(num_models * FIG_WIDTH, 2 * FIG_HEIGHT), squeeze=False)
        fig.suptitle(f'{metric_name} 性能对比', fontsize=FONT_SIZE_LABEL + 2, y=1.02)

        for j, model in enumerate(models):
            ax_top = axes[0, j]
            ax_bottom = axes[1, j]
            
            baseline_model_df = baseline_df[baseline_df['model'] == model].sort_values('batchsize')
            ours_model_df = ours_df[ours_df['model'] == model].sort_values('batchsize')
            batchsize_to_xpos = {bs: pos for pos, bs in enumerate(all_batchsizes)}
            
            # --- 绘制第一行子图 (指标百分比) ---
            baseline_threads = sorted(baseline_model_df['threadnum'].unique())
            for thread in baseline_threads:
                thread_df = baseline_model_df[baseline_model_df['threadnum'] == thread]
                if thread_df.empty:
                    continue
                x_vals = [batchsize_to_xpos[bs] for bs in thread_df['batchsize']]
                plot_data = thread_df[metric] * 100 if metric in ['SMACT', 'SMOCC', 'DRAMA'] else thread_df[metric]
                ax_top.plot(x_vals, plot_data, 
                            marker='o', linestyle='--', label=f'Baseline (workernum={thread})', 
                            color=baseline_colors[thread])

            x_vals = [batchsize_to_xpos[bs] for bs in ours_model_df['batchsize']]
            ours_plot_data = ours_model_df[metric] * 100 if metric in ['SMACT', 'SMOCC', 'DRAMA'] else ours_model_df[metric]
            ax_top.plot(x_vals, ours_plot_data, 
                        marker='s', linestyle='-', linewidth=2.5, label='OUR', 
                        color=ours_color)
            
            if j == 0:
                ax_top.set_ylabel(METRIC_LABELS_PERCENT[metric], fontsize=FONT_SIZE_LABEL)
            # ax_top.set_title(f'{model}', fontsize=FONT_SIZE_LABEL)
            ax_top.set_xticks(x_pos)
            ax_top.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
            ax_top.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
            ax_top.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)

            # --- 绘制第二行子图 (指标绝对提升值) ---
            for thread in baseline_threads:
                baseline_thread_df = baseline_model_df[baseline_model_df['threadnum'] == thread]
                improvement_values = []
                x_vals_plot = []
                for bs in all_batchsizes:
                    ours_metric_val = ours_model_df[ours_model_df['batchsize'] == bs][metric].values
                    baseline_metric_val = baseline_thread_df[baseline_thread_df['batchsize'] == bs][metric].values
                    
                    if ours_metric_val.size > 0 and baseline_metric_val.size > 0:
                        diff = ours_metric_val[0] - baseline_metric_val[0]
                        if metric in ['SMACT', 'SMOCC', 'DRAMA']:
                            diff *= 100
                        improvement_values.append(diff)
                        x_vals_plot.append(batchsize_to_xpos[bs])

                if improvement_values:
                    ax_bottom.plot(x_vals_plot, improvement_values, 
                                    marker='o', linestyle='-', label=f'Baseline (workernum={thread})', 
                                    color=baseline_colors[thread])
            
            if j == 0:
                ax_bottom.set_ylabel(METRIC_LABELS_IMPROVEMENT[metric], fontsize=FONT_SIZE_LABEL)
            ax_bottom.set_xlabel('批大小（Batch Size）', fontsize=FONT_SIZE_LABEL)
            ax_bottom.set_xticks(x_pos)
            ax_bottom.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
            ax_bottom.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
            ax_bottom.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)

        # 添加图例
        handles, labels = axes[0, 0].get_legend_handles_labels()
        fig.legend(handles, labels, loc='upper center', bbox_to_anchor=(0.5, 1.0), 
                  ncol=3, fontsize=FONT_SIZE_LEGEND, frameon=False)
        
        plt.tight_layout(rect=[0, 0, 1, 0.95])
        
        # 保存组合图
        filename = f"{combined_dir}/combined_{metric}_comparison.png"
        plt.savefig(filename, dpi=300, bbox_inches='tight')
        plt.close(fig)
        
    print(f"  组合图已保存到目录: {combined_dir}/")

def main():
    """
    主函数：读取文件，处理数据，并调用绘图函数。
    """
    global baseline_colors, ours_color
    
    parser = argparse.ArgumentParser(description='Plot and compare GPU performance metrics for different models and batch sizes.')
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
        # 统一数据处理：只筛选 'mean' 数据，并过滤掉 blocknum != 0 的 baseline 数据
        baseline_df = baseline_df[(baseline_df['index'] == 'mean') & (baseline_df['blocknum'] == 0)].copy()
        ours_df = ours_df[ours_df['index'] == 'mean'].copy()
        
        # 根据命令行参数过滤线程
        if args.draw_threads:
            threads_to_draw = [int(t) for t in args.draw_threads.split(',')]
            baseline_df = baseline_df[baseline_df['threadnum'].isin(threads_to_draw)].copy()
            if baseline_df.empty:
                print(f"Warning: No data found for specified threads: {threads_to_draw}. Skipping plotting.")
                sys.exit(0)
        
        # ===== 初始化全局颜色映射 =====
        baseline_threads = sorted(baseline_df['threadnum'].unique())
        
        # 自定义颜色列表
        custom_colors = [
            '#1f77b4',  # 蓝色
            '#2ca02c',  # 绿色
            '#ff0e22',  # 红色
            '#9467bd',  # 紫色
            '#8c564b',  # 棕色
            '#e377c2',  # 粉色
            '#7f7f7f',  # 灰色
            '#bcbd22',  # 橄榄绿
            '#17becf'   # 青色
        ]
        
        # 创建颜色映射字典
        baseline_colors = {thread: color for thread, color in zip(baseline_threads, custom_colors[:len(baseline_threads)])}
        
        # 为Ours设置单独颜色
        ours_color = '#ff7f0e'  # 橙色
        # ============================
        
        print(f"开始生成图表，输出目录: {args.output_dir}")
        print("=" * 50)
        
        # 1. 生成独立的性能指标百分比图
        save_individual_performance_plots(baseline_df, ours_df, args.output_dir)
        
        print("-" * 50)
        
        # 2. 生成独立的绝对提升值图
        save_individual_improvement_plots(baseline_df, ours_df, args.output_dir)
        
        print("-" * 50)
        
        # 3. 生成每个指标的组合图
        save_combined_per_metric_plots(baseline_df, ours_df, args.output_dir)
        
        print("=" * 50)
        print("所有图表生成完成!")
        print(f"总计生成了 {len(METRIC_ORDER)*len(MODEL_ORDER)*2 + len(METRIC_ORDER)} 张图:")
        print(f"  - 独立的性能指标百分比图: {len(METRIC_ORDER)*len(MODEL_ORDER)} 张")
        print(f"  - 独立的绝对提升值图: {len(METRIC_ORDER)*len(MODEL_ORDER)} 张")
        print(f"  - 每个指标的组合图: {len(METRIC_ORDER)} 张")
        print(f"图表已保存到目录: {args.output_dir}/")
        
        # 显示生成的图表文件结构
        print("\n生成的图表结构:")
        print(f"{args.output_dir}/")
        print(f"├── performance_percent/")
        print(f"│   ├── mobilenet_v2_SMACT_performance.png")
        print(f"│   ├── mobilenet_v2_SMOCC_performance.png")
        print(f"│   └── ...")
        print(f"├── improvement_absolute/")
        print(f"│   ├── mobilenet_v2_SMACT_improvement.png")
        print(f"│   ├── mobilenet_v2_SMOCC_improvement.png")
        print(f"│   └── ...")
        print(f"└── combined_per_metric/")
        print(f"    ├── combined_SMACT_comparison.png")
        print(f"    ├── combined_SMOCC_comparison.png")
        print(f"    └── ...")

    except KeyError as e:
        print(f"Error: Missing column in one of the CSV files. Missing column: {e}")
        print("Please ensure both files have the required columns: 'model', 'batchsize', 'blocknum', 'threadnum', 'index' and the specified metrics.")
        sys.exit(1)
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == '__main__':
    main()