#!/bin/bash
# 使用nvcc编译（需要CUDA工具包）
nvcc -std=c++11 -O2 -o write write.cpp -lpthread

# 或者使用g++配合CUDA库
g++ -std=c++11 -O2 -o write write.cpp -I/usr/local/cuda/include -L/usr/local/cuda/lib64 -lcudart -lpthread

#修改系统句柄上限
ulimit -n 20240


# 创建输出目录
# mkdir -p /mnt/beegfs/Baseline_write

# 运行程序 （16个线程，每个文件4KB）
# ./write /mnt/beegfs/Baseline_write 16 4

# for IOSIZE in "4KB"; # "16KB" "64KB" "256KB" "1MB" "4MB" "16MB" "64MB"; 
for testnum in {1..10};
do
    for IOSIZE in "512KB" "1MB" "2MB" "4MB" "8MB" #"4KB" "16KB" "512B" "1KB" "2KB" "8KB" ; # "1MB" "2MB" "8MB"  "4MB" "16MB" ; 
    do
        for thread_num in 1 2 4 8 16 32 64 128 256 512 1024;
        do
            echo "Running write with IO size: $IOSIZE and thread count: $thread_num"
            ./write /mnt/beegfs/Baseline_write $thread_num $IOSIZE
        done
    done
done
