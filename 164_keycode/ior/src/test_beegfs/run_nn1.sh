#!/bin/bash

# 定义测试参数
TEST_DIR="/mnt/beegfs/ior_test"  # 测试目录
LOG_DIR="./ior_logs_run_NtoN"             # 日志目录
mkdir -p $LOG_DIR

# 测试参数配置
TRANSFER_SIZE="4k"              # 单次传输大小
BLOCK_SIZE="1g"                 # 每个进程块大小
SEGMENT_COUNT=1                 # 文件分段数

# 进程数动态测试序列 (1到80每次翻倍)
# for num_procs in 1 2 4 8 16 32 64 80
for num_procs in 1 2 4 6 8 10 12 24 30 48

do
    echo "======= 开始测试进程数: $num_procs ======="
    
    # 生成唯一日志文件名
    LOG_FILE="${LOG_DIR}/ior_${num_procs}procs.log"
    
    # 关键修改：所有参数合并到同一行或用反斜杠正确换行
    # 使用 -O 参数为每个进程指定独立的文件名
    mpirun --allow-run-as-root -np $num_procs ior -w -r -t $TRANSFER_SIZE -b $BLOCK_SIZE \
        -s $SEGMENT_COUNT -o "${TEST_DIR}/file_#procid#" --posix.odirect -F -C -e -g -v >> $LOG_FILE 2>&1

    echo "======= 完成测试进程数: $num_procs ========"
    echo ""
done

echo "所有测试已完成，日志文件保存至: $LOG_DIR"