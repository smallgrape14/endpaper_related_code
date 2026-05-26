#!/bin/bash

# 测试原生的ext4 本地文件系统加载代码
# sudo python3 real_main.py -a resnet18 /mnt/nvme0/FlexGV_Test/ImageNet-1K -j 0 --epochs 1 -b 512 --gpu 0 > real_main_out.txt
# sudo python3 real_main.py -a resnet18 /mnt/nvme0/FlexGV_Test/ImageNet-1K -j 16 --epochs 10 -b 512 --gpu 0 > result/nodecode/out_epoch10_w16_b512.txt

# sudo python3 real_main.py -a resnet18 /home/oem/xyp/imagenet_test -j 0 --epochs 1 -b 512 --gpu 0 > real_main_out.txt


# sudo python3 real_main2.py -a resnet18 /mnt/nvme0/FlexGV_Test/ImageNet-1K -j 0 --epochs 1 -b 512 --gpu 0 > real_main_out.txt


# 单GPU的模式
# # 定义 Python 脚本的路径
# #【Baseline】版本
PYTHON_SCRIPT="real_main_nvtx.py"
MODEL_NAME="resnet18"
TRAIN_DIR="/mnt/nvme0/FlexGV_Test/ImageNet-1K"
EPOCH_NUM=1
MAX_Batch=5
gpu_id=0
for batch_size in 8 #32 64 128 256 512
do
    for worker_num in 0 1 2 4 8 #32 #32 40 64 128
    do  
        for prefetch_factor in 1 #2 3 4
        do
            # 执行 Python 脚本并传递参数
            Output_Name="output/real_main_1gpu/output_b${batch_size}_w${worker_num}.txt"
            nvtx_Name="output/real_main_1gpu/nvtx_b${batch_size}_w${worker_num}"
            echo $Output_Name
            # sudo python3 $PYTHON_SCRIPT -a $MODEL_NAME $TRAIN_DIR -b $batch_size --epochs $EPOCH_NUM -j $worker_num --max_batch $MAX_Batch --gpu $gpu_id > $Output_Name
            /opt/nvidia/nsight-systems/2024.5.1/target-linux-x64/nsys profile -o $nvtx_Name \
            --stats=true --trace=cuda,osrt,nvtx,syscall  --sample=process-tree  --force-overwrite=true \
            python3 $PYTHON_SCRIPT -a $MODEL_NAME $TRAIN_DIR -b $batch_size --epochs $EPOCH_NUM -j $worker_num --max_batch $MAX_Batch --gpu $gpu_id > $Output_Name
            
            # echo "已完成 worker_num = $worker_num"
        done
    done
done
