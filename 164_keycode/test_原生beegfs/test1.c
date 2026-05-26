#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

// 函数声明
void listFilesAndMeasureTime(const char *directoryPath, const char *csvFilePath);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("用法: %s <目录路径> <CSV文件路径>\n", argv[0]);
        return 1;
    }

    const char *directoryPath = argv[1];
    const char *csvFilePath = argv[2];

    listFilesAndMeasureTime(directoryPath, csvFilePath);

    return 0;
}

void listFilesAndMeasureTime(const char *directoryPath, const char *csvFilePath) {
    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;
    FILE *csvFile;
    struct timeval start, end;
    double elapsedTime;
    struct stat csvFileStat;

    // 打开目录
    dir = opendir(directoryPath);
    if (!dir) {
        perror("无法打开目录");
        return;
    }

    // 创建或打开CSV文件
    csvFile = fopen(csvFilePath, "a+");
    if (!csvFile) {
        perror("无法创建CSV文件");
        closedir(dir);
        return;
    }

    // 检查CSV文件是否为空
    if (fstat(fileno(csvFile), &csvFileStat) == 0 && csvFileStat.st_size == 0) {
        // 写入CSV文件的标题行
        fprintf(csvFile, "开始时间,结束时间,经过时间(秒)\n");
    }

    // 获取开始时间
    gettimeofday(&start, NULL);

    // 遍历目录中的文件
    while ((entry = readdir(dir)) != NULL) {
        // 跳过目录本身和父目录
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // 构造文件的完整路径
        char filePath[1024];
        snprintf(filePath, sizeof(filePath), "%s/%s", directoryPath, entry->d_name);

        // 获取文件状态信息
        if (stat(filePath, &fileStat) == 0) {
            // 如果是普通文件，读取其内容（模拟加载过程）
            if (S_ISREG(fileStat.st_mode)) {
                FILE *file = fopen(filePath, "r");
                if (file) {
                    // 读取文件内容到缓冲区（这里只是简单地读取，不进行处理）
                    char buffer[4096];
                    while (fread(buffer, sizeof(buffer), 1, file) == 1) {
                        // 模拟加载过程
                    }
                    fclose(file);
                }
            }
        }
    }

    // 获取结束时间
    gettimeofday(&end, NULL);

    // 计算经过的时间（秒）
    elapsedTime = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    // 将时间写入CSV文件
    char startTimeStr[64], endTimeStr[64];
    strftime(startTimeStr, sizeof(startTimeStr), "%Y-%m-%d %H:%M:%S", localtime(&start.tv_sec));
    strftime(endTimeStr, sizeof(endTimeStr), "%Y-%m-%d %H:%M:%S", localtime(&end.tv_sec));

    fprintf(csvFile, "%s,%s,%.6f\n", startTimeStr, endTimeStr, elapsedTime);

    // 遍历目录中的文件，将读取到的内容写入新文件
    rewinddir(dir);
    while ((entry = readdir(dir)) != NULL) {
        // 跳过目录本身和父目录
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // 构造文件的完整路径
        char filePath[1024];
        snprintf(filePath, sizeof(filePath), "%s/%s", directoryPath, entry->d_name);

        // 获取文件状态信息
        if (stat(filePath, &fileStat) == 0) {
            // 如果是普通文件，读取其内容（模拟加载过程）
            if (S_ISREG(fileStat.st_mode)) {
                // 构造新文件路径
                char newFilePath[1024];
                snprintf(newFilePath, sizeof(newFilePath), "result/outfile/out_%s", entry->d_name);
                printf("out_file_path: %s \n",newFilePath);
                // 打开原文件和新文件
                FILE *file = fopen(filePath, "r");
                FILE *newFile = fopen(newFilePath, "w");
                if (file && newFile) {
                    // 读取原文件内容并写入新文件
                    char buffer[4096];
                    while (fread(buffer, sizeof(buffer), 1, file) == 1) {
                        fwrite(buffer, sizeof(buffer), 1, newFile);
                    }
                    fclose(file);
                    fclose(newFile);
                }
            }
        }
    }

    // 关闭目录和CSV文件
    closedir(dir);
    fclose(csvFile);

    printf("目录加载时间已写入CSV文件: %s\n", csvFilePath);
}