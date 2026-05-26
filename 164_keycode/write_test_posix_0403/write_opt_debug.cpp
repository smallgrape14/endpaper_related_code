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
#include <cstring>
#include <cuda_runtime.h>
#include <algorithm>
#include <numeric>
#include <sys/stat.h>
#include <cctype>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <aio.h>

// 全局变量
std::atomic<int> file_counter(0);
std::mutex cout_mutex;
std::mutex csv_mutex;
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

bool directoryExists(const std::string& path) {
    struct stat info;
    return (stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR));
}

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
                       double total_copy_time_ms, double total_write_time_ms) {
    std::lock_guard<std::mutex> lock(csv_mutex);
    
    std::ofstream csv;
    bool file_exists = fileExists(csv_file);
    csv.open(csv_file, std::ios::app);
    
    if (!csv) {
        std::cerr << "Error: Failed to open CSV file: " << csv_file << std::endl;
        return;
    }
    
    if (!file_exists) {
        csv << "timestamp,threadnum,iosize_bytes,iosize_human,output_dir,total_files,total_data_GB,"
            << "total_time_sec,throughput_MB_s,IOPS,avg_file_write_time_ms,total_copy_time_ms,"
            << "total_write_time_ms" << std::endl;
    }
    
    csv << std::fixed << std::setprecision(3)
        << timestamp << ","
        << threadnum << ","
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
        << total_write_time_ms << std::endl;
    
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

// 方案1: 使用mmap + 批量写入
void worker_thread_mmap(int thread_id, int total_files, size_t iosize, 
                       const std::vector<int>& file_descriptors, void* d_buffer, 
                       long long& total_copy_time, long long& total_write_time, int& files_processed) {
    cudaSetDevice(0);
    
    char* h_buffer = new char[iosize];
    cudaStream_t stream = cuda_streams[thread_id];
    total_copy_time = 0;
    total_write_time = 0;
    files_processed = 0;
    
    // 使用mmap批量处理
    const int BATCH_SIZE = 16;  // 每批处理16个文件
    int batch_count = 0;
    int file_indices[BATCH_SIZE];
    char* mmap_buffers[BATCH_SIZE];
    
    while (true) {
        int file_idx = file_counter.fetch_add(BATCH_SIZE);
        if (file_idx >= total_files) break;
        
        int actual_batch = std::min(BATCH_SIZE, total_files - file_idx);
        
        // 批量mmap
        auto start_mmap = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < actual_batch; i++) {
            int current_idx = file_idx + i;
            file_indices[i] = current_idx;
            
            // 内存映射文件
            mmap_buffers[i] = (char*)mmap(NULL, iosize, PROT_WRITE, MAP_SHARED, 
                                        file_descriptors[current_idx], 0);
            if (mmap_buffers[i] == MAP_FAILED) {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cerr << "Thread " << thread_id << ": mmap failed for file " 
                          << current_idx << ", error: " << strerror(errno) << std::endl;
                mmap_buffers[i] = nullptr;
            }
        }
        
        auto end_mmap = std::chrono::high_resolution_clock::now();
        auto mmap_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_mmap - start_mmap);
        
        // 批量从GPU复制数据
        for (int i = 0; i < actual_batch; i++) {
            if (mmap_buffers[i] == nullptr) continue;
            
            auto start_copy = std::chrono::high_resolution_clock::now();
            
            cudaError_t err = cudaMemcpyAsync(h_buffer, 
                                            (char*)d_buffer + (file_indices[i] * iosize) % (1024 * 1024 * 1024),
                                            iosize, cudaMemcpyDeviceToHost, stream);
            CHECK_CUDA_ERROR(err);
            cudaStreamSynchronize(stream);
            
            auto end_copy = std::chrono::high_resolution_clock::now();
            auto copy_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_copy - start_copy);
            total_copy_time += copy_duration.count();
            
            // 复制到内存映射区域
            auto start_write = std::chrono::high_resolution_clock::now();
            memcpy(mmap_buffers[i], h_buffer, iosize);
            
            // msync确保数据写入文件
            msync(mmap_buffers[i], iosize, MS_ASYNC);
            
            auto end_write = std::chrono::high_resolution_clock::now();
            auto write_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_write - start_write);
            total_write_time += write_duration.count();
            
            files_processed++;
        }
        
        // 批量unmap
        for (int i = 0; i < actual_batch; i++) {
            if (mmap_buffers[i] != nullptr) {
                munmap(mmap_buffers[i], iosize);
            }
        }
        
        batch_count++;
        
        if (batch_count % 10 == 0) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            float progress = (float)(file_idx + actual_batch) / total_files * 100.0f;
            std::cout << "Progress: " << std::fixed << std::setprecision(1) << progress 
                     << "%, Thread " << thread_id << " processed batch " << batch_count 
                     << " (avg mmap: " << mmap_duration.count()/actual_batch << "µs)" << std::endl;
        }
    }
    
    delete[] h_buffer;
}

// 方案2: 使用pwrite + 无sync (最快，但可能丢失数据)
void worker_thread_pwrite_nosync(int thread_id, int total_files, size_t iosize, 
                                const std::vector<int>& file_descriptors, void* d_buffer, 
                                long long& total_copy_time, long long& total_write_time, int& files_processed) {
    cudaSetDevice(0);
    
    char* h_buffer = new char[iosize];
    cudaStream_t stream = cuda_streams[thread_id];
    total_copy_time = 0;
    total_write_time = 0;
    files_processed = 0;
    
    // 批量处理
    const int BATCH_SIZE = 32;
    
    while (true) {
        int file_idx = file_counter.fetch_add(BATCH_SIZE);
        if (file_idx >= total_files) break;
        
        int actual_batch = std::min(BATCH_SIZE, total_files - file_idx);
        
        for (int i = 0; i < actual_batch; i++) {
            int current_idx = file_idx + i;
            
            // 从GPU复制数据
            auto start_copy = std::chrono::high_resolution_clock::now();
            
            cudaError_t err = cudaMemcpyAsync(h_buffer, 
                                            (char*)d_buffer + (current_idx * iosize) % (1024 * 1024 * 1024),
                                            iosize, cudaMemcpyDeviceToHost, stream);
            CHECK_CUDA_ERROR(err);
            cudaStreamSynchronize(stream);
            
            auto end_copy = std::chrono::high_resolution_clock::now();
            auto copy_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_copy - start_copy);
            total_copy_time += copy_duration.count();
            
            // 使用pwrite直接写入，不移动文件指针
            auto start_write = std::chrono::high_resolution_clock::now();
            
            ssize_t bytes_written = pwrite(file_descriptors[current_idx], h_buffer, iosize, 0);
            if (bytes_written != static_cast<ssize_t>(iosize)) {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cerr << "Thread " << thread_id << ": pwrite failed for file " 
                          << current_idx << ", error: " << strerror(errno) << std::endl;
            }
            
            auto end_write = std::chrono::high_resolution_clock::now();
            auto write_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_write - start_write);
            total_write_time += write_duration.count();
            
            files_processed++;
        }
        
        if (file_idx % (total_files / 100) == 0) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            float progress = (float)file_idx / total_files * 100.0f;
            std::cout << "Progress: " << std::fixed << std::setprecision(1) << progress 
                     << "%, Thread " << thread_id << std::endl;
        }
    }
    
    delete[] h_buffer;
}

// 方案3: 使用writev批量写入
void worker_thread_writev(int thread_id, int total_files, size_t iosize, 
                         const std::vector<int>& file_descriptors, void* d_buffer, 
                         long long& total_copy_time, long long& total_write_time, int& files_processed) {
    cudaSetDevice(0);
    
    char* h_buffer = new char[iosize];
    cudaStream_t stream = cuda_streams[thread_id];
    total_copy_time = 0;
    total_write_time = 0;
    files_processed = 0;
    
    // 使用writev批量IO
    const int BATCH_SIZE = 8;
    struct iovec iovs[BATCH_SIZE];
    
    while (true) {
        int file_idx = file_counter.fetch_add(BATCH_SIZE);
        if (file_idx >= total_files) break;
        
        int actual_batch = std::min(BATCH_SIZE, total_files - file_idx);
        
        // 先复制所有数据
        for (int i = 0; i < actual_batch; i++) {
            int current_idx = file_idx + i;
            
            auto start_copy = std::chrono::high_resolution_clock::now();
            
            cudaError_t err = cudaMemcpyAsync(h_buffer, 
                                            (char*)d_buffer + (current_idx * iosize) % (1024 * 1024 * 1024),
                                            iosize, cudaMemcpyDeviceToHost, stream);
            CHECK_CUDA_ERROR(err);
            cudaStreamSynchronize(stream);
            
            auto end_copy = std::chrono::high_resolution_clock::now();
            auto copy_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_copy - start_copy);
            total_copy_time += copy_duration.count();
            
            // 为writev准备
            iovs[i].iov_base = h_buffer;
            iovs[i].iov_len = iosize;
        }
        
        // 批量写入
        auto start_write = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < actual_batch; i++) {
            int current_idx = file_idx + i;
            ssize_t bytes_written = writev(file_descriptors[current_idx], &iovs[i], 1);
            if (bytes_written != static_cast<ssize_t>(iosize)) {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cerr << "Thread " << thread_id << ": writev failed for file " 
                          << current_idx << ", error: " << strerror(errno) << std::endl;
            }
        }
        
        auto end_write = std::chrono::high_resolution_clock::now();
        auto write_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_write - start_write);
        total_write_time += write_duration.count() * actual_batch;
        
        files_processed += actual_batch;
        
        if (file_idx % (total_files / 100) == 0) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            float progress = (float)file_idx / total_files * 100.0f;
            std::cout << "Progress: " << std::fixed << std::setprecision(1) << progress 
                     << "%, Thread " << thread_id << std::endl;
        }
    }
    
    delete[] h_buffer;
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cout << "Usage: " << argv[0] << " <output_directory> <threadnum> <IOSIZE> <mode>" << std::endl;
        std::cout << "IOSIZE format examples: 512B, 1KB, 4MB, 1GB" << std::endl;
        std::cout << "Modes:" << std::endl;
        std::cout << "  1 - mmap (内存映射)" << std::endl;
        std::cout << "  2 - pwrite_nosync (最快，但可能丢失数据)" << std::endl;
        std::cout << "  3 - writev (批量写入)" << std::endl;
        std::cout << "\nNote: 10240 files should already exist in the output directory." << std::endl;
        return 1;
    }
    
    std::string output_dir = argv[1];
    int threadnum = std::stoi(argv[2]);
    std::string iosize_str = argv[3];
    int mode = std::stoi(argv[4]);
    
    if (mode < 1 || mode > 3) {
        std::cout << "Error: mode must be 1, 2, or 3" << std::endl;
        return 1;
    }
    
    if (iosize_str == "--test") {
        return 0;
    }
    
    size_t iosize = parseSizeString(iosize_str);
    constexpr int TOTAL_FILES = 10240;
    
    if (threadnum <= 0 || threadnum > 256) {
        std::cout << "Error: threadnum must be between 1 and 256" << std::endl;
        return 1;
    }
    
    if (!directoryExists(output_dir)) {
        std::cout << "Error: Output directory does not exist: " << output_dir << std::endl;
        return 1;
    }
    
    std::cout << "Starting multi-threaded GPU to file write (optimized mode " << mode << ")..." << std::endl;
    std::cout << "Total files: " << TOTAL_FILES << std::endl;
    std::cout << "File size: " << formatBytes(iosize) << " (" << iosize << " bytes)" << std::endl;
    std::cout << "Thread count: " << threadnum << std::endl;
    std::cout << "Output directory: " << output_dir << std::endl;
    std::cout << "Mode: " << mode << " (";
    if (mode == 1) std::cout << "mmap)";
    else if (mode == 2) std::cout << "pwrite_nosync)";
    else if (mode == 3) std::cout << "writev)";
    std::cout << std::endl;
    
    double total_data_bytes = static_cast<double>(TOTAL_FILES) * iosize;
    double total_data_mb = total_data_bytes / (1024.0 * 1024.0);
    double total_data_gb = total_data_mb / 1024.0;
    
    std::cout << "Total data to write: " << formatBytes(static_cast<size_t>(total_data_bytes)) 
              << " (" << total_data_mb << " MB, " << total_data_gb << " GB)" << std::endl;
    
    // 阶段1: 预先打开所有文件
    std::cout << "\n=== Phase 1: Pre-opening all files ===" << std::endl;
    auto start_open = std::chrono::high_resolution_clock::now();
    
    std::vector<int> file_descriptors(TOTAL_FILES);
    for (int i = 0; i < TOTAL_FILES; i++) {
        std::stringstream filename;
        filename << output_dir << "/file_" << std::setfill('0') << std::setw(6) << i << ".bin";
        
        int fd = open(filename.str().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            std::cout << "Error: Failed to open file " << filename.str() 
                      << ", error: " << strerror(errno) << std::endl;
            for (int j = 0; j < i; j++) close(file_descriptors[j]);
            return 1;
        }
        
        file_descriptors[i] = fd;
        
        if (i % 1000 == 0 && i > 0) {
            std::cout << "Opened " << i << "/" << TOTAL_FILES << " files" << std::endl;
        }
    }
    
    auto end_open = std::chrono::high_resolution_clock::now();
    auto open_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_open - start_open);
    std::cout << "Opened " << TOTAL_FILES << " files in " << open_duration.count() << " ms" << std::endl;
    
    // 阶段2: 初始化CUDA
    std::cout << "\n=== Phase 2: Initializing CUDA and test data ===" << std::endl;
    cudaSetDevice(0);
    
    void* d_buffer = nullptr;
    size_t buffer_size = 1024 * 1024 * 1024;  // 1GB
    CHECK_CUDA_ERROR(cudaMalloc(&d_buffer, buffer_size));
    
    char* h_test_data = new char[iosize];
    for (size_t i = 0; i < iosize; i++) h_test_data[i] = i % 256;
    
    for (size_t offset = 0; offset < buffer_size; offset += iosize) {
        CHECK_CUDA_ERROR(cudaMemcpy((char*)d_buffer + offset, h_test_data, 
                                   std::min(iosize, buffer_size - offset), 
                                   cudaMemcpyHostToDevice));
    }
    delete[] h_test_data;
    std::cout << "Test data generation complete." << std::endl;
    
    cuda_streams.resize(threadnum);
    for (int i = 0; i < threadnum; i++) {
        CHECK_CUDA_ERROR(cudaStreamCreate(&cuda_streams[i]));
    }
    
    // 阶段3: 多线程写入
    std::cout << "\n=== Phase 3: Multi-threaded writing ===" << std::endl;
    
    std::vector<long long> thread_copy_times_local(threadnum, 0);
    std::vector<long long> thread_write_times_local(threadnum, 0);
    std::vector<int> thread_file_counts_local(threadnum, 0);
    
    file_counter = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < threadnum; i++) {
        if (mode == 1) {
            threads.emplace_back([i, TOTAL_FILES, iosize, &file_descriptors, d_buffer, 
                                 &thread_copy_times_local, &thread_write_times_local, &thread_file_counts_local]() {
                worker_thread_mmap(i, TOTAL_FILES, iosize, file_descriptors, d_buffer, 
                                 thread_copy_times_local[i], thread_write_times_local[i], thread_file_counts_local[i]);
            });
        } else if (mode == 2) {
            threads.emplace_back([i, TOTAL_FILES, iosize, &file_descriptors, d_buffer, 
                                 &thread_copy_times_local, &thread_write_times_local, &thread_file_counts_local]() {
                worker_thread_pwrite_nosync(i, TOTAL_FILES, iosize, file_descriptors, d_buffer, 
                                          thread_copy_times_local[i], thread_write_times_local[i], thread_file_counts_local[i]);
            });
        } else if (mode == 3) {
            threads.emplace_back([i, TOTAL_FILES, iosize, &file_descriptors, d_buffer, 
                                 &thread_copy_times_local, &thread_write_times_local, &thread_file_counts_local]() {
                worker_thread_writev(i, TOTAL_FILES, iosize, file_descriptors, d_buffer, 
                                   thread_copy_times_local[i], thread_write_times_local[i], thread_file_counts_local[i]);
            });
        }
    }
    
    for (auto& t : threads) t.join();
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // 阶段4: 清理
    std::cout << "\n=== Phase 4: Cleaning up resources ===" << std::endl;
    
    // 关闭所有文件
    for (int i = 0; i < TOTAL_FILES; i++) {
        close(file_descriptors[i]);
    }
    
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
    double iops = TOTAL_FILES / total_time_sec;
    double throughput_mbps = total_data_mb / total_time_sec;
    double avg_file_write_time_ms = total_time_sec * 1000.0 / TOTAL_FILES;
    
    // 输出统计信息
    std::cout << "\n======= Performance Summary =======" << std::endl;
    std::cout << "Mode: " << mode;
    if (mode == 1) std::cout << " (mmap)";
    else if (mode == 2) std::cout << " (pwrite_nosync)";
    else if (mode == 3) std::cout << " (writev)";
    std::cout << std::endl;
    
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
    
    // 追加结果到CSV
    std::string csv_file = "gpu_write_performance.csv";
    std::string timestamp = getCurrentTimestamp();
    
    appendResultToCSV(csv_file, timestamp, threadnum, iosize, output_dir,
                      TOTAL_FILES, total_data_gb, total_time_sec,
                      throughput_mbps, iops, avg_file_write_time_ms,
                      total_copy_time_ms, total_write_time_ms);
    
    return 0;
}