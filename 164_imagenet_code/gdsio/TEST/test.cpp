#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

// 统计结果结构体
typedef struct {
    long file_count;      // 文件总数
    long long total_size; // 总字节数
    double total_time;    // 总耗时(秒)
} ScanResult;

// 递归扫描目录函数
ScanResult scan_directory(const char* path) {
    DIR* dir;
    struct dirent* entry;
    struct stat statbuf;
    ScanResult result = {0, 0, 0.0};
    char full_path[1024];
    
    clock_t start, end;
    
    // 尝试打开目录
    if ((dir = opendir(path)) == NULL) {
        perror("无法打开目录");
        return result;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        // 跳过 "." 和 ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // 构建完整路径
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        // 获取文件信息
        if (lstat(full_path, &statbuf) == -1) {
            continue; // 跳过无法访问的文件
        }
        
        // 测量读取时间
        start = clock();
        
        if (S_ISDIR(statbuf.st_mode)) {
            // 递归处理子目录
            ScanResult sub_result = scan_directory(full_path);
            result.file_count += sub_result.file_count;
            result.total_size += sub_result.total_size;
            result.total_time += sub_result.total_time;
        } else if (S_ISREG(statbuf.st_mode)) {
            // 处理普通文件 - 模拟读取操作
            FILE* file = fopen(full_path, "rb");
            if (file != NULL) {
                char buffer[4096];
                size_t bytes_read;
                while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                    // 模拟文件处理 - 实际应用中这里可以是实际的数据处理
                }
                fclose(file);
                
                result.file_count++;
                result.total_size += statbuf.st_size;
            }
        }
        
        end = clock();
        result.total_time += ((double)(end - start)) / CLOCKS_PER_SEC;
    }
    
    closedir(dir);
    return result;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("使用方法: %s <目录路径>\n", argv[0]);
        return 1;
    }
    
    printf("开始扫描目录: %s\n", argv[1]);
    printf("请稍候...\n\n");
    
    clock_t total_start = clock();
    ScanResult result = scan_directory(argv[1]);
    clock_t total_end = clock();
    
    double total_execution_time = ((double)(total_end - total_start)) / CLOCKS_PER_SEC;
    
    // 输出统计结果
    printf("============== 扫描结果 ==============\n");
    printf("扫描目录: %s\n", argv[1]);
    printf("文件总数: %ld 个\n", result.file_count);
    printf("总大小: %.2f MB\n", result.total_size / (1024.0 * 1024.0));
    printf("纯读取时间: %.6f 秒\n", result.total_time);
    printf("程序总执行时间: %.6f 秒\n", total_execution_time);
    printf("平均每个文件: %.6f 秒\n", 
           result.file_count > 0 ? result.total_time / result.file_count : 0);
    printf("平均速度: %.2f MB/秒\n", 
           result.total_time > 0 ? (result.total_size / (1024.0 * 1024.0)) / result.total_time : 0);
    
    return 0;
}