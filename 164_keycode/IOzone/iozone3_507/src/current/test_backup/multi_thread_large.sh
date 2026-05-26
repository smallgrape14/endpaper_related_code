#!/bin/bash

# 定义测试目录
TEST_DIR="/mnt/beegfs/iozone_test_large"

# 确保测试目录存在
if [ ! -d "$TEST_DIR" ]; then
    echo "测试目录 $TEST_DIR 不存在，正在创建..."
    mkdir -p "$TEST_DIR"
fi

# 定义线程数列表
THREADS=(12 18 24 30 36 42 48)

# 遍历每个线程数并生成文件和运行测试
for t in "${THREADS[@]}"; do
    echo "生成 $t 个测试文件..."
    
    # 生成测试文件
    for ((i=1; i<=t; i++)); do
        FILE="$TEST_DIR/file$i"
        if [ ! -f "$FILE" ]; then
            # 使用 dd 命令生成 1G 大小的文件
            dd if=/dev/zero of="$FILE" bs=1G count=1 status=progress
        else
            echo "文件 $FILE 已存在，跳过生成。"
        fi
    done
    
    echo "运行测试，线程数为 $t..."
    
    # 构建文件列表
    FILES=""
    for ((i=1; i<=t; i++)); do
        FILES="$FILES $TEST_DIR/file$i"
    done
    
    # 构建输出文件名
    OUTPUT_FILE="read1G_4k_t$t.xls"
    
    # 运行IOZone测试
    ./iozone -t $t -s 1G -r 4k -w -c -O -I -i 0 -i 1 -F $FILES -Rb $OUTPUT_FILE
    
    echo "测试完成，结果已保存到 $OUTPUT_FILE"
done

echo "所有测试完成。"