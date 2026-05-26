import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# ===== 可调参数区 =====
FIG_WIDTH = 8.8
FIG_HEIGHT = 7.6
GRID_ALPHA = 0.3
GRID_LINESTYLE = "--"
FONT_SIZE_LABEL = 28
FONT_SIZE_TICK = 26
FONT_SIZE_LEGEND = 20
FONT_SIZE_TITLE = 28

# ===== 字体配置 =====
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['DejaVu Sans']
plt.rcParams['mathtext.fontset'] = 'dejavusans'
plt.rcParams['axes.unicode_minus'] = False
plt.rcParams['axes.formatter.use_mathtext'] = False

# 设置全局字体
plt.rcParams.update({
    'font.size': FONT_SIZE_TICK,
    'axes.labelsize': FONT_SIZE_LABEL,
    'legend.fontsize': FONT_SIZE_LEGEND,
    'xtick.labelsize': FONT_SIZE_TICK,
    'ytick.labelsize': FONT_SIZE_TICK
})

# ===== 1️⃣ 创建虚拟数据只是为了获得图例句柄 =====
# 不需要完整数据，只需要创建一些虚拟线条来获取图例
plt.figure(figsize=(1, 1), dpi=100)  # 小图即可

# 定义颜色
colors = {
    'POSIX': "#1D4F91",
    'OUR': "#F28B82",
    'GDS': "#B388EB"
}

# 线型
linestyles = {
    'POSIX': '--',
    'OUR': '-',
    'GDS': '-'
}

# 标记
markers = {
    'POSIX': 'o',
    'OUR': 'P',
    'GDS': 'X'
}

# 创建虚拟线条来获得图例句柄
dummy_x = [0, 1]
dummy_y = [0, 1]

# 绘制虚拟线条
line_posix, = plt.plot(dummy_x, dummy_y, 
                       marker=markers['POSIX'], color=colors['POSIX'], 
                       linewidth=2.2, markersize=7, label='POSIX(CPU)', 
                       linestyle=linestyles['POSIX'], fillstyle='none')

line_gds, = plt.plot(dummy_x, dummy_y, 
                     marker=markers['GDS'], color=colors['GDS'], 
                     linewidth=2.2, markersize=8, label='GDS', 
                     linestyle=linestyles['GDS'])

line_our, = plt.plot(dummy_x, dummy_y, 
                     marker=markers['OUR'], color=colors['OUR'], 
                     linewidth=2.2, markersize=8, label='GD²FS(GPU)', 
                     linestyle=linestyles['OUR'])

lines = [line_posix, line_gds, line_our]
labels = [l.get_label() for l in lines]

# 关闭虚拟图形
plt.close()

# ===== 2️⃣ 创建专门用于图例的图形 =====
# 创建一个横向的长条形图例
fig_legend = plt.figure(figsize=(12, 1.0), dpi=100)  # 宽度12，高度1.0
ax_legend = fig_legend.add_subplot(111)
ax_legend.axis('off')  # 关闭坐标轴

# 创建图例，横向排列
legend = ax_legend.legend(
    lines, labels,
    loc='center',
    ncol=3,  # 3列横向排列
    fontsize=FONT_SIZE_LEGEND ,  # 稍微增大一点
    # frameon=False,  # 无边框
    handletextpad=0.5,  # 图例句柄和文本间距
    columnspacing=1.5,  # 列间距
    labelspacing=0.3,  # 标签间距
    markerscale=1.2,  # 标记大小缩放
    frameon=True,  # ✅ 开启边框
    fancybox=True,  # ✅ 使用圆角边框
    # shadow=False,  # 无阴影
    edgecolor="#cccccc",  # ✅ 边框颜色
    facecolor='white'  # ✅ 背景颜色
)

plt.tight_layout(pad=0)  # 移除内边距

# ===== 3️⃣ 保存图例 =====
legend_output_png = 'read_write_KIOPS_legend_only.png'
legend_output_pdf = 'read_write_KIOPS_legend_only.pdf'

plt.savefig(legend_output_png, dpi=300, bbox_inches='tight', pad_inches=0.05, facecolor='white')
plt.savefig(legend_output_pdf, dpi=300, bbox_inches='tight', pad_inches=0.05, facecolor='white')

plt.close()

print(f"✅ 图例PNG已保存为: {legend_output_png}")
print(f"✅ 图例PDF已保存为: {legend_output_pdf}")

# 可选：显示图例预览
fig_preview = plt.figure(figsize=(8, 0.8), dpi=100)
ax_preview = fig_preview.add_subplot(111)
ax_preview.axis('off')
ax_preview.legend(lines, labels, loc='center', ncol=3, fontsize=16, frameon=False)
plt.tight_layout(pad=0)
plt.show()