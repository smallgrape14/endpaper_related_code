#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cuda_runtime.h>  // 添加CUDA运行时头文件
#include <cufile.h>

// 配置参数
constexpr size_t IOSIZE = 4096;           // 每次写入的大小（字节）
constexpr int NUM_FILES = 10240;         // 总文件数
constexpr int NUM_THREADS = 32;          // 线程数（可根据需要调整）
constexpr int FILES_PER_THREAD = NUM_FILES / NUM_THREADS;  // 每个线程处理的文件数

// 全局统计变量
std::atomic<int> files_completed{0};
std::mutex cout_mutex;

// 检查cuFile API调用结果
#define CHECK_CUFILE(err) \
    if (err != 0) { \
        std::cerr << "cuFile error at " << __FILE__ << ":" << __LINE__ << ": " << err << std::endl; \
        exit(EXIT_FAILURE); \
    }

// 检查CUDA API调用结果
#define CHECK_CUDA(call) \
    { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            std::cerr << "CUDA error at " << __FILE__ << ":" << __LINE__ << ": " << cudaGetErrorString(err) << std::endl; \
            exit(EXIT_FAILURE); \
        } \
    }

// 线程工作函数
void write_files_thread(int thread_id, const std::string& dir_path, void* gpu_buffer) {
    CUfileError_t status;
    CUfileDescr_t desc;
    CUfileHandle_t handle;
    
    // 初始化文件描述符
    memset(&desc, 0, sizeof(CUfileDescr_t));
    desc.type = CU_FILE_HANDLE_TYPE_OPAQUE_FD;
    
    char filename[256];
    int local_completed = 0;
    
    for (int i = 0; i < FILES_PER_THREAD; i++) {
        int file_idx = thread_id * FILES_PER_THREAD + i;
        
        // 生成文件名
        snprintf(filename, sizeof(filename), "%s/%d.txt", dir_path.c_str(), file_idx);
        
        // 打开文件
        int fd = open(filename, O_CREAT | O_WRONLY | O_DIRECT, 0644);
        if (fd < 0) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            continue;
        }
        
        // 将文件描述符与cuFile关联
        desc.handle.fd = fd;
        status = cuFileHandleRegister(&handle, &desc);
        if (status.err != CU_FILE_SUCCESS) {
            close(fd);
            std::cerr << "cuFileHandleRegister failed for file: " << filename << std::endl;
            continue;
        }
        
        // 使用GDS写入数据
        ssize_t written = cuFileWrite(handle, gpu_buffer, IOSIZE, 0, 0);
        
        if (written != IOSIZE) {
            std::cerr << "Write incomplete for file " << filename 
                     << ": " << written << " of " << IOSIZE << " bytes" << std::endl;
        }
        
        // 清理资源
        cuFileHandleDeregister(handle);
        close(fd);
        
        local_completed++;
        files_completed++;
        
        // 每完成100个文件输出一次进度
        if (local_completed % 100 == 0) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Thread " << thread_id << " completed " << local_completed 
                     << " files. Total: " << files_completed << "/" << NUM_FILES << std::endl;
        }
    }
    
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Thread " << thread_id << " finished. Completed " 
                 << local_completed << " files." << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <output_directory>" << std::endl;
        return 1;
    }
    
    std::string output_dir = argv[1];
    
    // 检查输出目录是否存在
    struct stat st;
    if (stat(output_dir.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
        std::cerr << "Output directory does not exist: " << output_dir << std::endl;
        return 1;
    }
    
    // 初始化cuFile
    CUfileError_t status = cuFileDriverOpen();
    CHECK_CUFILE(status.err);
    
    std::cout << "Initializing GDS with parameters:" << std::endl;
    std::cout << "  Total files: " << NUM_FILES << std::endl;
    std::cout << "  IO size per file: " << IOSIZE << " bytes" << std::endl;
    std::cout << "  Threads: " << NUM_THREADS << std::endl;
    std::cout << "  Files per thread: " << FILES_PER_THREAD << std::endl;
    std::cout << "  Output directory: " << output_dir << std::endl;
    
    // 分配GPU内存
    void* gpu_buffer = nullptr;
    CHECK_CUDA(cudaMalloc(&gpu_buffer, IOSIZE));
    
    // 初始化GPU缓冲区（写入一些测试数据）
    char* test_data = new char[IOSIZE];
    for (size_t i = 0; i < IOSIZE; i++) {
        test_data[i] = static_cast<char>(i % 256);
    }
    
    CHECK_CUDA(cudaMemcpy(gpu_buffer, test_data, IOSIZE, cudaMemcpyHostToDevice));
    delete[] test_data;
    
    // 创建线程
    std::vector<std::thread> threads;
    files_completed = 0;
    
    std::cout << "\nStarting file writes..." << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_THREADS; i++) {
        threads.emplace_back(write_files_thread, i, output_dir, gpu_buffer);
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    
    // 计算并显示性能统计
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    double total_seconds = duration.count() / 1000.0;
    size_t total_bytes = static_cast<size_t>(NUM_FILES) * IOSIZE;
    double throughput = total_bytes / (1024.0 * 1024.0) / total_seconds;  // MB/s
    
    std::cout << "\n========== Performance Results ==========" << std::endl;
    std::cout << "Total files written: " << files_completed << std::endl;
    std::cout << "Total time: " << total_seconds << " seconds" << std::endl;
    std::cout << "Total data: " << total_bytes / (1024.0 * 1024.0) << " MB" << std::endl;
    std::cout << "Throughput: " << throughput << " MB/s" << std::endl;
    std::cout << "Files per second: " << NUM_FILES / total_seconds << std::endl;
    std::cout << "IOPS: " << NUM_FILES / total_seconds << std::endl;
    std::cout << "==========================================" << std::endl;
    
    // 清理资源
    CHECK_CUDA(cudaFree(gpu_buffer));
    cuFileDriverClose();
    
    return 0;
}