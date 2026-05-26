#!/bin/bash
directory="/mnt/beegfs/test_dir_1KB_10428"
# 遍历线程数量从1到256，每次以2的倍数递增
max_workers=1
# while [ $max_workers -le 1024 ]; do
while [ $max_workers -le 1 ]; do

    for i in $(seq 1 1)
    do
        echo "Running with max_workers: $max_workers"
        python3 gds.py "$directory" "$max_workers"
        
    done
    max_workers=$((max_workers * 2))
done