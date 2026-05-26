#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <cuda_runtime.h>
#include <cufile.h>
#include <fcntl.h>           // 修复：添加 O_RDONLY, O_DIRECT 等定义
#include <unistd.h>          // 修复：添加 close() 等系统调用
#include <time.h>            // 修复：添加 clock_gettime()
#include <cmath>             // 修复：添加 sqrt() 函数
#include <cufile.h>      // cuFile 主头文件
#include <cuda_runtime.h> // CUDA 运行时

// 确保包含必要的系统头文件
#include <fcntl.h>        // 定义 O_RDONLY, O_DIRECT
#include <unistd.h>       // 定义 close()
#include <sys/stat.h>     // 文件状态
#define BUFFER_SIZE (64 * 1024)  // 64KB buffer for GDS
#define MAX_FILES 10000
#define MAX_PATH 1024
// 如果头文件中缺少定义，手动添加
#ifndef CU_FILE_HANDLE_TYPE_OPENER
#define CU_FILE_HANDLE_TYPE_OPENER 1
#endif
// 统计结果结构体
typedef struct {
    long file_count;           // 文件总数
    long long total_size;     // 总字节数
    double gds_read_time;     // GDS读取时间
    double file_open_time;    // 文件打开注册时间
    double buffer_reg_time;   // 缓冲区注册时间
} GDSScanResult;

// 测试统计
typedef struct {
    int total_runs;
    double avg_gds_time;
    double avg_total_time;
    double min_gds_time;
    double max_gds_time;
    double std_dev_gds;
} GDSTestStats;

// 文件信息
typedef struct {
    char filename[MAX_PATH];
    long file_size;
    CUfileHandle_t file_handle;
} FileInfo;

// 初始化cuFile驱动
int init_cufile_driver() {
    CUfileError_t status = cuFileDriverOpen();
    if (status.err != CU_FILE_SUCCESS) {
        fprintf(stderr, "cuFile驱动初始化失败: %d\n", status.err);
        return -1;
    }
    printf("cuFile驱动初始化成功\n");
    return 0;
}

// 关闭cuFile驱动
void close_cufile_driver() {
    cuFileDriverClose();
    printf("cuFile驱动已关闭\n");
}

// 使用GDS读取单个文件
double read_file_with_gds(const char* filename, long file_size, void* d_buffer) {
    CUfileError_t status;
    CUfileHandle_t file_handle;
    CUfileDescr_t file_descr;
    double read_time = 0.0;
    
    struct timespec start, end;
    
    // 打开文件并注册到cuFile
    memset(&file_descr, 0, sizeof(file_descr));
    file_descr.handle.fd = open(filename, O_RDONLY | O_DIRECT);
    if (file_descr.handle.fd == -1) {
        return -1.0;
    }
    file_descr.type = CU_FILE_HANDLE_TYPE_OPENER;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    status = cuFileHandleRegister(&file_handle, &file_descr);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    if (status.err != CU_FILE_SUCCESS) {
        close(file_descr.handle.fd);
        return -1.0;
    }
    
    // 测量GDS读取时间
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    long bytes_remaining = file_size;
    long total_read = 0;
    ssize_t ret;
    
    while (bytes_remaining > 0) {
        size_t bytes_to_read = (bytes_remaining > BUFFER_SIZE) ? BUFFER_SIZE : bytes_remaining;
        ret = cuFileRead(file_handle, d_buffer, bytes_to_read, total_read, 0);
        
        if (ret < 0) {
            break;
        }
        
        total_read += ret;
        bytes_remaining -= ret;
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    read_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    // 清理资源
    cuFileHandleDeregister(file_handle);
    close(file_descr.handle.fd);
    
    return read_time;
}

// 递归扫描目录并使用GDS读取文件
GDSScanResult scan_directory_with_gds(const char* path, void* d_buffer) {
    DIR* dir;
    struct dirent* entry;
    struct stat statbuf;
    GDSScanResult result = {0, 0, 0.0, 0.0, 0.0};
    char full_path[MAX_PATH];
    
    if ((dir = opendir(path)) == NULL) {
        return result;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        if (lstat(full_path, &statbuf) == -1) {
            continue;
        }
        
        if (S_ISDIR(statbuf.st_mode)) {
            // 递归处理子目录
            GDSScanResult sub_result = scan_directory_with_gds(full_path, d_buffer);
            result.file_count += sub_result.file_count;
            result.total_size += sub_result.total_size;
            result.gds_read_time += sub_result.gds_read_time;
        } else if (S_ISREG(statbuf.st_mode) && statbuf.st_size > 0) {
            // 使用GDS读取普通文件
            double read_time = read_file_with_gds(full_path, statbuf.st_size, d_buffer);
            if (read_time >= 0) {
                result.file_count++;
                result.total_size += statbuf.st_size;
                result.gds_read_time += read_time;
                
                if (result.file_count % 100 == 0) {
                    printf("已处理 %ld 个文件...\n", result.file_count);
                }
            }
        }
    }
    
    closedir(dir);
    return result;
}

// 计算统计信息
GDSTestStats calculate_gds_statistics(double* gds_times, int num_runs) {
    GDSTestStats stats = {0};
    stats.total_runs = num_runs;
    
    if (num_runs == 0) return stats;
    
    double sum = 0.0;
    stats.min_gds_time = gds_times[0];
    stats.max_gds_time = gds_times[0];
    
    for (int i = 0; i < num_runs; i++) {
        sum += gds_times[i];
        if (gds_times[i] < stats.min_gds_time) stats.min_gds_time = gds_times[i];
        if (gds_times[i] > stats.max_gds_time) stats.max_gds_time = gds_times[i];
    }
    
    stats.avg_gds_time = sum / num_runs;
    
    // 计算标准差
    double variance = 0.0;
    for (int i = 0; i < num_runs; i++) {
        variance += (gds_times[i] - stats.avg_gds_time) * 
                   (gds_times[i] - stats.avg_gds_time);
    }
    stats.std_dev_gds = sqrt(variance / num_runs);
    
    return stats;
}

// 写入CSV结果文件
int write_gds_results_to_csv(const char* csv_filename, const char* scan_dir, 
                           GDSScanResult* results, GDSTestStats stats, int num_runs) {
    FILE* csv_file = fopen(csv_filename, "w");
    if (csv_file == NULL) {
        return -1;
    }
    
    // 写入CSV表头
    fprintf(csv_file, "运行次数,文件数量,总大小(GB),GDS读取时间(秒),吞吐量(GB/s),IOPS\n");
    
    // 写入每次运行的详细数据
    for (int i = 0; i < num_runs; i++) {
        double throughput = (results[i].total_size > 0 && results[i].gds_read_time > 0) ?
                          (results[i].total_size / (1024.0 * 1024.0 * 1024.0)) / results[i].gds_read_time : 0.0;
        double iops = (results[i].file_count > 0 && results[i].gds_read_time > 0) ?
                     results[i].file_count / results[i].gds_read_time : 0.0;
        
        fprintf(csv_file, "%d,%ld,%.3f,%.6f,%.3f,%.2f\n",
               i + 1,
               results[i].file_count,
               results[i].total_size / (1024.0 * 1024.0 * 1024.0),
               results[i].gds_read_time,
               throughput,
               iops);
    }
    
    // 写入统计摘要
    fprintf(csv_file, "\nGDS性能统计摘要\n");
    fprintf(csv_file, "统计项目,数值,单位\n");
    fprintf(csv_file, "扫描目录,%s,\n", scan_dir);
    fprintf(csv_file, "总运行次数,%d,次\n", stats.total_runs);
    fprintf(csv_file, "平均GDS读取时间,%.6f,秒\n", stats.avg_gds_time);
    fprintf(csv_file, "最短GDS读取时间,%.6f,秒\n", stats.min_gds_time);
    fprintf(csv_file, "最长GDS读取时间,%.6f,秒\n", stats.max_gds_time);
    fprintf(csv_file, "GDS时间标准差,%.6f,秒\n", stats.std_dev_gds);
    fprintf(csv_file, "波动系数,%.2f,%%\n", 
           (stats.std_dev_gds / stats.avg_gds_time) * 100);
    
    if (results[0].file_count > 0) {
        double avg_throughput = (results[0].total_size / (1024.0 * 1024.0 * 1024.0)) / stats.avg_gds_time;
        double avg_iops = results[0].file_count / stats.avg_gds_time;
        fprintf(csv_file, "平均吞吐量,%.3f,GB/秒\n", avg_throughput);
        fprintf(csv_file, "平均IOPS,%.2f,次/秒\n", avg_iops);
    }
    
    fclose(csv_file);
    return 0;
}
// 高精度时间函数
double get_current_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}
int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("使用方法: %s <扫描目录> <输出CSV文件>\n", argv[0]);
        printf("示例: %s /data gds_performance.csv\n", argv[0]);
        return 1;
    }
    
    const int NUM_RUNS = 10;
    GDSScanResult results[NUM_RUNS];
    double gds_times[NUM_RUNS];
    void* d_buffer = NULL;
    
    printf("=== CUDA Direct Storage (GDS) 性能测试 ===\n");
    printf("扫描目录: %s\n", argv[1]);
    printf("运行次数: %d\n", NUM_RUNS);
    printf("缓冲区大小: %d KB\n\n", BUFFER_SIZE / 1024);
    
    // 初始化CUDA
    cudaError_t cuda_status = cudaMalloc(&d_buffer, BUFFER_SIZE);
    if (cuda_status != cudaSuccess) {
        fprintf(stderr, "CUDA内存分配失败: %s\n", cudaGetErrorString(cuda_status));
        return 1;
    }
    
    // 初始化cuFile驱动
    if (init_cufile_driver() != 0) {
        cudaFree(d_buffer);
        return 1;
    }
    
    // 运行性能测试
    for (int i = 0; i < NUM_RUNS; i++) {
        printf("第 %d 次GDS测试运行...", i + 1);
        fflush(stdout);
        
        double start_time = get_current_time();
        results[i] = scan_directory_with_gds(argv[1], d_buffer);
        double end_time = get_current_time();
        
        gds_times[i] = results[i].gds_read_time;
        
        printf("完成 (文件: %ld, 大小: %.2f GB, 时间: %.3fs)\n",
               results[i].file_count,
               results[i].total_size / (1024.0 * 1024.0 * 1024.0),
               results[i].gds_read_time);
        
        if (i < NUM_RUNS - 1) {
            sleep(2);  // 让GPU和存储系统冷却
        }
    }
    
    // 计算统计信息
    GDSTestStats stats = calculate_gds_statistics(gds_times, NUM_RUNS);
    
    // 写入CSV文件
    if (write_gds_results_to_csv(argv[2], argv[1], results, stats, NUM_RUNS) == 0) {
        printf("\nGDS性能结果已保存到: %s\n", argv[2]);
    }
    
    // 控制台输出摘要
    printf("\n=== GDS性能测试结果 ===\n");
    printf("总文件数: %ld\n", results[0].file_count);
    printf("总数据量: %.3f GB\n", results[0].total_size / (1024.0 * 1024.0 * 1024.0));
    printf("平均GDS读取时间: %.3f ± %.3f 秒\n", stats.avg_gds_time, stats.std_dev_gds);
    printf("GDS吞吐量: %.3f GB/秒\n", 
          (results[0].total_size / (1024.0 * 1024.0 * 1024.0)) / stats.avg_gds_time);
    printf("GDS IOPS: %.2f 次/秒\n", results[0].file_count / stats.avg_gds_time);
    printf("波动系数: %.2f%%\n", (stats.std_dev_gds / stats.avg_gds_time) * 100);
    
    // 清理资源
    cudaFree(d_buffer);
    close_cufile_driver();
    
    printf("测试完成!\n");
    return 0;
}

