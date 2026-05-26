#!/bin/bash



## ------------------------------------------------------------------------------------------------
#**--------------两个GPU，分布式训练的测试脚本--------------**

# 定义 Python 脚本的路径
#【Baseline】版本
PYTHON_SCRIPT="real_main_nvtx.py"
MODEL_NAME="resnet18"
# TRAIN_DIR="/mnt/beegfs/test_dir"
# TRAIN_DIR="/mnt/beegfs/ImageNet_all"
TRAIN_DIR="/mnt/nvme0/FlexGV_Test/ImageNet-1K"
DIST_URL="tcp://127.0.0.1:3679"
DIST_BACKEND="nccl"
NODE_NUM=1
NODE_RANK=0
Print_freq=1
# 固定参数
# BATCH_SIZE=32
EPOCH_NUM=1
# CONN_MODE=(0 1 2 3)
# 循环 worker_num 从 1 到 256，2 倍数增长
MAX_Batch=5
GPU_NUM=2
for bsize in 8 #16 32 64 128 256 512 
do
    for wnum in 8 #16 #32 64 
    do  
        batch_size=$((bsize*GPU_NUM))
        worker_num=$((wnum*GPU_NUM))
        Output_Name="output/real_main/output_b${bsize}_w${wnum}.txt"
        nvtx_Name="output/real_main/nvtx_b${bsize}_w${wnum}"
        echo $Output_Name
        # 执行 Python 脚本并传递参数
        # sudo python3 $PYTHON_SCRIPT -b $batch_size -j $worker_num --epochs $EPOCH_NUM -p $Print_freq -a $MODEL_NAME $TRAIN_DIR --dist-url $DIST_URL --dist-backend $DIST_BACKEND --multiprocessing-distributed --world-size $NODE_NUM --rank $NODE_RANK --max_batch $MAX_Batch --gpu_num $GPU_NUM > $Output_Name
        
        /opt/nvidia/nsight-systems/2024.5.1/target-linux-x64/nsys profile -o $nvtx_Name \
        --stats=true --trace=cuda,osrt,nvtx,syscall  --sample=process-tree  --force-overwrite=true \
        python3 $PYTHON_SCRIPT -b $batch_size -j $worker_num --epochs $EPOCH_NUM -p $Print_freq -a $MODEL_NAME $TRAIN_DIR --dist-url $DIST_URL --dist-backend $DIST_BACKEND --multiprocessing-distributed --world-size $NODE_NUM --rank $NODE_RANK --max_batch $MAX_Batch --gpu_num $GPU_NUM > $Output_Name
            
        
    done
done
# 跑32 workernum 每次程序结束会卡住，需要手动ctrl+c结束
# for batch_size in 16 32 64 128 256 512 
# do
#     for worker_num in 32 64 
#     do  
        
#         # 执行 Python 脚本并传递参数
#         # sudo python3 $PYTHON_SCRIPT --batch_size $BATCH_SIZE --epochs $EPOCH_NUM --workers $worker_num
#         sudo python3 $PYTHON_SCRIPT -b $batch_size -j $worker_num --epochs $EPOCH_NUM -p $Print_freq -a $MODEL_NAME $TRAIN_DIR --dist-url $DIST_URL --dist-backend $DIST_BACKEND --multiprocessing-distributed --world-size $NODE_NUM --rank $NODE_RANK
            
        
#     done
# done
