#!/bin/bash
# 使用nvcc编译（需要CUDA工具包）
# nvcc -std=c++11 -O2 -o write write.cpp -lpthread
# g++ -std=c++11 -O2 -o write write.cpp -I/usr/local/cuda/include -L/usr/local/cuda/lib64 -lcudart -lpthread

# 如果使用nvcc编译器
nvcc -o gds gds.cpp -lcufile -lcuda -lcudart -lpthread -std=c++11

# 如果使用g++/gcc编译器
g++ -o gds gds.cpp -I/usr/local/cuda/include -L/usr/local/cuda/lib64 -lcufile -lcuda -lcudart -pthread -std=c++11
# nvcc -o write write.cpp -lcufile -lpthread -std=c++11

# 或者使用g++配合CUDA库
#修改系统句柄上限
ulimit -n 20240


# 创建输出目录
# mkdir -p /mnt/beegfs/Baseline_write

# 运行程序 （16个线程，每个文件4KB）
# ./write /mnt/beegfs/Baseline_write 16 4

for IOSIZE in "4KB"; # "16KB" "64KB" "256KB" "1MB" "4MB" "16MB" "64MB"; 
# for IOSIZE in "512B" "1KB" "2KB" "4KB" "8KB" "16KB" ; 
do
    for thread_num in 1 2 4 8 16 32 64 128 256 512 1024;
    do
        echo "Running write with IO size: $IOSIZE and thread count: $thread_num"
        ./gds /mnt/beegfs/Baseline_write $thread_num $IOSIZE "gds"
    done
done
