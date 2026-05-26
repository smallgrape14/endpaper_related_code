#### 86.5.1 五主线固定顺序

固定 legend / 绘制顺序如下：

1. `POSIX(CPU)`
3. `GDS`
5. `OURS(GPU)`

若某张图只出现其中一部分，则使用这个总顺序的子序列；不允许按“谁先读到 CSV 就先画谁”的方式漂移 legend 顺序。

#### 86.5.2 五主线固定样式

| canonical key | legend | color | marker | linestyle |
|---|---|---|---|---|
| `posix_cpu` | `POSIX(CPU)` | `#1D4F91` | `o`（空心） | `--` |
| `gds` | `GDS` | `#B388EB` | `X` | `-` |
| `offload_gpu` | `OURS(GPU)` | `#F28B82` | `P` | `-` |


横坐标：主机并发线程数
吞吐量 (MiB/s)


test/bench_offload_meta/v73_plot_style.py



SERIES_SPECS = {
    "posix_cpu": {
        "label": "POSIX(CPU)",
        "color": "#1D4F91",
        "marker": "o",
        "linestyle": "--",
        "linewidth": 2.2,
        "markersize": 7.0,
        "markerfacecolor": "none",
        "markeredgecolor": "#1D4F91",
        "markeredgewidth": 1.8,
    },

    "gds": {
        "label": "GDS",
        "color": "#B388EB",
        "marker": "X",
        "linestyle": "-",
        "linewidth": 2.2,
        "markersize": 8.0,
    },
    "offload_gpu": {
        "label": "OURS(GPU)",
        "color": "#F28B82",
        "marker": "P",
        "linestyle": "-",
        "linewidth": 2.2,
        "markersize": 8.0,
    },
}

FIG_WIDTH = 6.6
FIG_HEIGHT = 4.8
GRID_ALPHA = 0.3
GRID_LINESTYLE = "--"
FONT_SIZE_LABEL = 14
FONT_SIZE_TICK = 12
FONT_SIZE_LEGEND = 14
FONT_SIZE_TITLE = 14