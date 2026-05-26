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

// 配置参数
constexpr size_t TOTAL_FILES = 10240;      // 总文件数
constexpr size_t IOSIZE = 1024 * 1024;     // 每个文件大小 1MB
constexpr int THREAD_COUNT = 16;           // 线程数

// 全局变量
std::atomic<int> file_counter(0);
std::mutex cout_mutex;
cudaStream_t cuda_streams[THREAD_COUNT];

// 检查CUDA错误
#define CHECK_CUDA_ERROR(err) \
    if (err != cudaSuccess) { \
        std::cerr << "CUDA error: " << cudaGetErrorString(err) << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        exit(EXIT_FAILURE); \
    }

// 线程工作函数
void worker_thread(int thread_id, const std::string& output_dir, void* d_buffer) {
    cudaSetDevice(0);  // 使用GPU 0
    
    char* h_buffer = new char[IOSIZE];  // 主机缓冲区
    cudaStream_t stream = cuda_streams[thread_id];
    
    while (true) {
        // 获取下一个文件索引
        int file_idx = file_counter.fetch_add(1);
        if (file_idx >= TOTAL_FILES) break;
        
        // 从GPU显存复制数据到主机内存
        auto start_copy = std::chrono::high_resolution_clock::now();
        
        cudaError_t err = cudaMemcpyAsync(h_buffer, 
                                         (char*)d_buffer + (file_idx * IOSIZE) % (1024 * 1024 * 1024),  // 循环使用显存
                                         IOSIZE, 
                                         cudaMemcpyDeviceToHost, 
                                         stream);
        CHECK_CUDA_ERROR(err);
        cudaStreamSynchronize(stream);
        
        auto end_copy = std::chrono::high_resolution_clock::now();
        auto copy_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_copy - start_copy);
        
        // 创建文件名
        std::stringstream filename;
        filename << output_dir << "/file_" << std::setfill('0') << std::setw(6) << file_idx << ".txt";
        
        // 写入文件
        auto start_write = std::chrono::high_resolution_clock::now();
        
        std::ofstream outfile(filename.str(), std::ios::binary);
        if (!outfile) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cerr << "Thread " << thread_id << ": Failed to open file " << filename.str() << std::endl;
            continue;
        }
        
        outfile.write(h_buffer, IOSIZE);
        outfile.close();
        
        auto end_write = std::chrono::high_resolution_clock::now();
        auto write_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_write - start_write);
        
        // 进度输出（每完成1%输出一次）
        if (file_idx % (TOTAL_FILES / 100) == 0) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            float progress = (float)file_idx / TOTAL_FILES * 100.0f;
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
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <output_directory>" << std::endl;
        std::cout << "Example: " << argv[0] << " ./output" << std::endl;
        return 1;
    }
    
    std::string output_dir = argv[1];
    
    std::cout << "Starting multi-threaded GPU to file write..." << std::endl;
    std::cout << "Total files: " << TOTAL_FILES << std::endl;
    std::cout << "File size: " << IOSIZE / 1024 << " KB" << std::endl;
    std::cout << "Thread count: " << THREAD_COUNT << std::endl;
    std::cout << "Output directory: " << output_dir << std::endl;
    
    // 初始化CUDA
    cudaSetDevice(0);
    
    // 在GPU上分配1GB显存并填充测试数据
    void* d_buffer = nullptr;
    size_t buffer_size = 1024 * 1024 * 1024;  // 1GB
    CHECK_CUDA_ERROR(cudaMalloc(&d_buffer, buffer_size));
    
    // 创建测试数据（简单的模式）
    std::cout << "Generating test data on GPU..." << std::endl;
    char* h_test_data = new char[IOSIZE];
    for (size_t i = 0; i < IOSIZE; i++) {
        h_test_data[i] = i % 256;
    }
    
    // 将测试数据复制到GPU（重复填充1GB）
    for (size_t offset = 0; offset < buffer_size; offset += IOSIZE) {
        CHECK_CUDA_ERROR(cudaMemcpy((char*)d_buffer + offset, h_test_data, 
                                   std::min(IOSIZE, buffer_size - offset), 
                                   cudaMemcpyHostToDevice));
    }
    delete[] h_test_data;
    std::cout << "Test data generation complete." << std::endl;
    
    // 创建CUDA流
    for (int i = 0; i < THREAD_COUNT; i++) {
        CHECK_CUDA_ERROR(cudaStreamCreate(&cuda_streams[i]));
    }
    
    // 记录开始时间
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 创建并启动线程
    std::vector<std::thread> threads;
    for (int i = 0; i < THREAD_COUNT; i++) {
        threads.emplace_back(worker_thread, i, output_dir, d_buffer);
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    // 记录结束时间
    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // 清理CUDA资源
    for (int i = 0; i < THREAD_COUNT; i++) {
        cudaStreamDestroy(cuda_streams[i]);
    }
    cudaFree(d_buffer);
    
    // 输出统计信息
    std::cout << "\n======= Performance Summary =======" << std::endl;
    std::cout << "Total files written: " << TOTAL_FILES << std::endl;
    std::cout << "Total data written: " 
              << (double)TOTAL_FILES * IOSIZE / (1024.0 * 1024.0 * 1024.0) 
              << " GB" << std::endl;
    std::cout << "Total time: " << total_duration.count() / 1000.0 << " seconds" << std::endl;
    std::cout << "Average throughput: " 
              << (double)TOTAL_FILES * IOSIZE / (1024.0 * 1024.0) / (total_duration.count() / 1000.0) 
              << " MB/s" << std::endl;
    std::cout << "Average file write time: " 
              << (double)total_duration.count() / TOTAL_FILES << " ms/file" << std::endl;
    
    return 0;
}