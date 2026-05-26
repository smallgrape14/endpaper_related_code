#!/bin/bash

# IOzone 并发读取性能测试脚本
# 每个线程读取不同的文件

# 默认参数
# find /mnt/beegfs/image_32KB_500 -type f > filelist.txt
file_list="filelist.txt"  # 包含文件列表的文件
output_file="iozone_read_results.txt"  # 默认输出文件
num_threads=500                          # 默认线程数

# 帮助信息
usage() {
    echo "Usage: $0 [-f <file_list>] [-o <output_file>] [-n <num_threads>]"
    echo "Options:"
    echo "  -f <file_list>        指定包含文件列表的文件 (默认: /mnt/beegfs/file_list.txt)"
    echo "  -o <output_file>      指定输出文件名 (默认: iozone_read_results.txt)"
    echo "  -n <num_threads>      指定线程数 (默认: 1)"
    echo "  -h                    显示此帮助信息"
    exit 1
}

# 解析命令行参数
# while getopts ":f:o:n:h" opt; do
#     case $opt in
#         f)
#             file_list=$OPTARG
#             ;;
#         o)
#             output_file=$OPTARG
#             ;;
#         n)
#             num_threads=$OPTARG
#             ;;
#         h)
#             usage
#             ;;
#         \?)
#             echo "Invalid option: -$OPTARG" >&2
#             usage
#             ;;
#         :)
#             echo "Option -$OPTARG requires an argument." >&2
#             usage
#             ;;
#     esac
# done

# # 检查文件列表文件是否存在
# if [ ! -f "$file_list" ]; then
#     echo "Error: 文件列表文件 $file_list 不存在，请确保文件列表文件存在。"
#     exit 1
# fi

# # 检查是否安装了 IOzone
# if ! command -v iozone &> /dev/null; then
#     echo "Error: IOzone is not installed. Please install IOzone first."
#     exit 1
# fi

# # 创建输出文件（如果不存在）
# if [ ! -f "$output_file" ]; then
#     touch "$output_file"
#     echo "Output file created: $output_file"
# fi

# 清空输出文件
> "$output_file"

# 遍历线程数，运行测试
echo "Starting IOzone concurrent read test with the following parameters:"
echo "File list: $file_list"
echo "Output file: $output_file"
echo "Number of threads: $num_threads"
echo "----------------------------------------"
echo "IOzone Concurrent Read Performance Test Results" >> "$output_file"
echo "----------------------------------------" >> "$output_file"
echo "File list: $file_list" >> "$output_file"
echo "Number of threads: $num_threads" >> "$output_file"
echo "Date: $(date)" >> "$output_file"
echo "----------------------------------------" >> "$output_file"

# 获取文件列表
files=($(cat "$file_list"))

# 检查文件数量是否足够
if [ ${#files[@]} -lt $num_threads ]; then
    echo "Error: 文件数量不足，需要至少 $num_threads 个文件。"
    exit 1
fi

# 定义一个函数来运行 IOzone 读取测试
run_iozone_read() {
    local file=$1
    local output=$2
    echo "Testing read for file: $file"
    ./iozone -i 1 -r 4k -s 1M -f "$file" -R >> "$output"
    echo "Read test completed for file: $file"
}

# 启动多个线程进行并发读取
for ((i=0; i<$num_threads; i++)); do
    file=${files[$i]}
    echo "Starting thread $i to read file: $file"
    run_iozone_read "$file" "$output_file" &
done

# 等待所有线程完成
wait

echo "----------------------------------------"
echo "All tests completed. Results saved to $output_file."