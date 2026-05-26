## thread 1: 256M read IOPS =6K-IOPS

### 第一次
```SH
root@node9:/home/oem/xyp/Send_pkt/IOzone/iozone3_507/src/current/test# ./multi_thread.sh
Running test with 1 threads...
        Iozone: Performance Test of File I/O
                Version $Revision: 3.507 $
                Compiled for 64 bit mode.
                Build: linux-AMD64 

        Contributors:William Norcott, Don Capps, Isom Crawford, Kirby Collins
                     Al Slater, Scott Rhine, Mike Wisner, Ken Goss
                     Steve Landherr, Brad Smith, Mark Kelly, Dr. Alain CYR,
                     Randy Dunlap, Mark Montague, Dan Million, Gavin Brebner,
                     Jean-Marc Zucconi, Jeff Blomberg, Benny Halevy, Dave Boone,
                     Erik Habbinga, Kris Strecker, Walter Wong, Joshua Root,
                     Fabrice Bacchella, Zhenghua Xue, Qin Li, Darren Sawyer,
                     Vangel Bojaxhi, Ben England, Vikentsi Lapa,
                     Alexey Skidanov, Sudhir Kumar.

        Run began: Tue Apr 15 21:24:14 2025

        File size set to 262144 kB
        Record Size 4 kB
        Setting no_unlink
        OPS Mode. Output is in operations per second.
        O_DIRECT feature enabled
        Command line used: ./iozone -t 1 -s 256M -r 4k -w -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1
        Time Resolution = 0.000001 seconds.
        Processor cache size set to 1024 kBytes.
        Processor cache line size set to 32 bytes.
        File stride size set to 17 * record size.
        Throughput test with 1 process
        Each process writes a 262144 kByte file in 4 kByte records

        Children see throughput for  1 initial writers  =    3159.44 ops/sec
        Parent sees throughput for  1 initial writers   =    3157.97 ops/sec
        Min throughput per process                      =    3159.44 ops/sec 
        Max throughput per process                      =    3159.44 ops/sec
        Avg throughput per process                      =    3159.44 ops/sec
        Min xfer                                        =   65536.00 ops

        Children see throughput for  1 rewriters        =    2582.62 ops/sec
        Parent sees throughput for  1 rewriters         =    2581.64 ops/sec
        Min throughput per process                      =    2582.62 ops/sec 
        Max throughput per process                      =    2582.62 ops/sec
        Avg throughput per process                      =    2582.62 ops/sec
        Min xfer                                        =   65536.00 ops

        Children see throughput for  1 readers          =    6311.43 ops/sec
        Parent sees throughput for  1 readers           =    6309.48 ops/sec
        Min throughput per process                      =    6311.43 ops/sec 
        Max throughput per process                      =    6311.43 ops/sec
        Avg throughput per process                      =    6311.43 ops/sec
        Min xfer                                        =   65536.00 ops

        Children see throughput for 1 re-readers        =    2398.97 ops/sec
        Parent sees throughput for 1 re-readers         =    2398.68 ops/sec
        Min throughput per process                      =    2398.97 ops/sec 
        Max throughput per process                      =    2398.97 ops/sec
        Avg throughput per process                      =    2398.97 ops/sec
        Min xfer                                        =   65536.00 ops



iozone test complete.
Test with 1 threads completed. Results saved to result_modify_setting/noclose_randomread/read_256M_4k_t1.xls
All tests completed.

```

### 第二次

```sh
root@node9:/home/oem/xyp/Send_pkt/IOzone/iozone3_507/src/current/test# ./multi_thread.sh
Running test with 1 threads...
        Iozone: Performance Test of File I/O
                Version $Revision: 3.507 $
                Compiled for 64 bit mode.
                Build: linux-AMD64 

        Contributors:William Norcott, Don Capps, Isom Crawford, Kirby Collins
                     Al Slater, Scott Rhine, Mike Wisner, Ken Goss
                     Steve Landherr, Brad Smith, Mark Kelly, Dr. Alain CYR,
                     Randy Dunlap, Mark Montague, Dan Million, Gavin Brebner,
                     Jean-Marc Zucconi, Jeff Blomberg, Benny Halevy, Dave Boone,
                     Erik Habbinga, Kris Strecker, Walter Wong, Joshua Root,
                     Fabrice Bacchella, Zhenghua Xue, Qin Li, Darren Sawyer,
                     Vangel Bojaxhi, Ben England, Vikentsi Lapa,
                     Alexey Skidanov, Sudhir Kumar.

        Run began: Tue Apr 15 21:26:56 2025

        File size set to 262144 kB
        Record Size 4 kB
        Setting no_unlink
        OPS Mode. Output is in operations per second.
        O_DIRECT feature enabled
        Command line used: ./iozone -t 1 -s 256M -r 4k -w -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1
        Time Resolution = 0.000001 seconds.         beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2 /mnt/beegfs/iozone_test/file3 /mnt/beegfs/iozone_test/file4 /mnt/beegfs/iozone_test/file5 /mnt/beegfs/iozone_test/file6 /mnt/beegfs/iozone_test/file7 /mnt/beegfs/iozone_test/file8 /mnt/beegfs/iozone_test/file9 /mnt/beegfs/iozone_test/file10 -Rb result_modif
        Children see throughput for  1 initial writers      =    7012.85 ops/sec
        Parent sees throughput for  1 initial writers       =    7005.78 ops/sec
        Min throughput per process                 =    7012.85 ops/sec 
        Max throughput per process                 =    7012.85 ops/sec
        Avg throughput per process                 =    7012.85 ops/sec
        Min xfer         =   65536.00 ops

        Children see throughput for  1 rewriters        =    2350.10 ops/sec
        Parent sees throughput for  1 rewriters         =    2349.33 ops/sec
        Min throughput per process                      =    2350.10 ops/sec 
        Max throughput per process                      =    2350.10 ops/sec
        Avg throughput per process                      =    2350.10 ops/sec
        Min xfer                                        =   65536.00 ops

        Children see throughput for  1 readers          =    2761.33 ops/sec
        Parent sees throughput for  1 readers           =    2751.61 ops/sec
        Min throughput per process                      =    2761.33 ops/sec 
        Max throughput per process                      =    2761.33 ops/sec
        Avg throughput per process                      =    2761.33 ops/sec
        Min xfer                                        =   65536.00 ops

        Children see throughput for 1 re-readers        =    3147.20 ops/sec
        Parent sees throughput for 1 re-readers         =    3146.22 ops/sec
        Min throughput per process                      =    3147.20 ops/sec 
        Max throughput per process                      =    3147.20 ops/sec
        Avg throughput per process                      =    3147.20 ops/sec
        Min xfer                                        =   65536.00 ops



iozone test complete.
Test with 1 threads completed. Results saved to result_modify_setting/noclose_randomread/read_256M_4k_t1.xls
All tests completed.
```

### 第三次
```sh
root@node9:/home/oem/xyp/Send_pkt/IOzone/iozone3_507/src/current/test# ./multi_thread.sh
Running test with 1 threads...
        Iozone: Performance Test of File I/O
                Version $Revision: 3.507 $
                Compiled for 64 bit mode.
                Build: linux-AMD64 

        Contributors:William Norcott, Don Capps, Isom Crawford, Kirby Collins
                     Al Slater, Scott Rhine, Mike Wisner, Ken Goss
                     Steve Landherr, Brad Smith, Mark Kelly, Dr. Alain CYR,
                     Randy Dunlap, Mark Montague, Dan Million, Gavin Brebner,
                     Jean-Marc Zucconi, Jeff Blomberg, Benny Halevy, Dave Boone,
                     Erik Habbinga, Kris Strecker, Walter Wong, Joshua Root,
                     Fabrice Bacchella, Zhenghua Xue, Qin Li, Darren Sawyer,
                     Vangel Bojaxhi, Ben England, Vikentsi Lapa,
                     Alexey Skidanov, Sudhir Kumar.

        Run began: Tue Apr 15 21:29:51 2025

        File size set to 262144 kB
        Record Size 4 kB
        Setting no_unlink
        OPS Mode. Output is in operations per second.
        O_DIRECT feature enabled
        Command line used: ./iozone -t 1 -s 256M -r 4k -w -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1
        Time Resolution = 0.000001 seconds.
        Processor cache size set to 1024 kBytes.
        Processor cache line size set to 32 bytes.
        File stride size set to 17 * record size.
        Throughput test with 1 process
        Each process writes a 262144 kByte file in 4 kByte records

        Children see throughput for  1 initial writers  =    2613.57 ops/sec
        Parent sees throughput for  1 initial writers   =    2612.62 ops/sec
        Min throughput per process                      =    2613.57 ops/sec 
        Max throughput per process                      =    2613.57 ops/sec
        Avg throughput per process                      =    2613.57 ops/sec
        Min xfer                                        =   65536.00 ops

        Children see throughput for  1 rewriters        =    2809.47 ops/sec
        Parent sees throughput for  1 rewriters         =    2808.38 ops/sec
        Min throughput per process                      =    2809.47 ops/sec 
        Max throughput per process                      =    2809.47 ops/sec
        Avg throughput per process                      =    2809.47 ops/sec
        Min xfer                                        =   65536.00 ops

        Children see throughput for  1 readers          =    2513.37 ops/sec
        Parent sees throughput for  1 readers           =    2512.61 ops/sec
        Min throughput per process                      =    2513.37 ops/sec 
        Max throughput per process                      =    2513.37 ops/sec
        Avg throughput per process                      =    2513.37 ops/sec
        Min xfer                                        =   65536.00 ops

        Children see throughput for 1 re-readers        =    2047.24 ops/sec
        Parent sees throughput for 1 re-readers         =    2047.01 ops/sec
        Min throughput per process                      =    2047.24 ops/sec 
        Max throughput per process                      =    2047.24 ops/sec
        Avg throughput per process                      =    2047.24 ops/sec
        Min xfer                                        =   65536.00 ops



iozone test complete.
Test with 1 threads completed. Results saved to result_modify_setting/noclose_randomread/read_256M_4k_t1.xls
All tests completed.
```


## thread 1 :1G
```sh
./iozone -t 1 -s 1G -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 -Rb result_modify_setting/read1G_4k_t1.xls
        Run began: Tue Apr 15 20:40:02 2025

        File size set to 1048576 kB
        Record Size 4 kB
        Setting no_unlink
        Include close in write timing
        OPS Mode. Output is in operations per second.
        O_DIRECT feature enabled
        Excel chart generation enabled
        Command line used: ./iozone -t 1 -s 1G -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 -Rb result_modify_setting/read1G_4k_t1.xls
        Time Resolution = 0.000001 seconds.
        Processor cache size set to 1024 kBytes.
        Processor cache line size set to 32 bytes.
        File stride size set to 17 * record size.
        Throughput test with 1 process
        Each process writes a 1048576 kByte file in 4 kByte records

        Children see throughput for  1 initial writers  =    2632.07 ops/sec
        Parent sees throughput for  1 initial writers   =    2632.04 ops/sec
        Min throughput per process                      =    2632.07 ops/sec 
        Max throughput per process                      =    2632.07 ops/sec
        Avg throughput per process                      =    2632.07 ops/sec
        Min xfer                                        =  262144.00 ops

        Children see throughput for  1 rewriters        =    2527.00 ops/sec
        Parent sees throughput for  1 rewriters         =    2526.97 ops/sec
        Min throughput per process                      =    2527.00 ops/sec 
        Max throughput per process                      =    2527.00 ops/sec
        Avg throughput per process                      =    2527.00 ops/sec
        Min xfer                                        =  262144.00 ops

        Children see throughput for  1 readers          =    7629.02 ops/sec
        Parent sees throughput for  1 readers           =    7628.81 ops/sec
        Min throughput per process                      =    7629.02 ops/sec 
        Max throughput per process                      =    7629.02 ops/sec
        Avg throughput per process                      =    7629.02 ops/sec
        Min xfer                                        =  262144.00 ops

        Children see throughput for 1 re-readers        =    2603.66 ops/sec
        Parent sees throughput for 1 re-readers         =    2603.64 ops/sec
        Min throughput per process                      =    2603.66 ops/sec 
        Max throughput per process                      =    2603.66 ops/sec
        Avg throughput per process                      =    2603.66 ops/sec
        Min xfer                                        =  262144.00 ops



"Throughput report Y-axis is type of test X-axis is number of processes"
"Record size = 4 kBytes "
"Output is in ops/sec"

"  Initial write "    2632.07 

"        Rewrite "    2527.00 

"           Read "    7629.02 

"        Re-read "    2603.66 


iozone test complete.
Test with 1 threads completed. Results saved to result_modify_setting/read1G_4k_t1.xls
All tests completed.
```
## thread 2 : 256M
```sh
./iozone -t 2 -s 256M -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2 -Rb result_modify_setting/read_256M_4k_t2.xls

root@node9:/home/oem/xyp/Send_pkt/IOzone/iozone3_507/src/current/test# ./multi_thread.sh
Running test with 2 threads...
        Iozone: Performance Test of File I/O
                Version $Revision: 3.507 $
                Compiled for 64 bit mode.
                Build: linux-AMD64 

        Contributors:William Norcott, Don Capps, Isom Crawford, Kirby Collins
                     Al Slater, Scott Rhine, Mike Wisner, Ken Goss
                     Steve Landherr, Brad Smith, Mark Kelly, Dr. Alain CYR,
                     Randy Dunlap, Mark Montague, Dan Million, Gavin Brebner,
                     Jean-Marc Zucconi, Jeff Blomberg, Benny Halevy, Dave Boone,
                     Erik Habbinga, Kris Strecker, Walter Wong, Joshua Root,
                     Fabrice Bacchella, Zhenghua Xue, Qin Li, Darren Sawyer,
                     Vangel Bojaxhi, Ben England, Vikentsi Lapa,
                     Alexey Skidanov, Sudhir Kumar.

        Run began: Tue Apr 15 20:49:26 2025

        File size set to 262144 kB
        Record Size 4 kB
        Setting no_unlink
        Include close in write timing
        OPS Mode. Output is in operations per second.
        O_DIRECT feature enabled
        Excel chart generation enabled
        Command line used: ./iozone -t 2 -s 256M -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2 -Rb result_modify_setting/read_256M_4k_t2.xls
        Time Resolution = 0.000001 seconds.
        Processor cache size set to 1024 kBytes.
        Processor cache line size set to 32 bytes.
        File stride size set to 17 * record size.
        Throughput test with 2 processes
        Each process writes a 262144 kByte file in 4 kByte records

        Children see throughput for  2 initial writers  =    4000.45 ops/sec
        Parent sees throughput for  2 initial writers   =    3999.67 ops/sec
        Min throughput per process                      =    2000.08 ops/sec 
        Max throughput per process                      =    2000.37 ops/sec
        Avg throughput per process                      =    2000.23 ops/sec
        Min xfer                                        =   65527.00 ops

        Children see throughput for  2 rewriters        =    4077.80 ops/sec
        Parent sees throughput for  2 rewriters         =    4077.59 ops/sec
        Min throughput per process                      =    2037.13 ops/sec 
        Max throughput per process                      =    2040.68 ops/sec
        Avg throughput per process                      =    2038.90 ops/sec
        Min xfer                                        =   65425.00 ops

        Children see throughput for  2 readers          =    4615.01 ops/sec
        Parent sees throughput for  2 readers           =    4614.61 ops/sec
        Min throughput per process                      =    2166.85 ops/sec 
        Max throughput per process                      =    2448.15 ops/sec
        Avg throughput per process                      =    2307.50 ops/sec
        Min xfer                                        =   58008.00 ops

        Children see throughput for 2 re-readers        =    3860.69 ops/sec
        Parent sees throughput for 2 re-readers         =    3860.48 ops/sec
        Min throughput per process                      =    1852.51 ops/sec 
        Max throughput per process                      =    2008.18 ops/sec
        Avg throughput per process                      =    1930.34 ops/sec
        Min xfer                                        =   60458.00 ops



"Throughput report Y-axis is type of test X-axis is number of processes"
"Record size = 4 kBytes "
"Output is in ops/sec"

"  Initial write "    4000.45 

"        Rewrite "    4077.80 

"           Read "    4615.01 

"        Re-read "    3860.69 


iozone test complete.
Test with 2 threads completed. Results saved to result_modify_setting/read_256M_4k_t2.xls
All tests completed.
```
## thread 1 : 256M
```sh
./iozone -t 1 -s 256M -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 -Rb result_modify_setting/read_256M_4k_t1.xls

root@node9:/home/oem/xyp/Send_pkt/IOzone/iozone3_507/src/current/test# ./multi_thread.sh
Running test with 1 threads...
        Iozone: Performance Test of File I/O
                Version $Revision: 3.507 $
                Compiled for 64 bit mode.
                Build: linux-AMD64 

        Contributors:William Norcott, Don Capps, Isom Crawford, Kirby Collins
                     Al Slater, Scott Rhine, Mike Wisner, Ken Goss
                     Steve Landherr, Brad Smith, Mark Kelly, Dr. Alain CYR,
                     Randy Dunlap, Mark Montague, Dan Million, Gavin Brebner,
                     Jean-Marc Zucconi, Jeff Blomberg, Benny Halevy, Dave Boone,
                     Erik Habbinga, Kris Strecker, Walter Wong, Joshua Root,
                     Fabrice Bacchella, Zhenghua Xue, Qin Li, Darren Sawyer,
                     Vangel Bojaxhi, Ben England, Vikentsi Lapa,
                     Alexey Skidanov, Sudhir Kumar.

        Run began: Tue Apr 15 20:51:55 2025

        File size set to 262144 kB
        Record Size 4 kB
        Setting no_unlink
        Include close in write timing
        OPS Mode. Output is in operations per second.
        O_DIRECT feature enabled
        Excel chart generation enabled
        Command line used: ./iozone -t 1 -s 256M -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 -Rb result_modify_setting/read_256M_4k_t1.xls
        Time Resolution = 0.000001 seconds.
        Processor cache size set to 1024 kBytes.
        Processor cache line size set to 32 bytes.
        File stride size set to 17 * record size.
        Throughput test with 1 process
        Each process writes a 262144 kByte file in 4 kByte records

        Children see throughput for  1 initial writers  =    1908.07 ops/sec
        Parent sees throughput for  1 initial writers   =    1908.01 ops/sec
        Min throughput per process                      =    1908.07 ops/sec 
        Max throughput per process                      =    1908.07 ops/sec
        Avg throughput per process                      =    1908.07 ops/sec
        Min xfer                                        =   65536.00 ops

        Children see throughput for  1 rewriters        =    2067.34 ops/sec
        Parent sees throughput for  1 rewriters         =    2067.27 ops/sec
        Min throughput per process                      =    2067.34 ops/sec 
        Max throughput per process                      =    2067.34 ops/sec
        Avg throughput per process                      =    2067.34 ops/sec
        Min xfer                                        =   65536.00 ops

        Children see throughput for  1 readers          =    7032.12 ops/sec
        Parent sees throughput for  1 readers           =    7031.37 ops/sec
        Min throughput per process                      =    7032.12 ops/sec 
        Max throughput per process                      =    7032.12 ops/sec
        Avg throughput per process                      =    7032.12 ops/sec
        Min xfer                                        =   65536.00 ops

        Children see throughput for 1 re-readers        =    2046.51 ops/sec
        Parent sees throughput for 1 re-readers         =    2046.45 ops/sec
        Min throughput per process                      =    2046.51 ops/sec 
        Max throughput per process                      =    2046.51 ops/sec
        Avg throughput per process                      =    2046.51 ops/sec
        Min xfer                                        =   65536.00 ops



"Throughput report Y-axis is type of test X-axis is number of processes"
"Record size = 4 kBytes "
"Output is in ops/sec"

"  Initial write "    1908.07 

"        Rewrite "    2067.34 

"           Read "    7032.12 

"        Re-read "    2046.51 


iozone test complete.
Test with 1 threads completed. Results saved to result_modify_setting/read_256M_4k_t1.xls
All tests completed.
```
## thread 4
```sh
./iozone -t 4 -s 256M -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2 /mnt/beegfs/iozone_test/file3 /mnt/beegfs/iozone_test/file4 -Rb result_modify_setting/read_256M_4k_t4.xls

root@node9:/home/oem/xyp/Send_pkt/IOzone/iozone3_507/src/current/test# ./multi_thread.sh
Running test with 4 threads...
        Iozone: Performance Test of File I/O
                Version $Revision: 3.507 $
                Compiled for 64 bit mode.
                Build: linux-AMD64 

        Contributors:William Norcott, Don Capps, Isom Crawford, Kirby Collins
                     Al Slater, Scott Rhine, Mike Wisner, Ken Goss
                     Steve Landherr, Brad Smith, Mark Kelly, Dr. Alain CYR,
                     Randy Dunlap, Mark Montague, Dan Million, Gavin Brebner,
                     Jean-Marc Zucconi, Jeff Blomberg, Benny Halevy, Dave Boone,
                     Erik Habbinga, Kris Strecker, Walter Wong, Joshua Root,
                     Fabrice Bacchella, Zhenghua Xue, Qin Li, Darren Sawyer,
                     Vangel Bojaxhi, Ben England, Vikentsi Lapa,
                     Alexey Skidanov, Sudhir Kumar.

        Run began: Tue Apr 15 20:55:17 2025

        File size set to 262144 kB
        Record Size 4 kB
        Setting no_unlink
        Include close in write timing
        OPS Mode. Output is in operations per second.
        O_DIRECT feature enabled
        Excel chart generation enabled
        Command line used: ./iozone -t 4 -s 256M -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2 /mnt/beegfs/iozone_test/file3 /mnt/beegfs/iozone_test/file4 -Rb result_modify_setting/read_256M_4k_t4.xls
        Time Resolution = 0.000001 seconds.
        Processor cache size set to 1024 kBytes.
        Processor cache line size set to 32 bytes.
        File stride size set to 17 * record size.
        Throughput test with 4 processes
        Each process writes a 262144 kByte file in 4 kByte records

        Children see throughput for  4 initial writers  =    8857.66 ops/sec
        Parent sees throughput for  4 initial writers   =    8784.32 ops/sec
        Min throughput per process                      =    2204.07 ops/sec 
        Max throughput per process                      =    2224.36 ops/sec
        Avg throughput per process                      =    2214.41 ops/sec
        Min xfer                                        =   64992.00 ops

        Children see throughput for  4 rewriters        =   12188.07 ops/sec
        Parent sees throughput for  4 rewriters         =   12179.77 ops/sec
        Min throughput per process                      =    2909.09 ops/sec 
        Max throughput per process                      =    3362.71 ops/sec
        Avg throughput per process                      =    3047.02 ops/sec
        Min xfer                                        =   56701.00 ops

        Children see throughput for  4 readers          =    9892.25 ops/sec
        Parent sees throughput for  4 readers           =    9891.28 ops/sec
        Min throughput per process                      =    2464.02 ops/sec 
        Max throughput per process                      =    2493.25 ops/sec
        Avg throughput per process                      =    2473.06 ops/sec
        Min xfer                                        =   64770.00 ops

        Children see throughput for 4 re-readers        =   18452.05 ops/sec
        Parent sees throughput for 4 re-readers         =   18448.37 ops/sec
        Min throughput per process                      =    4296.99 ops/sec 
        Max throughput per process                      =    5119.90 ops/sec
        Avg throughput per process                      =    4613.01 ops/sec
        Min xfer                                        =   55006.00 ops



"Throughput report Y-axis is type of test X-axis is number of processes"
"Record size = 4 kBytes "
"Output is in ops/sec"

"  Initial write "    8857.66 

"        Rewrite "   12188.07 

"           Read "    9892.25 

"        Re-read "   18452.05 


iozone test complete.
Test with 4 threads completed. Results saved to result_modify_setting/read_256M_4k_t4.xls
```
## thread 6 

```sh
./iozone -t 6 -s 256M -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2 /mnt/beegfs/iozone_test/file3 /mnt/beegfs/iozone_test/file4 /mnt/beegfs/iozone_test/file5 /mnt/beegfs/iozone_test/file6 -Rb result_modify_setting/read_256M_4k_t6.xls

Running test with 6 threads...
        Iozone: Performance Test of File I/O
                Version $Revision: 3.507 $
                Compiled for 64 bit mode.
                Build: linux-AMD64 

        Contributors:William Norcott, Don Capps, Isom Crawford, Kirby Collins
                     Al Slater, Scott Rhine, Mike Wisner, Ken Goss
                     Steve Landherr, Brad Smith, Mark Kelly, Dr. Alain CYR,
                     Randy Dunlap, Mark Montague, Dan Million, Gavin Brebner,
                     Jean-Marc Zucconi, Jeff Blomberg, Benny Halevy, Dave Boone,
                     Erik Habbinga, Kris Strecker, Walter Wong, Joshua Root,
                     Fabrice Bacchella, Zhenghua Xue, Qin Li, Darren Sawyer,
                     Vangel Bojaxhi, Ben England, Vikentsi Lapa,
                     Alexey Skidanov, Sudhir Kumar.

        Run began: Tue Apr 15 20:56:55 2025

        File size set to 262144 kB
        Record Size 4 kB
        Setting no_unlink
        Include close in write timing
        OPS Mode. Output is in operations per second.
        O_DIRECT feature enabled
        Excel chart generation enabled
        Command line used: ./iozone -t 6 -s 256M -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2 /mnt/beegfs/iozone_test/file3 /mnt/beegfs/iozone_test/file4 /mnt/beegfs/iozone_test/file5 /mnt/beegfs/iozone_test/file6 -Rb result_modify_setting/read_256M_4k_t6.xls
        Time Resolution = 0.000001 seconds.
        Processor cache size set to 1024 kBytes.
        Processor cache line size set to 32 bytes.
        File stride size set to 17 * record size.
        Throughput test with 6 processes
        Each process writes a 262144 kByte file in 4 kByte records

        Children see throughput for  6 initial writers  =   17313.49 ops/sec
        Parent sees throughput for  6 initial writers   =   16878.06 ops/sec
        Min throughput per process                      =    2825.58 ops/sec 
        Max throughput per process                      =    2923.84 ops/sec
        Avg throughput per process                      =    2885.58 ops/sec
        Min xfer                                        =   63333.00 ops

        Children see throughput for  6 rewriters        =   17844.77 ops/sec
        Parent sees throughput for  6 rewriters         =   17833.26 ops/sec
        Min throughput per process                      =    2900.36 ops/sec 
        Max throughput per process                      =    3021.37 ops/sec
        Avg throughput per process                      =    2974.13 ops/sec
        Min xfer                                        =   62917.00 ops

        Children see throughput for  6 readers          =   13352.45 ops/sec
        Parent sees throughput for  6 readers           =   13351.51 ops/sec
        Min throughput per process                      =    2216.87 ops/sec 
        Max throughput per process                      =    2234.97 ops/sec
        Avg throughput per process                      =    2225.41 ops/sec
        Min xfer                                        =   65009.00 ops

        Children see throughput for 6 re-readers        =   14163.73 ops/sec
        Parent sees throughput for 6 re-readers         =   14162.86 ops/sec
        Min throughput per process                      =    2345.98 ops/sec 
        Max throughput per process                      =    2375.12 ops/sec
        Avg throughput per process                      =    2360.62 ops/sec
        Min xfer                                        =   64736.00 ops



"Throughput report Y-axis is type of test X-axis is number of processes"
"Record size = 4 kBytes "
"Output is in ops/sec"

"  Initial write "   17313.49 

"        Rewrite "   17844.77 

"           Read "   13352.45 

"        Re-read "   14163.73 


iozone test complete.
Test with 6 threads completed. Results saved to result_modify_setting/read_256M_4k_t6.xls
```
## thread 8: 256M
```sh
./iozone -t 8 -s 256M -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2 /mnt/beegfs/iozone_test/file3 /mnt/beegfs/iozone_test/file4 /mnt/beegfs/iozone_test/file5 /mnt/beegfs/iozone_test/file6 /mnt/beegfs/iozone_test/file7 /mnt/beegfs/iozone_test/file8 -Rb result_modify_setting/read_256M_4k_t8.xls

Running test with 8 threads...
        Iozone: Performance Test of File I/O
                Version $Revision: 3.507 $
                Compiled for 64 bit mode.
                Build: linux-AMD64 

        Contributors:William Norcott, Don Capps, Isom Crawford, Kirby Collins
                     Al Slater, Scott Rhine, Mike Wisner, Ken Goss
                     Steve Landherr, Brad Smith, Mark Kelly, Dr. Alain CYR,
                     Randy Dunlap, Mark Montague, Dan Million, Gavin Brebner,
                     Jean-Marc Zucconi, Jeff Blomberg, Benny Halevy, Dave Boone,
                     Erik Habbinga, Kris Strecker, Walter Wong, Joshua Root,
                     Fabrice Bacchella, Zhenghua Xue, Qin Li, Darren Sawyer,
                     Vangel Bojaxhi, Ben England, Vikentsi Lapa,
                     Alexey Skidanov, Sudhir Kumar.

 Run began: Tue Apr 15 20:58:47 2025

        File size set to 262144 kB
        Record Size 4 kB
        Setting no_unlink
        Include close in write timing
        OPS Mode. Output is in operations per second.
        O_DIRECT feature enabled
        Excel chart generation enabled
        Command line used: ./iozone -t 8 -s 256M -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2 /mnt/beegfs/iozone_test/file3 /mnt/beegfs/iozone_test/file4 /mnt/beegfs/iozone_test/file5 /mnt/beegfs/iozone_test/file6 /mnt/beegfs/iozone_test/file7 /mnt/beegfs/iozone_test/file8 -Rb result_modify_setting/read_256M_4k_t8.xls
        Time Resolution = 0.000001 seconds.
        Processor cache size set to 1024 kBytes.
        Processor cache line size set to 32 bytes.
        File stride size set to 17 * record size.
        Throughput test with 8 processes
        Each process writes a 262144 kByte file in 4 kByte records

        Children see throughput for  8 initial writers  =   26601.73 ops/sec
        Parent sees throughput for  8 initial writers   =   25537.33 ops/sec
        Min throughput per process                      =    3276.03 ops/sec 
        Max throughput per process                      =    3362.10 ops/sec
        Avg throughput per process                      =    3325.22 ops/sec
        Min xfer                                        =   63861.00 ops

        Children see throughput for  8 rewriters        =   19256.50 ops/sec
        Parent sees throughput for  8 rewriters         =   19254.83 ops/sec
        Min throughput per process                      =    2347.72 ops/sec 
        Max throughput per process                      =    2442.34 ops/sec
        Avg throughput per process                      =    2407.06 ops/sec
        Min xfer                                        =   63002.00 ops

        Children see throughput for  8 readers          =   22300.55 ops/sec
        Parent sees throughput for  8 readers           =   22297.80 ops/sec
        Min throughput per process                      =    2744.46 ops/sec 
        Max throughput per process                      =    2875.93 ops/sec
        Avg throughput per process                      =    2787.57 ops/sec
        Min xfer                                        =   62543.00 ops

        Children see throughput for 8 re-readers        =   16810.52 ops/sec
        Parent sees throughput for 8 re-readers         =   16809.24 ops/sec
        Min throughput per process                      =    2067.73 ops/sec 
        Max throughput per process                      =    2133.63 ops/sec
        Avg throughput per process                      =    2101.32 ops/sec
        Min xfer                                        =   63516.00 ops



"Throughput report Y-axis is type of test X-axis is number of processes"
"Record size = 4 kBytes "
"Output is in ops/sec"

"  Initial write "   26601.73 

"        Rewrite "   19256.50 

"           Read "   22300.55 

"        Re-read "   16810.52 


iozone test complete.
Test with 8 threads completed. Results saved to result_modify_setting/read_256M_4k_t8.xls
```
## thread 10
```sh
./iozone -t 10 -s 256M -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2 /mnt/beegfs/iozone_test/file3 /mnt/beegfs/iozone_test/file4 /mnt/beegfs/iozone_test/file5 /mnt/beegfs/iozone_test/file6 /mnt/beegfs/iozone_test/file7 /mnt/beegfs/iozone_test/file8 /mnt/beegfs/iozone_test/file9 /mnt/beegfs/iozone_test/file10 -Rb result_modify_setting/read_256M_4k_t10.xls

Running test with 10 threads...
        Iozone: Performance Test of File I/O
                Version $Revision: 3.507 $
                Compiled for 64 bit mode.
                Build: linux-AMD64 

        Contributors:William Norcott, Don Capps, Isom Crawford, Kirby Collins
                     Al Slater, Scott Rhine, Mike Wisner, Ken Goss
                     Steve Landherr, Brad Smith, Mark Kelly, Dr. Alain CYR,
                     Randy Dunlap, Mark Montague, Dan Million, Gavin Brebner,
                     Jean-Marc Zucconi, Jeff Blomberg, Benny Halevy, Dave Boone,
                     Erik Habbinga, Kris Strecker, Walter Wong, Joshua Root,
                     Fabrice Bacchella, Zhenghua Xue, Qin Li, Darren Sawyer,
                     Vangel Bojaxhi, Ben England, Vikentsi Lapa,
                     Alexey Skidanov, Sudhir Kumar.

        Run began: Tue Apr 15 21:00:38 2025

        File size set to 262144 kB
        Record Size 4 kB
        Setting no_unlink
        Include close in write timing
        OPS Mode. Output is in operations per second.
        O_DIRECT feature enabled
        Excel chart generation enabled
        Command line used: ./iozone -t 10 -s 256M -r 4k -w -c -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1 /mnt/beegfs/iozone_test/file2 /mnt/beegfs/iozone_test/file3 /mnt/beegfs/iozone_test/file4 /mnt/beegfs/iozone_test/file5 /mnt/beegfs/iozone_test/file6 /mnt/beegfs/iozone_test/file7 /mnt/beegfs/iozone_test/file8 /mnt/beegfs/iozone_test/file9 /mnt/beegfs/iozone_test/file10 -Rb result_modify_setting/read_256M_4k_t10.xls
        Time Resolution = 0.000001 seconds.
        Processor cache size set to 1024 kBytes.
        Processor cache line size set to 32 bytes.
        File stride size set to 17 * record size.
        Throughput test with 10 processes
        Each process writes a 262144 kByte file in 4 kByte records

        Children see throughput for 10 initial writers  =   26067.23 ops/sec
        Parent sees throughput for 10 initial writers   =   25647.89 ops/sec
        Min throughput per process                      =    2586.25 ops/sec 
        Max throughput per process                      =    2625.53 ops/sec
        Avg throughput per process                      =    2606.72 ops/sec
        Min xfer                                        =   64555.00 ops

        Children see throughput for 10 rewriters        =   27088.62 ops/sec
        Parent sees throughput for 10 rewriters         =   27086.24 ops/sec
        Min throughput per process                      =    2670.97 ops/sec 
        Max throughput per process                      =    2751.17 ops/sec
        Avg throughput per process                      =    2708.86 ops/sec
        Min xfer                                        =   63631.00 ops

        Children see throughput for 10 readers          =   27040.54 ops/sec
        Parent sees throughput for 10 readers           =   27015.81 ops/sec
        Min throughput per process                      =    2657.80 ops/sec 
        Max throughput per process                      =    2754.60 ops/sec
        Avg throughput per process                      =    2704.05 ops/sec
        Min xfer                                        =   63298.00 ops

        Children see throughput for 10 re-readers       =   27744.39 ops/sec
        Parent sees throughput for 10 re-readers        =   27740.07 ops/sec
        Min throughput per process                      =    2755.68 ops/sec 
        Max throughput per process                      =    2820.69 ops/sec
        Avg throughput per process                      =    2774.44 ops/sec
        Min xfer                                        =   64030.00 ops



"Throughput report Y-axis is type of test X-axis is number of processes"
"Record size = 4 kBytes "
"Output is in ops/sec"

"  Initial write "   26067.23 

"        Rewrite "   27088.62 

"           Read "   27040.54 

"        Re-read "   27744.39 


iozone test complete.
Test with 10 threads completed. Results saved to result_modify_setting/read_256M_4k_t10.xls
All tests completed.
```