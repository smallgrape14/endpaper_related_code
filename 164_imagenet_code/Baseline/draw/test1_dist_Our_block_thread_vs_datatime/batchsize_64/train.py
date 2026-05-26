import matplotlib.pyplot as plt
import numpy as np

# 新数据准备（Batchsize=64）
blocknum = [1, 1, 1, 2]
threadnum = [1, 32, 64, 32]
avg_data = [0.04684, 0.02617, 0.02610, 0.02817]
delay_reduction = [0, 44.13, 44.29, 39.86]

# 创建图表和双纵轴
fig, ax1 = plt.subplots(figsize=(12, 6))
ax2 = ax1.twinx()

# 生成x轴标签和位置
x_labels = [f"({b},{t})" for b, t in zip(blocknum, threadnum)]
x = np.arange(len(x_labels))

# 绘制AVG_data折线（左轴）
line1 = ax1.plot(x, avg_data, 'b-o', label='Processing Time (s)', linewidth=2, markersize=10)
ax1.set_xlabel('(blocknum, threadnum)', fontsize=14)
ax1.set_ylabel('Processing Time (seconds)', color='b', fontsize=14)
ax1.tick_params(axis='y', labelcolor='b')
ax1.set_ylim(0, 0.05)  # 调整Y轴范围适应新数据

# 绘制延时降低比例折线（右轴）
line2 = ax2.plot(x, delay_reduction, 'r--s', label='Delay Reduction (%)', linewidth=2, markersize=10)
ax2.set_ylabel('Delay Reduction (%)', color='r', fontsize=14)
ax2.tick_params(axis='y', labelcolor='r')
ax2.set_ylim(0, 50)  # 调整Y轴范围适应新数据
ax2.axhline(0, color='gray', linestyle='--', linewidth=1)  # 基准线

# 调整刻度字体大小
ax1.tick_params(axis='both', which='major', labelsize=12)
ax2.tick_params(axis='both', which='major', labelsize=12)

ax1.set_xticks(x)
ax1.set_xticklabels(x_labels, rotation=45, fontsize=12)

# 添加数据标签
for i, (avg, red) in enumerate(zip(avg_data, delay_reduction)):
    ax1.text(x[i], avg, f'{avg:.5f}', ha='center', va='bottom', 
             color='b', fontsize=12, bbox=dict(facecolor='white', alpha=0.8, pad=2))
    ax2.text(x[i], red, f'{red:.2f}%', ha='center', va='top', 
             color='r', fontsize=12, bbox=dict(facecolor='white', alpha=0.8, pad=2))

# 图表装饰
plt.title('Performance Analysis: data_time vs. Delay Reduction (Batchsize=64)', fontsize=16, pad=20)
ax1.grid(True, linestyle='--', alpha=0.6)

# 合并图例
lines = line1 + line2
labels = [l.get_label() for l in lines]
ax1.legend(lines, labels, loc='upper left', fontsize=12)

plt.tight_layout()
plt.savefig('performance_batch64.jpg', dpi=350, quality=95)
print("图表已保存为 performance_batch64.jpg")
