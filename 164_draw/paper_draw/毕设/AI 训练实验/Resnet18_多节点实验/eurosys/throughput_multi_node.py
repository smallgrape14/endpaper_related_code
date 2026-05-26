import os
import sys
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import argparse
from PIL import Image

# ===== 可调参数区 =====
# FONT_SIZE_LABEL = 12
# FONT_SIZE_TICK = 10
# FONT_SIZE_ANNOT = 10
# FONT_SIZE_LEGEND = 10
# FIG_WIDTH = 6
# FIG_HEIGHT = 4
# GRID_ALPHA = 0.5
# GRID_LINESTYLE = '--'
FIG_WIDTH = 8.8
FIG_HEIGHT = 6.8
GRID_ALPHA = 0.6
GRID_LINESTYLE = "--"
FONT_SIZE_LABEL = 32
FONT_SIZE_TICK = 30
FONT_SIZE_LEGEND = 28
FONT_SIZE_TITLE = 28


plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['DejaVu Sans','SimSun' ]  # Times + 宋体
plt.rcParams['mathtext.fontset'] = 'dejavusans'  # Math symbols matching Times
plt.rcParams['axes.unicode_minus'] = False
plt.rcParams['axes.formatter.use_mathtext'] = False
# plt.rcParams['xtick.direction'] = 'out'  # 将x周的刻度线方向设置向内
# plt.rcParams['ytick.direction'] = 'out'  # 将y轴的刻度方向设置向内

# 设置全局字体
plt.rcParams.update({
    'font.size': FONT_SIZE_TICK,
    'axes.labelsize': FONT_SIZE_LABEL,
    'legend.fontsize': FONT_SIZE_LEGEND,
    'xtick.labelsize': FONT_SIZE_TICK,
    'ytick.labelsize': FONT_SIZE_TICK
})

# ===== 模型顺序定义 =====
MODEL_ORDER = ['mobilenet_v2', 'shufflenet_v2_x1_0', 'resnet18', 'resnet50']

# ===== 全局颜色定义 =====
multi_node_colors = sns.color_palette("tab20", 20) 
batchsize_colors = sns.color_palette("husl", 6)

# 单个样本的大小，单位：B
SINGLE_SAMPLE_SIZE_B = 602240

def calculate_metrics(df):
    """
    计算 total_time 和 throughput。
    """
    df['total_time'] = df['data_avg_filter'] + df['train_avg_filter']
    df['throughput'] = df['batchsize'] / df['total_time']
    return df

def aggregate_data(base_dir):
    """
    遍历目录，汇总多机测试数据，并计算总吞吐量。
    返回一个包含每个节点吞吐量数据的DataFrame。
    """
    all_data = []

    if not os.path.isdir(base_dir):
        print(f"Error: 根目录 '{base_dir}' 不存在或不是一个目录。")
        return pd.DataFrame()

    for nodenum_dir in sorted(os.listdir(base_dir)):
        if "_servers" in nodenum_dir:
            nodenum_path = os.path.join(base_dir, nodenum_dir)
            num_nodes = int(nodenum_dir.split('_')[0])
            
            for server_dir in sorted(os.listdir(nodenum_path)):
                server_path = os.path.join(nodenum_path, server_dir)
                csv_path = os.path.join(server_path, 'time.csv')
                
                if os.path.exists(csv_path):
                    try:
                        df = pd.read_csv(csv_path)
                        required_cols = ['model', 'batchsize', 'data_avg_filter', 'train_avg_filter']
                        if not all(col in df.columns for col in required_cols):
                            print(f"Warning: 文件 '{csv_path}' 缺少必需的列，已跳过。")
                            continue
                            
                        df = calculate_metrics(df)
                        df['num_nodes'] = num_nodes
                        df['node_id'] = server_dir
                        all_data.append(df)
                            
                    except Exception as e:
                        print(f"处理文件 {csv_path} 时出错: {e}")
                        continue
    
    if not all_data:
        return pd.DataFrame()
        
    final_df = pd.concat(all_data, ignore_index=True)
    return final_df.sort_values(by=['model', 'batchsize', 'num_nodes'])


def plot_throughput_by_nodes(df_multi, unit='samples'):
    """
    绘制吞吐量随节点数量变化的趋势图（折线图）。
    """
    if df_multi.empty:
        print("没有可用于绘图的多机数据。")
        return

    sns.set_style("whitegrid")
    models = [m for m in MODEL_ORDER if m in df_multi['model'].unique()]
    
    num_models = len(models)
    num_cols = 1 if num_models == 1 else 2
    num_rows = (num_models + num_cols - 1) // num_cols
    
    unit_str = 'samples/s'
    file_suffix = ''
    if unit == 'gb':
        df_multi['total_throughput_gb'] = df_multi['total_throughput'] * SINGLE_SAMPLE_SIZE_B / (1024**3)
        unit_str = 'GB/s'
        file_suffix = '_gb'

    fig, axes = plt.subplots(num_rows, num_cols, figsize=(num_cols * FIG_WIDTH, num_rows * FIG_HEIGHT))
    if num_models == 1:
        axes = [axes]
    axes = np.array(axes).flatten()

    for i, model in enumerate(models):
        ax = axes[i]
        model_df = df_multi[df_multi['model'] == model].copy()
        
        if model_df.empty:
            continue
            
        nodes = sorted(model_df['num_nodes'].unique())
        all_batchsizes = sorted(model_df['batchsize'].unique())
        marker_list=['o', 's', 'D', '^', 'v', 'P', 'X', '*']  # 定义更多的标记样式，以适应更多的batchsize
        linestyle_list=['-', '--', '-.', ':','-', '--', '-.', ':']  # 定义更多的线型，以适应更多的batchsize
        print(batchsize_colors)
        for j, bs in enumerate(all_batchsizes):
            batchsize_df = model_df[model_df['batchsize'] == bs]
            throughput_vals = batchsize_df['total_throughput_gb'] if unit == 'gb' else batchsize_df['total_throughput']

            ax.plot(batchsize_df['num_nodes'], throughput_vals/1000,
                    marker=marker_list[j % len(marker_list)], linestyle=linestyle_list[j % len(linestyle_list)], label=f'Batchsize={bs}',
                    markersize=12, linewidth=2.5, 
                    color=batchsize_colors[j % len(batchsize_colors)])

        # ax.set_title(f'{model} - Throughput Scaling', fontsize=FONT_SIZE_LABEL)
        ax.set_xlabel('Number of Nodes', fontsize=FONT_SIZE_LABEL)
        ax.set_ylabel(f'Throughput (K {unit_str})', fontsize=FONT_SIZE_LABEL)
        ax.set_xticks(nodes)
        ax.set_yticks(np.arange(0, 15, 2))
        ax.tick_params(axis='x', labelsize=FONT_SIZE_TICK)
        ax.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
        # ax.legend(fontsize=FONT_SIZE_LEGEND)
        ax.grid(True, alpha=0.7,linestyle='--',linewidth=1.3)
        # ax.set_ylim(bottom=0)
        for spine in ax.spines.values():
            spine.set_color('black')
            spine.set_linewidth(1.5)
            spine.set_linestyle('-')  # 确保是实线（虽然默认就是实线）
    for j in range(i + 1, len(axes)):
        fig.delaxes(axes[j])
        
    plt.tight_layout()
    plt.savefig(f'throughput_by_nodes{file_suffix}.png', dpi=300)
    plt.savefig(f'throughput_by_nodes{file_suffix}.pdf', dpi=300)

    plt.close(fig)

def plot_scaling_efficiency(df_multi):
    """
    绘制吞吐量扩展效率图。
    横坐标是节点数，纵坐标是扩展效率百分比。
    """
    if df_multi.empty:
        print("没有可用于绘图的多机数据。")
        return

    sns.set_style("whitegrid")
    models = [m for m in MODEL_ORDER if m in df_multi['model'].unique()]
    
    num_models = len(models)
    num_cols = 1 if num_models == 1 else 2
    num_rows = (num_models + num_cols - 1) // num_cols
    fig, axes = plt.subplots(num_rows, num_cols, figsize=(num_cols * FIG_WIDTH, num_rows * FIG_HEIGHT))
    if num_models == 1:
        axes = [axes]
    axes = np.array(axes).flatten()

    for i, model in enumerate(models):
        ax = axes[i]
        model_df = df_multi[df_multi['model'] == model].copy()
        
        if model_df.empty:
            continue
            
        nodes = sorted(model_df['num_nodes'].unique())
        all_batchsizes = sorted(model_df['batchsize'].unique())
        
        single_node_df = model_df[model_df['num_nodes'] == 1].copy()
        single_node_df.set_index('batchsize', inplace=True)
        single_node_throughput = single_node_df['total_throughput'].to_dict()
        
        if not single_node_throughput:
            print(f"警告：模型 {model} 没有单节点数据，无法计算扩展效率。")
            continue
        marker_list=['o', 's', 'D', '^', 'v', 'P', 'X', '*']  # 定义更多的标记样式，以适应更多的batchsize
        linestyle_list=['-', '--', '-.', ':','-', '--', '-.', ':']  # 定义更多的线型，以适应更多的batchsize    
        for j, bs in enumerate(all_batchsizes):
            if bs not in single_node_throughput:
                continue
                
            batchsize_df = model_df[model_df['batchsize'] == bs].copy()
            batchsize_df['scaling_efficiency'] = (
                batchsize_df['total_throughput'] / (single_node_throughput[bs] * batchsize_df['num_nodes'])
            ) * 100
            
            ax.plot(batchsize_df['num_nodes'], batchsize_df['scaling_efficiency'],
                    marker=marker_list[j % len(marker_list)], linestyle=linestyle_list[j % len(linestyle_list)], label=f'Batchsize={bs}', 
                    markersize=12, linewidth=2.5,
                    color=batchsize_colors[j % len(batchsize_colors)])

        # ax.axhline(y=70, color='r', linestyle='--', linewidth=1, label='Target (70%)')

        # ax.set_title(f'{model} - Scaling Efficiency', fontsize=FONT_SIZE_LABEL)
        ax.set_xlabel('Number of Nodes', fontsize=FONT_SIZE_LABEL)
        ax.set_ylabel('Scaling Efficiency (%)', fontsize=FONT_SIZE_LABEL)

        ax.set_xticks(nodes, fontsize=FONT_SIZE_TICK,direction='out')
        # ax.tick_params(axis='x', labelsize=FONT_SIZE_TICK)
        # ax.tick_params(axis='y', labelsize=FONT_SIZE_TICK)
        ax.set_yticks(np.arange(70, 130, 10), fontsize=FONT_SIZE_TICK,direction='out')
        ax.tick_params(
            axis='both', 
            which='major', 
            direction='in',       # 刻度朝内
            length=6,             # 刻度长度
            width=1.0,             # 刻度宽度
            color='black',           # 明确设置黑色
            labelsize=FONT_SIZE_TICK,
            pad=8
        )
        # ax.legend(fontsize=FONT_SIZE_LEGEND)
        # ax.grid(True, alpha=GRID_ALPHA, linestyle=GRID_LINESTYLE)
        ax.grid(True, alpha=0.7,linestyle='--',linewidth=1.3)
        # ax.set_ylim(bottom=0, top=110)
        ax.set_xlim(left=0.5)
        # 设置所有边框为黑色实线
        for spine in ax.spines.values():
            spine.set_color('black')
            spine.set_linewidth(1.2)
            spine.set_linestyle('-')  # 确保是实线（虽然默认就是实线）

    # for j in range(i + 1, len(axes)):
    #     fig.delaxes(axes[j])
        
    plt.tight_layout()
    plt.savefig('throughput_scaling_efficiency.png', dpi=300)
    plt.savefig('throughput_scaling_efficiency.pdf', dpi=300)

    plt.close(fig)


   
def main():
    """
    主函数：解析命令行参数，读取数据，并调用绘图函数。
    """
    parser = argparse.ArgumentParser(description='Analyze and plot multi-node throughput scaling data.')
    parser.add_argument('data_dir', type=str, help='Path to the root directory of the test results.')
    parser.add_argument('-t', '--throughput_by_batchsize', action='store_true', help='Plot throughput vs. batchsize (samples/s).')
    parser.add_argument('-n', '--throughput_by_nodes', action='store_true', help='Plot throughput vs. number of nodes (samples/s).')
    parser.add_argument('-e', '--efficiency', action='store_true', help='Plot scaling efficiency vs. number of nodes.')
    parser.add_argument('-t_gb', '--throughput_by_batchsize_gb', action='store_true', help='Plot throughput vs. batchsize (GB/s).')
    parser.add_argument('-n_gb', '--throughput_by_nodes_gb', action='store_true', help='Plot throughput vs. number of nodes (GB/s).')
    parser.add_argument('-c', '--contribution', action='store_true', help='Plot stacked throughput bar charts showing individual node contribution.')
    parser.add_argument('-c_gb', '--contribution_gb', action='store_true', help='Plot stacked throughput bar charts showing individual node contribution (GB/s).')
    
    args = parser.parse_args()
    
    plot_all = not (args.throughput_by_batchsize or args.throughput_by_nodes or args.efficiency or
                    args.throughput_by_batchsize_gb or args.throughput_by_nodes_gb or
                    args.contribution or args.contribution_gb)
    
    try:
        print(f"正在聚合目录 '{args.data_dir}' 下的多机测试数据...")
        df_raw = aggregate_data(args.data_dir)
        
        if not df_raw.empty:
            print("数据聚合完成。开始绘制图表...")
            
            # 为折线图和效率图准备汇总数据
            df_sum = df_raw.groupby(['model', 'num_nodes', 'batchsize']).agg(
                total_throughput=('throughput', 'sum')
            ).reset_index()

            # if plot_all or args.throughput_by_batchsize:
            #     plot_throughput_by_batchsize(df_sum)
            
            if plot_all or args.throughput_by_nodes:
                plot_throughput_by_nodes(df_sum)

            # if plot_all or args.throughput_by_batchsize_gb:
            #     plot_throughput_by_batchsize(df_sum, unit='gb')

            # if plot_all or args.throughput_by_nodes_gb:
            #     plot_throughput_by_nodes(df_sum, unit='gb')
            
            if plot_all or args.efficiency:
                plot_scaling_efficiency(df_sum)

            # node_contribution_images_samples = []
            # if plot_all or args.contribution:
            #     node_contribution_images_samples.extend(plot_individual_node_contribution(df_raw, unit='samples'))
            
            # node_contribution_images_gb = []
            # if plot_all or args.contribution_gb:
            #     node_contribution_images_gb.extend(plot_individual_node_contribution(df_raw, unit='gb'))

            # if node_contribution_images_samples:
            #     merge_images(node_contribution_images_samples, 'merged_node_contribution_samples.png', cols=2)
            
            # if node_contribution_images_gb:
            #     merge_images(node_contribution_images_gb, 'merged_node_contribution_gb.png', cols=2)
            
            print("Plots generated successfully!")
            print("请检查当前目录下的 png 文件。")
        else:
            print("未找到有效的多机测试数据，无法生成图表。")
            sys.exit(0)

    except KeyError as e:
        print(f"Error: Missing column in one of the CSV files. Missing column: {e}")
        print("请确保所有 'time.csv' 文件都包含以下必需的列：'data_avg_filter', 'train_avg_filter', 'model', 'batchsize'。")
        sys.exit(1)
    except Exception as e:
        print(f"发生了一个意外错误: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()