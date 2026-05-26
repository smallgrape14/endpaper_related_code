#!/bin/bash

# 编译命令（需要CUDA和cuFile支持）
# nvcc -std=c++20 -o gds_multi gds_multi.cpp -lcufile -lcudart
# nvcc -std=c++20 -o gds_multi gds_multi_tmp.cpp -lcufile -lcudart
nvcc -std=c++20 -o gds_multi gds_multi_tmp2.cpp -lcufile -lcudart

# 简单的GDS批量测试脚本
# 用法: ./batch_gds_test.sh [GPU设备ID，默认为0]

# GPU_DEVICE=${0}  # 使用第一个参数作为GPU设备ID，默认为0

GPU_DEVICE=${1:-0}  # 使用第一个参数作为GPU设备ID，默认为0
BASE_DIR="/mnt/beegfs"  # 基础目录路径
# BASE_DIR="/mnt/nvme0"
# BASE_DIR="/home/oem/xyp/imagenet_code/gdsio/163test/data"  # 基础目录路径

# 定义要测试的文件大小数组（从512B到16KB）
# FILE_SIZES=("1KB")

# FILE_SIZES=("16KB")
# FILE_SIZES=("512B" "2KB" "8KB")

FILE_SIZES=("512B" "1KB" "2KB" "4KB" "8KB" "16KB" "32KB" "64KB" "128KB" "256KB" "512KB" "1MB" "2MB" "4MB" "8MB" "16MB")

# 缓存清理函数
clear_cache() {
    echo "开始清理系统缓存..."
    
    # 同步文件系统，确保数据写入磁盘[1,3](@ref)
    sync
    
    # 延迟一下确保同步完成[2,4](@ref)
    sleep 2
    
    # 清理页面缓存、目录项和inode缓存[1,8](@ref)
    # 注意：需要root权限执行
    if [ "$EUID" -eq 0 ]; then
        echo 3 > /proc/sys/vm/drop_caches
        echo "缓存清理完成"
    else
        # 如果没有root权限，尝试使用sudo
        if command -v sudo >/dev/null 2>&1; then
            sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
            echo "缓存清理完成（使用sudo）"
        else
            echo "警告：需要root权限清理系统缓存，当前用户无权限"
            echo "可以手动执行: echo 3 > /proc/sys/vm/drop_caches (需要root)"
        fi
    fi
    
    # 额外等待一下让系统稳定[4](@ref)
    sleep 3
    echo "缓存清理完毕"
}

echo "开始GDS批量测试"
echo "GPU设备: $GPU_DEVICE"
echo "基础目录: $BASE_DIR"
echo "======================================"

# 首次清理缓存
clear_cache

# 循环测试每个文件大小对应的目录
for testnum in {1..1};do
    # for threads in 1 2 4 8 16 32 64 128 256 512 1024; do
        for threads in 4;do
        for SIZE in "${FILE_SIZES[@]}"; do
            # 构建目录名称（根据您的实际目录命名规则调整）
            # DIR_NAME="test_dir_${SIZE}_10428_1000"  # 根据实际情况调整后缀
            DIR_NAME="test_dir_${SIZE}_10428"  # 根据实际情况调整后缀
            
            FULL_PATH="$BASE_DIR/$DIR_NAME"
            OUTPUT_FILE="output_multi_1126.csv"  # 输出文件包含文件大小信息
            # OUTPUT_FILE="output_multi_LOCALNVME.csv"  # 输出文件包含文件大小信息
            
            echo "正在测试: $SIZE 文件大小"
            echo "目录路径: $FULL_PATH"
            echo "输出文件: $OUTPUT_FILE"
            
            # 检查目录是否存在
            if [ ! -d "$FULL_PATH" ]; then
                echo "警告: 目录不存在 - $FULL_PATH，跳过测试"
                echo "--------------------------------------"
                continue
            fi
            
            # 每次测试前清理缓存
            echo "测试前清理缓存..."
            clear_cache
            
            # 执行GDS测试
            if ./gds_multi "$FULL_PATH" "$OUTPUT_FILE" "$GPU_DEVICE" "32KB" "$threads"; then
                echo "✓ 测试完成: $SIZE"
            else
                echo "✗ 测试失败: $SIZE"
            fi
            
            # 测试后也清理缓存，为下一次测试做准备
            echo "测试后清理缓存..."
            clear_cache
            
            echo "--------------------------------------"
        done
    done
done
echo "所有文件大小测试执行完毕"
echo "输出文件: output_[大小].csv (如: output_512B.csv, output_1KB.csv 等)"