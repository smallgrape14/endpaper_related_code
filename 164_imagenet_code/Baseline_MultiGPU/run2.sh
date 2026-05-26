#!/bin/bash

# ----单个GPU跑 Baseline

## ------------------------------------------------------------------------------------------------
# 定义 Python 脚本的路径
# PYTHON_SCRIPT="train41.py"

# # 固定参数
# # BATCH_SIZE=32
# EPOCH_NUM=1
# # CONN_MODE=(0 1 2 3)
# # 循环 worker_num 从 1 到 256，2 倍数增长
# for CONN_MODE in 1 #1 2 3
# do
#     for batch_size in 16 32 64 128 256 512
#     do
#         for worker_num in 8 16 #32 40 64 128
#         do
#             # 执行 Python 脚本并传递参数
#             # sudo python3 $PYTHON_SCRIPT --batch_size $BATCH_SIZE --epochs $EPOCH_NUM --workers $worker_num
#             sudo python3 $PYTHON_SCRIPT --batch_size $batch_size --epochs $EPOCH_NUM --workers $worker_num --conn_mode $CONN_MODE
#             echo "已完成 worker_num = $worker_num"

#         done
#     done
# done

## ------------------------------------------------------------------------------------------------

# # 定义 Python 脚本的路径
# PYTHON_SCRIPT="train4.py"

# # 固定参数
# # BATCH_SIZE=32
# EPOCH_NUM=1
# # CONN_MODE=(0 1 2 3)
# # 循环 worker_num 从 1 到 256，2 倍数增长

# for batch_size in 16 32 64 128 256 512
# do
#     for worker_num in 1 2 4 8 16 #32 40 64 128
#     do
#         # 执行 Python 脚本并传递参数
#         # sudo python3 $PYTHON_SCRIPT --batch_size $BATCH_SIZE --epochs $EPOCH_NUM --workers $worker_num
#         sudo python3 $PYTHON_SCRIPT --batch_size $batch_size --epochs $EPOCH_NUM --workers $worker_num 
#         echo "已完成 worker_num = $worker_num"

#     done
# done


## ------------------------------------------------------------------------------------------------


# ## ------------------------------------------------------------------------------------------------
# # 定义 Python 脚本的路径
# #【Baseline】版本
# PYTHON_SCRIPT="train52.py"
# # [下面这个版本是做了dataprefetcher优化]
# # PYTHON_SCRIPT="train54.py"


# # 固定参数
# # BATCH_SIZE=32
# EPOCH_NUM=8
# # CONN_MODE=(0 1 2 3)
# # 循环 worker_num 从 1 到 256，2 倍数增长
# for CONN_MODE in 1 #1 2 3
# do
#     for batch_size in 256 512 #32 64 128 256 512
#     do
#         for worker_num in 32 #32 #32 40 64 128
#         do  
#             for prefetch_factor in 1 #2 3 4
#             do
#                 # 执行 Python 脚本并传递参数
#                 # sudo python3 $PYTHON_SCRIPT --batch_size $BATCH_SIZE --epochs $EPOCH_NUM --workers $worker_num
#                 sudo python3 $PYTHON_SCRIPT --batch_size $batch_size --epochs $EPOCH_NUM --workers $worker_num --conn_mode $CONN_MODE --prefetch $prefetch_factor
#                 # echo "已完成 worker_num = $worker_num"
#             done
#         done
#     done
# done


## ------------------------------------------------------------------------------------------------
# **--------------单个GPU的训练脚本--------------**
# **--------------测试1：测试同时在两个GPU上跑训练，主要是为了排查多个dataloader worker的竞争情况--------------**

# # 定义 Python 脚本的路径
# #【Baseline】版本
# PYTHON_SCRIPT="train52.py"
# # [下面这个版本是做了dataprefetcher优化]
# # PYTHON_SCRIPT="train54.py"


# # 固定参数
# # BATCH_SIZE=32
# EPOCH_NUM=2
# # CONN_MODE=(0 1 2 3)
# # 循环 worker_num 从 1 到 256，2 倍数增长
# for CONN_MODE in 1 #1 2 3
# do
#     for batch_size in 16 #256 512 #16 32 64 128 256 512 
#     do
#         for worker_num in 1 #32 #0 4 8 16 #256  #32 64 #128  #32 40 64 128
#         do  
#             for prefetch_factor in 1 #2 3 4
#             do
#                 # 执行 Python 脚本并传递参数
#                 # sudo python3 $PYTHON_SCRIPT --batch_size $BATCH_SIZE --epochs $EPOCH_NUM --workers $worker_num
#                 sudo python3 $PYTHON_SCRIPT --batch_size $batch_size --epochs $EPOCH_NUM --workers $worker_num --conn_mode $CONN_MODE --prefetch $prefetch_factor --gpu 0 
#                 # & sudo python3 $PYTHON_SCRIPT --batch_size $batch_size --epochs $EPOCH_NUM --workers $worker_num --conn_mode $CONN_MODE --prefetch $prefetch_factor --gpu 1
#                 # echo "已完成 worker_num = $worker_num"
#             done
#         done
#     done
# done
## ------------------------------------------------------------------------------------------------
# # **--------------单个GPU的训练脚本--------------**
# # **--------------测试2：（与测试1不同的是这里启用DDP，但是实际上只用1个GPU）测试同时在两个GPU上跑训练，主要是为了排查多个dataloader worker的竞争情况，排查下DDP带来的影响--------------**


# # 定义 Python 脚本的路径
# #【Baseline】版本
# PYTHON_SCRIPT="main.py"
# MODEL_NAME="resnet18"
# TRAIN_DIR="/mnt/beegfs/test_dir"
# DIST_URL="tcp://127.0.0.1:3678"
# DIST_URL2="tcp://127.0.0.1:3679"

# DIST_BACKEND="nccl"
# NODE_NUM=1
# NODE_RANK=0
# Print_freq=50
# # 固定参数
# # BATCH_SIZE=32
# EPOCH_NUM=2
# # CONN_MODE=(0 1 2 3)
# # 循环 worker_num 从 1 到 256，2 倍数增长

# for batch_size in 256 #32 64 128 256 512 
# do
#     for worker_num in 8 #0 4 8 16 #32 64 
#     do  
        
#         # 执行 Python 脚本并传递参数
#         # sudo python3 $PYTHON_SCRIPT --batch_size $BATCH_SIZE --epochs $EPOCH_NUM --workers $worker_num
#         sudo python3 $PYTHON_SCRIPT -b $batch_size -j $worker_num --epochs $EPOCH_NUM -p $Print_freq -a $MODEL_NAME $TRAIN_DIR --dist-url $DIST_URL --dist-backend $DIST_BACKEND --multiprocessing-distributed --world-size $NODE_NUM --rank $NODE_RANK --gpu 0 & sudo python3 $PYTHON_SCRIPT -b $batch_size -j $worker_num --epochs $EPOCH_NUM -p $Print_freq -a $MODEL_NAME $TRAIN_DIR --dist-url $DIST_URL2 --dist-backend $DIST_BACKEND --multiprocessing-distributed --world-size $NODE_NUM --rank $NODE_RANK --gpu 1
            
        
#     done
# done
## ------------------------------------------------------------------------------------------------
#**--------------两个GPU，分布式训练的测试脚本--------------**

# 定义 Python 脚本的路径
#【Baseline】版本
PYTHON_SCRIPT="main.py"
MODEL_NAME="resnet18"
# TRAIN_DIR="/mnt/beegfs/test_dir"
TRAIN_DIR="/mnt/beegfs/ImageNet_all"
DIST_URL="tcp://127.0.0.1:3679"
DIST_BACKEND="nccl"
NODE_NUM=1
NODE_RANK=0
Print_freq=1
# 固定参数
# BATCH_SIZE=32
EPOCH_NUM=1
GPU_NUM=2
# CONN_MODE=(0 1 2 3)
# 循环 worker_num 从 1 到 256，2 倍数增长
max_batch=5
for bsize in 8 16 32 #16 32 64 128 256 512 #512 1024
do
    batch_size=$((bsize*GPU_NUM))
    for wnum in 4 8 #0 1 2 4 8 #0 1 2 #8 16 32 #32 64 #4 8 16  #16 #32 64 
    do  
        worker_num=$((wnum*GPU_NUM))
        Output_Name="output/output_b${bsize}_w${wnum}.txt"
        nvtx_file="output/nvtx/nvtx_b${bsize}_w${wnum}"
        echo $Output_Name
        # 执行 Python 脚本并传递参数
        # /opt/nvidia/nsight-systems/2024.5.1/target-linux-x64/nsys profile -o $nvtx_file --stats=true --trace=cuda,osrt,nvtx,syscall  --force-overwrite=true \
        # python3 $PYTHON_SCRIPT -b $batch_size -j $worker_num --epochs $EPOCH_NUM -p $Print_freq -a $MODEL_NAME $TRAIN_DIR \
        # --dist-url $DIST_URL --dist-backend $DIST_BACKEND --multiprocessing-distributed --world-size $NODE_NUM --rank $NODE_RANK \
        # --max_batch $max_batch > $Output_Name

        
        sudo python3 $PYTHON_SCRIPT -b $batch_size -j $worker_num --epochs $EPOCH_NUM -p $Print_freq -a $MODEL_NAME $TRAIN_DIR --dist-url $DIST_URL --dist-backend $DIST_BACKEND --multiprocessing-distributed --world-size $NODE_NUM --rank $NODE_RANK --max_batch $max_batch > $Output_Name  
    done
done
# max_batch=2000
# for bsize in 512 #8 16 32 64 128 256 512 #512 1024
# do
#     batch_size=$((bsize*3))
#     for wnum in 2 #0 1 2 #32 64 #4 8 16  #16 #32 64 
#     do  
#         worker_num=$((wnum*3))
#         Output_Name="output/output_b${bsize}_w${wnum}.txt"
#         echo $Output_Name
#         # 执行 Python 脚本并传递参数
#         # sudo python3 $PYTHON_SCRIPT --batch_size $BATCH_SIZE --epochs $EPOCH_NUM --workers $worker_num
#         sudo python3 $PYTHON_SCRIPT -b $batch_size -j $worker_num --epochs $EPOCH_NUM -p $Print_freq -a $MODEL_NAME $TRAIN_DIR --dist-url $DIST_URL --dist-backend $DIST_BACKEND --multiprocessing-distributed --world-size $NODE_NUM --rank $NODE_RANK --max_batch $max_batch > $Output_Name

#     done
# done
#### -----------------------------------------
# max_batch=1000
# for batch_size in 16 32 64 128 256 512 1024
# do
#     for worker_num in 0 2 #32 64 #4 8 16  #16 #32 64 
#     do  
#         Output_Name="output/output_b${batch_size}_w${worker_num}.txt"
#         echo $Output_Name
#         # 执行 Python 脚本并传递参数
#         # sudo python3 $PYTHON_SCRIPT --batch_size $BATCH_SIZE --epochs $EPOCH_NUM --workers $worker_num
#         sudo python3 $PYTHON_SCRIPT -b $batch_size -j $worker_num --epochs $EPOCH_NUM -p $Print_freq -a $MODEL_NAME $TRAIN_DIR --dist-url $DIST_URL --dist-backend $DIST_BACKEND --multiprocessing-distributed --world-size $NODE_NUM --rank $NODE_RANK --max_batch $max_batch > $Output_Name
            
        
#     done
# done
# max_batch=3000
# for batch_size in 16 32 64 128 256 512 1024
# do
#     for worker_num in 32 64 80 #4 8 16  #16 #32 64 
#     do  
#         Output_Name="output/output_b${batch_size}_w${worker_num}.txt"
#         echo $Output_Name
#         # 执行 Python 脚本并传递参数
#         # sudo python3 $PYTHON_SCRIPT --batch_size $BATCH_SIZE --epochs $EPOCH_NUM --workers $worker_num
#         sudo python3 $PYTHON_SCRIPT -b $batch_size -j $worker_num --epochs $EPOCH_NUM -p $Print_freq -a $MODEL_NAME $TRAIN_DIR --dist-url $DIST_URL --dist-backend $DIST_BACKEND --multiprocessing-distributed --world-size $NODE_NUM --rank $NODE_RANK --max_batch $max_batch > $Output_Name
            
        
#     done
# done









# for batch_size in  16 32 64 128 256 512 1024
# do
#     for worker_num in 0 1 2 #32 64 #4 8 16  #16 #32 64 
#     do  
#         Output_Name="output/output_b${batch_size}_w${worker_num}.txt"
#         echo $Output_Name
#         # 执行 Python 脚本并传递参数
#         # sudo python3 $PYTHON_SCRIPT --batch_size $BATCH_SIZE --epochs $EPOCH_NUM --workers $worker_num
#         sudo python3 $PYTHON_SCRIPT -b $batch_size -j $worker_num --epochs $EPOCH_NUM -p $Print_freq -a $MODEL_NAME $TRAIN_DIR --dist-url $DIST_URL --dist-backend $DIST_BACKEND --multiprocessing-distributed --world-size $NODE_NUM --rank $NODE_RANK > $Output_Name
            
        
#     done
# done

# for batch_size in  16 32 64 128 256 512 1024
# do
#     for worker_num in 64 80 #32 64 #4 8 16  #16 #32 64 
#     do  
#         Output_Name="output/output_b${batch_size}_w${worker_num}.txt"
#         echo $Output_Name
#         # 执行 Python 脚本并传递参数
#         # sudo python3 $PYTHON_SCRIPT --batch_size $BATCH_SIZE --epochs $EPOCH_NUM --workers $worker_num
#         sudo python3 $PYTHON_SCRIPT -b $batch_size -j $worker_num --epochs $EPOCH_NUM -p $Print_freq -a $MODEL_NAME $TRAIN_DIR --dist-url $DIST_URL --dist-backend $DIST_BACKEND --multiprocessing-distributed --world-size $NODE_NUM --rank $NODE_RANK > $Output_Name
            
        
#     done
# done
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

# 暂时没用上------------------------------------------------------------------------------------
# #!/bin/bash
# # 分布式训练启动脚本 - 参数变量化模板

# ############################ 用户配置区 ############################
# # 模型与训练配置
# MODEL_ARCH="resnet18"     # 模型架构（-a 参数）
# MAIN_SCRIPT="main.py"      # 主Python脚本路径

# # 分布式训练配置
# DIST_URL="tcp://127.0.0.1:3679"  # 通信地址（--dist-url）
# DIST_BACKEND="nccl"               # 通信后端（--dist-backend）
# WORLD_SIZE=1                      # 进程总数（--world-size）
# USE_MULTIPROCESSING="true"        # 启用多进程分布式模式（--multiprocessing-distributed）

# # 其他常用参数示例（按需扩展）
# BATCH_SIZE=256                    # 批次大小
# LEARNING_RATE=0.1                 # 学习率
# DATA_DIR="/dataset/imagenet"      # 数据集路径

# ############################ 命令生成区 ############################
# # 自动构建命令参数
# CMD="python3 ${MAIN_SCRIPT} -a ${MODEL_ARCH}"

# # 动态添加分布式参数
# if [ "$USE_MULTIPROCESSING" = "true" ]; then
#   CMD+=" --multiprocessing-distributed"
# fi

# CMD+=" --dist-url ${DIST_URL}"
# CMD+=" --dist-backend ${DIST_BACKEND}"
# CMD+=" --world-size ${WORLD_SIZE}"

# # 添加其他参数（示例）
# CMD+=" --batch-size ${BATCH_SIZE}"
# CMD+=" --lr ${LEARNING_RATE}"
# CMD+=" --data-dir ${DATA_DIR}"

# ############################ 执行命令 ############################
# echo "执行命令: ${CMD}"
# eval "${CMD}"