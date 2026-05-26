import os
import shutil
from PIL import Image
import numpy as np

# 数据集路径
# dataset_path = "/home/oem/xyp/imagenet_test/train/n07711569"
dataset_path = "/home/oem/xyp/imagenet_test/val/n07711569"

# 新目录路径
# new_dataset_path = "/home/oem/xyp/imagenet_test/train/n07711569_decode"
new_dataset_path = "/home/oem/xyp/imagenet_test/val/n07711569_decode"


# 确保新目录存在
os.makedirs(new_dataset_path, exist_ok=True)

filecnt=0
# 遍历数据集文件夹
for foldername, subfolders, filenames in os.walk(dataset_path):
    for filename in filenames:
        # 检查是否为JPEG文件
        if filename.lower().endswith(('.jpg', '.jpeg')):
            # 完整的文件路径
            file_path = os.path.join(foldername, filename)

            # 解码JPEG文件
            try:
                with Image.open(file_path) as img:
                    # 转换为NumPy数组
                    img_array = np.array(img)

                    # 创建新子目录
                    relative_path = os.path.relpath(foldername, dataset_path)
                    new_folder_path = os.path.join(new_dataset_path, relative_path)
                    os.makedirs(new_folder_path, exist_ok=True)

                    # 保存为新格式或直接复制文件
                    new_file_path = os.path.join(new_folder_path, filename)

                    # 如果你只是想复制文件，可以取消下面这行的注释
                    # shutil.copy2(file_path, new_file_path)

                    # 如果你需要对解码后的数据进行操作，可以在这里处理
                    # 例如保存为NumPy文件
                    np.save(new_file_path.replace('.jpg', '.npy'), img_array)
                    filecnt=filecnt+1

            except Exception as e:
                print(f"Error processing {file_path}: {e}")

print(f"{filecnt}个JPEG文件导入完成。")