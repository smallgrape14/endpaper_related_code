import cufile
import cupy as cp
import os

def read_directory_with_gds_cufile(directory_path, pattern="*.dat"):
    """
    使用cufile API读取目录下所有文件。
    """
    from pathlib import Path
    import glob
    
    path = Path(directory_path)
    
    if not path.exists() or not path.is_dir():
        print(f"错误：目录 '{directory_path}' 不存在或不是目录。")
        return
    
    # 查找匹配的文件
    file_list = list(path.glob(pattern))
    if not file_list:
        print(f"在目录 '{directory_path}' 中未找到匹配模式 '{pattern}' 的文件。")
        return
    
    print(f"找到 {len(file_list)} 个文件准备读取...")
    
    for file_path in file_list:
        print(f"正在读取: {file_path.name}")
        
        try:
            # 获取文件大小
            file_size = file_path.stat().st_size
            
            # 在GPU上分配内存
            gpu_buffer = cp.empty(file_size, dtype=cp.uint8)
            
            # 使用cufile将文件直接读取到GPU内存
            with cufile.CuFile(file_path, "r") as f:
                bytes_read = f.read(gpu_buffer)
            
            print(f"  成功读取 {bytes_read} 字节到GPU内存")
            
            # 此时数据已在GPU内存中，可用于后续处理
            
        except Exception as e:
            print(f"  读取文件 {file_path.name} 时出错: {str(e)}")

# 示例用法
read_directory_with_gds_cufile("/mnt/beegfs/test_dir_1KB_10428", "*.npy")