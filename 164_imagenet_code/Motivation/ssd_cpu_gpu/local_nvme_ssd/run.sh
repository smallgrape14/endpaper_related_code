#!/bin/bash
# compile_fixed_size_test.sh

echo "编译固定大小文件延迟测试程序..."

# 使用nvcc编译
nvcc -std=c++11 -O2 -o cpu cpu.cpp \
    -lcuda -lstdc++

if [ $? -eq 0 ]; then
    echo "编译成功！"
    echo "可执行文件: cpu"
else
    echo "编译失败！"
    exit 1
fi


# # 基本用法
# ./cpu 测试目录 IOSIZE字节 输出文件前缀

# 示例：测试1KB文件的延迟
# ./cpu /data/gdstest/test_dir_4KB_10428 4096 4kb_test
# ./cpu /data/gdstest/test_dir_1KB_10428 1024 1kb_test
# ./cpu /data/gdstest/test_dir_16KB_10428 16384 16kb_test

./cpu /mnt/nvme0/test_dir_4KB_10428 4096 4kb_test_localnvme
./cpu /mnt/nvme0/test_dir_1KB_10428 1024 1kb_test_localnvme
./cpu /mnt/nvme0/test_dir_16KB_10428 16384 16kb_test_localnvme
./cpu /mnt/nvme0/test_dir_512B_10428 512 512b_test_localnvme
./cpu /mnt/nvme0/test_dir_2KB_10428 2048 2kb_test_localnvme
./cpu /mnt/nvme0/test_dir_8KB_10428 8192 8kb_test_localnvme
# ./cpu /mnt/nvme0/test_dir_32KB_10428 32768 32kb_test_localnvme


