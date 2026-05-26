import matplotlib.pyplot as plt
import numpy as np

# 数据准备（已移除(1,256)的数据点）
blocknum = [1, 1, 1, 1, 2, 2, 2, 3, 3, 4, 4]
threadnum = [1, 32, 64, 128, 32, 64, 128, 32, 64, 32, 64]
avg_data = [0.114, 0.125, 0.114, 0.112, 0.087, 0.108, 0.089, 0.048, 0.082, 0.033, 0.038]

# 创建图表
fig, ax = plt.subplots(figsize=(12, 6))

# 生成x轴标签和位置
x_labels = [f"({b},{t})" for b, t in zip(blocknum, threadnum)]
x = np.arange(len(x_labels))

# 绘制AVG_data折线
ax.plot(x, avg_data, 'b-o', label='AVG_data (s)', linewidth=2, markersize=8)
ax.set_xlabel('(blocknum, threadnum)', fontsize=14)
ax.set_ylabel('Processing Time (seconds)', color='b', fontsize=14)

# 调整刻度字体大小
ax.tick_params(axis='both', which='major', labelsize=12)
ax.tick_params(axis='both', which='minor', labelsize=10)
ax.tick_params(axis='y', labelcolor='b')

ax.set_xticks(x)
ax.set_xticklabels(x_labels, rotation=45, fontsize=12)

# 添加数据标签（调大字体到14）
for i, avg in enumerate(avg_data):
    ax.text(x[i], avg, f'{avg:.3f}', 
            ha='center', va='bottom', 
            color='b', fontsize=14,  # 从10调整为14
            bbox=dict(facecolor='white', alpha=0.7, edgecolor='none', pad=2))  # 添加白色背景提高可读性

# 图表装饰
plt.title('Ours: DATA_time Vs (Blocknum,Threadnum)', fontsize=16, pad=20)
ax.grid(True, linestyle='--', alpha=0.6)
ax.legend(loc='upper right', fontsize=12)

plt.tight_layout()

# 保存为JPG文件
plt.savefig('performance_analysis.jpg', dpi=300, quality=95, format='jpg')
print("图表已保存为 performance_analysis.jpg")
