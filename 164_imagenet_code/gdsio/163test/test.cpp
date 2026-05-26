#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <iomanip>

#include <unistd.h>
#include <fcntl.h>
#include <cuda_runtime.h>
#include <cufile.h>

using namespace std;

// 配置参数
#define BLOCK_SIZE (4 * 1024) // 4KB
#define MAX_PATH_LENGTH 1024

// 文件信息结构体
struct FileInfo {
    string name;
    string path;
    size_t size;
};

// 性能统计结构体
struct PerfStats {
    double total_time_ms;
    double total_bytes;
    size_t files_processed;
    size_t total_operations;
    double avg_bandwidth_mbs;
    double avg_iops;
    double avg_latency_ms;
};

// 获取目录下所有文件列表
vector<FileInfo> getFilesInDirectory(const string& directoryPath) {
    vector<FileInfo> files;
    DIR *dir;
    struct dirent *ent;
    struct stat file_stat;
    
    if ((dir = opendir(directoryPath.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            // 跳过 "." 和 ".." 目录
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }
            
            string full_path = directoryPath + "/" + ent->d_name;
            
            // 获取文件信息
            if (stat(full_path.c_str(), &file_stat) == 0) {
                if (S_ISREG(file_stat.st_mode)) { // 只处理普通文件
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

// 读取单个文件并测量性能
bool readFileWithMeasurement(const string& file_path, PerfStats& stats, int device_id) {
    int fd;
    void *devPtr = NULL;
    CUfileDescr_t cf_descr;
    CUfileHandle_t cf_handle;
    CUfileError_t status;
    
    // 设置CUDA设备
    cudaSetDevice(device_id);
    
    // 创建CUDA事件用于计时
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    
    // 打开文件
    fd = open(file_path.c_str(), O_RDONLY | O_DIRECT);
    if (fd < 0) {
        cerr << "打开文件失败: " << file_path << " - " << strerror(errno) << endl;
        return false;
    }
    
    // 注册cuFile句柄
    memset((void *)&cf_descr, 0, sizeof(CUfileDescr_t));
    cf_descr.handle.fd = fd;
    cf_descr.type = CU_FILE_HANDLE_TYPE_OPAQUE_FD;
    status = cuFileHandleRegister(&cf_handle, &cf_descr);
    if (status.err != CU_FILE_SUCCESS) {
        cerr << "文件注册失败" << endl;
        close(fd);
        return false;
    }
    
    // 分配GPU内存
    cudaMalloc(&devPtr, BLOCK_SIZE);
    cudaMemset(devPtr, 0, BLOCK_SIZE);
    cudaStreamSynchronize(0);
    
    // 获取文件大小
    struct stat file_stat;
    stat(file_path.c_str(), &file_stat);
    size_t file_size = file_stat.st_size;
    size_t blocks = (file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
    double file_total_time = 0.0;
    size_t file_operations = 0;
    
    // 读取文件内容
    for (size_t offset = 0; offset < file_size; offset += BLOCK_SIZE) {
        size_t read_size = min(static_cast<size_t>(BLOCK_SIZE), file_size - offset);
        
        // 记录开始时间
        cudaEventRecord(start, 0);
        
        // 执行读取操作
        ssize_t bytes_read = cuFileRead(cf_handle, devPtr, read_size, offset, 0);
        
        // 记录结束时间
        cudaEventRecord(stop, 0);
        cudaEventSynchronize(stop);
        
        if (bytes_read < 0) {
            cerr << "读取失败在偏移量 " << offset << endl;
            break;
        }
        
        // 计算耗时
        float elapsed_time;
        cudaEventElapsedTime(&elapsed_time, start, stop);
        
        file_total_time += elapsed_time;
        file_operations++;
    }
    
    // 更新统计信息
    stats.total_time_ms += file_total_time;
    stats.total_bytes += file_size;
    stats.files_processed++;
    stats.total_operations += file_operations;
    
    // 清理资源
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    cudaFree(devPtr);
    cuFileHandleDeregister(cf_handle);
    close(fd);
    
    return true;
}

// 计算性能指标
void calculatePerformanceMetrics(PerfStats& stats) {
    if (stats.total_time_ms > 0) {
        double total_time_seconds = stats.total_time_ms / 1000.0;
        stats.avg_bandwidth_mbs = (stats.total_bytes / (1024.0 * 1024.0)) / total_time_seconds;
        stats.avg_iops = stats.total_operations / total_time_seconds;
        stats.avg_latency_ms = stats.total_time_ms / stats.total_operations;
    }
}

// 写入CSV文件（只写入汇总数据）
bool writeSummaryToCSV(const PerfStats& stats, const string& csv_filename, const string& directory_path) {
    ofstream csv_file(csv_filename, ios::out);
    
    if (!csv_file.is_open()) {
        cerr << "无法打开CSV文件: " << csv_filename << endl;
        return false;
    }
    
    // 写入CSV表头[6,8](@ref)
    csv_file << "测试目录,处理文件数,总操作数,总数据量(MB),总耗时(秒),平均带宽(MB/s),平均IOPS,平均延迟(ms)" << endl;
    
    // 写入汇总数据[7](@ref)
    csv_file << fixed << setprecision(2);
    csv_file << directory_path << ",";
    csv_file << stats.files_processed << ",";
    csv_file << stats.total_operations << ",";
    csv_file << stats.total_bytes / (1024.0 * 1024.0) << ",";
    csv_file << stats.total_time_ms / 1000.0 << ",";
    csv_file << stats.avg_bandwidth_mbs << ",";
    csv_file << stats.avg_iops << ",";
    csv_file << stats.avg_latency_ms << endl;
    
    csv_file.close();
    cout << "性能汇总数据已写入: " << csv_filename << endl;
    
    return true;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        cerr << "用法: " << argv[0] << " <目录路径> <输出CSV文件> <GPU设备ID>" << endl;
        cerr << "示例: " << argv[0] << " /path/to/directory results.csv 0" << endl;
        return 1;
    }
    
    const char *directory_path = argv[1];
    const char *csv_filename = argv[2];
    int device_id = atoi(argv[3]);
    
    // 初始化性能统计
    PerfStats stats = {0};
    
    // 获取目录中的文件列表
    vector<FileInfo> files = getFilesInDirectory(directory_path);
    if (files.empty()) {
        cerr << "目录中没有找到文件: " << directory_path << endl;
        return 1;
    }
    
    cout << "找到 " << files.size() << " 个文件进行测试..." << endl;
    
    // 遍历并测试每个文件
    for (const auto& file : files) {
        cout << "处理文件: " << file.name << " (" << file.size << " 字节)" << endl;
        
        if (!readFileWithMeasurement(file.path, stats, device_id)) {
            cerr << "文件处理失败: " << file.name << endl;
        }
    }
    
    // 计算性能指标
    calculatePerformanceMetrics(stats);
    
    // 输出性能报告到控制台
    cout << "\n========== 性能测试报告 ==========" << endl;
    cout << "测试目录: " << directory_path << endl;
    cout << "处理文件数: " << stats.files_processed << endl;
    cout << "总操作数: " << stats.total_operations << endl;
    cout << "总数据量: " << stats.total_bytes / (1024.0 * 1024.0) << " MB" << endl;
    cout << "总耗时: " << stats.total_time_ms / 1000.0 << " 秒" << endl;
    cout << "平均带宽: " << stats.avg_bandwidth_mbs << " MB/s" << endl;
    cout << "平均IOPS: " << stats.avg_iops << endl;
    cout << "平均延迟: " << stats.avg_latency_ms << " ms" << endl;
    cout << "==================================" << endl;
    
    // 写入CSV文件
    if (!writeSummaryToCSV(stats, csv_filename, directory_path)) {
        return 1;
    }
    
    return 0;
}