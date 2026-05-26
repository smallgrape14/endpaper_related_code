#!/bin/bash
for testnum in {1..2}
do
    # for IOSIZE in "512B" "1K" "2K" "4K" "8K" "16K" "32K" "64K" "128K" "256K" "512K";do
    for IOSIZE in "32K" "64K";do #"256K" "512K" "1M" "2M" "4M" "8M" "512B" "1K" "2K" "4K" "8K" "16K";do

        for threadnum in 1; do
            # NVMe SSD
            # output_file="output_Local_NVMe.txt"
            # /usr/local/cuda/gds/tools/gdsio -f /data/gdstest/gdsio.0 -w $threadnum -d 0 -I 0 -x 0 -s 1G -i $IOSIZE >> $output_file
            
            # BeeGFS
            output_file="output_BeeGFS_0407IO.txt"

            /usr/local/cuda-12.4/gds/tools/gdsio -D /mnt/beegfs/gdsio_0728/dir_64MB -w $threadnum -d 0 -I 0 -x 0 -s 64M -i $IOSIZE >> $output_file
            # output_file="output_BeeGFS_0407IO_singlefile.txt"
            
            # /usr/local/cuda-12.4/gds/tools/gdsio -f /mnt/beegfs/gdsio_0728/gdsio.0 -w $threadnum -d 0 -I 0 -x 0 -s 2G -i $IOSIZE >> $output_file

        done
    done
done