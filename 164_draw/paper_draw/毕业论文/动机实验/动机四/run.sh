#!/bin/bash

# ps: 164HOST 上 root权限运行
## ------------------------------------------------------------------------------------------------
#**--------------单个GPU的训练脚本--------------**

# 定义 Python 脚本的路径
#【Baseline】版本
# PYTHON_SCRIPT="train52.py"
PYTHON_SCRIPT="train52_noutils.py"




# 固定参数
EPOCH_NUM=1
gpu_id=2
for CONN_MODE in 1 #1 2 3
do
    for batch_size in 16 32 64 128 256 512 #512 #256 512  #256 512 #128 256 512 
    do
        for worker_num in 1 2 4 8 16 32 #4 8 16 #32 40 64 #40 64 #128 256 #256  #32 64 #128  #32 40 64 128
        do  
            for prefetch_factor in 9 10 11 12 13 14 15 16 
            do
                Output_Name="output/output_b${batch_size}_w${worker_num}_prefecth${prefetch_factor}.txt"
                echo $Output_Name
                nvtx_file="output/nvtx_b${batch_size}_w${worker_num}_prefetch${prefetch_factor}"
                # 执行 Python 脚本并传递参数
                sudo python3 $PYTHON_SCRIPT --batch_size $batch_size --epochs $EPOCH_NUM --workers $worker_num --conn_mode $CONN_MODE --prefetch $prefetch_factor --gpu $gpu_id > $Output_Name #output//0806_B512_W16_1000class_epoch25.txt
                
                # /opt/nvidia/nsight-systems/2024.5.1/target-linux-x64/nsys profile -o $nvtx_file \
                # --stats=true --trace=cuda,cublas,nvtx,syscall  --force-overwrite=true \
                # python3 $PYTHON_SCRIPT --batch_size $batch_size --epochs $EPOCH_NUM --workers $worker_num \
                # --conn_mode $CONN_MODE --prefetch $prefetch_factor --gpu $gpu_id > $Output_Name 
                
                # echo "已完成 worker_num = $worker_num"
            done
        done
    done
done

# for CONN_MODE in 1 #1 2 3
# do
#     for batch_size in 16 32 64 128 256 512 #128 256 512 
#     do
#         for worker_num in 4 8 16 32 #0 1 2 4 8 #8 #4 8 16 #32 40 64 #40 64 #128 256 #256  #32 64 #128  #32 40 64 128
#         do  
#             for prefetch_factor in 1 #2 3 4
#             do
#                 Output_Name="output/1014/output_b${batch_size}_w${worker_num}.txt"
#                 echo $Output_Name
#                 nvtx_file="output/1014/nvtx_b${batch_size}_w${worker_num}"
#                 # 执行 Python 脚本并传递参数
#                 sudo python3 $PYTHON_SCRIPT --batch_size $batch_size --epochs $EPOCH_NUM --workers $worker_num --conn_mode $CONN_MODE --prefetch $prefetch_factor --gpu $gpu_id > $Output_Name #output//0806_B512_W16_1000class_epoch25.txt
                
#                 # /opt/nvidia/nsight-systems/2024.5.1/target-linux-x64/nsys profile -o $nvtx_file \
#                 # --stats=true --trace=cuda,cublas,nvtx,syscall  --force-overwrite=true \
#                 # python3 $PYTHON_SCRIPT --batch_size $batch_size --epochs $EPOCH_NUM --workers $worker_num \
#                 # --conn_mode $CONN_MODE --prefetch $prefetch_factor --gpu $gpu_id > $Output_Name 
                
#                 # echo "已完成 worker_num = $worker_num"
#             done
#         done
#     done
# done



# for CONN_MODE in 1 #1 2 3
# do
#     for batch_size in 16 32 64 128 256 512 #512 #256 512  #256 512 #128 256 512 
#     do
#         for worker_num in 0 1 2 #8 #4 8 16 #32 40 64 #40 64 #128 256 #256  #32 64 #128  #32 40 64 128
#         do  
#             for prefetch_factor in 1 #2 3 4
#             do
#                 Output_Name="output/1014/output_b${batch_size}_w${worker_num}.txt"
#                 echo $Output_Name
#                 nvtx_file="output/1014/nvtx_b${batch_size}_w${worker_num}"
#                 # 执行 Python 脚本并传递参数
#                 sudo python3 $PYTHON_SCRIPT --batch_size $batch_size --epochs $EPOCH_NUM --workers $worker_num --conn_mode $CONN_MODE --prefetch $prefetch_factor --gpu $gpu_id > $Output_Name #output//0806_B512_W16_1000class_epoch25.txt
                
#                 # /opt/nvidia/nsight-systems/2024.5.1/target-linux-x64/nsys profile -o $nvtx_file \
#                 # --stats=true --trace=cuda,cublas,nvtx,syscall  --force-overwrite=true \
#                 # python3 $PYTHON_SCRIPT --batch_size $batch_size --epochs $EPOCH_NUM --workers $worker_num \
#                 # --conn_mode $CONN_MODE --prefetch $prefetch_factor --gpu $gpu_id > $Output_Name 
                
#                 # echo "已完成 worker_num = $worker_num"
#             done
#         done
#     done
# done

# for CONN_MODE in 1 #1 2 3
# do
#     for batch_size in 16 32 64 128 256 512 #512 #256 512  #256 512 #128 256 512 
#     do
#         for worker_num in 40 #8 #4 8 16 #32 40 64 #40 64 #128 256 #256  #32 64 #128  #32 40 64 128
#         do  
#             for prefetch_factor in 1 #2 3 4
#             do
#                 Output_Name="output/1014/output_b${batch_size}_w${worker_num}.txt"
#                 echo $Output_Name
#                 nvtx_file="output/1014/nvtx_b${batch_size}_w${worker_num}"
#                 # 执行 Python 脚本并传递参数
#                 sudo python3 $PYTHON_SCRIPT --batch_size $batch_size --epochs $EPOCH_NUM --workers $worker_num --conn_mode $CONN_MODE --prefetch $prefetch_factor --gpu $gpu_id > $Output_Name #output//0806_B512_W16_1000class_epoch25.txt
                
#                 # /opt/nvidia/nsight-systems/2024.5.1/target-linux-x64/nsys profile -o $nvtx_file \
#                 # --stats=true --trace=cuda,cublas,nvtx,syscall  --force-overwrite=true \
#                 # python3 $PYTHON_SCRIPT --batch_size $batch_size --epochs $EPOCH_NUM --workers $worker_num \
#                 # --conn_mode $CONN_MODE --prefetch $prefetch_factor --gpu $gpu_id > $Output_Name 
                
#                 # echo "已完成 worker_num = $worker_num"
#             done
#         done
#     done
# done

# CONN_MODE=(0 1 2 3)
# 循环 worker_num 从 1 到 256，2 倍数增长
# for CONN_MODE in 1 #1 2 3
# do
#     for batch_size in 512 256 128 64 32 #64 128 256 512  #256 512 #128 256 512 
#     do
#         for worker_num in 4 8 16 #64 128 256 #256  #32 64 #128  #32 40 64 128
#         do  
#             for prefetch_factor in 1 #2 3 4
#             do
#                 Output_Name="output/0811/output_b${batch_size}_w${worker_num}.txt"
#                 echo $Output_Name
#                 # 执行 Python 脚本并传递参数
#                 # sudo python3 $PYTHON_SCRIPT --batch_size $BATCH_SIZE --epochs $EPOCH_NUM --workers $worker_num
#                 sudo python3 $PYTHON_SCRIPT --batch_size $batch_size --epochs $EPOCH_NUM --workers $worker_num --conn_mode $CONN_MODE --prefetch $prefetch_factor --gpu $gpu_id > $Output_Name #output//0806_B512_W16_1000class_epoch25.txt
#                 # echo "已完成 worker_num = $worker_num"
#             done
#         done
#     done
# done


# for CONN_MODE in 1 #1 2 3
# do
#     for batch_size in 16 #32 64 128 256 512  #256 512 #128 256 512 
#     do
#         for worker_num in 4 8 16 #32 40 64 #40 64 #128 256 #256  #32 64 #128  #32 40 64 128
#         do  
#             for prefetch_factor in 1 #2 3 4
#             do
#                 Output_Name="output/0811/output_b${batch_size}_w${worker_num}.txt"
#                 echo $Output_Name
#                 # 执行 Python 脚本并传递参数
#                 # sudo python3 $PYTHON_SCRIPT --batch_size $BATCH_SIZE --epochs $EPOCH_NUM --workers $worker_num
#                 sudo python3 $PYTHON_SCRIPT --batch_size $batch_size --epochs $EPOCH_NUM --workers $worker_num --conn_mode $CONN_MODE --prefetch $prefetch_factor --gpu $gpu_id > $Output_Name #output//0806_B512_W16_1000class_epoch25.txt
#                 # echo "已完成 worker_num = $worker_num"
#             done
#         done
#     done
# done


# for CONN_MODE in 1 #1 2 3
# do
#     for batch_size in 16 32 64 128 256 512  #256 512 #128 256 512 
#     do
#         for worker_num in 32 #40 64 #128 256 #256  #32 64 #128  #32 40 64 128
#         do  
#             for prefetch_factor in 1 #2 3 4
#             do
#                 Output_Name="output/0811/output_b${batch_size}_w${worker_num}.txt"
#                 echo $Output_Name
#                 # 执行 Python 脚本并传递参数
#                 # sudo python3 $PYTHON_SCRIPT --batch_size $BATCH_SIZE --epochs $EPOCH_NUM --workers $worker_num
#                 sudo python3 $PYTHON_SCRIPT --batch_size $batch_size --epochs $EPOCH_NUM --workers $worker_num --conn_mode $CONN_MODE --prefetch $prefetch_factor --gpu $gpu_id > $Output_Name #output//0806_B512_W16_1000class_epoch25.txt
#                 # echo "已完成 worker_num = $worker_num"
#             done
#         done
#     done
# done

# 少workernum 就剩这一组数据没有测完了
# for CONN_MODE in 1 #1 2 3
# do
#     for batch_size in 512  #256 512 #128 256 512 
#     do
#         for worker_num in 1 2 #0 1 2 #64 128 256 #256  #32 64 #128  #32 40 64 128
#         do  
#             for prefetch_factor in 1 #2 3 4
#             do
#                 Output_Name="output/0811/output_b${batch_size}_w${worker_num}.txt"
#                 echo $Output_Name
#                 # 执行 Python 脚本并传递参数
#                 # sudo python3 $PYTHON_SCRIPT --batch_size $BATCH_SIZE --epochs $EPOCH_NUM --workers $worker_num
#                 sudo python3 $PYTHON_SCRIPT --batch_size $batch_size --epochs $EPOCH_NUM --workers $worker_num --conn_mode $CONN_MODE --prefetch $prefetch_factor --gpu $gpu_id > $Output_Name #output//0806_B512_W16_1000class_epoch25.txt
#                 # echo "已完成 worker_num = $worker_num"
#             done
#         done
#     done
# done
