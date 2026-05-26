#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cuda_runtime.h>
#include <cufile.h>

// 全局统计变量
std::atomic<int> files_completed{0};
std::mutex cout_mutex;
std::mutex csv_mutex;

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
void write_files_thread(int thread_id, const std::string& dir_path, void* gpu_buffer, 
                       size_t io_size, int files_per_thread, int total_files) {
    CUfileError_t status;
    CUfileDescr_t desc;
    CUfileHandle_t handle;
    
    // 初始化文件描述符
    memset(&desc, 0, sizeof(CUfileDescr_t));
    desc.type = CU_FILE_HANDLE_TYPE_OPAQUE_FD;
    
    char filename[256];
    int local_completed = 0;
    
    for (int i = 0; i < files_per_thread; i++) {
        int file_idx = thread_id * files_per_thread + i;
        if (file_idx >= total_files) {
            break;  // 防止超出总文件数
        }
        
        // 生成文件名
        snprintf(filename, sizeof(filename), "%s/%d.txt", dir_path.c_str(), file_idx);
        
        // 打开文件
        int fd = open(filename, O_CREAT | O_WRONLY | O_DIRECT, 0644);
        if (fd < 0) {
            std::cerr << "Failed to open file: " << filename << " (errno: " << errno << ")" << std::endl;
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
        ssize_t written = cuFileWrite(handle, gpu_buffer, io_size, 0, 0);
        
        if (written != static_cast<ssize_t>(io_size)) {
            std::cerr << "Write incomplete for file " << filename 
                     << ": " << written << " of " << io_size << " bytes" << std::endl;
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
                     << " files. Total: " << files_completed << "/" << total_files << std::endl;
        }
    }
    
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Thread " << thread_id << " finished. Completed " 
                 << local_completed << " files." << std::endl;
    }
}

// 解析大小字符串（如512B, 4KB, 8MB等）为字节数
size_t parse_size_string(const std::string& size_str) {
    std::string str = size_str;
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    
    size_t multiplier = 1;
    
    // 检查后缀
    if (str.find("KB") != std::string::npos) {
        multiplier = 1024;
        str = str.substr(0, str.length() - 2);
    } else if (str.find("MB") != std::string::npos) {
        multiplier = 1024 * 1024;
        str = str.substr(0, str.length() - 2);
    } else if (str.find("GB") != std::string::npos) {
        multiplier = 1024 * 1024 * 1024;
        str = str.substr(0, str.length() - 2);
    } else if (str.find("B") != std::string::npos) {
        str = str.substr(0, str.length() - 1);
    } else if (str.find("K") != std::string::npos) {
        multiplier = 1024;
        str = str.substr(0, str.length() - 1);
    } else if (str.find("M") != std::string::npos) {
        multiplier = 1024 * 1024;
        str = str.substr(0, str.length() - 1);
    } else if (str.find("G") != std::string::npos) {
        multiplier = 1024 * 1024 * 1024;
        str = str.substr(0, str.length() - 1);
    }
    
    // 转换为数字
    try {
        size_t value = std::stoul(str);
        return value * multiplier;
    } catch (const std::exception& e) {
        std::cerr << "Invalid size format: " << size_str << std::endl;
        std::cerr << "Valid formats: 512B, 4KB, 8MB, 1GB, etc." << std::endl;
        exit(1);
    }
}

// 将测试结果写入CSV文件
void write_results_to_csv(const std::string& method, size_t io_size_bytes, int thread_num,
                         int total_files, double total_time_sec, double throughput_mb_s,
                         double iops, double avg_file_write_time_ms) {
    std::lock_guard<std::mutex> lock(csv_mutex);
    
    std::ofstream csv_file;
    bool file_exists = false;
    
    // 检查文件是否存在
    std::ifstream infile("gds_test_results.csv");
    if (infile.good()) {
        file_exists = true;
    }
    infile.close();
    
    // 以追加模式打开文件
    csv_file.open("gds_test_results.csv", std::ios_base::app);
    
    if (!csv_file.is_open()) {
        std::cerr << "Failed to open CSV file for writing results." << std::endl;
        return;
    }
    
    // 如果文件不存在，写入表头
    if (!file_exists) {
        csv_file << "Method,iosize_kb,threadnum,total_files,total_time_sec,throughput_MB_s,IOPS,avg_file_write_time_ms\n";
    }
    
    // 写入测试结果
    double iosize_kb = io_size_bytes / 1024.0;
    
    csv_file << std::fixed << std::setprecision(2)
             << method << ","
             << iosize_kb << ","
             << thread_num << ","
             << total_files << ","
             << total_time_sec << ","
             << throughput_mb_s << ","
             << iops << ","
             << avg_file_write_time_ms << "\n";
    
    csv_file.close();
    
    std::cout << "Results saved to gds_test_results.csv" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <output_directory> <threadnum> <IOSIZE>" << std::endl;
        std::cerr << "Example: " << argv[0] << " /tmp/gds_output 32 4KB" << std::endl;
        std::cerr << "IOSIZE examples: 512B, 4KB, 8MB, etc." << std::endl;
        return 1;
    }
    
    std::string output_dir = argv[1];
    int num_threads = std::stoi(argv[2]);
    std::string io_size_str = argv[3];
    
    // 解析IO大小
    size_t io_size = parse_size_string(io_size_str);
    
    // 总文件数
    const int total_files = 10240;
    
    // 计算每个线程处理的文件数
    int files_per_thread = total_files / num_threads;
    if (total_files % num_threads != 0) {
        files_per_thread++;
    }
    
    // 检查输出目录是否存在
    struct stat st;
    if (stat(output_dir.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
        std::cerr << "Output directory does not exist: " << output_dir << std::endl;
        std::cerr << "Creating directory..." << std::endl;
        if (mkdir(output_dir.c_str(), 0755) != 0) {
            std::cerr << "Failed to create directory: " << output_dir << std::endl;
            return 1;
        }
    }
    
    // 初始化cuFile
    CUfileError_t status = cuFileDriverOpen();
    CHECK_CUFILE(status.err);
    
    std::cout << "Initializing GDS with parameters:" << std::endl;
    std::cout << "  Total files: " << total_files << std::endl;
    std::cout << "  IO size per file: " << io_size << " bytes (" << io_size_str << ")" << std::endl;
    std::cout << "  Threads: " << num_threads << std::endl;
    std::cout << "  Files per thread: " << files_per_thread << std::endl;
    std::cout << "  Output directory: " << output_dir << std::endl;
    
    // 分配GPU内存
    void* gpu_buffer = nullptr;
    CHECK_CUDA(cudaMalloc(&gpu_buffer, io_size));
    
    // 初始化GPU缓冲区（写入一些测试数据）
    char* test_data = new char[io_size];
    for (size_t i = 0; i < io_size; i++) {
        test_data[i] = static_cast<char>(i % 256);
    }
    
    CHECK_CUDA(cudaMemcpy(gpu_buffer, test_data, io_size, cudaMemcpyHostToDevice));
    delete[] test_data;
    
    // 创建线程
    std::vector<std::thread> threads;
    files_completed = 0;
    
    std::cout << "\nStarting file writes..." << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(write_files_thread, i, output_dir, gpu_buffer, io_size, files_per_thread, total_files);
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    
    // 计算并显示性能统计
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    double total_seconds = duration.count() / 1000.0;
    size_t total_bytes = static_cast<size_t>(files_completed) * io_size;
    double throughput = total_bytes / (1024.0 * 1024.0) / total_seconds;  // MB/s
    double iops = files_completed / total_seconds;  // IOPS
    double avg_file_write_time_ms = duration.count() / static_cast<double>(files_completed);
    
    std::cout << "\n========== Performance Results ==========" << std::endl;
    std::cout << "Total files written: " << files_completed << std::endl;
    std::cout << "Total time: " << total_seconds << " seconds" << std::endl;
    std::cout << "Total data: " << total_bytes / (1024.0 * 1024.0) << " MB" << std::endl;
    std::cout << "Throughput: " << throughput << " MB/s" << std::endl;
    std::cout << "Files per second: " << files_completed / total_seconds << std::endl;
    std::cout << "IOPS: " << iops << std::endl;
    std::cout << "Average file write time: " << avg_file_write_time_ms << " ms" << std::endl;
    std::cout << "==========================================" << std::endl;
    
    // 将结果写入CSV文件
    write_results_to_csv("GDS", io_size, num_threads, files_completed, 
                        total_seconds, throughput, iops, avg_file_write_time_ms);
    
    // 清理资源
    CHECK_CUDA(cudaFree(gpu_buffer));
    cuFileDriverClose();
    
    return 0;
}