#!/bin/bash

# ## Test1: 测试 IOSIZE=512B~16KB 的GDS read IOPS. read 
# output_file="output.txt"
# for testnum in {1..50}
# do
#     for IOSIZE in "512B" "1K" "2K" "4K" "8K" "16K"; do
#     # for IOSIZE in "512B"; do
#         for threadnum in 1 2 4 8 16 32 64 128 256; do
#         # for threadnum in 32; do
#             /usr/local/cuda-11.7/gds/tools/gdsio -f /mnt/beegfs/gdsio_0728/gdsio.0 -w $threadnum -d 0 -I 0 -x 0 -s 64M -i $IOSIZE >> $output_file
#         done
#     done
# done

## Test1: 测试 IOSIZE=512B~16KB 的GDS read IOPS. read 
read_dir="/mnt/beegfs/gdsio_0728/dir_64MB"
# read_dir="/mnt/beegfs/gdsio_0728/dir1"

output_file="output_0919.txt"
for testnum in {1..30}
do
    for IOSIZE in "512B" "1K" "2K" "4K" "8K" "16K"; do
    # for IOSIZE in "512B"; do
        for threadnum in 1 2 4 8 16 32 64 128 256; do
        # for threadnum in 32; do
            /usr/local/cuda-11.7/gds/tools/gdsio -D $read_dir -w $threadnum -d 0 -I 0 -x 0 -s 16K -i $IOSIZE >> $output_file
        done
    done
done