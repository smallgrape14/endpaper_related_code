import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# 设置中文字体
plt.rcParams['font.sans-serif'] = ['Noto Sans CJK JP']
plt.rcParams['axes.unicode_minus'] = False

# 创建数据框（更新后的数据）
data = {
    'Threads': [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024],
    'IOPS(BeeGFS)Cufile-1KB': [1385.23, 2310.66, 4035.6, 5266.67, 5482.65, 5152.17, 5071.98, 5001.44, 4942.18, 4693.07, 4597.88],
    'IOPS(localNVMeSSD)-1KB': [4553.71, 15047.62, 17555.56, 15113.04, 9277.58, 8596.87, 7984.69, 7606.13, 8071.21, 7443.25, 7098.71],
    'IOPS(BeeGFS)Cufile-4KB': [1299.12, 2334.45, 4365.01, 6225.67, 5227.07, 5059.68, 4968.08, 4932.83, 4638.79, 4796.69, 4473.62],
    'IOPS(localNVMeSSD)-4KB': [4914.23, 8033.9, 14284.93, 12326.24, 8058.73, 6350.79, 6723.4, 6933.51, 6591.66, 8108.86, 7395.74],
    'IOPS(BeeGFS)Cufile-16KB': [1194.09, 2068.23, 4174.54, 5491.31, 5366.96, 5114.27, 4987.09, 4893.48, 4868.35, 4832.25, 4387.04],
    'IOPS(localNVMeSSD)-16KB': [1692.03, 3492.3, 6676.06, 10828.66, 6112.54, 6055.75, 6066.32, 6000, 5780.49, 5799.78, 6112.54]
}

df = pd.DataFrame(data)

# 绘制所有数据在一张图上
plt.figure(figsize=(14, 10))

# 绘制BeeGFS Cufile系列数据
plt.plot(df['Threads'], df['IOPS(BeeGFS)Cufile-1KB'], marker='o', label='BeeGFS Cufile-1KB', linewidth=2, color=plt.cm.tab20c(0))
plt.plot(df['Threads'], df['IOPS(BeeGFS)Cufile-4KB'], marker='s', label='BeeGFS Cufile-4KB', linewidth=2, color=plt.cm.tab20c(1))
plt.plot(df['Threads'], df['IOPS(BeeGFS)Cufile-16KB'], marker='^', label='BeeGFS Cufile-16KB', linewidth=2, color=plt.cm.tab20c(2))

# 绘制localNVMeSSD系列数据
plt.plot(df['Threads'], df['IOPS(localNVMeSSD)-1KB'], marker='o', label='localNVMeSSD-1KB', linewidth=2, linestyle='--', color=plt.cm.tab20c(4))
plt.plot(df['Threads'], df['IOPS(localNVMeSSD)-4KB'], marker='s', label='localNVMeSSD-4KB', linewidth=2, linestyle='--', color=plt.cm.tab20c(5))
plt.plot(df['Threads'], df['IOPS(localNVMeSSD)-16KB'], marker='^', label='localNVMeSSD-16KB', linewidth=2, linestyle='--', color=plt.cm.tab20c(6))

# 设置标题和轴标签
plt.title('IOPS Performance Comparison Across Different IO Sizes', fontsize=20)
plt.xlabel('Threads', fontsize=16)
plt.ylabel('IOPS', fontsize=16)

# 设置坐标轴刻度
plt.xscale('log', base=2)
plt.yscale('log')

# 增加更多的x轴刻度
x_ticks = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024]
plt.xticks(x_ticks, [str(x) for x in x_ticks], fontsize=16)

# 增加更多的y轴刻度
y_ticks = [1000, 2000, 5000, 10000, 20000]
plt.yticks(y_ticks, [str(y) for y in y_ticks], fontsize=16)

# 设置网格
plt.grid(True, which="both", ls="-", alpha=0.2)

# 图例
plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left', fontsize=12)

# 调整布局以防止标签被截断
plt.tight_layout()

# 保存图片
img_path = 'iops_comparison_all_updated_enhanced.png'
plt.savefig(img_path, dpi=300, bbox_inches='tight')
print(f"增强版综合IOPS对比图已保存为 {img_path}")