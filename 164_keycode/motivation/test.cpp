#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <cuda_runtime.h>

class SSDToGPUBenchmark {
private:
    std::string test_filename;
    size_t file_size;
    size_t buffer_size;
    
public:
    SSDToGPUBenchmark(const std::string& filename, size_t size) 
        : test_filename(filename), file_size(size), buffer_size(size) {}
    
    void run_comparison() {
        // 分配GPU显存
        void* gpu_buffer;
        cudaMalloc(&gpu_buffer, buffer_size);
        
        std::cout << "=== SSD到GPU数据传输性能对比测试 ===" << std::endl;
        std::cout << "文件大小: " << file_size / (1024 * 1024) << " MB" << std::endl;
        
        // 测试传统方案
        std::cout << "\n1. 传统CPU中转方案:" << std::endl;
        auto traditional_times = test_traditional_scheme(gpu_buffer);
        
        // 测试GPU直通方案
        std::cout << "\n2. GPU直通SSD方案:" << std::endl;
        auto gpu_direct_times = test_gpu_direct_scheme(gpu_buffer);
        
        // 性能对比分析
        print_comparison(traditional_times, gpu_direct_times);
        
        cudaFree(gpu_buffer);
    }
    
private:
    struct TimeResults {
        long long total_time;    // 总耗时(微秒)
        long long ssd_read_time; // SSD读取耗时
        long long copy_time;     // 内存拷贝耗时
    };
    
    TimeResults test_traditional_scheme(void* gpu_buffer) {
        TimeResults results;
        
        // 运行多次取平均值
        int num_runs = 10;
        long long total_time_sum = 0;
        long long ssd_time_sum = 0;
        long long copy_time_sum = 0;
        
        for (int i = 0; i < num_runs; ++i) {
            void* cpu_buffer = malloc(buffer_size);
            
            auto start = std::chrono::high_resolution_clock::now();
            
            // SSD到CPU内存
            std::ifstream file(test_filename, std::ios::binary);
            file.read(static_cast<char*>(cpu_buffer), buffer_size);
            file.close();
            
            auto mid = std::chrono::high_resolution_clock::now();
            
            // CPU内存到GPU显存
            cudaMemcpy(gpu_buffer, cpu_buffer, buffer_size, cudaMemcpyHostToDevice);
            
            auto end = std::chrono::high_resolution_clock::now();
            
            auto ssd_time = std::chrono::duration_cast<std::chrono::microseconds>(mid - start);
            auto copy_time = std::chrono::duration_cast<std::chrono::microseconds>(end - mid);
            auto total_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            total_time_sum += total_time.count();
            ssd_time_sum += ssd_time.count();
            copy_time_sum += copy_time.count();
            
            free(cpu_buffer);
            
            // 预热后开始记录（跳过第一次）
            if (i == 0) {
                total_time_sum = ssd_time_sum = copy_time_sum = 0;
            }
        }
        
        results.total_time = total_time_sum / (num_runs - 1);
        results.ssd_read_time = ssd_time_sum / (num_runs - 1);
        results.copy_time = copy_time_sum / (num_runs - 1);
        
        std::cout << "SSD读取时间: " << results.ssd_read_time << " μs" << std::endl;
        std::cout << "内存拷贝时间: " << results.copy_time << " μs" << std::endl;
        std::cout << "总时间: " << results.total_time << " μs" << std::endl;
        
        return results;
    }
    
    TimeResults test_gpu_direct_scheme(void* gpu_buffer) {
        TimeResults results;
        
        // 运行多次取平均值
        int num_runs = 10;
        long long total_time_sum = 0;
        
        for (int i = 0; i < num_runs; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            
            // 使用GPU直通方案
            CUfileError_t status = cuFileDriverOpen();
            if (status.err != CU_FILE_SUCCESS) continue;
            
            int fd = open(test_filename.c_str(), O_RDONLY | O_DIRECT);
            CUfileDescr_t file_descr;
            memset(&file_descr, 0, sizeof(file_descr));
            file_descr.handle.fd = fd;
            file_descr.type = CU_FILE_HANDLE_TYPE_OPAQUE_FD;
            
            CUfileHandle_t file_handle;
            status = cuFileHandleRegister(&file_handle, &file_descr);
            if (status.err == CU_FILE_SUCCESS) {
                cuFileRead(file_handle, gpu_buffer, buffer_size, 0, 0);
                cuFileHandleDeregister(file_handle);
            }
            
            close(fd);
            cuFileDriverClose();
            
            auto end = std::chrono::high_resolution_clock::now();
            auto total_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            total_time_sum += total_time.count();
            
            // 预热后开始记录
            if (i == 0) {
                total_time_sum = 0;
            }
        }
        
        results.total_time = total_time_sum / (num_runs - 1);
        results.ssd_read_time = results.total_time; // GDS方案中读取和传输是连续的
        results.copy_time = 0; // 没有额外的拷贝步骤
        
        std::cout << "总时间: " << results.total_time << " μs" << std::endl;
        std::cout << "注: GPU直通方案无额外内存拷贝步骤" << std::endl;
        
        return results;
    }
    
    void print_comparison(const TimeResults& traditional, const TimeResults& gpu_direct) {
        std::cout << "\n=== 性能对比分析 ===" << std::endl;
        std::cout << "传统方案总耗时: " << traditional.total_time << " μs" << std::endl;
        std::cout << "GPU直通方案总耗时: " << gpu_direct.total_time << " μs" << std::endl;
        
        double speedup = static_cast<double>(traditional.total_time) / gpu_direct.total_time;
        std::cout << "性能提升倍数: " << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
        
        if (speedup > 1.0) {
            std::cout << "GPU直通方案比传统方案快 " << (speedup - 1.0) * 100 << "%" << std::endl;
        }
        
        // 分析瓶颈
        std::cout << "\n瓶颈分析:" << std::endl;
        std::cout << "传统方案中内存拷贝占比: " 
                  << (static_cast<double>(traditional.copy_time) / traditional.total_time) * 100 << "%" << std::endl;
    }
};

// 主函数
int main() {
    // 创建测试文件（1GB）
    std::string test_file = "test_data_1GB.bin";
    size_t file_size = 1024 * 1024 * 1024; // 1GB
    
    // 确保测试文件存在
    std::ifstream check_file(test_file);
    if (!check_file) {
        std::cout << "请先创建测试文件: " << test_file << " (大小: 1GB)" << std::endl;
        return -1;
    }
    
    SSDToGPUBenchmark benchmark(test_file, file_size);
    benchmark.run_comparison();
    
    return 0;
}