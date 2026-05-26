
"""
【Deepin20系统】Linux系统中永久解决matplotlib画图中文乱码问题和使用seaborn中文乱码问题
https://developer.aliyun.com/article/1577567
"""
# 查看系统可用字体

from matplotlib.font_manager import FontManager
fm = FontManager()
mat_fonts = set(f.name for f in fm.ttflist)
print("系统可用字体", mat_fonts)

# 查找matplotlib路径
import matplotlib
print("数据路径", matplotlib.get_data_path())  # 数据路径

"""
# 下载字体
root@ubuntu-PowerEdge-R750xa:/usr/local/lib/python3.10/dist-packages/matplotlib/mpl-data/fonts/ttf# wget http://d.xiazaiziti.com/en_fonts/fonts/s/SimHei.ttf
"""

# 查找缓存目录
import matplotlib    
print("查找缓存目录", matplotlib.get_cachedir())

"""
# 删除缓存目录
root@ubuntu-PowerEdge-R750xa:/doca_devel/doca_2.7.0085/GPUNetIO_Agent_wi_DPU/tmp/20250729_draw# rm /root/.cache/matplotlib -r
"""