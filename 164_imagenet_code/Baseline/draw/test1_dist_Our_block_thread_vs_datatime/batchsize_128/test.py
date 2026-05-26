import matplotlib.pyplot as plt
import numpy as np

# 新数据准备
blocknum = [1, 1, 1, 1, 2, 2, 3, 4]
threadnum = [1, 32, 64, 128, 32, 64, 32, 32]
avg_data = [0.06824, 0.05056, 0.03991, 0.04199, 0.02113, 0.03036, 0.01662, 0.01372]
delay_reduction = [0, 25.91, 41.52, 38.47, 69.04, 55.51, 75.64, 79.89]

# 创建图表和双纵轴
fig, ax1 = plt.subplots(figsize=(14, 7))
ax2 = ax1.twinx()

# 生成x轴标签和位置
x_labels = [f"({b},{t})" for b, t in zip(blocknum, threadnum)]
x = np.arange(len(x_labels))

# 绘制AVG_data折线（左轴）
line1 = ax1.plot(x, avg_data, 'b-o', label='Processing Time (s)', linewidth=2, markersize=10)
ax1.set_xlabel('(blocknum, threadnum)', fontsize=15)
ax1.set_ylabel('Processing Time (seconds)', color='b', fontsize=15)
ax1.tick_params(axis='y', labelcolor='b')
ax1.set_ylim(0, 0.08)  # 固定Y轴范围

# 绘制延时降低比例折线（右轴）
line2 = ax2.plot(x, delay_reduction, 'r--s', label='Delay Reduction (%)', linewidth=2, markersize=10)
ax2.set_ylabel('Delay Reduction (%)', color='r', fontsize=15)
ax2.tick_params(axis='y', labelcolor='r')
ax2.set_ylim(0, 100)  # 固定Y轴范围
ax2.axhline(0, color='gray', linestyle='--', linewidth=1)  # 基准线

# 调整刻度字体大小
ax1.tick_params(axis='both', which='major', labelsize=13)
ax1.tick_params(axis='both', which='minor', labelsize=11)
ax2.tick_params(axis='both', which='major', labelsize=13)
ax2.tick_params(axis='both', which='minor', labelsize=11)

ax1.set_xticks(x)
ax1.set_xticklabels(x_labels, rotation=45, fontsize=13)

# 添加数据标签
for i, (avg, red) in enumerate(zip(avg_data, delay_reduction)):
    ax1.text(x[i], avg, f'{avg:.5f}', ha='center', va='bottom', 
             color='b', fontsize=13, bbox=dict(facecolor='white', alpha=0.8, pad=3))
    ax2.text(x[i], red, f'{red:.2f}%', ha='center', va='top', 
             color='r', fontsize=13, bbox=dict(facecolor='white', alpha=0.8, pad=3))

# 图表装饰
plt.title('Performance Analysis: data_time vs. Delay Reduction (Batchsize=128)', fontsize=17, pad=25)
ax1.grid(True, linestyle='--', alpha=0.7)

# 合并图例
lines = line1 + line2
labels = [l.get_label() for l in lines]
ax1.legend(lines, labels, loc='center left', fontsize=13)

plt.tight_layout()
plt.savefig('performance_new_data.jpg', dpi=350, quality=100)
print("图表已保存为 performance_new_data.jpg")
