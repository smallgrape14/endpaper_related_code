#!/bin/bash

./run_all.sh
./run_all_bigsize.sh
# 编译命令（需要CUDA和cuFile支持）
# nvcc -o gds gds.cpp -lcufile -lcudart

# # 运行示例
# ./gds /mnt/beegfs/test_dir_1KB_10428 output.csv 0


# ./gds /mnt/beegfs/test_dir_2KB_10428 output.csv 0

# ./gds /mnt/beegfs/test_dir_4KB_10428 output.csv 0

# ./gds /mnt/nvme0/test_dir_1KB_10428 output.csv 0

# ./gds /home/oem/xyp/imagenet_code/gdsio/163test/data/test_dir_4KB_10428 output.csv 0



