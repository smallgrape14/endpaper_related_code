import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch

def box(ax, xy, text, color):
    x, y = xy
    rect = FancyBboxPatch(
        (x, y), 2.6, 0.9,
        boxstyle="round,pad=0.02",
        linewidth=1.5,
        edgecolor="black",
        facecolor=color
    )
    ax.add_patch(rect)
    ax.text(x+1.3, y+0.45, text, ha='center', va='center', fontsize=9)

def arrow(ax, p1, p2, color, style='-'):
    ax.add_patch(FancyArrowPatch(
        p1, p2,
        arrowstyle='->',
        linewidth=1.8,
        color=color,
        linestyle=style
    ))

fig, ax = plt.subplots(figsize=(10, 6))
ax.set_xlim(0, 12)
ax.set_ylim(0, 8)
ax.axis('off')

# ======================
# Storage / Network
# ======================
box(ax, (4.5, 7), "BeeGFS Storage", "#d9d9d9")
box(ax, (4.5, 6), "RDMA Network", "#cce5ff")
box(ax, (4.5, 5), "RNIC / DMA Engine", "#cce5ff")

# ======================
# GPU Data Path
# ======================
box(ax, (2, 4), "GPU-RDMA Pipeline", "#b6f2c2")
box(ax, (6, 4), "Double Buffer Queue", "#b6f2c2")
box(ax, (6, 2.8), "GPU Memory", "#ffcccc")
box(ax, (9, 2.8), "GPU Compute", "#ff9999")

# ======================
# Control Path
# ======================
box(ax, (1, 2.5), "DataLoader", "#ffe0b3")
box(ax, (1, 1.5), "CPU Controller", "#ffe0b3")

# ======================
# DATA PATH (blue)
# ======================
arrow(ax, (5.8, 7), (5.8, 6), "blue")
arrow(ax, (5.8, 6), (5.8, 5), "blue")
arrow(ax, (5.8, 5), (3.3, 4.5), "blue")
arrow(ax, (4.6, 4.5), (6.3, 3.7), "blue")
arrow(ax, (7.3, 3.7), (7.3, 3.2), "blue")
arrow(ax, (7.3, 3.2), (9.2, 3.2), "blue")

# ======================
# CONTROL PATH (orange)
# ======================
arrow(ax, (2.3, 2.5), (2.3, 2), "orange")
arrow(ax, (2.3, 2), (3.2, 4), "orange")
arrow(ax, (2.3, 2), (6.5, 4), "orange")

# overlap feedback
arrow(ax, (9, 3.2), (2.3, 1.8), "gray", "--")

plt.tight_layout()

# 输出论文可用文件
plt.savefig("gpu_io_architecture.pdf", bbox_inches='tight')
plt.savefig("gpu_io_architecture.png", dpi=300, bbox_inches='tight')

plt.show()