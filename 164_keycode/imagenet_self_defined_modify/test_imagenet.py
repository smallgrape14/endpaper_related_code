import os
import shutil

# 源文件夹路径
# source_folder = r"/mnt/beegfs/imagenet_test/train/n07711569"
source_folder = r"/home/oem/xyp/imagenet_test/train/n07711569"


# 目标文件夹路径
# target_folder = r"/mnt/beegfs/imagenet_test/train/n07711569_new_1MB"
target_folder = r"/home/oem/xyp/imagenet_test/val/n07711569_new_1MB"

# target_folder = r"/home/ubuntu/xyp/Send_pkt/image_32KB_500"


# 创建目标文件夹（如果不存在）
os.makedirs(target_folder, exist_ok=True)

# 用于计数已经复制的文件数量
copy_count = 0

# 遍历源文件夹中的所有文件
for filename in os.listdir(source_folder):
    # if copy_count >= 500:
    #     break  # 如果已经复制了500个文件，则退出循环
    
    source_path = os.path.join(source_folder, filename)
    
    # 检查是否是文件
    if os.path.isfile(source_path):
        # 获取文件大小，单位是字节
        file_size = os.path.getsize(source_path)
        
        # 检查文件大小是否小于等于32KB（32 * 1024字节）改成 1MB 就行
        if file_size <= 1*1024 * 1024:
            # 复制文件到目标文件夹
            shutil.copy2(source_path, target_folder)
            print(f"已复制: {filename}")
            copy_count += 1  # 增加已复制文件计数
        else:
            print(f"文件过大: {filename}，大小: {file_size / 1024:.2f} KB")
        

print("操作完成！")