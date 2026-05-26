#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#define TEST_READ 1 // 1 表示测试 read , 0 表示不测试读 
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
        fprintf(csvFile, "总经过时间(秒),readdir总时间(s),readdirlong总时间(s),open总时间(秒),read总时间(秒)\n");
    }

    // 获取开始时间
    gettimeofday(&start, NULL);

    // 初始化open和read时间总和
    double totalOpenTime = 0.0;
    double totalReadTime = 0.0;
    double totalReadDirTime = 0.0;
    double totalReadDirlongTime = 0.0;


    int file_num=0;
    // int up_file_num=1;
    // 遍历目录中的文件
    while (1) {
        struct timeval readdirStart, readdirEnd, readdirlongEnd;
        // 记录readdir开始时间
        gettimeofday(&readdirStart, NULL);
        entry = readdir(dir);
        // 记录readdir结束时间
        gettimeofday(&readdirEnd, NULL);
        totalReadDirTime+=(readdirEnd.tv_sec - readdirStart.tv_sec) + (readdirEnd.tv_usec - readdirStart.tv_usec) / 1000000.0;
        
        if(entry == NULL)
            break;
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
            // 在读取文件内容的部分进行修改
            if (S_ISREG(fileStat.st_mode)) {
                // 记录readdilong 结束时间 这个readdir 时间是比较长的时间范围，包括生成文件路径以及获取文件状态信息，做是否是文件的判断等等
                gettimeofday(&readdirlongEnd, NULL);
                totalReadDirlongTime += (readdirlongEnd.tv_sec - readdirStart.tv_sec) + (readdirlongEnd.tv_usec - readdirStart.tv_usec) / 1000000.0;

                struct timeval openStart, openEnd, readStart, readEnd;
                double openElapsedTime, readElapsedTime;

                // 记录open开始时间
                gettimeofday(&openStart, NULL);

                FILE *file = fopen(filePath, "r");
                if (file) {
                    // 记录open结束时间
                    gettimeofday(&openEnd, NULL);
                    openElapsedTime = (openEnd.tv_sec - openStart.tv_sec) + (openEnd.tv_usec - openStart.tv_usec) / 1000000.0;
                    totalOpenTime += openElapsedTime;
                    
                    #if TEST_READ == 1
                        // 记录read开始时间
                        gettimeofday(&readStart, NULL);

                        // 获取文件大小
                        off_t fileSize = fileStat.st_size;
                        printf("%d th file size is %ld\n",file_num,fileSize);

                        // 分配缓冲区
                        char *buffer = (char *)malloc(fileSize);
                        if (buffer) {
                            // 读取整个文件
                            size_t bytesRead = fread(buffer, 1, fileSize, file);

                            // 检查是否读取成功
                            if (bytesRead != fileSize) {
                                perror("读取文件失败");
                            }

                            // 释放缓冲区
                            free(buffer);
                        } else {
                            perror("内存分配失败");
                        }

                        // 记录read结束时间
                        gettimeofday(&readEnd, NULL);
                        readElapsedTime = (readEnd.tv_sec - readStart.tv_sec) + (readEnd.tv_usec - readStart.tv_usec) / 1000000.0;
                        totalReadTime += readElapsedTime;
                    #endif
                    
                    file_num++;
                    fclose(file);
                    // if(file_num>up_file_num)
                    // {
                    //     break;
                    // }
                    // printf("Finish Read %d th File \n",file_num);
                    
                }
            }
        }
    }

    // 获取结束时间
    gettimeofday(&end, NULL);
    printf("File_num : %d \n",file_num);
    // 计算经过的时间（秒）
    elapsedTime = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    // 将时间写入CSV文件
    char startTimeStr[64], endTimeStr[64];
    strftime(startTimeStr, sizeof(startTimeStr), "%Y-%m-%d %H:%M:%S", localtime(&start.tv_sec));
    strftime(endTimeStr, sizeof(endTimeStr), "%Y-%m-%d %H:%M:%S", localtime(&end.tv_sec));

    // fprintf(csvFile, "%s,%s,%.6f,%.6f,%.6f\n", startTimeStr, endTimeStr, elapsedTime, totalOpenTime, totalReadTime);
    fprintf(csvFile, "%.6f,%.6f,%.6f,%.6f,%.6f\n", elapsedTime, totalReadDirTime,totalReadDirlongTime,totalOpenTime, totalReadTime);


    // 关闭目录和CSV文件
    closedir(dir);
    fclose(csvFile);

    printf("目录加载时间已写入CSV文件: %s\n", csvFilePath);
}