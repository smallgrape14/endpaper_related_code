#!/bin/bash

## ------------------------------------------------------------------------------------------------
#**--------------两个GPU，分布式训练的测试脚本--------------**

# 定义 Python 脚本的路径
#【Baseline】版本
PYTHON_SCRIPT="main_nvtx.py"
MODEL_NAME="resnet18"
TRAIN_DIR="/mnt/beegfs/test_dir"
DIST_URL="tcp://127.0.0.1:3679"
DIST_BACKEND="nccl"
NODE_NUM=1
NODE_RANK=0
Print_freq=50
# 固定参数
# BATCH_SIZE=32
EPOCH_NUM=1
# CONN_MODE=(0 1 2 3)
# 循环 worker_num 从 1 到 256，2 倍数增长

for batch_size in 32 #512 #64 128 256 512 
do
    for worker_num in 16 #4 8 16 #32 64 
    do  
        
        # 执行 Python 脚本并传递参数
        # --gpu-metrics-set=ga100 --gpu-metrics-devices=all --gpu-metrics-frequency=10000 
        # --gpu-metrics-set=ga100 --gpu-metrics-devices=all --gpu-metrics-frequency=10000 这个参数不行
        # sudo python3 $PYTHON_SCRIPT --batch_size $BATCH_SIZE --epochs $EPOCH_NUM --workers $worker_num
        /opt/nvidia/nsight-systems/2024.5.1/target-linux-x64/nsys profile -o report_main_nvtx_b32_w16 --stats=true --trace=cuda,osrt,nvtx,syscall  --sample=process-tree  --force-overwrite=true  python3 $PYTHON_SCRIPT -b $batch_size -j $worker_num --epochs $EPOCH_NUM -p $Print_freq -a $MODEL_NAME $TRAIN_DIR --dist-url $DIST_URL --dist-backend $DIST_BACKEND --multiprocessing-distributed --world-size $NODE_NUM --rank $NODE_RANK
            
        
    done
done