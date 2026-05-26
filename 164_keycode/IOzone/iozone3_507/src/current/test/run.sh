#!/bin/sh
iozone -i $test -w -c -O -I -r 1m -s 10g

iozone -i 0 -i 1 -r 4k -t ${number_of_threads} -s ${file_size} -R ${output_file}

iozone -i 0 -i 1 -r 4k -t 1 -s 1G -R iozone_results.txt

./iozone -a -n 512m -g 4g -i 0 -i 1 -i 5 -f /mnt/beegfs/ -Rb ./iozone.xls

./iozone -a -n 512m -g 1g -i 0 -f /mnt/beegfs/iozone -Rb ./iozone.xls


iozone -i $test -w -c -O -I -r 1m -s 10g
iozone -i 2 -w -c -O -I -r 4K -s $Size -t $Thread -+n -+m /path/to/threadlist

iozone -i 1 -w -c -O -I -r 1m -s 10g
./iozone -i 0 -w -c -O -I -r 4k -s 1m -f /mnt/beegfs/iozone

iozone -i $test -w -c -O -I -r 1m -s 10g
./iozone -i 0 -i 1 -w -c -O -I -r 4k -s 1g -f /mnt/beegfs/iozone -Rb ./write_read.xls
./iozone -i 1 -w -c -O -I -r 4k -s 1g -f /mnt/beegfs/iozone2 -Rb ./write_read2.xls
./iozone -i 1 -w -c -O -I -r 4k -s 1m -f /mnt/beegfs/iozone -Rb ./write_read2.xls

# 先写 后读
./iozone -i 0 -w -c -O -I -r 4k -s 1g -f /mnt/beegfs/iozone -Rb ./write_250414.xls
./iozone -i 1 -w -c -O -I -r 4k -s 1g -f /mnt/beegfs/iozone -Rb ./read_250414.xls
```sh
-I

Use DIRECT IO if possible for all file operations. Tells the filesystem that all operations to the file are to bypass the buffer cache and go directly to disk. (not available on all platforms)
-O

Give results in operations per second.
```


# 写测试 

```sh
./iozone -i 0 -w -c -O -I -r 4k -s 1g -f /mnt/beegfs/iozone -Rb ./write_250414.xls

"Writer report"
        "4"
"1048576"   2102 

"Re-writer report"
        "4"
"1048576"   2178 

```
# 读测试
```sh
./iozone -i 1 -w -c -O -I -r 4k -s 1g -f /mnt/beegfs/iozone -Rb ./read_250414.xls
iozone test complete.
Excel output is below:

"Reader report"
        "4"
"1048576"   2107 

"Re-Reader report"
        "4"
"1048576"   2290
```

# 多线程全面测试 

```sh
./iozone -t 10 -s 1G -r 4k -w -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2 /mnt/beegfs/iozone_test/file3 /mnt/beegfs/iozone_test/file4 /mnt/beegfs/iozone_test/file5 /mnt/beegfs/iozone_test/file6 /mnt/beegfs/iozone_test/file7 /mnt/beegfs/iozone_test/file8 /mnt/beegfs/iozone_test/file9 /mnt/beegfs/iozone_test/file10 -Rb iozone_results.xls

        Children see throughput for 10 initial writers  = 3647896.16 kB/sec
        Parent sees throughput for 10 initial writers   = 2492917.39 kB/sec
        Min throughput per process                      =  359196.06 kB/sec 
        Max throughput per process                      =  369482.84 kB/sec
        Avg throughput per process                      =  364789.62 kB/sec
        Min xfer                                        = 1019388.00 kB

        Children see throughput for 10 rewriters        = 6024523.94 kB/sec
        Parent sees throughput for 10 rewriters         = 2557879.11 kB/sec
        Min throughput per process                      =  591650.62 kB/sec 
        Max throughput per process                      =  608613.88 kB/sec
        Avg throughput per process                      =  602452.39 kB/sec
        Min xfer                                        = 1019388.00 kB

        Children see throughput for 10 readers          = 5073124.22 kB/sec
        Parent sees throughput for 10 readers           = 4931935.90 kB/sec
        Min throughput per process                      =  480681.44 kB/sec 
        Max throughput per process                      =  531980.44 kB/sec
        Avg throughput per process                      =  507312.42 kB/sec
        Min xfer                                        =  947712.00 kB

        Children see throughput for 10 re-readers       = 5657825.62 kB/sec
        Parent sees throughput for 10 re-readers        = 5498340.90 kB/sec
        Min throughput per process                      =  555069.38 kB/sec 
        Max throughput per process                      =  573614.25 kB/sec
        Avg throughput per process                      =  565782.56 kB/sec
        Min xfer                                        = 1014784.00 kB

        Children see throughput for 10 reverse readers  =  115695.22 kB/sec
        Parent sees throughput for 10 reverse readers   =  115673.94 kB/sec
        Min throughput per process                      =   11323.47 kB/sec 
        Max throughput per process                      =   11793.07 kB/sec
        Avg throughput per process                      =   11569.52 kB/sec
        Min xfer                                        = 1006824.00 kB

        Children see throughput for 10 stride readers   =  318264.96 kB/sec
        Parent sees throughput for 10 stride readers    =  318120.86 kB/sec
        Min throughput per process                      =   31773.56 kB/sec 
        Max throughput per process                      =   31920.47 kB/sec
        Avg throughput per process                      =   31826.50 kB/sec
        Min xfer                                        = 1043780.00 kB

        Children see throughput for 10 random readers   =  117741.01 kB/sec
        Parent sees throughput for 10 random readers    =  117718.58 kB/sec
        Min throughput per process                      =   11389.50 kB/sec 
        Max throughput per process                      =   12084.58 kB/sec
        Avg throughput per process                      =   11774.10 kB/sec
        Min xfer                                        =  988264.00 kB

        Children see throughput for 10 mixed workload   =  113862.12 kB/sec
        Parent sees throughput for 10 mixed workload    =  110267.37 kB/sec
        Min throughput per process                      =   11157.36 kB/sec 
        Max throughput per process                      =   11596.93 kB/sec
        Avg throughput per process                      =   11386.21 kB/sec
        Min xfer                                        = 1008836.00 kB

        Children see throughput for 10 random writers   =  111719.32 kB/sec
        Parent sees throughput for 10 random writers    =  105477.45 kB/sec
        Min throughput per process                      =   10899.35 kB/sec 
        Max throughput per process                      =   11439.79 kB/sec
        Avg throughput per process                      =   11171.93 kB/sec
        Min xfer                                        =  999040.00 kB

        Children see throughput for 10 pwrite writers   = 4034473.00 kB/sec
        Parent sees throughput for 10 pwrite writers    = 2051159.47 kB/sec
        Min throughput per process                      =  389159.44 kB/sec 
        Max throughput per process                      =  448753.94 kB/sec
        Avg throughput per process                      =  403447.30 kB/sec
        Min xfer                                        =  909308.00 kB

        Children see throughput for 10 pread readers    = 5004193.84 kB/sec
        Parent sees throughput for 10 pread readers     = 4958279.62 kB/sec
        Min throughput per process                      =  483216.75 kB/sec 
        Max throughput per process                      =  509005.12 kB/sec
        Avg throughput per process                      =  500419.38 kB/sec
        Min xfer                                        =  995444.00 kB

        Children see throughput for 10 fwriters         = 6275277.31 kB/sec
        Parent sees throughput for 10 fwriters          = 2771678.19 kB/sec
        Min throughput per process                      =  595953.44 kB/sec 
        Max throughput per process                      =  668384.25 kB/sec
        Avg throughput per process                      =  627527.73 kB/sec
        Min xfer                                        = 1048576.00 kB

        Children see throughput for 10 freaders         = 5503340.38 kB/sec
        Parent sees throughput for 10 freaders          = 5304104.96 kB/sec
        Min throughput per process                      =  530795.56 kB/sec 
        Max throughput per process                      =  559242.31 kB/sec
        Avg throughput per process                      =  550334.04 kB/sec
        Min xfer                                        = 1048576.00 kB



"Throughput report Y-axis is type of test X-axis is number of processes"
    "Record size = 4 kBytes "
    "Output is in kBytes/sec"

    "  Initial write " 3647896.16 

    "        Rewrite " 6024523.94 

    "           Read " 5073124.22 

    "        Re-read " 5657825.62 

    "   Reverse Read "  115695.22 

    "    Stride read "  318264.96 

    "    Random read "  117741.01 

    " Mixed workload "  113862.12 

    "   Random write "  111719.32 

    "         Pwrite " 4034473.00 

    "          Pread " 5004193.84 

    "         Fwrite " 6275277.31 

    "          Fread " 5503340.38 


    iozone test complete.
```

# 单个线程
```sh
# 第一次
./iozone -t 1 -s 1G -r 4k -w -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 -Rb read1G_4k_t1_1.xls
        Children see throughput for  1 initial writers  =  477185.91 kB/sec
        Parent sees throughput for  1 initial writers   =  426858.59 kB/sec
        Min throughput per process                      =  477185.91 kB/sec 
        Max throughput per process                      =  477185.91 kB/sec
        Avg throughput per process                      =  477185.91 kB/sec
        Min xfer                                        = 1048576.00 kB

        Children see throughput for  1 rewriters        =  482114.97 kB/sec
        Parent sees throughput for  1 rewriters         =  430843.08 kB/sec
        Min throughput per process                      =  482114.97 kB/sec 
        Max throughput per process                      =  482114.97 kB/sec
        Avg throughput per process                      =  482114.97 kB/sec
        Min xfer                                        = 1048576.00 kB

        Children see throughput for  1 readers          =  597340.31 kB/sec
        Parent sees throughput for  1 readers           =  591290.04 kB/sec
        Min throughput per process                      =  597340.31 kB/sec 
        Max throughput per process                      =  597340.31 kB/sec
        Avg throughput per process                      =  597340.31 kB/sec
        Min xfer                                        = 1048576.00 kB

        Children see throughput for 1 re-readers        =  599390.75 kB/sec
        Parent sees throughput for 1 re-readers         =  595409.04 kB/sec
        Min throughput per process                      =  599390.75 kB/sec 
        Max throughput per process                      =  599390.75 kB/sec
        Avg throughput per process                      =  599390.75 kB/sec
        Min xfer                                        = 1048576.00 kB


"Throughput report Y-axis is type of test X-axis is number of processes"
    "Record size = 4 kBytes "
    "Output is in kBytes/sec"

    "  Initial write "  477185.91 

    "        Rewrite "  482114.97 

    "           Read "  597340.31 

    "        Re-read "  599390.75 


iozone test complete.

#第二次
./iozone -t 1 -s 1G -r 4k -w -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 -Rb read1G_4k_t1_2.xls
        Children see throughput for  1 initial writers  =  299977.56 kB/sec
        Parent sees throughput for  1 initial writers   =  277470.88 kB/sec
        Min throughput per process                      =  299977.56 kB/sec 
        Max throughput per process                      =  299977.56 kB/sec
        Avg throughput per process                      =  299977.56 kB/sec
        Min xfer                                        = 1048576.00 kB

        Children see throughput for  1 rewriters        =  479326.47 kB/sec
        Parent sees throughput for  1 rewriters         =  424390.70 kB/sec
        Min throughput per process                      =  479326.47 kB/sec 
        Max throughput per process                      =  479326.47 kB/sec
        Avg throughput per process                      =  479326.47 kB/sec
        Min xfer                                        = 1048576.00 kB

        Children see throughput for  1 readers          =  626289.19 kB/sec
        Parent sees throughput for  1 readers           =  619697.96 kB/sec
        Min throughput per process                      =  626289.19 kB/sec 
        Max throughput per process                      =  626289.19 kB/sec
        Avg throughput per process                      =  626289.19 kB/sec
        Min xfer                                        = 1048576.00 kB

        Children see throughput for 1 re-readers        =  577753.25 kB/sec
        Parent sees throughput for 1 re-readers         =  573904.36 kB/sec
        Min throughput per process                      =  577753.25 kB/sec 
        Max throughput per process                      =  577753.25 kB/sec
        Avg throughput per process                      =  577753.25 kB/sec
        Min xfer                                        = 1048576.00 kB



"Throughput report Y-axis is type of test X-axis is number of processes"
    "Record size = 4 kBytes "
    "Output is in kBytes/sec"

    "  Initial write "  299977.56 

    "        Rewrite "  479326.47 

    "           Read "  626289.19 

    "        Re-read "  577753.25 


    iozone test complete.


# 启动 -I
./iozone -t 1 -s 1G -r 4k -w -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 -Rb read1G_4k_t1_3.xls
        Children see throughput for  1 initial writers  =    8058.11 kB/sec
        Parent sees throughput for  1 initial writers   =    8057.50 kB/sec
        Min throughput per process                      =    8058.11 kB/sec 
        Max throughput per process                      =    8058.11 kB/sec
        Avg throughput per process                      =    8058.11 kB/sec
        Min xfer                                        = 1048576.00 kB

        Children see throughput for  1 rewriters        =    7738.82 kB/sec
        Parent sees throughput for  1 rewriters         =    7738.30 kB/sec
        Min throughput per process                      =    7738.82 kB/sec 
        Max throughput per process                      =    7738.82 kB/sec
        Avg throughput per process                      =    7738.82 kB/sec
        Min xfer                                        = 1048576.00 kB

        Children see throughput for  1 readers          =    8620.83 kB/sec
        Parent sees throughput for  1 readers           =    8620.48 kB/sec
        Min throughput per process                      =    8620.83 kB/sec 
        Max throughput per process                      =    8620.83 kB/sec
        Avg throughput per process                      =    8620.83 kB/sec
        Min xfer                                        = 1048576.00 kB

        Children see throughput for 1 re-readers        =    7923.50 kB/sec
        Parent sees throughput for 1 re-readers         =    7923.28 kB/sec
        Min throughput per process                      =    7923.50 kB/sec 
        Max throughput per process                      =    7923.50 kB/sec
        Avg throughput per process                      =    7923.50 kB/sec
        Min xfer                                        = 1048576.00 kB



"Throughput report Y-axis is type of test X-axis is number of processes"
    "Record size = 4 kBytes "
    "Output is in kBytes/sec"

    "  Initial write "    8058.11 

    "        Rewrite "    7738.82 

    "           Read "    8620.83 

    "        Re-read "    7923.50 


    iozone test complete.


# 第四次 启动 -O 显示 IOPS 
./iozone -t 1 -s 1G -r 4k -w -c -O -I -i 1 -F /mnt/beegfs/iozone_test/file1 -Rb read1G_4k_t1_4.xls

        Children see throughput for  1 readers          =    1994.33 ops/sec
        Parent sees throughput for  1 readers           =    1994.31 ops/sec
        Min throughput per process                      =    1994.33 ops/sec 
        Max throughput per process                      =    1994.33 ops/sec
        Avg throughput per process                      =    1994.33 ops/sec
        Min xfer                                        =  262144.00 ops

        Children see throughput for 1 re-readers        =    2032.95 ops/sec
        Parent sees throughput for 1 re-readers         =    2032.94 ops/sec
        Min throughput per process                      =    2032.95 ops/sec 
        Max throughput per process                      =    2032.95 ops/sec
        Avg throughput per process                      =    2032.95 ops/sec
        Min xfer                                        =  262144.00 ops



"Throughput report Y-axis is type of test X-axis is number of processes"
    "Record size = 4 kBytes "
    "Output is in ops/sec"

    "  Initial write "       0.00 

    "                "       0.00 

    "           Read "    1994.33 

    "        Re-read "    2032.95 


    iozone test complete.
# 第五次
./iozone -t 2 -s 1G -r 4k -w -c -O -I -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2 -Rb read1G_4k_t2_1.xls

# 第六次
./iozone -t 2 -s 1G -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2 -Rb read1G_4k_t2_2.xls

# 第七次

./iozone -t 4 -s 1G -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2  /mnt/beegfs/iozone_test/file3 /mnt/beegfs/iozone_test/file4 -Rb read1G_4k_t4_1.xls

# thread 6
./iozone -t 6 -s 1G -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2  /mnt/beegfs/iozone_test/file3 /mnt/beegfs/iozone_test/file4 /mnt/beegfs/iozone_test/file5 /mnt/beegfs/iozone_test/file6 -Rb read1G_4k_t6_1.xls

# thread 8

./iozone -t 6 -s 1G -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2  /mnt/beegfs/iozone_test/file3 /mnt/beegfs/iozone_test/file4 /mnt/beegfs/iozone_test/file5 /mnt/beegfs/iozone_test/file6 /mnt/beegfs/iozone_test/file7 /mnt/beegfs/iozone_test/file8 -Rb read1G_4k_t8_1.xls

# thread 10 
./iozone -t 6 -s 1G -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2  /mnt/beegfs/iozone_test/file3 /mnt/beegfs/iozone_test/file4 /mnt/beegfs/iozone_test/file5 /mnt/beegfs/iozone_test/file6 /mnt/beegfs/iozone_test/file7 /mnt/beegfs/iozone_test/file9 /mnt/beegfs/iozone_test/file10 -Rb read1G_4k_t10_1.xls


```

# 2个