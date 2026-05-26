#!/bin/sh

# thread 2
# echo "thread = 2 test"
# ./iozone -t 2 -s 1G -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2 -Rb read1G_4k_t2_2.xls

# thread 4
echo "thread = 4 test"
./iozone -t 4 -s 1G -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2  /mnt/beegfs/iozone_test/file3 /mnt/beegfs/iozone_test/file4 -Rb read1G_4k_t4_1.xls

# thread 6
echo "thread = 6 test"
./iozone -t 6 -s 1G -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2  /mnt/beegfs/iozone_test/file3 /mnt/beegfs/iozone_test/file4 /mnt/beegfs/iozone_test/file5 /mnt/beegfs/iozone_test/file6 -Rb read1G_4k_t6_1.xls

# thread 8
echo "thread = 8 test"
./iozone -t 8 -s 1G -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2  /mnt/beegfs/iozone_test/file3 /mnt/beegfs/iozone_test/file4 /mnt/beegfs/iozone_test/file5 /mnt/beegfs/iozone_test/file6 /mnt/beegfs/iozone_test/file7 /mnt/beegfs/iozone_test/file8 -Rb read1G_4k_t8_1.xls

# thread 10 
echo "thread = 10 test"
./iozone -t 10 -s 1G -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2  /mnt/beegfs/iozone_test/file3 /mnt/beegfs/iozone_test/file4 /mnt/beegfs/iozone_test/file5 /mnt/beegfs/iozone_test/file6 /mnt/beegfs/iozone_test/file7 /mnt/beegfs/iozone_test/file9 /mnt/beegfs/iozone_test/file10 -Rb read1G_4k_t10_1.xls

