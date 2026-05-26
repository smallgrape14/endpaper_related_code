#!/bin/bash

# ## ------------------------------------------------------------------------------------------------
# 定义 Python 脚本的路径
#【Baseline】版本
PYTHON_SCRIPT="train52.py"
# PYTHON_SCRIPT="train52_flame.py"

# [下面这个版本是做了dataprefetcher优化]
# PYTHON_SCRIPT="train54.py"


# 固定参数
# BATCH_SIZE=32
EPOCH_NUM=20
# CONN_MODE=(0 1 2 3)
# 循环 worker_num 从 1 到 256，2 倍数增长
for CONN_MODE in 1 #1 2 3
do
    for batch_size in 16 #256 512 #32 64 128 256 512
    do
        for worker_num in 8 #32 #32 40 64 128
        do  
            for prefetch_factor in 1 #2 3 4
            do
                # 执行 Python 脚本并传递参数
                # sudo python3 $PYTHON_SCRIPT --batch_size $BATCH_SIZE --epochs $EPOCH_NUM --workers $worker_num
                # sudo python3 $PYTHON_SCRIPT --batch_size $batch_size --epochs $EPOCH_NUM --workers $worker_num --conn_mode $CONN_MODE --prefetch $prefetch_factor
                /opt/nvidia/nsight-systems/2024.5.1/target-linux-x64/nsys profile -o report_train52_b16_w8 --stats=true --trace=cuda,osrt,nvtx,syscall  --force-overwrite=true  python3 $PYTHON_SCRIPT --batch_size $batch_size --epochs $EPOCH_NUM --workers $worker_num --conn_mode $CONN_MODE --prefetch $prefetch_factor
                # echo "已完成 worker_num = $worker_num"
            done
        done
    done
done