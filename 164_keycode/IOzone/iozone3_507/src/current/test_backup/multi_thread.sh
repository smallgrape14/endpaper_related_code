#!/bin/bash

# 定义测试目录
TEST_DIR="/mnt/beegfs/iozone_test"

# 确保测试目录存在
if [ ! -d "$TEST_DIR" ]; then
    echo "测试目录 $TEST_DIR 不存在，请先创建目录。"
    exit 1
fi

# 定义线程数列表
# THREADS=(2 4 6 8 10)
THREADS=(4 6 8 10)


# 遍历每个线程数并运行测试
for t in "${THREADS[@]}"; do
    echo "Running test with $t threads..."
    
    # 构建文件列表
    FILES=""
    for ((i=1; i<=t; i++)); do
        FILES="$FILES $TEST_DIR/file$i"
    done
    
    # 构建输出文件名
    OUTPUT_FILE="read1G_4k_t$t.xls"
    
    # 运行IOZone测试
    ./iozone -t $t -s 1G -r 4k -w -c -O -I -i 0 -i 1 -F $FILES -Rb $OUTPUT_FILE
    
    echo "Test with $t threads completed. Results saved to $OUTPUT_FILE"
done

echo "All tests completed."