import matplotlib.pyplot as plt
import numpy as np

# 数据准备
blocknum = [1, 1, 1, 1, 2, 2, 2, 3, 3, 4, 4]
threadnum = [1, 32, 64, 128, 32, 64, 128, 32, 64, 32, 64]
avg_data = [0.114, 0.125, 0.114, 0.112, 0.087, 0.108, 0.089, 0.048, 0.082, 0.033, 0.038]

# 创建图表和双纵轴
fig, ax = plt.subplots(figsize=(12, 6))

# 生成x轴标签和位置
x_labels = [f"({b},{t})" for b, t in zip(blocknum, threadnum)]
x = np.arange(len(x_labels))

# 绘制第一条线（原始数据）
line1, = ax.plot(x, avg_data, 'b-o', label='Original Data', linewidth=2, markersize=8, zorder=3)

# 绘制第二条线（偏移数据）
offset = 0.03  # 固定垂直偏移量
scaled_data = [d + offset for d in avg_data]
line2, = ax.plot(x, scaled_data, 'r--s', label='Offset Data (+0.03)', linewidth=2, markersize=8, zorder=2)

# 设置轴标签和刻度
ax.set_xlabel('(blocknum, threadnum)', fontsize=14)
ax.set_ylabel('Processing Time (seconds)', fontsize=14)
ax.set_xticks(x)
ax.set_xticklabels(x_labels, rotation=45, fontsize=12)
ax.tick_params(axis='both', which='major', labelsize=12)

# 添加数据标签
for i, (d1, d2) in enumerate(zip(avg_data, scaled_data)):
    ax.text(x[i], d1, f'{d1:.3f}', ha='center', va='bottom', color='b', fontsize=12, 
            bbox=dict(facecolor='white', alpha=0.8, edgecolor='none', pad=2))
    ax.text(x[i], d2, f'{d1:.3f}', ha='center', va='bottom', color='r', fontsize=12,
            bbox=dict(facecolor='white', alpha=0.8, edgecolor='none', pad=2))

# 图表装饰
plt.title('Performance Analysis with Separated Lines', fontsize=16, pad=20)
ax.grid(True, linestyle='--', alpha=0.6)
ax.legend(fontsize=12)

plt.tight_layout()
plt.savefig('performance_correct.jpg', dpi=300, quality=95)
print("图表已保存为 performance_correct.jpg")
