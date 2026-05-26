#!/bin/bash

# # 定义测试参数
# TEST_DIR="/mnt/beegfs/ior_test"  # 测试目录
# TEST_FILE="testfile"             # 测试文件名
# NUM_CLIENTS=1                    # 客户端数量
# BLOCK_SIZE="4k" #"8m"                  # 每个 I/O 操作的块大小
# TOTAL_SIZE="1g" #"16g"                 # 总数据量
# ITERATIONS=1                     # 测试迭代次数
# IOR_RESULTS_FILE="ior_results.txt"  # 结果文件

# # 确保测试目录存在
# mkdir -p "$TEST_DIR"

# # 完整的测试文件路径
# TEST_FILE_PATH="$TEST_DIR/$TEST_FILE"

# # 运行 IOR 测试
# echo "Running IOR N-1 test with $NUM_CLIENTS clients..."
# mpirun -np $NUM_CLIENTS ior -a POSIX -t $BLOCK_SIZE -b $TOTAL_SIZE -i $ITERATIONS -o $TEST_FILE_PATH -w -r -e -E -s 1 >> $IOR_RESULTS_FILE

# # 输出结果


# 定义测试参数
TEST_DIR="/mnt/beegfs/ior_test"  # 测试目录
TEST_FILE="testfile"             # 测试文件名
NUM_CLIENTS=4                    # 客户端数量
BLOCK_SIZE="8m"                  # 每个 I/O 操作的块大小
TOTAL_SIZE="16g"                 # 总数据量
ITERATIONS=3                     # 测试迭代次数
IOR_RESULTS_FILE="ior_results.txt"  # 结果文件

# 确保测试目录存在
mkdir -p "$TEST_DIR"

# 完整的测试文件路径
TEST_FILE_PATH="$TEST_DIR/$TEST_FILE"

# 运行 IOR 测试
echo "Running IOR N-1 test with $NUM_CLIENTS clients..."
mpirun -np $NUM_CLIENTS ior -a POSIX -t $BLOCK_SIZE -b $TOTAL_SIZE -i $ITERATIONS -o $TEST_FILE_PATH -w -r -e -E -s 1 --posix.odirect -F -C >> $IOR_RESULTS_FILE

# 输出结果
echo "IOR N-1 test completed. Results saved to $IOR_RESULTS_FILE"