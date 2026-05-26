#include <cstdio>
#include <cuda_runtime.h>
#include <fstream>
#include <iostream>
#include <dirent.h>
#include <vector>
#include <string>

const char *ipc_handle_file = "/home/oem/xyp/imagenet_code/GPUNetIO/ipc_handle.bin";
const char *data_dir = "/home/oem/xyp/imagenet_test_decode3/train/n02138441"; // 替换为你的数据文件路径

int main() {
    // 获取目录下的文件列表
    DIR *dir;
    struct dirent *entry;
    std::vector<std::string> files;

    dir = opendir(data_dir);
    if (dir == nullptr) {
        std::cerr << "Failed to open directory " << data_dir << std::endl;
        return 1;
    }

    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_REG) { // 只处理普通文件
            files.push_back(entry->d_name);
        }
    }
    closedir(dir);

    // 确保只处理10个文件
    if (files.size() > 10) {
        files.resize(10);
    }

    // 计算总的内存需求
    size_t data_size_per_file = 1024 * 1024; // 假设每个文件大小为1MB
    size_t total_data_size = files.size() * data_size_per_file;

    // 分配GPU内存
    void *gpu_memory;
    cudaError_t cuda_ret = cudaMalloc(&gpu_memory, total_data_size);
    if (cuda_ret != cudaSuccess) {
        std::cerr << "Failed to allocate GPU memory: " << cudaGetErrorString(cuda_ret) << std::endl;
        return 1;
    }

    // 从文件加载数据到GPU内存
    for (size_t i = 0; i < files.size(); ++i) {
        std::string file_path = std::string(data_dir) + "/" + files[i];
        FILE *f_data = fopen(file_path.c_str(), "rb");
        if (f_data == nullptr) {
            std::cerr << "Failed to open file " << file_path << std::endl;
            cudaFree(gpu_memory);
            return 1;
        }

        // 将文件数据复制到GPU内存
        void *gpu_ptr = static_cast<char *>(gpu_memory) + i * data_size_per_file;
        cuda_ret = cudaMemcpy(gpu_ptr, f_data, data_size_per_file, cudaMemcpyHostToDevice);
        if (cuda_ret != cudaSuccess) {
            std::cerr << "Failed to copy data to GPU: " << cudaGetErrorString(cuda_ret) << std::endl;
            fclose(f_data);
            cudaFree(gpu_memory);
            return 1;
        }

        fclose(f_data);
    }

    // 获取IPC句柄
    cudaIpcMemHandle_t handle;
    cuda_ret = cudaIpcGetMemHandle(&handle, gpu_memory);
    if (cuda_ret != cudaSuccess) {
        std::cerr << "Failed to get IPC handle: " << cudaGetErrorString(cuda_ret) << std::endl;
        cudaFree(gpu_memory);
        return 1;
    }

    // 将句柄写入文件
    FILE *f_ipc = fopen(ipc_handle_file, "wb");
    if (f_ipc == nullptr) {
        std::cerr << "Failed to open IPC handle file " << ipc_handle_file << std::endl;
        cudaFree(gpu_memory);
        return 1;
    }
    fwrite(&handle, sizeof(cudaIpcMemHandle_t), 1, f_ipc);
    fclose(f_ipc);

    std::cout << "CUDA IPC handle written to file: " << ipc_handle_file << std::endl;

    // 等待输入以保持程序运行
    std::cout << "Press any key to exit...";
    getchar();

    // 清理
    cudaFree(gpu_memory);
    return 0;
}