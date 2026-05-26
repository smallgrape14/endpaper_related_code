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

# ===== 全局颜色定义 =====
# 在 main 函数中根据数据实际 unique 值初始化
baseline_colors = {} 
ours_color = ''

def calculate_times(df):
    """
    计算 total_time 和 data_time_percent。
    """
    df['total_time'] = df['data_avg_filter'] + df['train_avg_filter']
    df['data_time_percent'] = (df['data_avg_filter'] / df['total_time']) * 100
    return df

def plot_data_time_percent(baseline_df, ours_df):
    """
    生成第一组图：比较不同 model 下，baseline 和 ours 的 data time percent 随 batchsize 的变化。
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
    # 使用可调参数设置图表尺寸
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
        baseline_threads = sorted(baseline_model_df['threadnum'].unique()) # 确保线程顺序一致
        for thread in baseline_threads:
            thread_df = baseline_model_df[baseline_model_df['threadnum'] == thread]
            # 使用等距的 x_pos 进行绘图
            x_vals = [batchsize_to_xpos[bs] for bs in thread_df['batchsize']]
            ax.plot(x_vals, thread_df['data_time_percent'], 
                    marker='o', linestyle='--', label=f'Baseline (worker={thread})', color=baseline_colors[thread]) # 使用全局颜色

        # 绘制 ours 的数据
        # 使用等距的 x_pos 进行绘图
        x_vals = [batchsize_to_xpos[bs] for bs in ours_model_df['batchsize']]
        ax.plot(x_vals, ours_model_df['data_time_percent'], 
                marker='s', linestyle='-', linewidth=2.5, label='Ours', color=ours_color) # 使用全局颜色

        # 使用可调参数设置字体和样式
        ax.set_title(f'{model} - Data Time Percentage', fontsize=FONT_SIZE_LABEL)
        ax.set_xlabel('Batch Size', fontsize=FONT_SIZE_LABEL)
        ax.set_ylabel('Data Time Percentage (%)', fontsize=FONT_SIZE_LABEL)
        ax.set_xticks(x_pos)
        ax.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
        ax.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
        ax.legend(fontsize=FONT_SIZE_LEGEND)
        ax.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
        

    # 隐藏多余的子图
    for j in range(i + 1, len(axes)):
        fig.delaxes(axes[j])
        
    plt.tight_layout()
    plt.savefig('time_data_time_percent_comparison.png', dpi=300) # 添加前缀
    plt.show()

def plot_total_time_reduce_percent(baseline_df, ours_df):
    """
    生成第二组图：比较不同 model 下，不同 baseline 配置相对 ours 的 total_time_reduce_percent。
    现在与第三组图的画法、配置保持一致。
    """
    # 根据指定的顺序筛选和排序模型
    models = [m for m in MODEL_ORDER if m in baseline_df['model'].unique()]

    sns.set_style("whitegrid")
    
    all_batchsizes = sorted(baseline_df['batchsize'].unique())
    x_pos = np.arange(len(all_batchsizes))

    # 获取 baseline 的所有线程配置，用于区分不同折线
    baseline_threads = sorted(baseline_df['threadnum'].unique())
    
    num_models = len(models)
    num_cols = 2 # 即使只有一类图，也保持两列的布局以便统一 figsize 计算
    num_rows = (num_models + num_cols - 1) // num_cols
    # 使用可调参数设置图表尺寸，这里为了保持和第一组、第三组图相同的布局结构，仍然按两列计算总宽度
    # 但实际只使用每个模型的一列（右侧）
    fig, axes = plt.subplots(num_rows, num_cols, figsize=(num_cols * FIG_WIDTH, num_rows * FIG_HEIGHT))
    if num_models == 1:
        axes = np.array([axes])
    axes = axes.flatten()

    for i, model in enumerate(models):
        ax = axes[i] # 这里直接使用当前模型的子图
        
        # 筛选 ours 的数据（每个模型只有一行数据）
        ours_model_df = ours_df[ours_df['model'] == model]
        if ours_model_df.empty:
            continue
        
        # 将 batchsize 映射到等距的 x 坐标
        batchsize_to_xpos = {bs: pos for pos, bs in enumerate(all_batchsizes)}

        # 绘制不同 baseline 配置的 total time reduce percent
        for thread in baseline_threads:
            baseline_config_df = baseline_df[(baseline_df['model'] == model) & (baseline_df['threadnum'] == thread)].sort_values('batchsize')
            
            total_time_reduce_percent = []
            x_vals_plot = []
            for bs in all_batchsizes:
                ours_time = ours_model_df[ours_model_df['batchsize'] == bs]['total_time'].values
                baseline_time = baseline_config_df[baseline_config_df['batchsize'] == bs]['total_time'].values
                
                if ours_time.size > 0 and baseline_time.size > 0:
                    reduce_percent = ((baseline_time[0] - ours_time[0]) / baseline_time[0]) * 100
                    total_time_reduce_percent.append(reduce_percent)
                    x_vals_plot.append(batchsize_to_xpos[bs])

            ax.plot(x_vals_plot, total_time_reduce_percent, marker='o', 
                    linestyle='-', label=f'Baseline (worker={thread})', color=baseline_colors[thread]) # 使用全局颜色
            
        # 使用可调参数设置字体和样式
        ax.set_title(f'{model} - Total Time Reduction', fontsize=FONT_SIZE_LABEL)
        ax.set_xlabel('Batch Size', fontsize=FONT_SIZE_LABEL)
        ax.set_ylabel('Total Time Reduction (%)', fontsize=FONT_SIZE_LABEL)
        ax.set_xticks(x_pos)
        ax.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
        ax.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
        ax.axhline(y=20, color='r', linestyle='--', linewidth=1, label='target (20%)')
        ax.legend(fontsize=FONT_SIZE_LEGEND)
        ax.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
        # ax.set_ylim(bottom=0) # 设置y轴从0开始，避免负值误导
        # 如果有右侧子图被分配，但未绘制，这里需要隐藏
        if num_cols == 2 and i < num_models:
            if (i % num_cols) == (num_cols - 1) and i+1 < len(axes): # 如果是每行的第二个子图，且不是最后一个模型
                 if (i+1) % num_cols != 0 : # 并且下一个位置没有模型了
                     fig.delaxes(axes[i+1]) # 隐藏掉它右侧的空子图

    # 隐藏多余的子图（只显示第一列，隐藏所有第二列的空子图）
    # 在这个函数中，我们期望只画一列图（Total Time Reduce Percentage），所以将每行第二列的子图删除
    for j in range(len(models), len(axes)):
        fig.delaxes(axes[j])
        
    plt.tight_layout()
    plt.savefig('time_total_time_reduce_percent_comparison.png', dpi=300) # 添加前缀
    plt.show()


def plot_combined_results(baseline_df, ours_df):
    """
    生成第三组图：
    1. 一张总的组合图：每行展示一个模型，左侧为data time percent，右侧为total time reduce percent。
    2. 每个模型单独拆分成一张图片，包含该模型的左右两个子图。
    """
    # 获取所有唯一的模型名称
    models = [m for m in MODEL_ORDER if m in baseline_df['model'].unique()]

    # 设置图表样式
    sns.set_style("whitegrid")
    
    # 获取所有唯一的 batchsize 并排序，用于等距绘制和刻度标注
    all_batchsizes = sorted(baseline_df['batchsize'].unique())
    x_pos = np.arange(len(all_batchsizes)) # 生成等距的 x 坐标值
    
    # 获取所有 baseline 的线程配置，用于统一颜色
    # baseline_threads = sorted(baseline_df['threadnum'].unique()) # 已全局初始化

    # 为每个模型生成一行子图
    num_models = len(models)
    fig_combined, axes_combined = plt.subplots(num_models, 2, figsize=(2 * FIG_WIDTH, num_models * FIG_HEIGHT))
    if num_models == 1:
        axes_combined = np.array([axes_combined])

    for i, model in enumerate(models):
        # 左侧子图：data time percent
        ax_left_combined = axes_combined[i, 0]
        # 右侧子图：total time reduce percent
        ax_right_combined = axes_combined[i, 1]
        
        # 筛选 baseline 和 ours 的数据
        baseline_model_df = baseline_df[baseline_df['model'] == model].sort_values('batchsize')
        ours_model_df = ours_df[ours_df['model'] == model].sort_values('batchsize')
        
        # 将 batchsize 映射到等距的 x 坐标
        batchsize_to_xpos = {bs: pos for pos, bs in enumerate(all_batchsizes)}
        
        # --- 绘制左侧子图 (data time percent) ---
        for thread in baseline_colors.keys(): # 使用全局颜色字典的键
            thread_df = baseline_model_df[baseline_model_df['threadnum'] == thread]
            x_vals = [batchsize_to_xpos[bs] for bs in thread_df['batchsize']]
            ax_left_combined.plot(x_vals, thread_df['data_time_percent'], 
                                 marker='o', linestyle='--', label=f'Baseline (worker={thread})', color=baseline_colors[thread])

        x_vals = [batchsize_to_xpos[bs] for bs in ours_model_df['batchsize']]
        ax_left_combined.plot(x_vals, ours_model_df['data_time_percent'], 
                             marker='s', linestyle='-', linewidth=2.5, label='Ours', color=ours_color)
        
        ax_left_combined.set_title(f'{model} - Data Time Percentage', fontsize=FONT_SIZE_LABEL)
        ax_left_combined.set_xlabel('Batch Size', fontsize=FONT_SIZE_LABEL)
        ax_left_combined.set_ylabel('Data Time Percentage (%)', fontsize=FONT_SIZE_LABEL)
        ax_left_combined.set_xticks(x_pos)
        ax_left_combined.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
        ax_left_combined.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
        ax_left_combined.legend(fontsize=FONT_SIZE_LEGEND)
        ax_left_combined.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)


        # --- 绘制右侧子图 (total time reduce percent) ---
        for thread in baseline_colors.keys(): # 使用全局颜色字典的键
            baseline_config_df = baseline_df[(baseline_df['model'] == model) & (baseline_df['threadnum'] == thread)].sort_values('batchsize')
            
            total_time_reduce_percent = []
            x_vals_plot = []
            for bs in all_batchsizes:
                ours_time = ours_model_df[ours_model_df['batchsize'] == bs]['total_time'].values
                baseline_time = baseline_config_df[baseline_config_df['batchsize'] == bs]['total_time'].values
                
                if ours_time.size > 0 and baseline_time.size > 0:
                    reduce_percent = ((baseline_time[0] - ours_time[0]) / baseline_time[0]) * 100
                    total_time_reduce_percent.append(reduce_percent)
                    x_vals_plot.append(batchsize_to_xpos[bs])

            ax_right_combined.plot(x_vals_plot, total_time_reduce_percent, marker='o', 
                                  linestyle='-', label=f'Baseline (worker={thread})', color=baseline_colors[thread])
            
        ax_right_combined.set_title(f'{model} - Total Time Reduction', fontsize=FONT_SIZE_LABEL)
        ax_right_combined.set_xlabel('Batch Size', fontsize=FONT_SIZE_LABEL)
        ax_right_combined.set_ylabel('Total Time Reduction (%)', fontsize=FONT_SIZE_LABEL)
        ax_right_combined.set_xticks(x_pos)
        ax_right_combined.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
        ax_right_combined.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
        ax_right_combined.axhline(y=20, color='r', linestyle='--', linewidth=1, label='target (20%)')
        ax_right_combined.legend(fontsize=FONT_SIZE_LEGEND)
        ax_right_combined.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
        # ax_right_combined.set_ylim(bottom=-5)

    plt.tight_layout()
    plt.savefig('time_combined_comparison_all_models.png', dpi=300) # 总图文件名
    plt.close(fig_combined) # 关闭总图，以便后续单独绘制

    # --- 额外单独拆分成不同的图片 ---
    for model in models:
        fig_single, (ax_left_single, ax_right_single) = plt.subplots(1, 2, figsize=(2 * FIG_WIDTH, FIG_HEIGHT))

        # 筛选 baseline 和 ours 的数据
        baseline_model_df = baseline_df[baseline_df['model'] == model].sort_values('batchsize')
        ours_model_df = ours_df[ours_df['model'] == model].sort_values('batchsize')
        
        # 将 batchsize 映射到等距的 x 坐标
        batchsize_to_xpos = {bs: pos for pos, bs in enumerate(all_batchsizes)}
        
        # --- 绘制左侧子图 (data time percent) ---
        for thread in baseline_colors.keys(): # 使用全局颜色字典的键
            thread_df = baseline_model_df[baseline_model_df['threadnum'] == thread]
            x_vals = [batchsize_to_xpos[bs] for bs in thread_df['batchsize']]
            ax_left_single.plot(x_vals, thread_df['data_time_percent'], 
                                marker='o', linestyle='--', label=f'Baseline (worker={thread})', color=baseline_colors[thread])

        x_vals = [batchsize_to_xpos[bs] for bs in ours_model_df['batchsize']]
        ax_left_single.plot(x_vals, ours_model_df['data_time_percent'], 
                            marker='s', linestyle='-', linewidth=2.5, label='Ours', color=ours_color)
        
        ax_left_single.set_title(f'{model} - Data Time Percentage', fontsize=FONT_SIZE_LABEL)
        ax_left_single.set_xlabel('Batch Size', fontsize=FONT_SIZE_LABEL)
        ax_left_single.set_ylabel('Data Time Percentage (%)', fontsize=FONT_SIZE_LABEL)
        ax_left_single.set_xticks(x_pos)
        ax_left_single.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
        ax_left_single.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
        ax_left_single.legend(fontsize=FONT_SIZE_LEGEND)
        ax_left_single.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)

        # --- 绘制右侧子图 (total time reduce percent) ---
        for thread in baseline_colors.keys(): # 使用全局颜色字典的键
            baseline_config_df = baseline_df[(baseline_df['model'] == model) & (baseline_df['threadnum'] == thread)].sort_values('batchsize')
            
            total_time_reduce_percent = []
            x_vals_plot = []
            for bs in all_batchsizes:
                ours_time = ours_model_df[ours_model_df['batchsize'] == bs]['total_time'].values
                baseline_time = baseline_config_df[baseline_config_df['batchsize'] == bs]['total_time'].values
                
                if ours_time.size > 0 and baseline_time.size > 0:
                    reduce_percent = ((baseline_time[0] - ours_time[0]) / baseline_time[0]) * 100
                    total_time_reduce_percent.append(reduce_percent)
                    x_vals_plot.append(batchsize_to_xpos[bs])

            ax_right_single.plot(x_vals_plot, total_time_reduce_percent, marker='o', 
                                 linestyle='-', label=f'Baseline (worker={thread})', color=baseline_colors[thread])
            
        ax_right_single.set_title(f'{model} - Total Time Reduction', fontsize=FONT_SIZE_LABEL)
        ax_right_single.set_xlabel('Batch Size', fontsize=FONT_SIZE_LABEL)
        ax_right_single.set_ylabel('Total Time Reduction (%)', fontsize=FONT_SIZE_LABEL)
        ax_right_single.set_xticks(x_pos)
        ax_right_single.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK)
        ax_right_single.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
        ax_right_single.axhline(y=20, color='r', linestyle='--', linewidth=1, label='target (20%)')
        ax_right_single.legend(fontsize=FONT_SIZE_LEGEND)
        ax_right_single.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
        # ax_right_single.set_ylim(bottom=-5)

        plt.tight_layout()
        plt.savefig(f'time_combined_comparison_{model}.png', dpi=300) # 单独图片文件名
        plt.close(fig_single) # 关闭单独的图

def main():
    """
    主函数：读取文件，处理数据，并调用绘图函数。
    """
    global baseline_colors, ours_color # 声明使用全局变量
    
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
        
        # 统一计算 total_time 和 data_time_percent
        baseline_df = calculate_times(baseline_df)
        ours_df = calculate_times(ours_df)
        
        # --- 新增：为每个 threadnum 生成一个宽格式的 CSV 文件 ---
        baseline_threads = sorted(baseline_df['threadnum'].unique())
        
        for thread in baseline_threads:
            # 筛选当前 threadnum 的数据
            thread_df = baseline_df[baseline_df['threadnum'] == thread].copy()
            
            # 使用 pivot_table 将数据转换为宽格式
            # index 为 model，columns 为 batchsize，values 为 data_time_percent
            pivot_df = thread_df.pivot_table(
                index='model',
                columns='batchsize',
                values='data_time_percent',
                aggfunc='first' # 假设每个组合只有一个值
            )
            
            # 重命名索引列名以匹配期望格式
            pivot_df.index.name = 'model\\bs'
            
            # 重新排序行（model）以匹配 MODEL_ORDER
            pivot_df = pivot_df.reindex(MODEL_ORDER, fill_value='')
            
            # 保存为 CSV 文件
            output_filename = f"baseline_data_time_percent_thread_{thread}.csv"
            pivot_df.to_csv(output_filename, float_format='%.2f')
            print(f"CSV file '{output_filename}' generated successfully!")
        # -------------------------------------------------------------
        
        # ===== 初始化全局颜色映射 =====
        # base_palette = sns.color_palette("viridis", n_colors=len(baseline_threads))
        # baseline_colors = {thread: base_palette[i] for i, thread in enumerate(baseline_threads)}
        # ours_color = sns.color_palette("plasma")[3] # ours 使用一个单独的颜色
        # print("Baseline colors assigned:", baseline_colors)
        # print("Ours color assigned:", ours_color)
       # ===== 初始化全局颜色映射 =====
        # 定义线程配置列表
        baseline_threads = [1, 4] # 您的线程配置

        # 自定义颜色列表（与 baseline_threads 长度对应）
        custom_colors = [
            '#1f77b4', 
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
        
        # 绘制第一组图
        plot_data_time_percent(baseline_df, ours_df)

        # 绘制第二组图
        plot_total_time_reduce_percent(baseline_df, ours_df)
        
        # 绘制第三组图（组合图和单独拆分图）
        plot_combined_results(baseline_df, ours_df)
        
        print("Plots generated successfully!")
        print("Check 'time_data_time_percent_comparison.png', 'time_total_time_reduce_percent_comparison.png'.")
        print("Check 'time_combined_comparison_all_models.png' and individual model files (e.g., 'time_combined_comparison_mobilenet_v2.png').")

    except KeyError as e:
        print(f"Error: Missing column in one of the CSV files. Missing column: {e}")
        print("Please ensure both files have the required columns: 'data_avg_filter', 'train_avg_filter', 'model', 'batchsize', 'blocknum', 'threadnum'.")
        sys.exit(1)
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()