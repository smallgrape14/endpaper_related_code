#!/bin/bash

# 定义一个函数来递归处理目录
process_directory() {
    local dir=$1

    echo "正在检查目录: $dir"

    # 检查当前目录中是否存在 txt 文件
    if compgen -G "$dir/*.txt" > /dev/null; then
        echo "在目录 $dir 中检测到 .txt 文件，开始执行指令..."

        # 确认训练程序均正常结束
        python3 check_train_comp.py "$dir"
        # 运行解析程序
        ./log_parser "$dir"
        ./time_parser "$dir"
    else
        echo "在目录 $dir 中没有找到 .txt 文件，跳过指令执行。"
    fi

    echo "正在检查子目录..."

    # 遍历当前目录下的所有子目录
    for subdir in "$dir"/*; do
        if [ -d "$subdir" ]; then
            # 递归调用函数处理子目录
            process_directory "$subdir"
        fi
    done
}

# 检查是否传入待解析目录
if [ $# -eq 0 ]; then
    echo "用法: $0 <directory>"
    exit 1
fi

make
if [ $? -ne 0 ]; then
    echo "编译失败，请检查代码。"
    exit 1
fi

# 启动递归处理
process_directory "$1"