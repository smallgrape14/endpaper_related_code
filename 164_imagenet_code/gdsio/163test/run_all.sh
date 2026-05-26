#!/bin/bash

# 编译命令（需要CUDA和cuFile支持）
nvcc -o gds gds.cpp -lcufile -lcudart


# 简单的GDS批量测试脚本
# 用法: ./batch_gds_test.sh [GPU设备ID，默认为0]

GPU_DEVICE=${1:-0}  # 使用第一个参数作为GPU设备ID，默认为0
BASE_DIR="/mnt/beegfs"  # 基础目录路径
# BASE_DIR="/home/oem/xyp/imagenet_code/gdsio/163test/data"  # 基础目录路径

# 定义要测试的文件大小数组（从512B到16KB）
# FILE_SIZES=("1KB" "4KB")
FILE_SIZES=("8KB" "16KB")

# FILE_SIZES=("512B" "1KB" "2KB" "4KB" "8KB" "16KB")

echo "开始GDS批量测试"
echo "GPU设备: $GPU_DEVICE"
echo "基础目录: $BASE_DIR"
echo "======================================"

# 循环测试每个文件大小对应的目录
for testnum in {1..1}
do
    for SIZE in "${FILE_SIZES[@]}"; do
        # 构建目录名称（根据您的实际目录命名规则调整）
        DIR_NAME="test_dir_${SIZE}_10428"  # 根据实际情况调整后缀
        FULL_PATH="$BASE_DIR/$DIR_NAME"
        # OUTPUT_FILE="output_${SIZE}.csv"  # 输出文件包含文件大小信息
        OUTPUT_FILE="output.csv"  # 输出文件包含文件大小信息
        
        echo "正在测试: $SIZE 文件大小"
        echo "目录路径: $FULL_PATH"
        echo "输出文件: $OUTPUT_FILE"
        
        # 检查目录是否存在
        if [ ! -d "$FULL_PATH" ]; then
            echo "警告: 目录不存在 - $FULL_PATH，跳过测试"
            echo "--------------------------------------"
            continue
        fi
        echo 3 > /proc/sys/vm/drop_caches
        # 执行GDS测试
        if ./gds "$FULL_PATH" "$OUTPUT_FILE" "$GPU_DEVICE" "32KB"; then
            echo "✓ 测试完成: $SIZE"
        else
            echo "✗ 测试失败: $SIZE"
        fi
        echo "--------------------------------------"
    done
done

echo "所有文件大小测试执行完毕"
echo "输出文件: output_[大小].csv (如: output_512B.csv, output_1KB.csv 等)"