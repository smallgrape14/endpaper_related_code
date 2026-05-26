import ctypes
import os
import torch
# import cuda

# 加载 libcufile.so
try:
    libcufile = ctypes.CDLL("libcufile.so")
except OSError:
    raise RuntimeError("libcufile.so not found. Ensure NVIDIA GDS SDK is installed.")

# 定义 cuFile API 函数原型
class CUfileError_t(ctypes.Structure):
    _fields_ = [("error", ctypes.c_int)]

# 函数声明
libcufile.cuFileDriverOpen.argtypes = []
libcufile.cuFileDriverOpen.restype = CUfileError_t

libcufile.cuFileDriverClose.argtypes = []
libcufile.cuFileDriverClose.restype = CUfileError_t

libcufile.cuFileHandleRegister.argtypes = [ctypes.POINTER(ctypes.c_void_p), ctypes.POINTER(ctypes.c_int), ctypes.c_int]
libcufile.cuFileHandleRegister.restype = CUfileError_t

libcufile.cuFileRead.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t, ctypes.c_size_t, ctypes.c_size_t]
libcufile.cuFileRead.restype = CUfileError_t

libcufile.cuFileBufRegister.argtypes = [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_uint]
libcufile.cuFileBufRegister.restype = CUfileError_t

# 初始化 GDS 驱动
def init_gds():
    err = libcufile.cuFileDriverOpen()
    if err.error != 0:
        raise RuntimeError(f"cuFileDriverOpen failed: error code {err.error}")

# 注册文件句柄
def register_file(fd):
    handle = ctypes.c_void_p()
    desc = ctypes.c_int(fd)
    err = libcufile.cuFileHandleRegister(ctypes.byref(handle), ctypes.byref(desc), 0)
    if err.error != 0:
        raise RuntimeError(f"cuFileHandleRegister failed: error code {err.error}")
    return handle

# 新增 CUfileDescr_t 结构体定义
# class CUfileDescr_t(ctypes.Structure):
#     _fields_ = [("type", ctypes.c_int), ("handle", ctypes.c_void_p)]

# def register_file(fd):
#     handle = ctypes.c_void_p()
#     desc = CUfileDescr_t()
#     desc.type = 1  # CU_FILE_HANDLE_TYPE_OPAQUE_FD
#     desc.handle = ctypes.c_void_p(fd)  # 关键：将 fd 转为 c_void_p
    
    err = libcufile.cuFileHandleRegister(ctypes.byref(handle), ctypes.byref(desc), 0)
    if err.error != 0:
        raise RuntimeError(f"cuFileHandleRegister failed: error {err.error}")
    return handle

# 注册 GPU 显存缓冲区
def register_gpu_buffer(gpu_ptr, size):
    err = libcufile.cuFileBufRegister(gpu_ptr, size, 0)
    if err.error != 0:
        raise RuntimeError(f"cuFileBufRegister failed: error code {err.error}")

# 从文件直接读取到 GPU 显存
def read_to_gpu(handle, gpu_ptr, size, offset):
    err = libcufile.cuFileRead(handle, gpu_ptr, size, offset, 0)
    if err.error != 0:
        raise RuntimeError(f"cuFileRead failed: error code {err.error}")

# 清理资源
def cleanup(handle, gpu_ptr, size):
    libcufile.cuFileBufDeregister(gpu_ptr)
    libcufile.cuFileHandleDeregister(handle)
    libcufile.cuFileDriverClose()

# 主函数示例
def main():
    # 0. 前置条件检查
    # if not os.path.exists("/dev/nvidia-fs"):
    #     raise RuntimeError("nvidia-fs kernel module not loaded. Run: `sudo modprobe nvidia-fs`")

    # 1. 初始化 GDS
    init_gds()

    # 2. 打开文件（需 O_DIRECT 模式）
    # file_path = "/mnt/beegfs/imagenet_10/train/n01943899/n01943899_6015.JPEG"  # BeeGFS 路径
    file_path = "/mnt/nvme0/n01943899_6015.JPEG"  # BeeGFS 路径

    
    fd = os.open(file_path, os.O_RDONLY | os.O_DIRECT)

    # 3. 分配 GPU 显存（对齐到 4KB）
    size = 1024 * 1024  # 1MB
    gpu_buf = torch.empty(size, dtype=torch.uint8, device="cuda")
    gpu_ptr = ctypes.c_void_p(gpu_buf.data_ptr())

    # 4. 注册资源
    handle = register_file(fd)
    register_gpu_buffer(gpu_ptr, size)

    # 5. 读取数据（同步模式）
    offset = 0  # 文件起始位置
    read_to_gpu(handle, gpu_ptr, size, offset)

    # 6. 异步读取示例（需 CUDA Stream）
    stream = torch.cuda.Stream()
    with torch.cuda.stream(stream):
        read_to_gpu(handle, gpu_ptr, size, offset)
    stream.synchronize()  # 等待异步操作完成

    # 7. 清理资源
    os.close(fd)
    cleanup(handle, gpu_ptr, size)
    print("Data transferred directly to GPU memory!")

if __name__ == "__main__":
    main()