#!/bin/bash
ulimit -n 65536
directory="/mnt/beegfs/test_dir_1MB_1024"

# 定义要测试的IOSIZE列表
iosizes=("32KB" "64KB" "128KB" "256KB" "512KB" "1MB")

# 遍历每个IOSIZE
for iosize in "${iosizes[@]}"; do
    echo "Testing IOSIZE: $iosize"
    
    # 遍历线程数量从1到1024，每次乘以2
    max_workers=1
    while [ $max_workers -le 1024 ]; do
        echo "Running with max_workers: $max_workers, iosize: $iosize"
        python3 read_opt_bigio.py "$directory" "$max_workers" "$iosize"
        
        max_workers=$((max_workers * 2))
    done
    
    echo "Finished testing IOSIZE: $iosize"
    echo
done

echo "All bigIO tests completed!"


# 定义要测试的IOSIZE列表
iosizes=("512B" "1KB" "2KB" "4KB" "8KB" "16KB")

# 遍历每个IOSIZE
for iosize in "${iosizes[@]}"; do
    # 根据iosize动态设置目录名
    directory="/mnt/beegfs/test_dir_${iosize}_10428"
    echo "Testing IOSIZE: $iosize"
    echo "Directory: $directory"
    
    # 遍历线程数量从1到1024，每次乘以2
    max_workers=1
    while [ $max_workers -le 1024 ]; do
        echo "Running with max_workers: $max_workers, iosize: $iosize"
        python3 read_opt_bigio.py "$directory" "$max_workers" "$iosize"
        
        max_workers=$((max_workers * 2))
    done
    
    echo "Finished testing IOSIZE: $iosize"
    echo
done

echo "All smallIO tests completed!"