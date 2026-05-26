#!/bin/bash

## Test1: 测试 IOSIZE=512B~16KB 的GDS read IOPS. read 
# output_file="output.txt"
# for testnum in {1..1}
# do
#     # for IOSIZE in "512B" "1K" "2K" "4K" "8K" "16K"; do
#     for IOSIZE in "32K" "64K" "128K" "256K" "512K" "1M" "2M" "4M" "8M" "16M" "32M" "64M"; do
#         # for threadnum in 1 2 4 8 16 32 64 128 256; do
#         # for threadnum in 256 128 64 32 16; do
#         # for threadnum in 1 2 4 8; do
#         for threadnum in 1; do

#             /usr/local/cuda-11.7/gds/tools/gdsio -f /mnt/beegfs/gdsio_0728/gdsio.0 -w $threadnum -d 0 -I 0 -x 0 -s 2G -i $IOSIZE >> $output_file
#             # /usr/local/cuda-11.7/gds/tools/gdsio -D /mnt/beegfs/gdsio_0728/dir_64MB -w $threadnum -d 0 -I 0 -x 0 -s 64M -i $IOSIZE >> $output_file

#         done
#     done
# done
# ## Write
# /usr/local/cuda-11.7/gds/tools/gdsio -D /mnt/nvme0/gdstest -w 1 -d 0 -I 1 -x 0 -s 1G -i 1G



for testnum in {1..2}
do
    # for IOSIZE in "4K"; do
    # for IOSIZE in "512B" "1K" "2K" "4K" "8K" "16K" "256B" "128B" "64B" "32B" "16B"; do
    for threadnum in 256;do #1 2 4 8 16 32 64 128 256; do
        for IOSIZE in "16K" "32K" "64K" "128K" "256K" "512K" "1M" "1K" "2K" "4K" "8K" ;do # "1M" "2M" "4M" "8M" "16M" "32M" "64M"; do
        # for threadnum in 1 2 4 8 16 32 64 128 256; do
        # for threadnum in 256 128 64 32 16; do
        # for threadnum in 1 2 4 8; do
        

            # /usr/local/cuda-11.7/gds/tools/gdsio -f /mnt/beegfs/gdsio_0728/gdsio.0 -w $threadnum -d 0 -I 0 -x 0 -s 2G -i $IOSIZE >> $output_file
            # NVME SSD
            # output_file="output_Local_Nvme.txt"
            # /usr/local/cuda-11.7/gds/tools/gdsio -f /mnt/nvme0/gdstest/gdsio.0 -w $threadnum -d 0 -I 0 -x 0 -s 1G -i $IOSIZE >> $output_file
            # 
            output_file="output_Beegfs_1201_night.txt"

            /usr/local/cuda-12.4/gds/tools/gdsio -D /mnt/beegfs/gdsio_0728/dir_64MB -w $threadnum -d 0 -I 0 -x 0 -s 64M -i $IOSIZE >> $output_file
            # /usr/local/cuda-12.4/gds/tools/gdsio -f /mnt/beegfs/gdsio_0728/gdsio.0 -w $threadnum -d 0 -I 0 -x 0 -s 2G -i $IOSIZE >> $output_file

        done
    done
done