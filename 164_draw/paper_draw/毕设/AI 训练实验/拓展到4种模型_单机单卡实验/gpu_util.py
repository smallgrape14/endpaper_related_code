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

# ===== 性能指标定义 =====
METRIC_ORDER = ['SMACT', 'SMOCC', 'DRAMA', 'GPU_Util']
METRIC_LABELS_PERCENT = { # 用于第一组图的百分比标签
    'SMACT': 'SM Activity (%)',
    'SMOCC': 'SM Occupancy (%)',
    'DRAMA': 'DRAM Activity (%)',
    'GPU_Util': 'GPU Utilization (%)'
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

def plot_performance_metrics(baseline_df, ours_df):
    """
    生成第一组图：在各个指标下(SMACT,SMOCC,DRAMA,GPU_Util)比较我们的方法和baseline。
    每行一个指标，每列一个模型。所有指标以百分比形式输出。
    """
    # 根据指定的顺序筛选和排序模型
    models = [m for m in MODEL_ORDER if m in baseline_df['model'].unique()]

    # 设置图表样式
    sns.set_style("whitegrid")
    
    # 获取所有唯一的 batchsize 并排序
    all_batchsizes = sorted(baseline_df['batchsize'].unique())
    x_pos = np.arange(len(all_batchsizes))
    
    # 绘制多行多列子图
    num_metrics = len(METRIC_ORDER)
    num_models = len(models)
    
    fig, axes = plt.subplots(num_metrics, num_models, figsize=(num_models * FIG_WIDTH, num_metrics * FIG_HEIGHT), squeeze=False)

    for i, metric in enumerate(METRIC_ORDER):
        for j, model in enumerate(models):
            ax = axes[i, j]
            
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
                        marker='o', linestyle='--', label=f'Baseline (worker={thread})', color=baseline_colors[thread])

            # 绘制 ours 的数据
            x_vals = [batchsize_to_xpos[bs] for bs in ours_model_df['batchsize']]
            ours_plot_data = ours_model_df[metric] * 100 if metric in ['SMACT', 'SMOCC', 'DRAMA'] else ours_model_df[metric]
            ax.plot(x_vals, ours_plot_data, 
                    marker='s', linestyle='-', linewidth=2.5, label='Ours', color=ours_color)

            # 设置标题和标签
            if i == 0:  # 第一行子图设置模型名称作为标题
                ax.set_title(f'{model}', fontsize=FONT_SIZE_LABEL)
            
            if j == 0: # 第一列子图设置纵坐标标签
                ax.set_ylabel(f'{METRIC_LABELS_PERCENT[metric]}', fontsize=FONT_SIZE_LABEL)

            if i == num_metrics - 1: # 最后一行子图设置横坐标标签
                ax.set_xlabel('Batch Size', fontsize=FONT_SIZE_LABEL)
            
            ax.set_xticks(x_pos)
            ax.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
            ax.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
            ax.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)

    # 在最右侧添加一个总图例
    handles, labels = axes[0, 0].get_legend_handles_labels()
    fig.legend(handles, labels, loc='upper right', bbox_to_anchor=(1.0, 1.0), fontsize=FONT_SIZE_LEGEND)

    plt.tight_layout(rect=[0, 0, 0.9, 1])  # 调整布局以容纳图例
    plt.savefig('metrics_performance_comparison.png', dpi=300) 
    plt.show()


def plot_metric_improvement_absolute(baseline_df, ours_df):
    """
    生成第二组图：比较不同 model 、指标下，ours 相对不同 baseline 配置的指标提升情况 (ours - baseline)。
    每行一个指标，每列一个模型。所有指标的提升值以百分点形式输出。
    """
    models = [m for m in MODEL_ORDER if m in baseline_df['model'].unique()]

    sns.set_style("whitegrid")
    
    all_batchsizes = sorted(baseline_df['batchsize'].unique())
    x_pos = np.arange(len(all_batchsizes))
    
    num_metrics = len(METRIC_ORDER)
    num_models = len(models)
    
    fig, axes = plt.subplots(num_metrics, num_models, figsize=(num_models * FIG_WIDTH, num_metrics * FIG_HEIGHT), squeeze=False)

    for i, metric in enumerate(METRIC_ORDER):
        for j, model in enumerate(models):
            ax = axes[i, j]
            
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

                if improvement_values: # 只有当有数据时才绘制
                    ax.plot(x_vals_plot, improvement_values, 
                            marker='o', linestyle='-', label=f'Ours - Baseline (worker={thread})', color=baseline_colors[thread])
            
            # 设置标题和标签
            if i == 0:
                ax.set_title(f'{model}', fontsize=FONT_SIZE_LABEL)
            
            if j == 0:
                ax.set_ylabel(f'{METRIC_LABELS_IMPROVEMENT[metric]}', fontsize=FONT_SIZE_LABEL)

            if i == num_metrics - 1:
                ax.set_xlabel('Batch Size', fontsize=FONT_SIZE_LABEL)
            
            ax.set_xticks(x_pos)
            ax.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
            ax.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
            ax.axhline(y=10, color='r', linestyle='--', linewidth=1, label='target (10%)') # 添加10%基准线
            ax.legend(fontsize=FONT_SIZE_LEGEND) # 每个子图都显示图例，方便查看
            ax.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)

    plt.tight_layout(rect=[0, 0, 0.9, 1]) # 调整布局以容纳图例
    plt.savefig('metrics_performance_improvement_absolute_comparison.png', dpi=300)
    plt.show()

def plot_combined_per_metric(baseline_df, ours_df):
    """
    生成第三组图：按指标独立保存图片，每张图片包含两行子图，
    第一行为指标的百分比，第二行为指标的绝对提升值。
    """
    models = [m for m in MODEL_ORDER if m in baseline_df['model'].unique()]
    all_batchsizes = sorted(baseline_df['batchsize'].unique())
    x_pos = np.arange(len(all_batchsizes))

    sns.set_style("whitegrid")

    for metric in METRIC_ORDER:
        num_models = len(models)
        fig, axes = plt.subplots(2, num_models, figsize=(num_models * FIG_WIDTH, 2 * FIG_HEIGHT), squeeze=False)
        fig.suptitle(f'Comparison for {METRIC_LABELS_PERCENT[metric]}', fontsize=FONT_SIZE_LABEL + 2)

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
                            marker='o', linestyle='--', label=f'Baseline (worker={thread})', color=baseline_colors[thread])

            x_vals = [batchsize_to_xpos[bs] for bs in ours_model_df['batchsize']]
            ours_plot_data = ours_model_df[metric] * 100 if metric in ['SMACT', 'SMOCC', 'DRAMA'] else ours_model_df[metric]
            ax_top.plot(x_vals, ours_plot_data, 
                        marker='s', linestyle='-', linewidth=2.5, label='Ours', color=ours_color)
            
            if j == 0:
                ax_top.set_ylabel(f'{METRIC_LABELS_PERCENT[metric]}', fontsize=FONT_SIZE_LABEL)
            ax_top.set_title(f'{model}', fontsize=FONT_SIZE_LABEL)
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
                                    marker='o', linestyle='-', label=f'Ours - Baseline (worker={thread})', color=baseline_colors[thread])
            
            if j == 0:
                ax_bottom.set_ylabel(f'{METRIC_LABELS_IMPROVEMENT[metric]}', fontsize=FONT_SIZE_LABEL)
            ax_bottom.set_xlabel('Batch Size', fontsize=FONT_SIZE_LABEL)
            ax_bottom.set_xticks(x_pos)
            ax_bottom.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
            ax_bottom.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
            ax_bottom.axhline(y=10, color='r', linestyle='--', linewidth=1, label='target (10%)')
            ax_bottom.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)

        # 在最右侧添加总图例
        handles, labels = axes[0, 0].get_legend_handles_labels()
        fig.legend(handles, labels, loc='upper right', bbox_to_anchor=(1.0, 1.0), fontsize=FONT_SIZE_LEGEND)
        
        plt.tight_layout(rect=[0, 0, 0.9, 1])
        plt.savefig(f'metrics_{metric}_comparison.png', dpi=300)
        plt.close(fig) # 关闭当前图表，避免内存占用过高
        
def main():
    """
    主函数：读取文件，处理数据，并调用绘图函数。
    """
    global baseline_colors, ours_color
    
    parser = argparse.ArgumentParser(description='Plot and compare GPU performance metrics for different models and batch sizes.')
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
        base_palette = sns.color_palette("viridis", n_colors=len(baseline_threads))
        baseline_colors = {thread: base_palette[i] for i, thread in enumerate(baseline_threads)}
        ours_color = sns.color_palette("plasma")[3] # Ours的颜色也可以全局定义，方便统一
        # ============================
        
        # 绘制第一组图 (性能指标的绝对值)
        plot_performance_metrics(baseline_df, ours_df)

        # 绘制第二组图 (指标的绝对提升值)
        plot_metric_improvement_absolute(baseline_df, ours_df)

        # 绘制第三组图 (按指标拆分的上下组合图)
        plot_combined_per_metric(baseline_df, ours_df)
        
        print("Plots generated successfully!")
        print("Check 'metrics_performance_comparison.png', 'metrics_performance_improvement_absolute_comparison.png',")
        print("and the individual metric files (e.g., 'SMACT_comparison.png').")

    except KeyError as e:
        print(f"Error: Missing column in one of the CSV files. Missing column: {e}")
        print("Please ensure both files have the required columns: 'model', 'batchsize', 'blocknum', 'threadnum', 'index' and the specified metrics.")
        sys.exit(1)
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()