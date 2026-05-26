#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("用法: %s <CSV文件路径>\n", argv[0]);
        return 1;
    }

    const char *csvFilePath = argv[1];
    FILE *csvFile = fopen(csvFilePath, "r");
    if (!csvFile) {
        perror("无法打开CSV文件");
        return 1;
    }

    char line[1024];
    int count = 0;
    double totalElapsedTime = 0.0;

    // 跳过标题行
    if (fgets(line, sizeof(line), csvFile)) {
        // Do nothing
    }

    // 读取数据行
    while (fgets(line, sizeof(line), csvFile)) {
        char *token = strtok(line, ","); // 开始时间
        token = strtok(NULL, ","); // 结束时间
        token = strtok(NULL, ","); // 经过时间(秒)

        if (token) {
            double elapsedTime = atof(token);
            totalElapsedTime += elapsedTime;
            count++;
        }
    }

    fclose(csvFile);

    if (count > 0) {
        double averageElapsedTime = totalElapsedTime / count;
        printf("平均经过时间: %.6f 秒\n", averageElapsedTime);
    } else {
        printf("CSV文件中没有数据行\n");
    }

    return 0;
}