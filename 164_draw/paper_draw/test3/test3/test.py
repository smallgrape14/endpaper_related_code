import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# 数据集（仅包含指定的5条折线）
data = {
    "batch_size": [16, 32, 64, 128, 256, 512],
    "Baseline worker=8": [0.001356369, 0.00496969, 0.01492066, 0.028603378, 0.076399502, 0.157854138],
    "Baseline worker=16": [0.001336731, 0.002014723, 0.003644474, 0.006998049, 0.0135518, 0.024252935],
    "Ours prefetch=1": [0.002936787, 0.003732909, 0.006691542, 0.013216008, 0.023752612, 0.051714178],
    "Ours prefetch=2": [0.001782213, 0.002713001, 0.004117876, 0.007672081, 0.017764, np.nan],
    "Ours prefetch=3": [0.002025039, 0.003035537, 0.003951914, 0.008269395, 0.015095616, np.nan]
}
df = pd.DataFrame(data)

# 创建画布（尺寸与之前一致）
plt.figure(figsize=(14, 8), dpi=120)  # 14x8英寸画布
plt.rcParams['font.family'] = 'DejaVu Sans'  # 字体兼容中文

# --- 严格保持原始配色和样式 [1,3](@ref) ---
color_map = {
    "Baseline worker=8": "#9467bd",   # 紫色
    "Baseline worker=16": "#8c564b",  # 棕色
    "Ours prefetch=1": "#e377c2",     # 粉色
    "Ours prefetch=2": "#7f7f7f",     # 灰色
    "Ours prefetch=3": "#bcbd22"      # 黄绿色
}
marker_map = {
    "Baseline worker=8": "D",        # 菱形
    "Baseline worker=16": "P",        # 五角星
    "Ours prefetch=1": "*",           # 星形
    "Ours prefetch=2": "X",           # X形
    "Ours prefetch=3": "d"            # 薄菱形
}
linestyle_map = {
    "Baseline worker=8": "--",        # 虚线
    "Baseline worker=16": "--",       # 虚线
    "Ours prefetch=1": "-",            # 实线
    "Ours prefetch=2": "-",            # 实线
    "Ours prefetch=3": "-"            # 实线
}

# 绘制折线（复用原始样式参数）
for col in df.columns[1:]:
    plt.plot(
        df['batch_size'], 
        df[col],
        label=col,
        color=color_map[col],          # 固定颜色
        linestyle=linestyle_map[col],  # 固定线型
        marker=marker_map[col],        # 固定标记
        markersize=12,                  # 标记大小不变
        linewidth=2.5,                 # 线宽不变
        markeredgewidth=1.5,           # 标记描边宽度不变
        markeredgecolor='white'        # 白色描边增强对比度 [5](@ref)
    )

# --- 坐标轴与辅助元素（与之前完全一致）---
plt.xscale('log', base=2)  # 对数坐标
plt.yscale('log')
plt.xticks(df['batch_size'], labels=df['batch_size'], fontsize=16)
plt.yticks(fontsize=16)
plt.xlabel("Batch Size", fontsize=16, labelpad=10)
plt.ylabel("Processing Time (s) - Lower is Better", fontsize=16, labelpad=10)
plt.ylim(0.001, 0.2)  # 聚焦核心对比区间 [4](@ref)

# 网格和标题设置
plt.grid(True, which='both', linestyle='--', alpha=0.4)  # 半透明虚线网格 [2](@ref)
plt.title("Data Processing Time Comparison", fontsize=20, pad=20)

# 图例位置与样式（右上角外部）[1](@ref)
plt.legend(
    loc='upper left', 
    bbox_to_anchor=(1.05, 1), 
    frameon=True, 
    shadow=True,
    fontsize=12,
    title='Configuration',
    title_fontsize='13'
)

# # 复用原始注释（Prefetch=2超越点）
# plt.annotate(
#     'Prefetch=2 surpasses\nBaseline worker=16', 
#     xy=(128, 0.007), 
#     xytext=(200, 0.02),
#     arrowprops=dict(arrowstyle="->", color='black', lw=1.5),
#     fontsize=11,
#     bbox=dict(boxstyle="round,pad=0.3", fc="white", ec="gray", alpha=0.8)
# )

# 输出文件（命名格式延续）
plt.tight_layout()
plt.savefig('focus_comparison.png', bbox_inches='tight', dpi=300)
plt.savefig('focus_comparison.pdf', format='pdf', bbox_inches='tight')
plt.show()