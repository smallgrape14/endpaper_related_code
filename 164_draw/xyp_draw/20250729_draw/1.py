import matplotlib
import matplotlib.pyplot as plt
import numpy as np

def draw_comparison_chart(batch_sizes, baseline_times, ours_times, improvement_percent, res_name):
    plt.rcParams['font.sans-serif'] = ['SimHei']  # 或者其他支持中文的字体，例如 'SimHei' 或 'FangSong'
    plt.rcParams['axes.unicode_minus'] = False  # 解决负号显示为方块的问题

    # 创建等距的x坐标位置
    x_positions = np.arange(len(batch_sizes))

    # 创建画布和双坐标轴
    fig, ax1 = plt.subplots(figsize=(10, 6))
    plt.title('数据处理时间对比与性能提升分析', fontsize=14, pad=20)

    # 绘制处理时间折线图（线性坐标）
    ax1.set_xlabel('Batch Size', fontsize=18)
    ax1.set_ylabel('Data Time (us)', fontsize=18)  # 单位改为微秒
    ax1.grid(True, linestyle='--', alpha=0.7)

    line1, = ax1.plot(x_positions, baseline_times, 's-', color='royalblue', 
                    markersize=8, linewidth=2.5, label='Baseline')
    line2, = ax1.plot(x_positions, ours_times, 'o-', color='crimson', 
                    markersize=8, linewidth=2.5, label='Ours')

    # 设置x轴刻度和标签
    ax1.set_xticks(x_positions)
    ax1.set_xticklabels(batch_sizes)

    # 创建第二个Y轴显示提升百分比
    ax2 = ax1.twinx()
    ax2.set_ylabel('性能提升百分比 (%)', fontsize=18, color='forestgreen')
    ax2.tick_params(axis='y', labelcolor='forestgreen')

    # 绘制柱状图显示提升比例（调整宽度和位置）
    bar_width = 0.6
    bars = ax2.bar(x_positions, improvement_percent, width=bar_width, alpha=0.4, 
                color='limegreen', edgecolor='darkgreen', linewidth=1.2, 
                label='提升百分比')

    # 在柱状图顶部添加数据标签
    for i, rect in enumerate(bars):
        height = rect.get_height()
        ax2.text(rect.get_x() + rect.get_width()/2., height,
                f'{height:.2f}%', ha='center', va='bottom', 
                fontsize=18, color='darkgreen', weight='bold')

    # 添加组合图例
    lines = [line1, line2]
    labels = [l.get_label() for l in lines]
    ax1.legend(lines, labels, loc='upper left', fontsize=20)

    # 添加柱状图的图例
    ax2.legend([bars], ['提升百分比'], loc='lower right', fontsize=20)

    plt.tight_layout()

    # 添加分析结论标注
    # plt.figtext(0.5, 1, "分析结论：Ours方法在较大Batch Size下优势显著，当Batch Size=512时性能提升达94.74%", ha="center", fontsize=11, bbox={"facecolor":"lightyellow", "alpha":0.4, "pad":5})

    # 保存高清图像
    plt.savefig(res_name, dpi=300, bbox_inches='tight')
    plt.show()
    
    
if __name__ == '__main__':
    mode = 2
    if mode == 1: #单GPU
        batch_sizes = [16, 32, 64, 128, 256, 512]
        # 转换为微秒(us)
        baseline_times = [t * 1e6 for t in [0.00237, 0.00540, 0.01818, 0.06015, 0.23704, 0.82261]]
        ours_times = [t * 1e6 for t in [0.00222, 0.00376, 0.00628, 0.01126, 0.02395, 0.04329]]
        # improvement_percent = [6.17, 30.29, 65.47, 81.28, 89.90, 94.74]
        res_name = '1.1.png'
    else:#多GPU
        batch_sizes = [16, 32, 64, 128, 256, 512]
        # 转换为微秒(us)
        baseline_times = [t * 1e6 for t in [0.02178, 0.04543, 0.10038, 0.23121, 0.56707, 1.31692]]
        ours_times = [t * 1e6 for t in [0.00726, 0.00758, 0.00624, 0.01276, 0.02406, 0.04589]]
        # improvement_percent = [66.67, 83.32, 93.78, 94.48, 95.76, 94.42]
        res_name = '1.2.png'
    
    # (baseline_times - ours_times) / baseline_times * 100
    improvement_percent = [
        (base - ours) / base * 100 if base > 0 else 0 
        for base, ours in zip(baseline_times, ours_times)
    ]
    # 调用绘图函数
    draw_comparison_chart(batch_sizes, baseline_times, ours_times, improvement_percent, res_name)
    print(f"图像已保存为 {res_name}")