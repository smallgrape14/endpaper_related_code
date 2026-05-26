# import os

# # 创建目录
# directory = "/home/ubuntu/xyp/Send_pkt/self_defined_generate_file/dir_500"
# os.makedirs(directory, exist_ok=True)

# # 生成500个文件
# for x in range(1, 501):
#     # 文件名格式为 file_x.txt
#     filename = os.path.join(directory, f"file_{x}.txt")
    
#     # 文件内容为序号 x，重复填充到512字节
#     content = str(x) * (512 // len(str(x)))  # 计算需要重复的次数
#     if len(content) < 512:
#         content += str(x) * (512 - len(content))  # 补充剩余字节
    
#     # 写入文件
#     with open(filename, "w") as f:
#         f.write(content)

# print(f"成功生成 {directory} 目录，包含500个文件，每个文件大小为512字节。")

# import os

# # 创建目录
# directory = "/home/ubuntu/xyp/Send_pkt/self_defined_generate_file/dir_500"
# os.makedirs(directory, exist_ok=True)

# # 生成500个文件
# for x in range(1, 501):
#     # 文件名格式为 file_x.txt
#     filename = os.path.join(directory, f"file_{x}.txt")
    
#     # 文件内容为 "@x"，其中 x 是文件的序号
#     content = f"@{x}"
    
#     # 计算需要填充的字符数，使文件大小为512字节
#     required_length = 512
#     current_length = len(content)
#     if current_length < required_length:
#         # 用空格或其他字符填充到512字节
#         content += ' ' * (required_length - current_length)
    
#     # 写入文件
#     with open(filename, "w") as f:
#         f.write(content)

# print(f"成功生成 {directory} 目录，包含500个文件，每个文件大小为512字节。")






# import os

# # 创建目录
# directory = "/home/ubuntu/xyp/Send_pkt/self_defined_generate_file/dir_500"
# os.makedirs(directory, exist_ok=True)

# # 生成500个文件
# for x in range(1, 501):
#     # 文件名格式为 file_x.txt
#     filename = os.path.join(directory, f"file_{x}.txt")
    
#     # 文件内容为序号 x，每个 x 之间用 @ 分隔
#     content = f"@{x}" * (512 // len(f"@{x}"))  # 计算需要重复的次数
    
#     # 如果内容长度不足512字节，补充剩余部分
#     if len(content) < 512:
#         content += '@' * (512 - len(content))
    
#     # 写入文件
#     with open(filename, "w") as f:
#         f.write(content)

# print(f"成功生成 {directory} 目录，包含500个文件，每个文件大小为512字节。")




import os

# 创建目录
# directory = "/home/oem/xyp/Send_pkt/self_defined_generate_file/test_dir_4KB_50"
# directory = "/home/oem/xyp/Send_pkt/self_defined_generate_file/test_dir_4KB_10428"
# directory = "/home/oem/xyp/Send_pkt/self_defined_generate_file/test_dir_16KB_10428"
# directory = "/home/oem/xyp/Send_pkt/self_defined_generate_file/test_dir_1KB_10428"
# directory = "/home/oem/xyp/Send_pkt/self_defined_generate_file/test_dir_512B_10428"
# directory = "/home/oem/xyp/Send_pkt/self_defined_generate_file/test_dir_8KB_10428"

directory ="/home/oem/xyp/Send_pkt/test_dir_2KB_10428"


# directory = "/home/oem/xyp/Send_pkt/self_defined_generate_file/test_dir_32KB_256"

# directory = "/mnt/beegfs/test_dir_1MB_10428"

os.makedirs(directory, exist_ok=True)

# 生成500个文件
file_num = 10428
for x in range(1, file_num+1):
    # file size
    file_size = 2*1024 #1*1024*1024  # 每个文件大小为512字节/32KB
    # 文件名格式为 file_x.txt
    filename = os.path.join(directory, f"file_{x}.txt")
    
    # 文件内容为序号 x，每个 x 之间用 @ 分隔
    content = f"@{x}" * (file_size // len(f"@{x}"))  # 计算需要重复的次数
    
    # 如果内容长度不足512字节，补充剩余部分
    if len(content) < file_size:
        content += '@' * (file_size - len(content))
    
    # 写入文件
    with open(filename, "w") as f:
        f.write(content)

print(f"成功生成 {directory} 目录，包含{file_num}个文件，每个文件大小为{file_size}字节。")