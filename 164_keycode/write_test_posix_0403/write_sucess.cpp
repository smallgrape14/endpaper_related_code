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
#include <cctype>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstring> 
// 全局变量
std::atomic<int> file_counter(0);
std::mutex cout_mutex;
std::mutex csv_mutex;  // CSV文件写入互斥锁
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

// 格式化和类显示函数
std::string formatBytes(size_t bytes) {
    const char* sizes[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    double dblBytes = static_cast<double>(bytes);
    
    while (dblBytes >= 1024 && i < 4) {
        dblBytes /= 1024;
        i++;
    }
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << dblBytes << " " << sizes[i];
    return ss.str();
}

// 将带单位的字符串转换为字节数
size_t parseSizeString(const std::string& sizeStr) {
    if (sizeStr.empty()) {
        std::cerr << "Error: Empty size string" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    std::string str = sizeStr;
    // 转换为大写以便处理
    for (char& c : str) {
        c = std::toupper(c);
    }
    
    // 查找数字部分的结束位置
    size_t i = 0;
    while (i < str.length() && (std::isdigit(str[i]) || str[i] == '.')) {
        i++;
    }
    
    if (i == 0) {
        std::cerr << "Error: Invalid size format: " << sizeStr << std::endl;
        exit(EXIT_FAILURE);
    }
    
    // 提取数字部分
    double value = std::stod(str.substr(0, i));
    
    // 提取单位部分
    std::string unit = str.substr(i);
    
    // 移除空格
    unit.erase(std::remove_if(unit.begin(), unit.end(), ::isspace), unit.end());
    
    // 根据单位转换
    if (unit.empty() || unit == "B") {
        return static_cast<size_t>(value);
    } else if (unit == "KB") {
        return static_cast<size_t>(value * 1024);
    } else if (unit == "MB") {
        return static_cast<size_t>(value * 1024 * 1024);
    } else if (unit == "GB") {
        return static_cast<size_t>(value * 1024 * 1024 * 1024);
    } else if (unit == "TB") {
        return static_cast<size_t>(value * 1024 * 1024 * 1024 * 1024);
    } else {
        std::cerr << "Error: Unknown unit: " << unit << std::endl;
        std::cerr << "Supported units: B, KB, MB, GB, TB" << std::endl;
        exit(EXIT_FAILURE);
    }
}

// 测试解析函数
void testParseSizeString() {
    std::cout << "Testing size parsing function:" << std::endl;
    struct TestCase {
        std::string input;
        size_t expected;
    };
    
    TestCase testCases[] = {
        {"512", 512},
        {"512B", 512},
        {"512b", 512},
        {"1KB", 1024},
        {"1Kb", 1024},
        {"1kb", 1024},
        {"1.5KB", 1536},
        {"4MB", 4 * 1024 * 1024},
        {"4mb", 4 * 1024 * 1024},
        {"1GB", 1024 * 1024 * 1024},
        {"1gb", 1024 * 1024 * 1024},
        {"1.5GB", static_cast<size_t>(1.5 * 1024 * 1024 * 1024)}
    };
    
    for (const auto& test : testCases) {
        try {
            size_t result = parseSizeString(test.input);
            std::cout << "  " << test.input << " -> " << formatBytes(result) 
                      << " (" << (result == test.expected ? "PASS" : "FAIL") << ")" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "  " << test.input << " -> ERROR: " << e.what() << std::endl;
        }
    }
    std::cout << std::endl;
}

// 追加结果到CSV文件
void appendResultToCSV(const std::string& csv_file, const std::string& timestamp, 
                       int threadnum, size_t iosize_bytes, const std::string& output_dir,
                       int total_files, double total_data_gb, double total_time_sec,
                       double throughput_mbps, double iops, double avg_file_write_time_ms,
                       double total_copy_time_ms, double total_write_time_ms) {
    std::lock_guard<std::mutex> lock(csv_mutex);
    
    std::ofstream csv;
    
    // 检查文件是否存在，如果不存在则写入表头
    bool file_exists = fileExists(csv_file);
    csv.open(csv_file, std::ios::app);
    
    if (!csv) {
        std::cerr << "Error: Failed to open CSV file: " << csv_file << std::endl;
        return;
    }
    
    // 如果文件不存在，写入CSV表头
    if (!file_exists) {
        csv << "Method,iosize_kb,threadnum,total_files,total_time_sec,"
            << "throughput_MB_s,IOPS,avg_file_write_time_ms" << std::endl;
        // csv << "Method,threadnum,iosize_kb,output_dir,total_files,total_data_GB,total_time_sec,"
        //     << "throughput_MB_s,IOPS,avg_file_write_time_ms,copy_time_ms,write_time_ms" << std::endl;
    }
    
    
 // 写入数据
    csv << std::fixed << std::setprecision(2)
        << "POSIX" << ","
        << iosize_bytes / 1024 << ","

        << threadnum << ","
        // << "\"" << output_dir << "\","
        << total_files << ","
        // << total_data_gb << ","
        << total_time_sec << ","
        << throughput_mbps << ","
        << iops << ","
        << avg_file_write_time_ms<<"\n";

    
    csv.close();
    
    std::cout << "Results saved to CSV file: " << csv_file << std::endl;
}

// 获取当前时间戳
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// 优化的工作线程函数 - 只执行数据拷贝和写入，文件已预先打开
void worker_thread_optimized(int thread_id, int total_files, size_t iosize, 
                            const std::vector<int>& file_descriptors, void* d_buffer, 
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
        
        auto start_write = std::chrono::high_resolution_clock::now();
        
        // 使用预先打开的文件描述符进行写入
        int fd = file_descriptors[file_idx];
        ssize_t bytes_written = 0;
        ssize_t total_written = 0;
        
        // 循环写入直到写入所有数据
        while (total_written < static_cast<ssize_t>(iosize)) {
            bytes_written = write(fd, h_buffer + total_written, iosize - total_written);
            if (bytes_written < 0) {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cerr << "Thread " << thread_id << ": Failed to write to file " << file_idx << std::endl;
                break;
            }
            total_written += bytes_written;
        }
        
        // 确保数据写入磁盘
        fsync(fd);
        
        auto end_write = std::chrono::high_resolution_clock::now();
        auto write_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_write - start_write);
        total_write_time += write_duration.count();
        
        files_processed++;
        
        // // 进度输出（每完成1%输出一次）
        // if (file_idx % (total_files / 100) == 0) {
        //     std::lock_guard<std::mutex> lock(cout_mutex);
        //     float progress = (float)file_idx / total_files * 100.0f;
        //     std::cout << "Progress: " << std::fixed << std::setprecision(1) << progress 
        //              << "%, Thread " << thread_id << " processed file " << file_idx 
        //              << " (Copy: " << copy_duration.count() << "µs, Write: " 
        //              << write_duration.count() << "µs)" << std::endl;
        // }
    }
    
    delete[] h_buffer;
}

int main(int argc, char* argv[]) {
    // 检查参数
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <output_directory> <threadnum> <IOSIZE>" << std::endl;
        std::cout << "IOSIZE format examples:" << std::endl;
        std::cout << "  - 512          (512 bytes)" << std::endl;
        std::cout << "  - 512B         (512 bytes)" << std::endl;
        std::cout << "  - 1KB          (1 kilobyte = 1024 bytes)" << std::endl;
        std::cout << "  - 4MB          (4 megabytes = 4194304 bytes)" << std::endl;
        std::cout << "  - 1GB          (1 gigabyte = 1073741824 bytes)" << std::endl;
        std::cout << "  - 1.5KB        (1.5 kilobytes = 1536 bytes)" << std::endl;
        std::cout << "\nNote: 10240 files should already exist in the output directory." << std::endl;
        return 1;
    }
    
    std::string output_dir = argv[1];
    int threadnum = std::stoi(argv[2]);
    std::string iosize_str = argv[3];
    
    // 测试解析函数（可选）
    if (iosize_str == "--test") {
        testParseSizeString();
        return 0;
    }
    
    // 解析IOSIZE
    size_t iosize = parseSizeString(iosize_str);
    
    constexpr int TOTAL_FILES = 10240;
    
    if (threadnum <= 0 || threadnum > 256) {
        std::cout << "Error: threadnum must be between 1 and 256" << std::endl;
        return 1;
    }
    
    if (iosize <= 0 || iosize > 1024ULL * 1024 * 1024 * 1024) {  // 最大1TB
        std::cout << "Error: IOSIZE must be between 1 and 1TB" << std::endl;
        return 1;
    }
    
    if (iosize < 512) {
        std::cout << "Warning: IOSIZE is very small (" << iosize << " bytes). "
                  << "Performance may be affected by I/O overhead." << std::endl;
    }
    
    // 检查输出目录是否存在
    if (!directoryExists(output_dir)) {
        std::cout << "Error: Output directory does not exist: " << output_dir << std::endl;
        std::cout << "Please create the directory and ensure it contains 10240 files." << std::endl;
        return 1;
    }
    
    std::cout << "Starting multi-threaded GPU to file write (optimized)..." << std::endl;
    std::cout << "Total files: " << TOTAL_FILES << std::endl;
    std::cout << "File size: " << formatBytes(iosize) << " (" << iosize << " bytes)" << std::endl;
    std::cout << "Thread count: " << threadnum << std::endl;
    std::cout << "Output directory: " << output_dir << std::endl;
    std::cout << "Note: Assuming 10240 files already exist in the directory." << std::endl;
    
    // 计算总数据量
    double total_data_bytes = static_cast<double>(TOTAL_FILES) * iosize;
    double total_data_kb = total_data_bytes / 1024.0;
    double total_data_mb = total_data_kb / 1024.0;
    double total_data_gb = total_data_mb / 1024.0;
    
    std::cout << "Total data to write: " << formatBytes(static_cast<size_t>(total_data_bytes)) 
              << " (" << total_data_mb << " MB, " << total_data_gb << " GB)" << std::endl;
    
    // 阶段1: 预先打开所有文件
    std::cout << "\n=== Phase 1: Pre-opening all files ===" << std::endl;
    auto start_open = std::chrono::high_resolution_clock::now();
    
    std::vector<int> file_descriptors(TOTAL_FILES);
    int files_opened = 0;
    
    for (int i = 0; i < TOTAL_FILES; i++) {
        std::stringstream filename;
        filename << output_dir << "/" << i << ".txt";
        
        // 使用O_DIRECT标志绕过内核缓存（可选，需要对齐）
        // int fd = open(filename.str().c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_DIRECT, 0644);
        
        // 使用标准方式打开
        int fd = open(filename.str().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            std::cout << "Error: Failed to open file " << filename.str() 
                      << ", error: " << strerror(errno) 
                      << std::endl;
            // 关闭之前已打开的文件
            for (int j = 0; j < i; j++) {
                close(file_descriptors[j]);
            }
            return 1;
        }
        
        file_descriptors[i] = fd;
        files_opened++;
        
        if (i % 1000 == 0 && i > 0) {
            std::cout << "Opened " << i << "/" << TOTAL_FILES << " files" << std::endl;
        }
    }
    
    auto end_open = std::chrono::high_resolution_clock::now();
    auto open_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_open - start_open);
    std::cout << "Opened " << files_opened << " files in " << open_duration.count() << " ms" << std::endl;
    
    // 阶段2: 初始化CUDA和测试数据
    std::cout << "\n=== Phase 2: Initializing CUDA and test data ===" << std::endl;
    
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
    size_t num_copies = buffer_size / iosize;
    if (buffer_size % iosize != 0) num_copies++;
    
    std::cout << "Filling GPU memory with test data (" << num_copies << " copies of " 
              << formatBytes(iosize) << " blocks)..." << std::endl;
    
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
    
    // 阶段3: 多线程写入
    std::cout << "\n=== Phase 3: Multi-threaded writing ===" << std::endl;
    
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
        threads.emplace_back([i, TOTAL_FILES, iosize, &file_descriptors, d_buffer, 
                             &thread_copy_times_local, &thread_write_times_local, &thread_file_counts_local]() {
            worker_thread_optimized(i, TOTAL_FILES, iosize, file_descriptors, d_buffer, 
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
    
    // 阶段4: 清理资源
    std::cout << "\n=== Phase 4: Cleaning up resources ===" << std::endl;
    
    // 关闭所有文件描述符
    std::cout << "Closing all file descriptors..." << std::endl;
    for (int i = 0; i < TOTAL_FILES; i++) {
        close(file_descriptors[i]);
    }
    
    // 清理CUDA资源
    for (int i = 0; i < threadnum; i++) {
        cudaStreamDestroy(cuda_streams[i]);
    }
    cudaFree(d_buffer);
    
    // 计算统计信息
    long long total_copy_time_us = std::accumulate(thread_copy_times_local.begin(), thread_copy_times_local.end(), 0LL);
    long long total_write_time_us = std::accumulate(thread_write_times_local.begin(), thread_write_times_local.end(), 0LL);
    double total_copy_time_ms = total_copy_time_us / 1000.0;
    double total_write_time_ms = total_write_time_us / 1000.0;
    
    double total_time_sec = total_duration.count() / 1000.0;
    
    // 计算IOPS和吞吐量
    double iops = TOTAL_FILES / total_time_sec;
    double throughput_mbps = total_data_mb / total_time_sec;
    double avg_file_write_time_ms = total_time_sec * 1000.0 / TOTAL_FILES;
    
    // 输出统计信息
    std::cout << "\n======= Performance Summary =======" << std::endl;
    std::cout << "File open time: " << open_duration.count() << " ms" << std::endl;
    std::cout << "Total files written: " << TOTAL_FILES << std::endl;
    std::cout << "Total data written: " << formatBytes(static_cast<size_t>(total_data_bytes)) 
              << " (" << total_data_mb << " MB, " << total_data_gb << " GB)" << std::endl;
    std::cout << "Write phase time: " << total_time_sec << " seconds" << std::endl;
    std::cout << "Write throughput: " << throughput_mbps << " MB/s" << std::endl;
    std::cout << "IOPS: " << iops << " operations/second" << std::endl;
    std::cout << "Average file write time: " << avg_file_write_time_ms << " ms/file" << std::endl;
    std::cout << "\n======= Detailed Statistics =======" << std::endl;
    std::cout << "Total copy time (all threads): " << total_copy_time_ms << " ms" << std::endl;
    std::cout << "Total write time (all threads): " << total_write_time_ms << " ms" << std::endl;
    std::cout << "Average copy time per file: " << (double)total_copy_time_us / TOTAL_FILES << " µs" << std::endl;
    std::cout << "Average write time per file: " << (double)total_write_time_us / TOTAL_FILES << " µs" << std::endl;
    
    // 线程负载均衡信息
    std::cout << "\n======= Thread Load Balancing =======" << std::endl;
    for (int i = 0; i < threadnum; i++) {
        std::cout << "Thread " << i << ": " << thread_file_counts_local[i] 
                  << " files, Copy: " << thread_copy_times_local[i] / 1000.0 
                  << " ms, Write: " << thread_write_times_local[i] / 1000.0 << " ms" << std::endl;
    }
    
    // 追加结果到CSV文件
    std::string csv_file = "gpu_write_performance.csv";
    std::string timestamp = getCurrentTimestamp();
    
    appendResultToCSV(csv_file, timestamp, threadnum, iosize, output_dir,
                      TOTAL_FILES, total_data_gb, total_time_sec,
                      throughput_mbps, iops, avg_file_write_time_ms,
                      total_copy_time_ms, total_write_time_ms);
    
    return 0;
}