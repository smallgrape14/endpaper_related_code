#!/bin/bash

# # 循环运行 20 次 FashionMNIST 
# for i in {1..20}
# do
#     echo "Running test $i"
#     ./test /mnt/beegfs/image out_imagenet.csv
# done

# # 循环运行 20 次 image_32KB 4KB /IO 
# for i in {1..20}
# do
#     echo "Running test $i"
#     ./test_decom /mnt/beegfs/image_32KB out_image_32KB.csv
# done

# 循环运行 20 次 image_32KB 4KB /IO 
for i in {1..7}
do
    echo "Running test $i"
    # ./test_decom_all /mnt/beegfs/image_32KB_10428 out_image_32KB_10428_all.csv
    ./test_decom_all /mnt/beegfs/image_32KB_10428 out_0414.csv
    # ./test_decom_all /mnt/beegfs/image_32KB out_image_32KB_all.csv

done

# ./test_decom_all /mnt/beegfs/image_32KB_500 out_image_32KB_500_all.csv


# ./test_decom_all /mnt/beegfs/test_dir_32KB out_test_dir_32KB_all.csv
