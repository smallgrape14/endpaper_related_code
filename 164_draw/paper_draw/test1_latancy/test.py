import matplotlib.pyplot as plt
import numpy as np
import matplotlib.ticker as ticker

# 数据准备
thread_num = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512]
baseline = [8.092500, 4.243920, 1.942470, 1.260100, 0.551010, 0.310621, 0.308982, 0.679518, 1.122690, 2.262990]
ours = [1.241239174, 0.927576187, 0.63956302, 0.557203681, 0.462035282, 0.192651263, 0.173545453, 0.167072829, 0.156637784, 0.15781712]

# 设置柱状图参数
x = np.arange(len(thread_num))
width = 0.35  # 柱宽

# 创建图表
plt.figure(figsize=(14, 7))
ax = plt.subplot()

# # 使用新的配色方案：深青为Baseline，橘红为Ours
# rects1 = ax.bar(x - width/2, baseline, width, label='Baseline', color='#1a5276', alpha=0.85)
# rects2 = ax.bar(x + width/2, ours, width, label='Ours(Block=2)', color='#d35400', alpha=0.85)
# 使用浅色配色方案：浅蓝色为Baseline，浅橙色为Ours
rects1 = ax.bar(x - width/2, baseline, width, label='Baseline', color='#7FB3D5', alpha=0.85)
rects2 = ax.bar(x + width/2, ours, width, label='Ours(Block=2)', color='#F8C471', alpha=0.85)
# 添加标签和标题
ax.set_xlabel('Thread Number', fontsize=13, labelpad=12)
ax.set_ylabel('Read Latancy (s)', fontsize=13, labelpad=12)
ax.set_title('Baseline vs Ours(BlockNum=2) ThreadNum vs Read Latancy IO_Size=1KB', fontsize=15, pad=15)
ax.set_xticks(x)
ax.set_xticklabels(thread_num)
ax.legend(frameon=False, fontsize=12)

# 设置Y轴刻度0.2为步长
ax.yaxis.set_major_locator(ticker.MultipleLocator(0.2))
ax.grid(axis='y', linestyle='--', alpha=0.4)

# 为所有柱子添加数值标签（水平显示）
def add_labels(rects):
    for rect in rects:
        height = rect.get_height()
        ax.annotate(f'{height:.3f}',
                    xy=(rect.get_x() + rect.get_width() / 2, height),
                    xytext=(0, 3),
                    textcoords="offset points",
                    ha='center', va='bottom', fontsize=9,
                    rotation=0)  # 水平显示标签

add_labels(rects1)
add_labels(rects2)

# 设置对数刻度
# plt.yscale('log')  # 保持对数刻度确保小数值可见性
plt.yscale('log', base=2)
ax.set_ylim(bottom=0.1)  # 设置y轴下限

# 自动调整布局
plt.tight_layout(pad=2.0)

# 显示图表
plt.savefig('performance_comparison.png', dpi=300, bbox_inches='tight')
plt.show()
