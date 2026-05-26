#!/bin/bash
# 读取文件列表并传递给 IOzone
find /mnt/beegfs/image_32KB_500 -type f > filelist.txt
mapfile -t files < filelist.txt
./iozone -i 1 -t 1 -F "${files[@]}" -r 4k -Rb single_thread_read.xls