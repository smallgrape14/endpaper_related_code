#!/bin/bash

# 定义原始文件名
files=("train-images-idx3-ubyte" "train-labels-idx1-ubyte" "t10k-images-idx3-ubyte" "t10k-labels-idx1-ubyte")

# 定义两个目录的路径
dir1="/mnt/beegfs/FashionMNIST"
dir2="/home/xyp/test_beegfs/result/outfile"
# dir2="/doca_devel/doca_2.7.0085/applications/gpu_packet_processing/data/output/output"


# 遍历文件数组，比较每个文件
for file in "${files[@]}"; do
    # 构建完整的文件路径
    original_file="$dir1/$file"
    modified_file="$dir2/out_$file"

    # 检查两个文件是否存在
    if [ -f "$original_file" ] && [ -f "$modified_file" ]; then
        # 使用 diff 命令比较文件
        # diff "$original_file" "$modified_file" > /dev/null
        # if [ $? -ne 0 ]; then
        #     echo "Differences found in $file"
        # else
        #     echo "No differences found in $file"
        # fi
        diff -u "$original_file" "$modified_file"
        if [ $? -eq 0 ]; then
            echo "No differences found in $file"
        else
            echo "Differences found in $file"
        fi
    else
        echo "One or both files are missing: $original_file, $modified_file"
    fi
done

