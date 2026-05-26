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
#include <cufile.h>  // GDS cuFile头文件
#include <algorithm>
#include <numeric>
#include <sys/stat.h>
#include <cctype>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

// 全局变量
std::mutex cout_mutex;
std::mutex csv_mutex;
std::vector<long long> thread_copy_times;
std::vector<long long> thread_write_times;
std::vector<int> thread_file_counts;

// 检查CUDA错误
#define CHECK_CUDA_ERROR(err) \
    do { \
        if (err != cudaSuccess) { \
            std::lock_guard<std::mutex> lock(cout_mutex); \
            std::cerr << "CUDA error: " << cudaGetErrorString(err) << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

// 检查cuFile错误
#define CHECK_CUFILE_ERROR(ret) \
    do { \
        if (ret < 0) { \
            std::lock_guard<std::mutex> lock(cout_mutex); \
            std::cerr << "cuFile error: " << ret << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

// 检查GDS可用性
void checkGdsAvailable() {
    int driver_version = 0;
    cudaDriverGetVersion(&driver_version);
    
    std::cout << "CUDA Driver Version: " << driver_version / 1000 << "." 
              << (driver_version % 100) / 10 << std::endl;
    
    if (driver_version < 11010) {  // 需要CUDA 11.1+
        std::cout << "Warning: GDS requires CUDA 11.1 or later" << std::endl;
    }
    
    // 注意：CUFILE_CHECK_VERSION 在较新版本的cuFile中可能被移除
    // 我们使用另一种方法来检查GDS
    std::cout << "Initializing GDS driver..." << std::endl;
    
    CUfileError_t status = cuFileDriverOpen();
    if (status.err != CU_FILE_SUCCESS) {
        std::cout << "Warning: GDS driver not available: " << (int)status.err << std::endl;
        std::cout << "Falling back to standard I/O" << std::endl;
    } else {
        std::cout << "GDS driver initialized successfully" << std::endl;
    }
}

bool directoryExists(const std::string& path) {
    struct stat info;
    return (stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR));
}

// 添加缺失的fileExists函数
bool fileExists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

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

size_t parseSizeString(const std::string& sizeStr) {
    if (sizeStr.empty()) {
        std::cerr << "Error: Empty size string" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    std::string str = sizeStr;
    for (char& c : str) c = std::toupper(c);
    
    size_t i = 0;
    while (i < str.length() && (std::isdigit(str[i]) || str[i] == '.')) i++;
    
    if (i == 0) {
        std::cerr << "Error: Invalid size format: " << sizeStr << std::endl;
        exit(EXIT_FAILURE);
    }
    
    double value = std::stod(str.substr(0, i));
    std::string unit = str.substr(i);
    unit.erase(std::remove_if(unit.begin(), unit.end(), ::isspace), unit.end());
    
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

void appendResultToCSV(const std::string& csv_file, const std::string& timestamp, 
                       int threadnum, size_t iosize_bytes, const std::string& output_dir,
                       int total_files, double total_data_gb, double total_time_sec,
                       double throughput_mbps, double iops, double avg_file_write_time_ms,
                       double total_copy_time_ms, double total_write_time_ms,
                       int actual_threadnum, const std::string& mode) {
    std::lock_guard<std::mutex> lock(csv_mutex);
    
    std::ofstream csv;
    bool file_exists = fileExists(csv_file);
    csv.open(csv_file, std::ios::app);
    
    if (!csv) {
        std::cerr << "Error: Failed to open CSV file: " << csv_file << std::endl;
        return;
    }
    
    if (!file_exists) {
        csv << "timestamp,threadnum,actual_threads,iosize_bytes,iosize_human,output_dir,total_files,total_data_GB,"
            << "total_time_sec,throughput_MB_s,IOPS,avg_file_write_time_ms,total_copy_time_ms,"
            << "total_write_time_ms,mode" << std::endl;
    }
    
    csv << std::fixed << std::setprecision(3)
        << timestamp << ","
        << threadnum << ","
        << actual_threadnum << ","
        << iosize_bytes << ","
        << "\"" << formatBytes(iosize_bytes) << "\","
        << "\"" << output_dir << "\","
        << total_files << ","
        << total_data_gb << ","
        << total_time_sec << ","
        << throughput_mbps << ","
        << iops << ","
        << avg_file_write_time_ms << ","
        << total_copy_time_ms << ","
        << total_write_time_ms << ","
        << "\"" << mode << "\"" << std::endl;
    
    csv.close();
    std::cout << "Results saved to CSV file: " << csv_file << std::endl;
}

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// 使用GDS的工作线程函数
void worker_thread_gds(int thread_id, int start_idx, int end_idx, size_t iosize, 
                      const std::vector<std::string>& filenames, void* d_buffer, 
                      long long& total_write_time, int& files_processed) {
    cudaSetDevice(0);
    
    total_write_time = 0;
    files_processed = 0;
    
    int files_in_thread = end_idx - start_idx;
    if (files_in_thread <= 0) {
        return;
    }
    
    for (int file_idx = start_idx; file_idx < end_idx; file_idx++) {
        std::string filename = filenames[file_idx];
        
        auto start_write = std::chrono::high_resolution_clock::now();
        
        // 打开文件
        int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cerr << "Thread " << thread_id << ": Failed to open file " 
                      << filename << ", error: " << strerror(errno) << std::endl;
            continue;
        }
        
        // 注册文件描述符到cuFile
        CUfileDescr_t desc;
        memset(&desc, 0, sizeof(CUfileDescr_t));
        desc.handle.fd = fd;
        desc.type = CU_FILE_HANDLE_TYPE_OPAQUE_FD;
        
        CUfileHandle_t cf_handle = nullptr;
        CUfileError_t status = cuFileHandleRegister(&cf_handle, &desc);
        if (status.err != CU_FILE_SUCCESS) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cerr << "Thread " << thread_id << ": cuFileHandleRegister failed for " 
                      << filename << ", error: " << (int)status.err << std::endl;
            close(fd);
            continue;
        }
        
        // 使用cuFileWrite从GPU显存直接写入文件
        ssize_t ret = cuFileWrite(cf_handle, 
                                 (char*)d_buffer + (file_idx * iosize) % (1024 * 1024 * 1024),
                                 iosize, 0, 0);
        
        if (ret != (ssize_t)iosize) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cerr << "Thread " << thread_id << ": cuFileWrite failed for " 
                      << filename << ", wrote " << ret << " of " << iosize << " bytes" << std::endl;
        }
        
        // 取消注册并关闭文件
        cuFileHandleDeregister(cf_handle);
        close(fd);
        
        auto end_write = std::chrono::high_resolution_clock::now();
        auto write_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_write - start_write);
        total_write_time += write_duration.count();
        
        files_processed++;
        
        // 进度输出
        if ((file_idx - start_idx) % 100 == 0) {
            float progress = (float)(file_idx - start_idx) / files_in_thread * 100.0f;
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Thread " << thread_id << ": " << std::fixed << std::setprecision(1) << progress 
                     << "% (files " << file_idx - start_idx << "/" << files_in_thread << ")" << std::endl;
        }
    }
}

// 回退到标准I/O的工作线程函数
void worker_thread_standard(int thread_id, int start_idx, int end_idx, size_t iosize, 
                          const std::vector<std::string>& filenames, void* d_buffer, 
                          long long& total_copy_time, long long& total_write_time, int& files_processed) {
    cudaSetDevice(0);
    
    char* h_buffer = new char[iosize];
    total_copy_time = 0;
    total_write_time = 0;
    files_processed = 0;
    
    std::fill_n(h_buffer, iosize, static_cast<char>(thread_id % 256));
    
    int files_in_thread = end_idx - start_idx;
    if (files_in_thread <= 0) {
        delete[] h_buffer;
        return;
    }
    
    for (int file_idx = start_idx; file_idx < end_idx; file_idx++) {
        std::string filename = filenames[file_idx];
        
        // 从GPU显存复制数据到主机内存
        auto start_copy = std::chrono::high_resolution_clock::now();
        
        cudaError_t err = cudaMemcpy(h_buffer, 
                                     (char*)d_buffer + (file_idx * iosize) % (1024 * 1024 * 1024),
                                     iosize, cudaMemcpyDeviceToHost);
        CHECK_CUDA_ERROR(err);
        
        auto end_copy = std::chrono::high_resolution_clock::now();
        auto copy_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_copy - start_copy);
        total_copy_time += copy_duration.count();
        
        // 使用标准write写入文件
        auto start_write = std::chrono::high_resolution_clock::now();
        
        int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cerr << "Thread " << thread_id << ": Failed to open file " 
                      << filename << ", error: " << strerror(errno) << std::endl;
            continue;
        }
        
        ssize_t bytes_written = write(fd, h_buffer, iosize);
        if (bytes_written != static_cast<ssize_t>(iosize)) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cerr << "Thread " << thread_id << ": write failed for file " 
                      << filename << ", error: " << strerror(errno) << std::endl;
        }
        
        close(fd);
        
        auto end_write = std::chrono::high_resolution_clock::now();
        auto write_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_write - start_write);
        total_write_time += write_duration.count();
        
        files_processed++;
        
        // 进度输出
        if ((file_idx - start_idx) % 100 == 0) {
            float progress = (float)(file_idx - start_idx) / files_in_thread * 100.0f;
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Thread " << thread_id << ": " << std::fixed << std::setprecision(1) << progress 
                     << "% (files " << file_idx - start_idx << "/" << files_in_thread << ")" << std::endl;
        }
    }
    
    delete[] h_buffer;
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cout << "Usage: " << argv[0] << " <output_directory> <threadnum> <IOSIZE> <mode>" << std::endl;
        std::cout << "IOSIZE format examples: 512B, 1KB, 4MB, 1GB" << std::endl;
        std::cout << "Modes:" << std::endl;
        std::cout << "  gds    - Use NVIDIA GDS (GPUDirect Storage)" << std::endl;
        std::cout << "  std    - Use standard I/O (copy to host then write)" << std::endl;
        std::cout << "\nNote: 10240 files will be created in the output directory." << std::endl;
        return 1;
    }
    
    std::string output_dir = argv[1];
    int requested_threadnum = std::stoi(argv[2]);
    std::string iosize_str = argv[3];
    std::string mode = argv[4];
    
    if (mode != "gds" && mode != "std") {
        std::cout << "Error: mode must be 'gds' or 'std'" << std::endl;
        return 1;
    }
    
    size_t iosize = parseSizeString(iosize_str);
    constexpr int TOTAL_FILES = 10240;
    
    if (requested_threadnum <= 0) {
        std::cout << "Error: threadnum must be positive" << std::endl;
        return 1;
    }
    
    // 实际使用的线程数：不能超过总文件数
    int actual_threadnum = std::min(requested_threadnum, TOTAL_FILES);
    if (requested_threadnum > TOTAL_FILES) {
        std::cout << "Warning: Requested " << requested_threadnum << " threads, but only " 
                  << TOTAL_FILES << " files available. Using " << actual_threadnum << " threads." << std::endl;
    }
    
    if (!directoryExists(output_dir)) {
        std::cout << "Creating output directory: " << output_dir << std::endl;
        if (mkdir(output_dir.c_str(), 0755) != 0) {
            std::cout << "Error: Failed to create output directory: " << output_dir 
                      << ", error: " << strerror(errno) << std::endl;
            return 1;
        }
    }
    
    std::cout << "Starting multi-threaded GPU to file write..." << std::endl;
    std::cout << "Mode: " << mode;
    if (mode == "gds") std::cout << " (NVIDIA GDS - Direct GPU to Storage)";
    else std::cout << " (Standard I/O)";
    std::cout << std::endl;
    
    std::cout << "Total files: " << TOTAL_FILES << std::endl;
    std::cout << "File size: " << formatBytes(iosize) << " (" << iosize << " bytes)" << std::endl;
    std::cout << "Requested threads: " << requested_threadnum << std::endl;
    std::cout << "Actual threads: " << actual_threadnum << std::endl;
    std::cout << "Output directory: " << output_dir << std::endl;
    
    double total_data_bytes = static_cast<double>(TOTAL_FILES) * iosize;
    double total_data_mb = total_data_bytes / (1024.0 * 1024.0);
    double total_data_gb = total_data_mb / 1024.0;
    
    std::cout << "Total data to write: " << formatBytes(static_cast<size_t>(total_data_bytes)) 
              << " (" << total_data_mb << " MB, " << total_data_gb << " GB)" << std::endl;
    
    // 检查GDS可用性
    bool gds_available = false;
    if (mode == "gds") {
        // 尝试初始化GDS
        CUfileError_t status = cuFileDriverOpen();
        if (status.err != CU_FILE_SUCCESS) {
            std::cout << "Warning: GDS driver not available, falling back to standard I/O" << std::endl;
            mode = "std";
        } else {
            gds_available = true;
            std::cout << "GDS driver initialized successfully" << std::endl;
        }
    }
    
    // 阶段1: 创建文件列表
    std::cout << "\n=== Phase 1: Creating file list ===" << std::endl;
    auto start_prep = std::chrono::high_resolution_clock::now();
    
    std::vector<std::string> filenames(TOTAL_FILES);
    for (int i = 0; i < TOTAL_FILES; i++) {
        std::stringstream filename;
        filename << output_dir << "/"<< i << ".txt";
        filenames[i] = filename.str();
    }
    
    auto end_prep = std::chrono::high_resolution_clock::now();
    auto prep_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_prep - start_prep);
    
    // 阶段2: 初始化CUDA和测试数据
    std::cout << "\n=== Phase 2: Initializing CUDA and test data ===" << std::endl;
    
    cudaSetDevice(0);
    
    // 在GPU上分配1GB显存并填充测试数据
    void* d_buffer = nullptr;
    size_t buffer_size = 1024 * 1024 * 1024;  // 1GB
    CHECK_CUDA_ERROR(cudaMalloc(&d_buffer, buffer_size));
    
    // 创建测试数据
    std::cout << "Generating test data on GPU..." << std::endl;
    
    // 分配主机内存用于初始化
    char* h_init_data = new char[iosize];
    for (size_t i = 0; i < iosize; i++) {
        h_init_data[i] = i % 256;
    }
    
    // 将测试数据复制到GPU
    for (size_t offset = 0; offset < buffer_size; offset += iosize) {
        CHECK_CUDA_ERROR(cudaMemcpy((char*)d_buffer + offset, h_init_data, 
                                   std::min(iosize, buffer_size - offset), 
                                   cudaMemcpyHostToDevice));
    }
    delete[] h_init_data;
    
    std::cout << "Test data generation complete." << std::endl;
    
    // 阶段3: 计算负载均衡
    std::cout << "\n=== Phase 3: Setting up load balancing ===" << std::endl;
    
    int base_files_per_thread = TOTAL_FILES / actual_threadnum;
    int extra_files = TOTAL_FILES % actual_threadnum;
    
    std::vector<int> thread_start_idx(actual_threadnum);
    std::vector<int> thread_end_idx(actual_threadnum);
    
    int current_idx = 0;
    for (int i = 0; i < actual_threadnum; i++) {
        thread_start_idx[i] = current_idx;
        int files_for_this_thread = base_files_per_thread + (i < extra_files ? 1 : 0);
        current_idx += files_for_this_thread;
        thread_end_idx[i] = current_idx;
        
        std::cout << "Thread " << std::setw(3) << i << ": files " 
                  << std::setw(5) << thread_start_idx[i] << " to " 
                  << std::setw(5) << thread_end_idx[i]-1 
                  << " (" << std::setw(4) << files_for_this_thread << " files)" << std::endl;
    }
    
    // 准备线程统计信息
    std::vector<long long> thread_copy_times_local(actual_threadnum, 0);
    std::vector<long long> thread_write_times_local(actual_threadnum, 0);
    std::vector<int> thread_file_counts_local(actual_threadnum, 0);
    
    // 阶段4: 多线程写入
    std::cout << "\n=== Phase 4: Multi-threaded writing ===" << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    
    if (mode == "gds" && gds_available) {
        // 使用GDS模式
        for (int i = 0; i < actual_threadnum; i++) {
            threads.emplace_back([i, iosize, &thread_start_idx, &thread_end_idx, 
                                 &filenames, d_buffer, 
                                 &thread_write_times_local, &thread_file_counts_local]() {
                worker_thread_gds(i, thread_start_idx[i], thread_end_idx[i], iosize, 
                                 filenames, d_buffer, 
                                 thread_write_times_local[i], thread_file_counts_local[i]);
            });
        }
    } else {
        // 使用标准I/O模式
        for (int i = 0; i < actual_threadnum; i++) {
            threads.emplace_back([i, iosize, &thread_start_idx, &thread_end_idx, 
                                 &filenames, d_buffer, 
                                 &thread_copy_times_local, &thread_write_times_local, 
                                 &thread_file_counts_local]() {
                worker_thread_standard(i, thread_start_idx[i], thread_end_idx[i], iosize, 
                                      filenames, d_buffer, 
                                      thread_copy_times_local[i], thread_write_times_local[i], 
                                      thread_file_counts_local[i]);
            });
        }
    }
    
    for (auto& t : threads) t.join();
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // 阶段5: 清理
    std::cout << "\n=== Phase 5: Cleaning up resources ===" << std::endl;
    
    cudaFree(d_buffer);
    
    if (gds_available) {
        cuFileDriverClose();
    }
    
    // 计算统计信息
    long long total_copy_time_us = std::accumulate(thread_copy_times_local.begin(), thread_copy_times_local.end(), 0LL);
    long long total_write_time_us = std::accumulate(thread_write_times_local.begin(), thread_write_times_local.end(), 0LL);
    double total_copy_time_ms = total_copy_time_us / 1000.0;
    double total_write_time_ms = total_write_time_us / 1000.0;
    
    double total_time_sec = total_duration.count() / 1000.0;
    double iops = TOTAL_FILES / total_time_sec;
    double throughput_mbps = total_data_mb / total_time_sec;
    double avg_file_write_time_ms = total_time_sec * 1000.0 / TOTAL_FILES;
    
    // 输出统计信息
    std::cout << "\n======= Performance Summary =======" << std::endl;
    std::cout << "Mode: " << mode;
    if (mode == "gds" && gds_available) std::cout << " (NVIDIA GDS)";
    else std::cout << " (Standard I/O)";
    std::cout << std::endl;
    
    std::cout << "Preparation time: " << prep_duration.count() << " ms" << std::endl;
    std::cout << "Total files written: " << TOTAL_FILES << std::endl;
    std::cout << "Total data written: " << formatBytes(static_cast<size_t>(total_data_bytes)) 
              << " (" << total_data_mb << " MB, " << total_data_gb << " GB)" << std::endl;
    std::cout << "Write phase time: " << total_time_sec << " seconds" << std::endl;
    std::cout << "Write throughput: " << throughput_mbps << " MB/s" << std::endl;
    std::cout << "IOPS: " << iops << " operations/second" << std::endl;
    std::cout << "Average file write time: " << avg_file_write_time_ms << " ms/file" << std::endl;
    
    if (mode == "std") {
        std::cout << "\n======= Detailed Statistics =======" << std::endl;
        std::cout << "Total copy time (GPU->Host): " << total_copy_time_ms << " ms" << std::endl;
        std::cout << "Total write time (Host->Disk): " << total_write_time_ms << " ms" << std::endl;
        std::cout << "Average copy time per file: " << (double)total_copy_time_us / TOTAL_FILES << " µs" << std::endl;
        std::cout << "Average write time per file: " << (double)total_write_time_us / TOTAL_FILES << " µs" << std::endl;
    }
    
    // 线程负载均衡信息
    std::cout << "\n======= Thread Load Balancing =======" << std::endl;
    
    int zero_work_threads = 0;
    int min_files = INT_MAX;
    int max_files = 0;
    
    for (int i = 0; i < actual_threadnum; i++) {
        int files_processed = thread_file_counts_local[i];
        double copy_time_ms = thread_copy_times_local[i] / 1000.0;
        double write_time_ms = thread_write_times_local[i] / 1000.0;
        
        std::cout << "Thread " << std::setw(3) << i << ": " 
                  << std::setw(5) << files_processed << " files";
        
        if (mode == "std") {
            std::cout << ", Copy: " << std::setw(8) << std::fixed << std::setprecision(1) 
                      << copy_time_ms << " ms, Write: " << std::setw(8) << write_time_ms << " ms";
        } else {
            std::cout << ", Write: " << std::setw(8) << write_time_ms << " ms";
        }
        
        if (files_processed == 0) {
            std::cout << " [NO WORK]";
            zero_work_threads++;
        }
        std::cout << std::endl;
        
        if (files_processed > 0) {
            min_files = std::min(min_files, files_processed);
            max_files = std::max(max_files, files_processed);
        }
    }
    
    if (zero_work_threads > 0) {
        std::cout << "\nWARNING: " << zero_work_threads << " threads had no work!" << std::endl;
    }
    
    if (min_files != INT_MAX && max_files > 0) {
        double load_imbalance = (double)(max_files - min_files) / min_files * 100.0;
        std::cout << "Load imbalance: " << std::fixed << std::setprecision(1) << load_imbalance << "%" << std::endl;
    }
    
    // 追加结果到CSV
    std::string csv_file = "gpu_write_performance.csv";
    std::string timestamp = getCurrentTimestamp();
    
    appendResultToCSV(csv_file, timestamp, requested_threadnum, iosize, output_dir,
                      TOTAL_FILES, total_data_gb, total_time_sec,
                      throughput_mbps, iops, avg_file_write_time_ms,
                      total_copy_time_ms, total_write_time_ms,
                      actual_threadnum, mode);
    
    return 0;
}