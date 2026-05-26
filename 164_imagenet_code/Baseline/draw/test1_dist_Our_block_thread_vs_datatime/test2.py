import matplotlib.pyplot as plt
import numpy as np

# 数据准备
# blocknum = [1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 4, 4]
# threadnum = [1, 32, 64, 128, 256, 32, 64, 128, 32, 64, 32, 64]
# avg_data = [0.114, 0.125, 0.114, 0.112, 0.216, 0.087, 0.108, 0.089, 0.048, 0.082, 0.033, 0.038]
# delay_reduction = [0, -9.63, 0.25, 1.51, -89.73, 23.45, 5.34, 22.34, 57.55, 28.12, 71.00, 67.10]
blocknum = [1, 1, 1, 1,  2, 2, 2, 3, 3, 4, 4]
threadnum = [1, 32, 64, 128,  32, 64, 128, 32, 64, 32, 64]
avg_data = [0.114, 0.125, 0.114, 0.112,  0.087, 0.108, 0.089, 0.048, 0.082, 0.033, 0.038]
delay_reduction = [0, -9.63, 0.25, 1.51,  23.45, 5.34, 22.34, 57.55, 28.12, 71.00, 67.10]
# 创建图表和双纵轴
fig, ax1 = plt.subplots(figsize=(14, 6))
ax2 = ax1.twinx()

# 绘制AVG_data折线（左轴）
x_labels = [f"({b},{t})" for b, t in zip(blocknum, threadnum)]
x = np.arange(len(x_labels))
ax1.plot(x, avg_data, 'b-o', label='AVG_data (s)', linewidth=2, markersize=8)
ax1.set_xlabel('(blocknum, threadnum)', fontsize=14)
ax1.set_ylabel('data_time (seconds)', color='b', fontsize=14)
ax1.tick_params(axis='both', which='major', labelsize=12)
ax1.tick_params(axis='both', which='minor', labelsize=10)
ax1.tick_params(axis='y', labelcolor='b')
ax1.set_xticks(x)
ax1.set_xticklabels(x_labels, rotation=45)

# 绘制延时降低比例折线（右轴）
ax2.plot(x, delay_reduction, 'r--s', label='Delay Reduction (%)', linewidth=2, markersize=8)
ax2.set_ylabel('Delay Reduction (%)', color='r', fontsize=14)
ax2.tick_params(axis='both', which='major', labelsize=12)
ax2.tick_params(axis='both', which='minor', labelsize=10)
ax2.tick_params(axis='y', labelcolor='r')
ax2.axhline(0, color='gray', linestyle='--', linewidth=1)  # 基准线

# 添加数据标签
for i, (avg, red) in enumerate(zip(avg_data, delay_reduction)):
    ax1.text(x[i], avg, f'{avg:.3f}', ha='center', va='bottom', color='b',fontsize=14, bbox=dict(facecolor='white', alpha=0.7, edgecolor='none', pad=2))
    ax2.text(x[i], red, f'{red:.2f}%', ha='center', va='top', color='r',fontsize=14, bbox=dict(facecolor='white', alpha=0.7, edgecolor='none', pad=2))

# 图表装饰
plt.title('Performance Analysis: data_time vs. Delay Reduction', fontsize=16, pad=20)
ax1.grid(True, linestyle='--', alpha=0.6)
lines1, labels1 = ax1.get_legend_handles_labels()
lines2, labels2 = ax2.get_legend_handles_labels()
# ax1.legend(lines1 + lines2, labels1 + labels2, loc='upper left')
ax1.legend(lines1 + lines2, labels1 + labels2, loc='center left')


plt.tight_layout()
plt.show()
plt.savefig("test2.jpg")