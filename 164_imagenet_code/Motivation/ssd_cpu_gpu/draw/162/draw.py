import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# 创建数据
# 使用新数据
data = {
    'IOsize(KB)': [4, 8, 16, 32, 64, 128, 256],
    'CPU': [2095.5481, 2075.7202, 1935.5423, 1985.761, 2590.4447, 2264.7868, 2212.785],
    'GDS': [886.871587, 1058.933644, 1128.921473, 1135.550565, 1188.220847, 1433.227873, 1683.355607],
    '提速百分比': [57.68, 48.98, 41.67, 42.82, 54.13, 36.72, 23.93]
}
df = pd.DataFrame(data)

# 设置中文字体
plt.rcParams['font.sans-serif'] = ['Noto Sans CJK JP']
plt.rcParams['axes.unicode_minus'] = False

# 创建图表
fig, ax = plt.subplots(figsize=(12, 7))

# 绘制CPU和GDS延迟曲线
line1 = ax.plot(df['IOsize(KB)'], df['CPU'], marker='o', label='CPU延迟', 
               color='#1f77b4', linewidth=2, markersize=8)
line2 = ax.plot(df['IOsize(KB)'], df['GDS'], marker='s', label='GDS延迟', 
               color='#ff7f0e', linewidth=2, markersize=8)

# 为每个点添加箭头和提速百分比标签
for i, row in df.iterrows():
    x = row['IOsize(KB)']
    cpu_y = row['CPU']
    gds_y = row['GDS']
    speedup = row['提速百分比']
    
    # 计算中间点位置用于标签
    mid_y = (cpu_y + gds_y) / 2
    
    # 添加垂直箭头
    ax.annotate('', xy=(x, gds_y), xytext=(x, cpu_y),
                arrowprops=dict(arrowstyle='<->', color='green', lw=1.5))
    
    # 添加提速百分比标签
    ax.text(x, mid_y, f'{speedup}%', ha='center', va='center', 
            fontsize=14, fontweight='bold', color='darkgreen',
            bbox=dict(boxstyle="round,pad=0.3", facecolor="lightyellow", edgecolor="darkgreen", alpha=0.8))

# 设置坐标轴标签
ax.set_xlabel('IOsize(KB)', fontsize=16)
ax.set_ylabel('延迟 (us)', fontsize=16)

# 设置网格
ax.grid(True, alpha=0.3, linestyle='--')

# 设置X轴为对数刻度
ax.set_xscale('log')
ax.set_xticks(df['IOsize(KB)'],fontsize=14)
ax.set_xticklabels([f"{x}" for x in df['IOsize(KB)']],fontsize=14)

# 设置Y轴范围，留出一些空间给箭头标签
ax.set_ylim(400, 2600)

# 调整Y轴刻度字体大小
ax.tick_params(axis='y', labelsize=14)
# 添加图例
ax.legend(fontsize=12, loc='upper right')

# 设置标题
plt.title('不同IOsize下的延迟对比（提速百分比以箭头标注）', fontsize=14, fontweight='bold', pad=20)

# 添加网格线
ax.yaxis.grid(True, alpha=0.2)
ax.xaxis.grid(True, alpha=0.2)

# 调整布局
plt.tight_layout()

# 保存图表
plt.savefig('latency_comparison_with_arrows_162.png', dpi=300, bbox_inches='tight')
print("图表已保存为: latency_comparison_with_arrows.png")

# 显示图表
plt.show()