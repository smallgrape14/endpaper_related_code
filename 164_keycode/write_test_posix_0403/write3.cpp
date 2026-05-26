#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>
#include <cuda_runtime.h>
#include <algorithm>
#include <numeric>
#include <sys/stat.h>
#include <dirent.h>

// 全局变量
std::atomic<int> file_counter(0);
std::mutex cout_mutex;
std::vector<cudaStream_t> cuda_streams;
std::vector<long long> thread_copy_times;
std::vector<long long> thread_write_times;
std::vector<int> thread_file_counts;

// 检查CUDA错误
#define CHECK_CUDA_ERROR(err) \
    if (err != cudaSuccess) { \
        std::lock_guard<std::mutex> lock(cout_mutex); \
        std::cerr << "CUDA error: " << cudaGetErrorString(err) << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        exit(EXIT_FAILURE); \
    }

// 检查目录是否存在
bool directoryExists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
}

// 检查文件是否存在
bool fileExists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

// 创建目录
bool createDirectory(const std::string& path) {
    #ifdef _WIN32
        return _mkdir(path.c_str()) == 0;
    #else
        return mkdir(path.c_str(), 0755) == 0;
    #endif
}

// 工作线程函数
void worker_thread(int thread_id, int total_files, size_t iosize, const std::string& output_dir, void* d_buffer, 
                   long long& total_copy_time, long long& total_write_time, int& files_processed) {
    cudaSetDevice(0);  // 使用GPU 0
    
    char* h_buffer = new char[iosize];  // 主机缓冲区
    cudaStream_t stream = cuda_streams[thread_id];
    total_copy_time = 0;
    total_write_time = 0;
    files_processed = 0;
    
    // 初始化缓冲区
    std::fill_n(h_buffer, iosize, static_cast<char>(thread_id % 256));
    
    while (true) {
        // 获取下一个文件索引
        int file_idx = file_counter.fetch_add(1);
        if (file_idx >= total_files) break;
        
        // 从GPU显存复制数据到主机内存
        auto start_copy = std::chrono::high_resolution_clock::now();
        
        cudaError_t err = cudaMemcpyAsync(h_buffer, 
                                         (char*)d_buffer + (file_idx * iosize) % (1024 * 1024 * 1024),  // 循环使用显存
                                         iosize, 
                                         cudaMemcpyDeviceToHost, 
                                         stream);
        CHECK_CUDA_ERROR(err);
        cudaStreamSynchronize(stream);
        
        auto end_copy = std::chrono::high_resolution_clock::now();
        auto copy_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_copy - start_copy);
        total_copy_time += copy_duration.count();
        
        // 打开已存在的文件并写入数据
        std::stringstream filename;
        filename << output_dir << "/"  << file_idx << ".txt";
        
        auto start_write = std::chrono::high_resolution_clock::now();
        
        std::ofstream outfile(filename.str(), std::ios::binary | std::ios::trunc);
        if (!outfile) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cerr << "Thread " << thread_id << ": Failed to open file " << filename.str() << std::endl;
            continue;
        }
        
        outfile.write(h_buffer, iosize);
        outfile.close();
        
        auto end_write = std::chrono::high_resolution_clock::now();
        auto write_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_write - start_write);
        total_write_time += write_duration.count();
        
        files_processed++;
        
        // 进度输出（每完成1%输出一次）
        if (file_idx % (total_files / 100) == 0) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            float progress = (float)file_idx / total_files * 100.0f;
            std::cout << "Progress: " << std::fixed << std::setprecision(1) << progress 
                     << "%, Thread " << thread_id << " processed file " << file_idx 
                     << " (Copy: " << copy_duration.count() << "µs, Write: " 
                     << write_duration.count() << "µs)" << std::endl;
        }
    }
    
    delete[] h_buffer;
}

int main(int argc, char* argv[]) {
    // 检查参数
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <output_directory> <threadnum> <IOSIZE_KB>" << std::endl;
        std::cout << "Example: " << argv[0] << " ./output 16 1024" << std::endl;
        std::cout << "IOSIZE_KB: Size in KB (e.g., 1024 for 1MB)" << std::endl;
        std::cout << "\nNote: 10240 files should already exist in the output directory." << std::endl;
        return 1;
    }
    
    std::string output_dir = argv[1];
    int threadnum = std::stoi(argv[2]);
    size_t iosize_kb = std::stoull(argv[3]);
    size_t iosize = iosize_kb * 1024;  // 转换为字节
    
    constexpr int TOTAL_FILES = 10240;
    
    if (threadnum <= 0 || threadnum > 256) {
        std::cout << "Error: threadnum must be between 1 and 256" << std::endl;
        return 1;
    }
    
    if (iosize_kb <= 0 || iosize_kb > 1024 * 1024) {  // 最大1GB
        std::cout << "Error: IOSIZE_KB must be between 1 and 1048576 (1GB)" << std::endl;
        return 1;
    }
    
    // 检查输出目录是否存在
    if (!directoryExists(output_dir)) {
        std::cout << "Error: Output directory does not exist: " << output_dir << std::endl;
        std::cout << "Please create the directory and ensure it contains 10240 files." << std::endl;
        return 1;
    }
    
    // 检查文件数量（只检查前几个和最后几个）
    bool files_exist = true;
    int check_files[] = {0, 1000, 5000, 9999};
    for (int idx : check_files) {
        std::stringstream filename;
        filename << output_dir << "/" << idx << ".txt";
        if (!fileExists(filename.str())) {
            std::cout << "Warning: File does not exist: " << filename.str() << std::endl;
            files_exist = false;
        }
    }
    
    if (!files_exist) {
        std::cout << "Warning: Not all expected files exist. Continuing anyway..." << std::endl;
    }
    
    std::cout << "Starting multi-threaded GPU to file write..." << std::endl;
    std::cout << "Total files: " << TOTAL_FILES << std::endl;
    std::cout << "File size: " << iosize / 1024 << " KB (" << iosize << " bytes)" << std::endl;
    std::cout << "Thread count: " << threadnum << std::endl;
    std::cout << "Output directory: " << output_dir << std::endl;
    std::cout << "Note: Assuming 10240 files already exist in the directory." << std::endl;
    
    // 初始化CUDA
    cudaSetDevice(0);
    
    // 在GPU上分配1GB显存并填充测试数据
    void* d_buffer = nullptr;
    size_t buffer_size = 1024 * 1024 * 1024;  // 1GB
    CHECK_CUDA_ERROR(cudaMalloc(&d_buffer, buffer_size));
    
    // 创建测试数据（简单的模式）
    std::cout << "Generating test data on GPU..." << std::endl;
    char* h_test_data = new char[iosize];
    for (size_t i = 0; i < iosize; i++) {
        h_test_data[i] = i % 256;
    }
    
    // 将测试数据复制到GPU（重复填充1GB）
    for (size_t offset = 0; offset < buffer_size; offset += iosize) {
        CHECK_CUDA_ERROR(cudaMemcpy((char*)d_buffer + offset, h_test_data, 
                                   std::min(iosize, buffer_size - offset), 
                                   cudaMemcpyHostToDevice));
    }
    delete[] h_test_data;
    std::cout << "Test data generation complete." << std::endl;
    
    // 创建CUDA流
    cuda_streams.resize(threadnum);
    for (int i = 0; i < threadnum; i++) {
        CHECK_CUDA_ERROR(cudaStreamCreate(&cuda_streams[i]));
    }
    
    // 准备线程统计信息
    std::vector<long long> thread_copy_times_local(threadnum, 0);
    std::vector<long long> thread_write_times_local(threadnum, 0);
    std::vector<int> thread_file_counts_local(threadnum, 0);
    
    // 记录开始时间
    file_counter = 0;  // 重置文件计数器
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 创建并启动工作线程
    std::vector<std::thread> threads;
    for (int i = 0; i < threadnum; i++) {
        threads.emplace_back([i, TOTAL_FILES, iosize, &output_dir, d_buffer, 
                             &thread_copy_times_local, &thread_write_times_local, &thread_file_counts_local]() {
            worker_thread(i, TOTAL_FILES, iosize, output_dir, d_buffer, 
                         thread_copy_times_local[i], thread_write_times_local[i], thread_file_counts_local[i]);
        });
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    // 记录结束时间
    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // 清理CUDA资源
    for (int i = 0; i < threadnum; i++) {
        cudaStreamDestroy(cuda_streams[i]);
    }
    cudaFree(d_buffer);
    
    // 计算统计信息
    long long total_copy_time = std::accumulate(thread_copy_times_local.begin(), thread_copy_times_local.end(), 0LL);
    long long total_write_time = std::accumulate(thread_write_times_local.begin(), thread_write_times_local.end(), 0LL);
    double total_time_sec = total_duration.count() / 1000.0;
    
    // 计算IOPS
    double iops = TOTAL_FILES / total_time_sec;
    
    // 输出统计信息
    std::cout << "\n======= Performance Summary =======" << std::endl;
    std::cout << "Total files written: " << TOTAL_FILES << std::endl;
    std::cout << "Total data written: " 
              << (double)TOTAL_FILES * iosize / (1024.0 * 1024.0 * 1024.0) 
              << " GB" << std::endl;
    std::cout << "Total time: " << total_time_sec << " seconds" << std::endl;
    std::cout << "Total throughput: " 
              << (double)TOTAL_FILES * iosize / (1024.0 * 1024.0) / total_time_sec 
              << " MB/s" << std::endl;
    std::cout << "IOPS: " << iops << " operations/second" << std::endl;
    std::cout << "Average file write time: " 
              << total_time_sec * 1000.0 / TOTAL_FILES << " ms/file" << std::endl;
    std::cout << "\n======= Detailed Statistics =======" << std::endl;
    std::cout << "Total copy time (all threads): " << total_copy_time / 1000.0 << " ms" << std::endl;
    std::cout << "Total write time (all threads): " << total_write_time / 1000.0 << " ms" << std::endl;
    std::cout << "Average copy time per file: " << (double)total_copy_time / TOTAL_FILES << " µs" << std::endl;
    std::cout << "Average write time per file: " << (double)total_write_time / TOTAL_FILES << " µs" << std::endl;
    
    // 线程负载均衡信息
    std::cout << "\n======= Thread Load Balancing =======" << std::endl;
    for (int i = 0; i < threadnum; i++) {
        std::cout << "Thread " << i << ": " << thread_file_counts_local[i] 
                  << " files, Copy: " << thread_copy_times_local[i] / 1000.0 
                  << " ms, Write: " << thread_write_times_local[i] / 1000.0 << " ms" << std::endl;
    }
    
    return 0;
}