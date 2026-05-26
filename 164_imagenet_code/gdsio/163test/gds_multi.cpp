#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <dirent.h>
#include <sys/stat.h>
#include <iomanip>
#include <unistd.h>
#include <fcntl.h>
#include <cuda_runtime.h>
#include <cufile.h>
#include <algorithm>
#include <cctype>
#include <string>
#include <chrono>
#include <map>

using namespace std;

#define vary_iosize 0 //1表示启用iosize参数
#define BLOCK_SIZE (1024 * 1024UL)  //1MB
#define MAX_PATH_LENGTH 1024
#define MAX_THREADS 1024 // 最大线程数

// 文件信息结构体
struct FileInfo {
    string name;
    string path;
    size_t size;
};

// 预分配的文件句柄和资源
struct FileResources {
    int fd;
    CUfileHandle_t cf_handle;
    void* devPtr;
    size_t file_size;
};

// 每个文件的性能统计
struct FilePerfStats {
    string filename;
    double read_time_ms;
    size_t bytes_read;
    size_t operations;
    bool success;
};

// 全局性能统计结构体
struct PerfStats {
    atomic<double> total_time_ms;
    atomic<size_t> total_bytes;
    atomic<size_t> files_processed;
    atomic<size_t> total_operations;
    double avg_bandwidth_mbs;
    double avg_iops;
    double avg_latency_ms;
};

// 线程池任务队列
class ThreadPool {
private:
    vector<thread> workers;
    queue<function<void()>> tasks;
    mutex queue_mutex;
    condition_variable condition;
    condition_variable completion_condition;
    atomic<bool> stop;
    atomic<size_t> active_tasks;

public:
    ThreadPool(size_t threads) : stop(false), active_tasks(0) {
        for(size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                while(true) {
                    function<void()> task;
                    {
                        unique_lock<mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this] {
                            return this->stop || !this->tasks.empty();
                        });
                        if(this->stop && this->tasks.empty()) return;
                        task = move(this->tasks.front());
                        this->tasks.pop();
                        active_tasks++;
                    }
                    task();
                    {
                        unique_lock<mutex> lock(this->queue_mutex);
                        active_tasks--;
                        if(active_tasks == 0 && tasks.empty()) {
                            completion_condition.notify_all();
                        }
                    }
                }
            });
        }
    }

    template<class F>
    void enqueue(F&& f) {
        {
            unique_lock<mutex> lock(queue_mutex);
            tasks.emplace(forward<F>(f));
        }
        condition.notify_one();
    }

    void waitComplete() {
        unique_lock<mutex> lock(queue_mutex);
        completion_condition.wait(lock, [this] {
            return tasks.empty() && active_tasks == 0;
        });
    }

    ~ThreadPool() {
        {
            unique_lock<mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for(thread &worker : workers) {
            worker.join();
        }
    }
};

// 获取目录下所有文件列表
vector<FileInfo> getFilesInDirectory(const string& directoryPath) {
    vector<FileInfo> files;
    DIR *dir;
    struct dirent *ent;
    struct stat file_stat;
    
    if ((dir = opendir(directoryPath.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }
            
            string full_path = directoryPath + "/" + ent->d_name;
            
            if (stat(full_path.c_str(), &file_stat) == 0) {
                if (S_ISREG(file_stat.st_mode)) {
                    FileInfo file_info;
                    file_info.name = ent->d_name;
                    file_info.path = full_path;
                    file_info.size = file_stat.st_size;
                    files.push_back(file_info);
                }
            }
        }
        closedir(dir);
    } else {
        cerr << "无法打开目录: " << directoryPath << endl;
    }
    
    return files;
}

// 预分配文件资源（在多线程启动前执行）
map<string, FileResources> preallocateFileResources(const vector<FileInfo>& files, int device_id) {
    map<string, FileResources> resources;
    
    // 设置CUDA设备
    cudaError_t cuda_status = cudaSetDevice(device_id);
    if (cuda_status != cudaSuccess) {
        cerr << "设置CUDA设备失败: " << cudaGetErrorString(cuda_status) << endl;
        return resources;
    }
    
    for (const auto& file : files) {
        FileResources res;
        res.fd = -1;
        res.devPtr = NULL;
        res.file_size = file.size;
        
        // 打开文件
        res.fd = open(file.path.c_str(), O_RDONLY | O_DIRECT);
        if (res.fd < 0) {
            cerr << "打开文件失败: " << file.path << " - " << strerror(errno) << endl;
            continue;
        }
        
        // 注册cuFile句柄
        CUfileDescr_t cf_descr;
        memset((void *)&cf_descr, 0, sizeof(CUfileDescr_t));
        cf_descr.handle.fd = res.fd;
        cf_descr.type = CU_FILE_HANDLE_TYPE_OPAQUE_FD;
        CUfileError_t status = cuFileHandleRegister(&res.cf_handle, &cf_descr);
        if (status.err != CU_FILE_SUCCESS) {
            cerr << "文件注册失败: " << file.path << endl;
            close(res.fd);
            continue;
        }
        
        // 分配GPU内存
        cuda_status = cudaMalloc(&res.devPtr, BLOCK_SIZE);
        if (cuda_status != cudaSuccess) {
            cerr << "GPU内存分配失败: " << file.path << " - " << cudaGetErrorString(cuda_status) << endl;
            cuFileHandleDeregister(res.cf_handle);
            close(res.fd);
            continue;
        }
        
        cudaMemset(res.devPtr, 0, BLOCK_SIZE);
        resources[file.path] = res;
    }
    
    return resources;
}

// 释放预分配的资源
void cleanupFileResources(map<string, FileResources>& resources) {
    for (auto& [path, res] : resources) {
        if (res.devPtr) {
            cudaFree(res.devPtr);
        }
        if (res.fd >= 0) {
            cuFileHandleDeregister(res.cf_handle);
            close(res.fd);
        }
    }
    resources.clear();
}

// 纯读操作测量（线程安全版本）
FilePerfStats measureReadOperationOnly(const string& file_path, FileResources& res, size_t iosize) {
    FilePerfStats file_stats;
    file_stats.filename = file_path;
    file_stats.success = false;
    file_stats.bytes_read = 0;
    file_stats.operations = 0;
    file_stats.read_time_ms = 0.0;
    
    if (res.fd < 0 || res.devPtr == NULL) {
        return file_stats;
    }
    
    // 创建CUDA事件用于计时
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    
    double file_total_time = 0.0;
    size_t file_operations = 0;
    size_t total_bytes_read = 0;
    
    // 仅测量读操作
    for (size_t offset = 0; offset < res.file_size; offset += BLOCK_SIZE) {
        size_t read_size;
        #if vary_iosize == 1
            read_size = iosize;
        #else
            read_size = min(static_cast<size_t>(BLOCK_SIZE), res.file_size - offset);
        #endif
        
        cudaEventRecord(start, 0);
        ssize_t bytes_read = cuFileRead(res.cf_handle, res.devPtr, read_size, offset, 0);
        cudaEventRecord(stop, 0);
        cudaEventSynchronize(stop);
        
        if (bytes_read < 0) {
            cerr << "读取失败在偏移量 " << offset << " 文件: " << file_path <<"iosize: "<<read_size<< endl;
            break;
        }
        
        float elapsed_time;
        cudaEventElapsedTime(&elapsed_time, start, stop);
        
        file_total_time += elapsed_time;
        file_operations++;
        total_bytes_read += bytes_read;
    }
    
    file_stats.read_time_ms = file_total_time;
    file_stats.bytes_read = total_bytes_read;
    file_stats.operations = file_operations;
    file_stats.success = true;
    
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    
    return file_stats;
}

// 计算性能指标
void calculatePerformanceMetrics(PerfStats& stats,int64_t duration) {
    if (stats.total_time_ms > 0) {
        double total_time_seconds = duration / 1000.0;
        stats.avg_bandwidth_mbs = (stats.total_bytes / (1024.0 * 1024.0)) / total_time_seconds;
        stats.avg_iops = stats.total_operations / total_time_seconds;
        if (stats.total_operations > 0) {
            stats.avg_latency_ms = stats.total_time_ms / stats.total_operations;
        }
        // double total_time_seconds = stats.total_time_ms / 1000.0;
        // stats.avg_bandwidth_mbs = (stats.total_bytes / (1024.0 * 1024.0)) / total_time_seconds;
        // stats.avg_iops = stats.total_operations / total_time_seconds;
        // if (stats.total_operations > 0) {
        //     stats.avg_latency_ms = stats.total_time_ms / stats.total_operations;
        // }
    }
}

// 写入CSV文件（追加模式）
bool writeSummaryToCSV(const PerfStats& stats, const string& csv_filename, 
                      const string& directory_path, const string& iosize, int thread_count,int64_t duration) {
    ofstream csv_file(csv_filename, ios::app);
    
    if (!csv_file.is_open()) {
        cerr << "无法打开CSV文件: " << csv_filename << endl;
        return false;
    }
    
    csv_file.seekp(0, ios::end);
    if (csv_file.tellp() == 0) {
        csv_file << "测试目录,线程数,IOSIZE,平均IOPS,总耗时(秒),平均带宽(MB/s),处理文件数,总操作数,总数据量(MB),平均延迟(ms)" << endl;
    }
    
    csv_file << fixed << setprecision(2);
    csv_file << directory_path << ",";
    csv_file << thread_count << ",";
    csv_file << iosize << ",";
    csv_file << stats.avg_iops << ",";
    csv_file << duration/ 1000.0 << ",";
    csv_file << stats.avg_bandwidth_mbs << ",";
    csv_file << stats.files_processed << ",";
    csv_file << stats.total_operations << ",";
    csv_file << stats.total_bytes / (1024.0 * 1024.0) << ",";
    csv_file << stats.avg_latency_ms << endl;
    
    csv_file.close();
    cout << "性能汇总数据已追加写入: " << csv_filename << endl;
    
    return true;
}

// 将字符串IOSIZE转换为字节数
size_t parseIOSize(const string& ioSizeStr) {
    string sizeStr = ioSizeStr;
    transform(sizeStr.begin(), sizeStr.end(), sizeStr.begin(), ::toupper);
    
    size_t multiplier = 1;
    if (sizeStr.find("K") != string::npos) {
        multiplier = 1024;
        sizeStr.erase(sizeStr.find("K"), 1);
    } else if (sizeStr.find("M") != string::npos) {
        multiplier = 1024 * 1024;
        sizeStr.erase(sizeStr.find("M"), 1);
    } else if (sizeStr.find("G") != string::npos) {
        multiplier = 1024 * 1024 * 1024;
        sizeStr.erase(sizeStr.find("G"), 1);
    }
    
    try {
        size_t baseSize = stoul(sizeStr);
        return baseSize * multiplier;
    } catch (const exception& e) {
        cerr << "错误的IOSIZE格式: " << ioSizeStr << endl;
        cerr << "支持格式: 数字+K/M/G (如: 4K, 1M, 64K等)" << endl;
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        cerr << "用法: " << argv[0] << " <目录路径> <输出CSV文件> <GPU设备ID> <IOSIZE> <线程数>" << endl;
        cerr << "示例: " << argv[0] << " /path/to/directory results.csv 0 1K 4" << endl;
        cerr << "IOSIZE支持格式: 4K, 64K, 1M, 4M等 (K=KB, M=MB)" << endl;
        cerr << "线程数: 1-" << MAX_THREADS << " (建议不超过GPU流处理器组数)" << endl;
        return 1;
    }
    
    const char *directory_path = argv[1];
    const char *csv_filename = argv[2];
    int device_id = atoi(argv[3]);
    const char *io_size_str = argv[4];
    int thread_count = atoi(argv[5]);
    
    // 验证线程数
    if (thread_count < 1 || thread_count > MAX_THREADS) {
        cerr << "线程数必须在1-" << MAX_THREADS << "之间" << endl;
        return 1;
    }
    
    // 解析IOSIZE参数
    size_t io_size = parseIOSize(io_size_str);
    cout << "设置IOSIZE: " << io_size_str << " = " << io_size << " 字节" << endl;
    cout << "使用线程数: " << thread_count << endl;
    
    // 初始化性能统计
    PerfStats stats = {0};
    
    // 获取目录中的文件列表
    vector<FileInfo> files = getFilesInDirectory(directory_path);
    if (files.empty()) {
        cerr << "目录中没有找到文件: " << directory_path << endl;
        return 1;
    }
    
    cout << "找到 " << files.size() << " 个文件" << endl;
    
    // 初始化CUDA驱动
    CUfileError_t status = cuFileDriverOpen();
    if (status.err != CU_FILE_SUCCESS) {
        cerr << "cuFile驱动初始化失败: " << status.err << endl;
        return 1;
    }
    
    // 预分配所有文件资源（在多线程启动前）
    cout << "预分配文件资源..." << endl;
    auto start_prealloc = chrono::high_resolution_clock::now();
    auto file_resources = preallocateFileResources(files, device_id);
    auto end_prealloc = chrono::high_resolution_clock::now();
    auto prealloc_duration = chrono::duration_cast<chrono::milliseconds>(end_prealloc - start_prealloc);
    
    cout << "资源预分配完成，耗时: " << prealloc_duration.count() << " ms" << endl;
    cout << "成功分配资源文件数: " << file_resources.size() << "/" << files.size() << endl;
    
    if (file_resources.empty()) {
        cerr << "没有成功分配任何文件资源" << endl;
        cuFileDriverClose();
        return 1;
    }
    
    auto start_time = chrono::high_resolution_clock::now();
    
    // 创建线程池
    ThreadPool pool(thread_count);
    vector<future<FilePerfStats>> futures;
    mutex cout_mutex;
    
    // 提交纯读操作任务到线程池
    cout << "开始多线程读操作测试..." << endl;
    for (const auto& file : files) {
        if (file_resources.find(file.path) != file_resources.end()) {
            futures.push_back(async(launch::async, [&, file]() {
                auto& res = file_resources[file.path];
                auto file_stats = measureReadOperationOnly(file.path, res, io_size);
                
                // 更新全局统计（原子操作，线程安全）
                if (file_stats.success) {
                    stats.total_time_ms.fetch_add(file_stats.read_time_ms);
                    stats.total_bytes.fetch_add(file_stats.bytes_read);
                    stats.files_processed.fetch_add(1);
                    stats.total_operations.fetch_add(file_stats.operations);
                    
                    {
                        lock_guard<mutex> lock(cout_mutex);
                        // cout << "完成: " << file.name << " (" << file_stats.bytes_read / 1024 << " KB, " 
                        //      << fixed << setprecision(2) << file_stats.read_time_ms << " ms)" << endl;
                    }
                } else {
                    lock_guard<mutex> lock(cout_mutex);
                    cerr << "失败: " << file.name << endl;
                }
                
                return file_stats;
            }));
        }
    }
    
    // 等待所有任务完成
    for (auto& future : futures) {
        future.get();
    }
    
    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
    
    // 释放资源
    cleanupFileResources(file_resources);
    cuFileDriverClose();
    
    // 计算性能指标
    calculatePerformanceMetrics(stats,duration.count() );
    
    // 输出性能报告到控制台
    cout << "\n========== 多线程纯读操作性能测试报告 ==========" << endl;
    cout << "测试目录: " << directory_path << endl;
    cout << "使用线程数: " << thread_count << endl;
    cout << "IOSIZE: " << io_size_str << endl;
    cout << "资源预分配耗时: " << prealloc_duration.count() << " ms" << endl;
    cout << "纯读操作总耗时: " << duration.count() << " ms" << endl;
    // cout << "GPU读操作时间: " << stats.total_time_ms << " ms" << endl;
    cout << "平均带宽: " << stats.avg_bandwidth_mbs << " MB/s" << endl;
    cout << "平均IOPS: " << stats.avg_iops << endl;
    cout << "平均延迟: " << stats.avg_latency_ms << " ms" << endl;
    cout << "处理文件数: " << stats.files_processed << endl;
    cout << "总操作数: " << stats.total_operations << endl;
    cout << "总数据量: " << stats.total_bytes / (1024.0 * 1024.0) << " MB" << endl;
    cout << "吞吐量: " << (stats.total_bytes / (1024.0 * 1024.0)) / (duration.count() / 1000.0) << " MB/s" << endl;
    cout << "===============================================" << endl;
    
    // 写入CSV文件
    if (!writeSummaryToCSV(stats, csv_filename, directory_path, io_size_str, thread_count,duration.count())) {
        return 1;
    }
    
    return 0;
}