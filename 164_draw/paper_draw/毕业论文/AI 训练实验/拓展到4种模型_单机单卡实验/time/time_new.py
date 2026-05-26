import sys
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import argparse
import os
import matplotlib

# ===== 可调参数区 =====
FONT_SIZE_LABEL = 20         # 坐标轴标签字体大小
FONT_SIZE_TICK = 20          # 坐标轴刻度字体大小
FONT_SIZE_ANNOT = 18         # 数据标注字体大小
FONT_SIZE_LEGEND = 18        # 图例字体大小
FIG_WIDTH = 5.8              # 单个子图的图宽
FIG_HEIGHT = 4.8             # 单个子图的图高
GRID_ALPHA = 0.5             # 网格透明度
GRID_LINESTYLE = '--'        # 网格线型
# ===== FIXED FONT CONFIGURATION =====
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['SimSun','Times New Roman' ]  # Times + 宋体
plt.rcParams['mathtext.fontset'] = 'stix'  # Math symbols matching Times
plt.rcParams['axes.unicode_minus'] = False
plt.rcParams['axes.formatter.use_mathtext'] = False


# # ===== 可调参数区 =====
# FONT_SIZE_LABEL = 10         # 坐标轴标签字体大小
# FONT_SIZE_TICK = 10          # 坐标轴刻度字体大小
# FONT_SIZE_ANNOT = 10         # 数据标注字体大小
# FONT_SIZE_LEGEND = 9         # 图例字体大小
# FIG_WIDTH = 6                # 单个图的宽度
# FIG_HEIGHT = 4               # 单个图的高度
# GRID_ALPHA = 0.5             # 网格透明度
# GRID_LINESTYLE = '--'        # 网格线型

# ===== 模型顺序定义 =====
MODEL_ORDER = ['mobilenet_v2', 'shufflenet_v2_x1_0', 'resnet18', 'resnet50']

# # ===== 解决中文显示问题的关键设置 =====
# plt.rcParams['font.sans-serif'] = ['SimHei']  # 用来正常显示中文标签
# plt.rcParams['axes.unicode_minus'] = False  # 用来正常显示负号

# 设置全局字体
plt.rcParams.update({
    # 'font.sans-serif': ['SimHei', 'Arial'],  # 中文字体在前
    # 'font.family': 'sans-serif',  # 明确指定字体族
    # 'axes.unicode_minus': False,
    'font.size': FONT_SIZE_TICK,
    'axes.labelsize': FONT_SIZE_LABEL,
    'legend.fontsize': FONT_SIZE_LEGEND,
    'xtick.labelsize': FONT_SIZE_TICK,
    'ytick.labelsize': FONT_SIZE_TICK
})

# 设置seaborn的字体参数
# sns.set(font='SimHei')  # 设置seaborn使用中文字体
# sns.set_style("whitegrid", {
#     'font.sans-serif': ['SimHei', 'Arial']
# })
sns.set(font='SimSun')  # 设置seaborn使用中文字体
sns.set_style("whitegrid", {
    'font.sans-serif': ['SimSun','Times New Roman']
})

# ===== 全局颜色定义 =====
baseline_colors = {} 
ours_color = ''

def calculate_times(df):
    """
    计算 total_time 和 data_time_percent。
    """
    df['total_time'] = df['data_avg_filter'] + df['train_avg_filter']
    df['data_time_percent'] = (df['data_avg_filter'] / df['total_time']) * 100
    return df

def save_individual_plots(baseline_df, ours_df, output_dir="individual_plots"):
    """
    为每个模型生成并保存两个独立的图形文件
    
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
    
    for model in models:
        print(f"正在处理模型: {model}")
        
        # 筛选 baseline 和 ours 的数据
        baseline_model_df = baseline_df[baseline_df['model'] == model].sort_values('batchsize')
        ours_model_df = ours_df[ours_df['model'] == model].sort_values('batchsize')
        
        # 将 batchsize 映射到等距的 x 坐标
        batchsize_to_xpos = {bs: pos for pos, bs in enumerate(all_batchsizes)}
        
        # ================== 图1: Data Time Percentage ==================
        fig1, ax1 = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))
        
        # 绘制 baseline 的数据
        baseline_threads = sorted(baseline_model_df['threadnum'].unique())
        for thread in baseline_threads:
            thread_df = baseline_model_df[baseline_model_df['threadnum'] == thread]
            x_vals = [batchsize_to_xpos[bs] for bs in thread_df['batchsize']]
            ax1.plot(x_vals, thread_df['data_time_percent'], 
                    marker='o', linestyle='--', label=f'Baseline (Workernum={thread})', 
                    color=baseline_colors[thread])
        
        # 绘制 ours 的数据
        x_vals = [batchsize_to_xpos[bs] for bs in ours_model_df['batchsize']]
        ax1.plot(x_vals, ours_model_df['data_time_percent'], 
                marker='s', linestyle='-', linewidth=2.5, label='OUR', 
                color=ours_color)
        
        # 设置图表属性
        # ax1.set_title(f'{model} - 数据加载时间占比', fontsize=FONT_SIZE_LABEL+2, pad=20, fontproperties='SimHei')
        ax1.set_xlabel('批大小（Batch Size）', fontsize=FONT_SIZE_LABEL, fontproperties='SimSun')
        ax1.set_ylabel('数据加载时间占比 (%)', fontsize=FONT_SIZE_LABEL, fontproperties='SimSun')
        ax1.set_xticks(x_pos)
        ax1.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK, fontproperties='SimSun')
        y_ticks = np.arange(0, 101, 20)  # 0, 50, 100, ..., 450
        ax1.set_yticks(y_ticks)
        ax1.set_yticklabels(y_ticks, fontsize=FONT_SIZE_TICK)
        ax1.tick_params(axis='y', labelsize=FONT_SIZE_TICK)

        # 修改图例位置：放在图表上方，水平排列
        # ax1.legend(loc='upper center', bbox_to_anchor=(0.5, 1.3), 
        #           ncol=2, fontsize=10, frameon=False, 
        #           prop={'family': 'SimSun','size':18})
        # prop={'size': LEGEND_FONT_SIZE, 'weight': 'bold', 'family': 'serif'},
        
        ax1.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
        
        # 调整布局，为上方图例留出空间
        plt.tight_layout(rect=[0, 0, 1, 0.95])
        
        # 保存第一个图
        filename1 = f"{output_dir}/{model}_data_time_percent.png"
        plt.savefig(filename1, dpi=300, bbox_inches='tight')
        plt.close(fig1)
        print(f"  已保存: {filename1}")
        
        # ================== 图2: Total Time Reduction ==================
        # fig2, ax2 = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))
        
        # # 绘制不同 baseline 配置的 total time reduce percent
        # for thread in baseline_threads:
        #     baseline_config_df = baseline_model_df[baseline_model_df['threadnum'] == thread].sort_values('batchsize')
            
        #     total_time_reduce_percent = []
        #     x_vals_plot = []
        #     for bs in all_batchsizes:
        #         ours_time = ours_model_df[ours_model_df['batchsize'] == bs]['total_time'].values
        #         baseline_time = baseline_config_df[baseline_config_df['batchsize'] == bs]['total_time'].values
                
        #         if ours_time.size > 0 and baseline_time.size > 0:
        #             reduce_percent = ((baseline_time[0] - ours_time[0]) / baseline_time[0]) * 100
        #             total_time_reduce_percent.append(reduce_percent)
        #             x_vals_plot.append(batchsize_to_xpos[bs])
            
        #     ax2.plot(x_vals_plot, total_time_reduce_percent, marker='o', 
        #             linestyle='-', label=f'Baseline (workernum={thread})', 
        #             color=baseline_colors[thread])
        
        # # 设置图表属性
        # # ax2.set_title(f'{model} - 端到端时间降低百分比', fontsize=FONT_SIZE_LABEL+2, pad=20, fontproperties='SimHei')
        # ax2.set_xlabel('批大小（Batch Size）', fontsize=FONT_SIZE_LABEL, fontproperties='SimHei')
        # ax2.set_ylabel('端到端时间降低百分比 (%)', fontsize=FONT_SIZE_LABEL, fontproperties='SimHei')
        # ax2.set_xticks(x_pos)
        # ax2.set_xticklabels(all_batchsizes, fontsize=FONT_SIZE_TICK, fontproperties='SimHei')
        # ax2.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
        
        # # 修改图例位置：放在图表上方，水平排列
        # ax2.legend(loc='upper center', bbox_to_anchor=(0.5, 1.2), 
        #           ncol=3, fontsize=FONT_SIZE_LEGEND, frameon=False, 
        #           prop={'family': 'SimHei'})
        
        # ax2.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
        
        # # 确保y轴从0开始
        # ax2.set_ylim(bottom=0)
        
        # # 调整布局，为上方图例留出空间
        # plt.tight_layout(rect=[0, 0, 1, 0.95])
        
        # # 保存第二个图
        # filename2 = f"{output_dir}/{model}_total_time_reduction.png"
        # plt.savefig(filename2, dpi=300, bbox_inches='tight')
        # plt.close(fig2)
        # print(f"  已保存: {filename2}")
    
    print(f"\n所有独立图表已保存到目录: {output_dir}/")

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
        
        # 统一计算 total_time 和 data_time_percent
        baseline_df = calculate_times(baseline_df)
        ours_df = calculate_times(ours_df)
        
        # 初始化全局颜色映射
        baseline_threads = sorted(baseline_df['threadnum'].unique())
        custom_colors = [
            '#1f77b4',  # 蓝色 (workernum=1)
            '#2ca02c',  # 绿色 (workernum=4)
            '#ff0e22',  # 红色
        ]
        
        # 创建颜色映射字典
        baseline_colors = {thread: color for thread, color in zip(baseline_threads, custom_colors[:len(baseline_threads)])}
        
        # 为Ours设置单独颜色
        ours_color = '#ff7f0e'  # 橙色
        
        # 调用新函数生成并保存独立的图表
        print("\n开始生成独立图表...")
        save_individual_plots(baseline_df, ours_df, args.output_dir)
        
        print("\n所有图表生成完成!")
        print("独立图表保存到目录:", args.output_dir)

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