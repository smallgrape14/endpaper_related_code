#!/bin/bash

# 目录路径
# directory="/mnt/beegfs/test_dir_32KB_256"
# directory="/mnt/beegfs/test_dir_512B_256"
# directory="/mnt/beegfs/test_dir"



# 清空或创建CSV文件
# > program_timing_opt.csv

# 遍历线程数量从1到256
# for max_workers in {1..256}
# do
#     echo "Running with max_workers: $max_workers"
#     python3 read_opt.py "$directory" "$max_workers"
# done

# 遍历线程数量从1到256，每次以2的倍数递增
# max_workers=1
# while [ $max_workers -le 256 ]; do
#     echo "Running with max_workers: $max_workers"
#     ./read "$max_workers"
#     max_workers=$((max_workers * 2))
# done


# max_workers=1
# for i in {1..5}
# do
#     echo "Running with max_workers: $max_workers"
#     ./read "$max_workers"
# done


# ----------------- perfile 记录每个file 的读取时间
# 遍历线程数量从1到256，每次以2的倍数递增
# 定义线程数列表
# THREADS=(72 80 88 96 104 112 120)
# THREADS=(72 512 1024)
# THREADS=(16 24 32 40 48 56 64)
# THREADS=(40 42 44 46 48 50 52 54 56)





# THREADS=(32)
### -------------- 测试local nvme ssd ----------------------

# THREADS=(1 2 4 8 16 32 64 128 256 512 1024)

# dir="/mnt/nvme0/test_dir_1KB_10428"
# output="nvme0_read_result.csv"
# for max_workers in "${THREADS[@]}";
# do
#     # THREADS=(1 2 4 8 16 24 32 40 48 56 64 128 256 512 1024)
#     for i in {1..30}
#     do
#         echo "Running with max_workers: $max_workers"
#         ./read "$max_workers" "$dir" "$output"
#         # ./read_perfile "$max_workers"
#     done
# done
### -------------- 测试remote NFS  ----------------------

# THREADS=(32 64 128 256 512 1024)

# dir="/mnt/nfs/Bak163/test_dir_1KB_10428"
# output="NFS_read_result.csv"
# for max_workers in "${THREADS[@]}";
# do
#     # THREADS=(1 2 4 8 16 24 32 40 48 56 64 128 256 512 1024)
#     for i in {1..30}
#     do
#         echo "Running with max_workers: $max_workers"
#         ./read "$max_workers" "$dir" "$output"
#         # ./read_perfile "$max_workers"
#     done
# done

## 测试不同线程数量的文件读取时间 --------------------------------- 0905

# THREADS=(32)
THREADS=(1 2 4 8 16 32 64 128 256 512 1024)


output="BeeGFS_read_result_164.csv"

# dir="/mnt/beegfs/test_dir_512B_10428"
# for i in {1..50}
# do
#     # THREADS=(1 2 4 8 16 24 32 40 48 56 64 128 256 512 1024)
#     for max_workers in "${THREADS[@]}";
#     do
#         echo "Running with max_workers: $max_workers"
#         ./read "$max_workers" "$dir" "$output"
#         # ./read_perfile "$max_workers"
#     done
# done

# dir="/mnt/beegfs/test_dir_2KB_10428"
# for i in {1..50}
# do
#     # THREADS=(1 2 4 8 16 24 32 40 48 56 64 128 256 512 1024)
#     for max_workers in "${THREADS[@]}";
#     do
#         echo "Running with max_workers: $max_workers"
#         ./read "$max_workers" "$dir" "$output"
#         # ./read_perfile "$max_workers"
#     done
# done

# dir="/mnt/beegfs/test_dir_8KB_10428"
# for i in {1..50}
# do
#     # THREADS=(1 2 4 8 16 24 32 40 48 56 64 128 256 512 1024)
#     for max_workers in "${THREADS[@]}";
#     do
#         echo "Running with max_workers: $max_workers"
#         ./read "$max_workers" "$dir" "$output"
#         # ./read_perfile "$max_workers"
#     done
# done


# dir="/mnt/beegfs/test_dir_1KB_10428"
# for i in {1..50}
# do
#     # THREADS=(1 2 4 8 16 24 32 40 48 56 64 128 256 512 1024)
#     for max_workers in "${THREADS[@]}";
#     do
#         echo "Running with max_workers: $max_workers"
#         ./read "$max_workers" "$dir" "$output"
#         # ./read_perfile "$max_workers"
#     done
# done


dir="/mnt/beegfs/test_dir_4KB_10428"
for i in {1..30}
do
    # THREADS=(1 2 4 8 16 24 32 40 48 56 64 128 256 512 1024)
    for max_workers in "${THREADS[@]}";
    do
        echo "Running with max_workers: $max_workers"
        ./read "$max_workers" "$dir" "$output"
        # ./read_perfile "$max_workers"
    done
done

# dir="/mnt/beegfs/test_dir_16KB_10428"
# for i in {1..80}
# do
#     # THREADS=(1 2 4 8 16 24 32 40 48 56 64 128 256 512 1024)
#     for max_workers in "${THREADS[@]}";
#     do
#         echo "Running with max_workers: $max_workers"
#         ./read "$max_workers" "$dir" "$output"
#         # ./read_perfile "$max_workers"
#     done
# done


## 0425 测试相同线程数量，不同IO size下的时间 读文件时间 ------------------------------------- 0905
# IOSIZE=(1 2 4 8 16 32 64 128 256 512 1024)
# IOSIZE=(256)

# THREADS=(1 2 4 8 16 24 32 40 48 56 64 128 256 512 1024)
# # THREADS=(1 4 16 32 40)

# for i in {1..1}
# do
    
#     for max_workers in "${THREADS[@]}";
#     do
#         for iosize in "${IOSIZE[@]}";
#         do
#             echo "Running with max_workers: $max_workers ,iosize : $iosize KB"
#             ./read_vary_iosize "$max_workers" "$iosize"
#             # ./read_perfile "$max_workers"
#         done
#     done
# done


# max_workers=256

# for i in {1..20}
# do
#     echo "Running with max_workers: $max_workers"
#     ./read "$max_workers"
# done

# sudo python3 read_opt.py /mnt/beegfs/test_dir_32KB_256 8
# sudo python3 read_opt.py /mnt/beegfs/test_dir_512B_256 8
