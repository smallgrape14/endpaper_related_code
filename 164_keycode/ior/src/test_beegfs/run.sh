#!/bin/bash
# IOR 动态进程数测试脚本
# 测试目录: /mnt/beegfs/ 时间: 2025-04-15

# module load openmpi/4.1.5  # 加载MPI环境（根据实际环境修改）
# module load ior/3.3.1      # 加载IOR模块（根据实际环境修改）

OUTFILE="/mnt/beegfs/ior_test_debug/ior_testfile_NtoN"  # 测试文件路径
LOG_DIR="./ior_logs_debug"                # 日志目录
mkdir -p $LOG_DIR

# 测试参数配置
# TRANSFER_SIZE=4k #"1m"    # 单次传输大小 (参考网页1示例)
# BLOCK_SIZE=1g #"16g"      # 每个进程块大小 (参考网页6示例)
# SEGMENT_COUNT=1 #64      # 文件分段数 (依据网页1建议)

TRANSFER_SIZE="1m" #"1m"    # 单次传输大小 (参考网页1示例)
# BLOCK_SIZE="16m" #"16g"      # 每个进程块大小 (参考网页6示例)
BLOCK_SIZE="1g" #"16g"      # 每个进程块大小 (参考网页6示例)
SEGMENT_COUNT=1 #64      # 文件分段数 (依据网页1建议)

# F_flag=1
F_flag=0

# 进程数动态测试序列 (1到80每次翻倍)
for num_procs in 1 2 4 8 12 16 18 #18 24 32 #2 4 8 12 16 #32 64 80
do
    echo "======= 开始测试进程数: $num_procs ======="
    
    # 生成唯一日志文件名
    LOG_FILE="${LOG_DIR}/ior_proc${num_procs}_iosize${TRANSFER_SIZE}_filesize${BLOCK_SIZE}_segmentcnt${SEGMENT_COUNT}_F${F_flag}.log"
    
    # 关键修改：所有参数合并到同一行或用反斜杠正确换行
    if [ "$F_flag" -eq 1 ]; then
        mpirun --allow-run-as-root -np $num_procs ior -w -r -t $TRANSFER_SIZE -b $BLOCK_SIZE \
            -s $SEGMENT_COUNT -o $OUTFILE --posix.odirect -C -F -e -k -g -v >> $LOG_FILE 2>&1
            # -s $SEGMENT_COUNT -o $OUTFILE --posix.odirect -F -C -e -g -v >> $LOG_FILE 2>&1
    else 
        mpirun --allow-run-as-root -np $num_procs ior -w -r -t $TRANSFER_SIZE -b $BLOCK_SIZE \
            -s $SEGMENT_COUNT -o $OUTFILE --posix.odirect -C -e -k -g -v >> $LOG_FILE 2>&1
    fi
    echo "======= 完成测试进程数: $num_procs ========"
    echo ""
done

echo "所有测试已完成，日志文件保存至: $LOG_DIR"