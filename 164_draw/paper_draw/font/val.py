# import matplotlib
# # print(matplotlib.get_cachedir())  
# import matplotlib.font_manager
# print([f.name for f in matplotlib.font_manager.fontManager.ttflist if 'SimSun' in f.name])

import matplotlib.font_manager as fm

fonts = [f.name for f in fm.fontManager.ttflist]
print("SimSun" in fonts)