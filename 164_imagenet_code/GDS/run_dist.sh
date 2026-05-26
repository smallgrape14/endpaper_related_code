#!/bin/bash

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
#**--------------单个GPU的训练脚本--------------**

# # # 定义 Python 脚本的路径
# # #【Baseline】版本
# # PYTHON_SCRIPT="DALI.py"
# PYTHON_SCRIPT="DALI.py"

# # PYTHON_SCRIPT="main_DALI.py"

# # # [下面这个版本是做了dataprefetcher优化]
# # # PYTHON_SCRIPT="train54.py"


# # 固定参数
# # BATCH_SIZE=32
# EPOCH_NUM=1
# gpu_id=0
# # CONN_MODE=(0 1 2 3)
# # 循环 worker_num 从 1 到 256，2 倍数增长
# for CONN_MODE in 1 #1 2 3
# do
#     for batch_size in 64 128 256 512  #16 32 #64  128 256 512 
#     do
#         for worker_num in 8 16 32 #16 32 1 8 16 32 64 #256  #32 64 #128  #32 40 64 128
#         do  
#             for prefetch_factor in 1 #2 3 4
#             do
#                 # 执行 Python 脚本并传递参数
#                 # sudo python3 $PYTHON_SCRIPT --batch_size $BATCH_SIZE --epochs $EPOCH_NUM --workers $worker_num
#                 # sudo python3 $PYTHON_SCRIPT --batch_size $batch_size --epochs $EPOCH_NUM --workers $worker_num --conn_mode $CONN_MODE --prefetch $prefetch_factor --gpu $gpu_id
#                 sudo python $PYTHON_SCRIPT -a resnet18 /mnt/beegfs/test_dir -j $worker_num --epochs $EPOCH_NUM -b $batch_size #--gpu $gpu_id
#                 # sudo python $PYTHON_SCRIPT -a resnet18 /mnt/beegfs/imagenet_decode -j $worker_num --epochs $EPOCH_NUM -b $batch_size --gpu $gpu_id
                
#                 # echo "已完成 worker_num = $worker_num"
#             done
#         done
#     done
# done

## ------------------------------------------------------------------------------------------------
#**--------------两个GPU，分布式训练的测试脚本--------------**

# 定义 Python 脚本的路径
#【Baseline】版本
# PYTHON_SCRIPT="main.py"
PYTHON_SCRIPT="DALI.py"

# PYTHON_SCRIPT="main_debug.py"

MODEL_NAME="resnet18"
TRAIN_DIR="/mnt/beegfs/test_dir"
DIST_URL="tcp://127.0.0.1:3679"
DIST_BACKEND="nccl"
NODE_NUM=1
NODE_RANK=0
Print_freq=50
EPOCH_NUM=1
# 固定参数
# BATCH_SIZE=32

# CONN_MODE=(0 1 2 3)
# 循环 worker_num 从 1 到 256，2 倍数增长
# batch_size=512
# worker_num=16
# sudo python3 $PYTHON_SCRIPT -b $batch_size -j $worker_num --epochs $EPOCH_NUM -p $Print_freq -a $MODEL_NAME $TRAIN_DIR --dist-url $DIST_URL --dist-backend $DIST_BACKEND --multiprocessing-distributed --world-size $NODE_NUM --rank $NODE_RANK

# batch_size=64
# worker_num=16
# sudo python3 $PYTHON_SCRIPT -b $batch_size -j $worker_num --epochs $EPOCH_NUM -p $Print_freq -a $MODEL_NAME $TRAIN_DIR --dist-url $DIST_URL --dist-backend $DIST_BACKEND --multiprocessing-distributed --world-size $NODE_NUM --rank $NODE_RANK

# batch_size=256
# worker_num=16
# sudo python3 $PYTHON_SCRIPT -b $batch_size -j $worker_num --epochs $EPOCH_NUM -p $Print_freq -a $MODEL_NAME $TRAIN_DIR --dist-url $DIST_URL --dist-backend $DIST_BACKEND --multiprocessing-distributed --world-size $NODE_NUM --rank $NODE_RANK


for batch_size in 16 #32 64 128 256 512 
do
    for worker_num in 1 #1 4 8 16 32 #0 8 32 40 #0 4 8 16 32 64 
    do  
        
        # 执行 Python 脚本并传递参数
        # sudo python3 $PYTHON_SCRIPT --batch_size $BATCH_SIZE --epochs $EPOCH_NUM --workers $worker_num
        sudo python3 $PYTHON_SCRIPT -b $batch_size -j $worker_num --epochs $EPOCH_NUM -p $Print_freq -a $MODEL_NAME $TRAIN_DIR --dist-url $DIST_URL --dist-backend $DIST_BACKEND --multiprocessing-distributed --world-size $NODE_NUM --rank $NODE_RANK
           
        
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