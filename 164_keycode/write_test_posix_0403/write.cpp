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

// 鍏ㄥ眬鍙橀噺
std::mutex cout_mutex;
std::mutex csv_mutex;
std::vector<cudaStream_t> cuda_streams;
std::vector<long long> thread_copy_times;
std::vector<long long> thread_write_times;
std::vector<int> thread_file_counts;

// 妫€鏌UDA閿欒
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
                       double total_copy_time_ms, double total_write_time_ms,
                       int balanced_threads) {
    std::lock_guard<std::mutex> lock(csv_mutex);
    
    std::ofstream csv;
    bool file_exists = fileExists(csv_file);
    csv.open(csv_file, std::ios::app);
    
    if (!csv) {
        std::cerr << "Error: Failed to open CSV file: " << csv_file << std::endl;
        return;
    }
    
    // 濡傛灉鏂囦欢涓嶅瓨鍦紝鍐欏叆CSV琛ㄥご
    if (!file_exists) {
        csv << "Method,iosize_kb,threadnum,total_files,total_time_sec,"
            << "throughput_MB_s,IOPS,avg_file_write_time_ms" << std::endl;
        // csv << "Method,threadnum,iosize_kb,output_dir,total_files,total_data_GB,total_time_sec,"
        //     << "throughput_MB_s,IOPS,avg_file_write_time_ms,copy_time_ms,write_time_ms" << std::endl;
    }
    
    // csv << std::fixed << std::setprecision(3)
    //     << timestamp << ","
    //     << threadnum << ","
    //     << balanced_threads << ","
    //     << iosize_bytes << ","
    //     << "\"" << formatBytes(iosize_bytes) << "\","
    //     << "\"" << output_dir << "\","
    //     << total_files << ","
    //     << total_data_gb << ","
    //     << total_time_sec << ","
    //     << throughput_mbps << ","
    //     << iops << ","
    //     << avg_file_write_time_ms << ","
    //     << total_copy_time_ms << ","
    //     << total_write_time_ms << std::endl;
     // 鍐欏叆鏁版嵁
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

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// 浣跨敤闈欐€佸垎閰嶇殑worker_thread鍑芥暟
void worker_thread_static(int thread_id, int start_idx, int end_idx, size_t iosize, 
                         const std::vector<int>& file_descriptors, void* d_buffer, 
                         long long& total_copy_time, long long& total_write_time, int& files_processed) {
    cudaSetDevice(0);
    
    char* h_buffer = new char[iosize];
    cudaStream_t stream = cuda_streams[thread_id];
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
        // 浠嶨PU鏄惧瓨澶嶅埗鏁版嵁鍒颁富鏈哄唴瀛?
        auto start_copy = std::chrono::high_resolution_clock::now();
        
        cudaError_t err = cudaMemcpyAsync(h_buffer, 
                                         (char*)d_buffer + (file_idx * iosize) % (1024 * 1024 * 1024),
                                         iosize, cudaMemcpyDeviceToHost, stream);
        CHECK_CUDA_ERROR(err);
        cudaStreamSynchronize(stream);
        
        auto end_copy = std::chrono::high_resolution_clock::now();
        auto copy_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_copy - start_copy);
        total_copy_time += copy_duration.count();
        
        // 浣跨敤pwrite鐩存帴鍐欏叆
        auto start_write = std::chrono::high_resolution_clock::now();
        
        ssize_t bytes_written = pwrite(file_descriptors[file_idx], h_buffer, iosize, 0);
        if (bytes_written != static_cast<ssize_t>(iosize)) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cerr << "Thread " << thread_id << ": pwrite failed for file " 
                      << file_idx << ", error: " << strerror(errno) << std::endl;
        }
        
        auto end_write = std::chrono::high_resolution_clock::now();
        auto write_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_write - start_write);
        total_write_time += write_duration.count();
        
        files_processed++;
        
        // 杩涘害杈撳嚭
        if (file_idx % 1000 == 0) {
            float progress = (float)(file_idx - start_idx) / files_in_thread * 100.0f;
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Thread " << thread_id << ": " << std::fixed << std::setprecision(1) << progress 
                     << "% (files " << file_idx - start_idx << "/" << files_in_thread << ")" << std::endl;
        }
    }
    
    delete[] h_buffer;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <output_directory> <threadnum> <IOSIZE>" << std::endl;
        std::cout << "IOSIZE format examples: 512B, 1KB, 4MB, 1GB" << std::endl;
        std::cout << "\nNote: Using static load balancing. Each thread gets equal number of files." << std::endl;
        return 1;
    }
    
    std::string output_dir = argv[1];
    int requested_threadnum = std::stoi(argv[2]);  // 鐢ㄦ埛璇锋眰鐨勭嚎绋嬫暟
    std::string iosize_str = argv[3];
    
    size_t iosize = parseSizeString(iosize_str);
    constexpr int TOTAL_FILES = 10240;
    
    if (requested_threadnum <= 0) {
        std::cout << "Error: threadnum must be positive" << std::endl;
        return 1;
    }
    
    // 瀹為檯浣跨敤鐨勭嚎绋嬫暟锛氫笉鑳借秴杩囨€绘枃浠舵暟
    int actual_threadnum = std::min(requested_threadnum, TOTAL_FILES);
    if (requested_threadnum > TOTAL_FILES) {
        std::cout << "Warning: Requested " << requested_threadnum << " threads, but only " 
                  << TOTAL_FILES << " files available. Using " << actual_threadnum << " threads." << std::endl;
    }
    
    if (!directoryExists(output_dir)) {
        std::cout << "Error: Output directory does not exist: " << output_dir << std::endl;
        return 1;
    }
    
    std::cout << "Starting multi-threaded GPU to file write with static load balancing..." << std::endl;
    std::cout << "Total files: " << TOTAL_FILES << std::endl;
    std::cout << "File size: " << formatBytes(iosize) << " (" << iosize << " bytes)" << std::endl;
    std::cout << "Requested threads: " << requested_threadnum << std::endl;
    std::cout << "Actual threads (balanced): " << actual_threadnum << std::endl;
    std::cout << "Output directory: " << output_dir << std::endl;
    
    double total_data_bytes = static_cast<double>(TOTAL_FILES) * iosize;
    double total_data_mb = total_data_bytes / (1024.0 * 1024.0);
    double total_data_gb = total_data_mb / 1024.0;
    
    std::cout << "Total data to write: " << formatBytes(static_cast<size_t>(total_data_bytes)) 
              << " (" << total_data_mb << " MB, " << total_data_gb << " GB)" << std::endl;
    
    // 璁＄畻姣忎釜绾跨▼澶勭悊鐨勬枃浠舵暟
    int base_files_per_thread = TOTAL_FILES / actual_threadnum;
    int extra_files = TOTAL_FILES % actual_threadnum;
    
    std::cout << "\nLoad balancing:" << std::endl;
    std::cout << "  Base files per thread: " << base_files_per_thread << std::endl;
    std::cout << "  Threads with one extra file: " << extra_files << std::endl;
    
    // 闃舵1: 棰勫厛鎵撳紑鎵€鏈夋枃浠?
    std::cout << "\n=== Phase 1: Pre-opening all files ===" << std::endl;
    auto start_open = std::chrono::high_resolution_clock::now();
    
    std::vector<int> file_descriptors(TOTAL_FILES);
    for (int i = 0; i < TOTAL_FILES; i++) {
        std::stringstream filename;
        // filename << output_dir << "/file_" << std::setfill('0') << std::setw(6) << i << ".bin";
        filename << output_dir << "/" << i << ".txt";

        
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
    
    // 闃舵2: 鍒濆鍖朇UDA
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
    
    cuda_streams.resize(actual_threadnum);
    for (int i = 0; i < actual_threadnum; i++) {
        CHECK_CUDA_ERROR(cudaStreamCreate(&cuda_streams[i]));
    }
    
    // 闃舵3: 澶氱嚎绋嬪啓鍏ワ紙闈欐€佸垎閰嶏級
    std::cout << "\n=== Phase 3: Multi-threaded writing (static load balancing) ===" << std::endl;
    
    std::vector<long long> thread_copy_times_local(actual_threadnum, 0);
    std::vector<long long> thread_write_times_local(actual_threadnum, 0);
    std::vector<int> thread_file_counts_local(actual_threadnum, 0);
    
    // 璁＄畻姣忎釜绾跨▼鐨勬枃浠惰寖鍥?
    std::vector<int> thread_start_idx(actual_threadnum);
    std::vector<int> thread_end_idx(actual_threadnum);
    
    int current_idx = 0;
    for (int i = 0; i < actual_threadnum; i++) {
        thread_start_idx[i] = current_idx;
        int files_for_this_thread = base_files_per_thread + (i < extra_files ? 1 : 0);
        current_idx += files_for_this_thread;
        thread_end_idx[i] = current_idx;
        
        std::cout << "Thread " << i << ": files " << thread_start_idx[i] 
                  << " to " << thread_end_idx[i]-1 
                  << " (" << files_for_this_thread << " files)" << std::endl;
    }
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < actual_threadnum; i++) {
        threads.emplace_back([i, iosize, &thread_start_idx, &thread_end_idx, 
                             &file_descriptors, d_buffer, 
                             &thread_copy_times_local, &thread_write_times_local, &thread_file_counts_local]() {
            worker_thread_static(i, thread_start_idx[i], thread_end_idx[i], iosize, 
                               file_descriptors, d_buffer, 
                               thread_copy_times_local[i], thread_write_times_local[i], thread_file_counts_local[i]);
        });
    }
    
    for (auto& t : threads) t.join();
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // 闃舵4: 娓呯悊
    std::cout << "\n=== Phase 4: Cleaning up resources ===" << std::endl;
    
    for (int i = 0; i < TOTAL_FILES; i++) {
        close(file_descriptors[i]);
    }
    
    for (int i = 0; i < actual_threadnum; i++) {
        cudaStreamDestroy(cuda_streams[i]);
    }
    cudaFree(d_buffer);
    
    // 璁＄畻缁熻淇℃伅
    long long total_copy_time_us = std::accumulate(thread_copy_times_local.begin(), thread_copy_times_local.end(), 0LL);
    long long total_write_time_us = std::accumulate(thread_write_times_local.begin(), thread_write_times_local.end(), 0LL);
    double total_copy_time_ms = total_copy_time_us / 1000.0;
    double total_write_time_ms = total_write_time_us / 1000.0;
    
    double total_time_sec = total_duration.count() / 1000.0;
    double iops = TOTAL_FILES / total_time_sec;
    double throughput_mbps = total_data_mb / total_time_sec;
    double avg_file_write_time_ms = total_time_sec * 1000.0 / TOTAL_FILES;
    
    // 杈撳嚭缁熻淇℃伅
    std::cout << "\n======= Performance Summary =======" << std::endl;
    std::cout << "File open time: " << open_duration.count() << " ms" << std::endl;
    std::cout << "Total files written: " << TOTAL_FILES << std::endl;
    std::cout << "Total data written: " << formatBytes(static_cast<size_t>(total_data_bytes)) 
              << " (" << total_data_mb << " MB, " << total_data_gb << " GB)" << std::endl;
    std::cout << "Write phase time: " << total_time_sec << " seconds" << std::endl;
    std::cout << "Write throughput: " << throughput_mbps << " MB/s" << std::endl;
    std::cout << "IOPS: " << iops << " operations/second" << std::endl;
    std::cout << "Average file write time: " << avg_file_write_time_ms << " ms/file" << std::endl;
    
    // 绾跨▼璐熻浇鍧囪　淇℃伅
    std::cout << "\n======= Thread Load Balancing =======" << std::endl;
    
    int zero_work_threads = 0;
    int min_files = INT_MAX;
    int max_files = 0;
    
    for (int i = 0; i < actual_threadnum; i++) {
        int files_processed = thread_file_counts_local[i];
        double copy_time_ms = thread_copy_times_local[i] / 1000.0;
        double write_time_ms = thread_write_times_local[i] / 1000.0;
        
        std::cout << "Thread " << std::setw(3) << i << ": " 
                  << std::setw(5) << files_processed << " files, "
                  << "Copy: " << std::setw(8) << std::fixed << std::setprecision(1) << copy_time_ms << " ms, "
                  << "Write: " << std::setw(8) << write_time_ms << " ms";
        
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
    
    if (min_files != INT_MAX) {
        double load_imbalance = (double)(max_files - min_files) / min_files * 100.0;
        std::cout << "Load imbalance: " << std::fixed << std::setprecision(1) << load_imbalance << "%" << std::endl;
    }
    
    // 杩藉姞缁撴灉鍒癈SV
    // std::string csv_file = "result/POSIX_performance.csv"; iosize_str
    std::string csv_file = "result/POSIX_performance_" + iosize_str + ".csv";
    std::string timestamp = getCurrentTimestamp();
    
    appendResultToCSV(csv_file, timestamp, requested_threadnum, iosize, output_dir,
                      TOTAL_FILES, total_data_gb, total_time_sec,
                      throughput_mbps, iops, avg_file_write_time_ms,
                      total_copy_time_ms, total_write_time_ms,
                      actual_threadnum);
    
    return 0;
}