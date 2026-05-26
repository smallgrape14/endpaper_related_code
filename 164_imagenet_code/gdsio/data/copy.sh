#!/bin/bash

# 定义源目录和目标目录
SOURCE_DIR="/home/oem/xyp/imagenet_code/gdsio/163test/data/test_dir_1KB_10428"
DEST_DIR="/mnt/beegfs/test_dir_1KB_10428_1000"

# 创建目标目录（如果它不存在）
mkdir -p "$DEST_DIR"

# 初始化计数器
count=0

# 检查源目录是否存在并可访问
if [ ! -d "$SOURCE_DIR" ]; then
    echo "错误：源目录 $SOURCE_DIR 不存在。"
    exit 1
fi

# 遍历源目录中的文件
for file in "$SOURCE_DIR"/*; do
    # 确保是文件，而不是目录
    if [ -f "$file" ]; then
        # 复制文件到目标目录
        cp "$file" "$DEST_DIR/"
        # 计数器加1
        count=$((count + 1))
        echo "已复制: $(basename "$file")"
        
        # 如果已经复制了1000个文件，则退出循环
        if [ "$count" -eq 1000 ]; then
            break
        fi
    fi
done

echo "任务完成！总共复制了 $count 个文件到 $DEST_DIR。"