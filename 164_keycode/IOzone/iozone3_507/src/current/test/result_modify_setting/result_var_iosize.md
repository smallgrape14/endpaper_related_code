
## iosize 512B
```sh
        Run began: Tue Apr 15 21:42:49 2025

        File size set to 262144 kB
        Record Size 512 kB
        Setting no_unlink
        OPS Mode. Output is in operations per second.
        O_DIRECT feature enabled
        Command line used: ./iozone -t 1 -s 256M -r 512 -w -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1
        Time Resolution = 0.000001 seconds.
        Processor cache size set to 1024 kBytes.
        Processor cache line size set to 32 bytes.
        File stride size set to 17 * record size.
        Throughput test with 1 process
        Each process writes a 262144 kByte file in 512 kByte records

        Children see throughput for  1 initial writers  =    1074.97 ops/sec
        Parent sees throughput for  1 initial writers   =    1053.56 ops/sec
        Min throughput per process                      =    1074.97 ops/sec 
        Max throughput per process                      =    1074.97 ops/sec
        Avg throughput per process                      =    1074.97 ops/sec
        Min xfer                                        =     512.00 ops

        Children see throughput for  1 rewriters        =    1102.53 ops/sec
        Parent sees throughput for  1 rewriters         =    1081.05 ops/sec
        Min throughput per process                      =    1102.53 ops/sec 
        Max throughput per process                      =    1102.53 ops/sec
        Avg throughput per process                      =    1102.53 ops/sec
        Min xfer                                        =     512.00 ops

        Children see throughput for  1 readers          =     952.91 ops/sec
        Parent sees throughput for  1 readers           =     936.12 ops/sec
        Min throughput per process                      =     952.91 ops/sec 
        Max throughput per process                      =     952.91 ops/sec
        Avg throughput per process                      =     952.91 ops/sec
        Min xfer                                        =     512.00 ops

        Children see throughput for 1 re-readers        =     935.25 ops/sec
        Parent sees throughput for 1 re-readers         =     929.87 ops/sec
        Min throughput per process                      =     935.25 ops/sec 
        Max throughput per process                      =     935.25 ops/sec
        Avg throughput per process                      =     935.25 ops/sec
        Min xfer                                        =     512.00 ops



iozone test complete.
```
## iosize 2k
```sh
        Run began: Tue Apr 15 21:44:04 2025

        File size set to 262144 kB
        Record Size 2 kB
        Setting no_unlink
        OPS Mode. Output is in operations per second.
        O_DIRECT feature enabled
        Command line used: ./iozone -t 1 -s 256M -r 2k -w -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1
        Time Resolution = 0.000001 seconds.
        Processor cache size set to 1024 kBytes.
        Processor cache line size set to 32 bytes.
        File stride size set to 17 * record size.
        Throughput test with 1 process
        Each process writes a 262144 kByte file in 2 kByte records

        Children see throughput for  1 initial writers  =    8523.28 ops/sec
        Parent sees throughput for  1 initial writers   =    8518.52 ops/sec
        Min throughput per process                      =    8523.28 ops/sec 
        Max throughput per process                      =    8523.28 ops/sec
        Avg throughput per process                      =    8523.28 ops/sec
        Min xfer                                        =  131072.00 ops

        Children see throughput for  1 rewriters        =    3662.21 ops/sec
        Parent sees throughput for  1 rewriters         =    3661.28 ops/sec
        Min throughput per process                      =    3662.21 ops/sec 
        Max throughput per process                      =    3662.21 ops/sec
        Avg throughput per process                      =    3662.21 ops/sec
        Min xfer                                        =  131072.00 ops

        Children see throughput for  1 readers          =    2785.31 ops/sec
        Parent sees throughput for  1 readers           =    2785.08 ops/sec
        Min throughput per process                      =    2785.31 ops/sec 
        Max throughput per process                      =    2785.31 ops/sec
        Avg throughput per process                      =    2785.31 ops/sec
        Min xfer                                        =  131072.00 ops

        Children see throughput for 1 re-readers        =    2332.18 ops/sec
        Parent sees throughput for 1 re-readers         =    2332.04 ops/sec
        Min throughput per process                      =    2332.18 ops/sec 
        Max throughput per process                      =    2332.18 ops/sec
        Avg throughput per process                      =    2332.18 ops/sec
        Min xfer                                        =  131072.00 ops



iozone test complete.
```
## iosize 4k
## iosize 8k
```sh
Run began: Tue Apr 15 21:39:10 2025

        File size set to 262144 kB
        Record Size 8 kB
        Setting no_unlink
        OPS Mode. Output is in operations per second.
        O_DIRECT feature enabled
        Command line used: ./iozone -t 1 -s 256M -r 8k -w -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1
        Time Resolution = 0.000001 seconds.
        Processor cache size set to 1024 kBytes.
        Processor cache line size set to 32 bytes.
        File stride size set to 17 * record size.
        Throughput test with 1 process
        Each process writes a 262144 kByte file in 8 kByte records

        Children see throughput for  1 initial writers  =    2041.53 ops/sec
        Parent sees throughput for  1 initial writers   =    2040.35 ops/sec
        Min throughput per process                      =    2041.53 ops/sec 
        Max throughput per process                      =    2041.53 ops/sec
        Avg throughput per process                      =    2041.53 ops/sec
        Min xfer                                        =   32768.00 ops

        Children see throughput for  1 rewriters        =    2561.32 ops/sec
        Parent sees throughput for  1 rewriters         =    2559.47 ops/sec
        Min throughput per process                      =    2561.32 ops/sec 
        Max throughput per process                      =    2561.32 ops/sec
        Avg throughput per process                      =    2561.32 ops/sec
        Min xfer                                        =   32768.00 ops

        Children see throughput for  1 readers          =    2281.07 ops/sec
        Parent sees throughput for  1 readers           =    2280.34 ops/sec
        Min throughput per process                      =    2281.07 ops/sec 
        Max throughput per process                      =    2281.07 ops/sec
        Avg throughput per process                      =    2281.07 ops/sec
        Min xfer                                        =   32768.00 ops

        Children see throughput for 1 re-readers        =    1853.53 ops/sec
        Parent sees throughput for 1 re-readers         =    1853.16 ops/sec
        Min throughput per process                      =    1853.53 ops/sec 
        Max throughput per process                      =    1853.53 ops/sec
        Avg throughput per process                      =    1853.53 ops/sec
        Min xfer                                        =   32768.00 ops



iozone test complete.
```
## iosize 16k
## iosize 32k
```sh
        Run began: Tue Apr 15 21:42:01 2025

        File size set to 262144 kB
        Record Size 32 kB
        Setting no_unlink
        OPS Mode. Output is in operations per second.
        O_DIRECT feature enabled
        Command line used: ./iozone -t 1 -s 256M -r 32k -w -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1
        Time Resolution = 0.000001 seconds.
        Processor cache size set to 1024 kBytes.
        Processor cache line size set to 32 bytes.
        File stride size set to 17 * record size.
        Throughput test with 1 process
        Each process writes a 262144 kByte file in 32 kByte records

        Children see throughput for  1 initial writers  =    1720.91 ops/sec
        Parent sees throughput for  1 initial writers   =    1715.63 ops/sec
        Min throughput per process                      =    1720.91 ops/sec 
        Max throughput per process                      =    1720.91 ops/sec
        Avg throughput per process                      =    1720.91 ops/sec
        Min xfer                                        =    8192.00 ops

        Children see throughput for  1 rewriters        =    1742.97 ops/sec
        Parent sees throughput for  1 rewriters         =    1739.37 ops/sec
        Min throughput per process                      =    1742.97 ops/sec 
        Max throughput per process                      =    1742.97 ops/sec
        Avg throughput per process                      =    1742.97 ops/sec
        Min xfer                                        =    8192.00 ops

        Children see throughput for  1 readers          =    1731.68 ops/sec
        Parent sees throughput for  1 readers           =    1728.15 ops/sec
        Min throughput per process                      =    1731.68 ops/sec 
        Max throughput per process                      =    1731.68 ops/sec
        Avg throughput per process                      =    1731.68 ops/sec
        Min xfer                                        =    8192.00 ops

        Children see throughput for 1 re-readers        =    1717.70 ops/sec
        Parent sees throughput for 1 re-readers         =    1716.52 ops/sec
        Min throughput per process                      =    1717.70 ops/sec 
        Max throughput per process                      =    1717.70 ops/sec
        Avg throughput per process                      =    1717.70 ops/sec
        Min xfer                                        =    8192.00 ops



iozone test complete.

```
## iosize 64k

```sh
Run began: Tue Apr 15 21:32:43 2025

        File size set to 262144 kB
        Record Size 64 kB
        Setting no_unlink
        OPS Mode. Output is in operations per second.
        O_DIRECT feature enabled
        Command line used: ./iozone -t 1 -s 256M -r 64k -w -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1
        Time Resolution = 0.000001 seconds.
        Processor cache size set to 1024 kBytes.
        Processor cache line size set to 32 bytes.
        File stride size set to 17 * record size.
        Throughput test with 1 process
        Each process writes a 262144 kByte file in 64 kByte records

        Children see throughput for  1 initial writers  =    2780.92 ops/sec
        Parent sees throughput for  1 initial writers   =    2764.16 ops/sec
        Min throughput per process                      =    2780.92 ops/sec 
        Max throughput per process                      =    2780.92 ops/sec
        Avg throughput per process                      =    2780.92 ops/sec
        Min xfer                                        =    4096.00 ops

        Children see throughput for  1 rewriters        =    3211.63 ops/sec
        Parent sees throughput for  1 rewriters         =    3189.12 ops/sec
        Min throughput per process                      =    3211.63 ops/sec 
        Max throughput per process                      =    3211.63 ops/sec
        Avg throughput per process                      =    3211.63 ops/sec
        Min xfer                                        =    4096.00 ops

        Children see throughput for  1 readers          =    3060.01 ops/sec
        Parent sees throughput for  1 readers           =    3040.16 ops/sec
        Min throughput per process                      =    3060.01 ops/sec 
        Max throughput per process                      =    3060.01 ops/sec
        Avg throughput per process                      =    3060.01 ops/sec
        Min xfer                                        =    4096.00 ops

        Children see throughput for 1 re-readers        =    3659.37 ops/sec
        Parent sees throughput for 1 re-readers         =    3650.66 ops/sec
        Min throughput per process                      =    3659.37 ops/sec 
        Max throughput per process                      =    3659.37 ops/sec
        Avg throughput per process                      =    3659.37 ops/sec
        Min xfer                                        =    4096.00 ops



iozone test complete.
```

## iosize 128k
```sh
        Run began: Tue Apr 15 21:40:34 2025

        File size set to 262144 kB
        Record Size 128 kB
        Setting no_unlink
        OPS Mode. Output is in operations per second.
        O_DIRECT feature enabled
        Command line used: ./iozone -t 1 -s 256M -r 128k -w -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1
        Time Resolution = 0.000001 seconds.
        Processor cache size set to 1024 kBytes.
        Processor cache line size set to 32 bytes.
        File stride size set to 17 * record size.
        Throughput test with 1 process
        Each process writes a 262144 kByte file in 128 kByte records

        Children see throughput for  1 initial writers  =    1512.37 ops/sec
        Parent sees throughput for  1 initial writers   =    1502.05 ops/sec
        Min throughput per process                      =    1512.37 ops/sec 
        Max throughput per process                      =    1512.37 ops/sec
        Avg throughput per process                      =    1512.37 ops/sec
        Min xfer                                        =    2048.00 ops

        Children see throughput for  1 rewriters        =    1478.25 ops/sec
        Parent sees throughput for  1 rewriters         =    1468.38 ops/sec
        Min throughput per process                      =    1478.25 ops/sec 
        Max throughput per process                      =    1478.25 ops/sec
        Avg throughput per process                      =    1478.25 ops/sec
        Min xfer                                        =    2048.00 ops

        Children see throughput for  1 readers          =    1548.17 ops/sec
        Parent sees throughput for  1 readers           =    1537.11 ops/sec
        Min throughput per process                      =    1548.17 ops/sec 
        Max throughput per process                      =    1548.17 ops/sec
        Avg throughput per process                      =    1548.17 ops/sec
        Min xfer                                        =    2048.00 ops

        Children see throughput for 1 re-readers        =    1548.23 ops/sec
        Parent sees throughput for 1 re-readers         =    1544.01 ops/sec
        Min throughput per process                      =    1548.23 ops/sec 
        Max throughput per process                      =    1548.23 ops/sec
        Avg throughput per process                      =    1548.23 ops/sec
        Min xfer                                        =    2048.00 ops



iozone test complete.
```
## iosize 256k

## iosize 1m

```sh
        Run began: Tue Apr 15 21:38:17 2025

        File size set to 262144 kB
        Record Size 1024 kB
        Setting no_unlink
        OPS Mode. Output is in operations per second.
        O_DIRECT feature enabled
        Command line used: ./iozone -t 1 -s 256M -r 1m -w -O -I -i 0 -i 1 -F /mnt/beegfs/iozone_test/file1
        Time Resolution = 0.000001 seconds.
        Processor cache size set to 1024 kBytes.
        Processor cache line size set to 32 bytes.
        File stride size set to 17 * record size.
        Throughput test with 1 process
        Each process writes a 262144 kByte file in 1024 kByte records

        Children see throughput for  1 initial writers  =     532.81 ops/sec
        Parent sees throughput for  1 initial writers   =     522.25 ops/sec
        Min throughput per process                      =     532.81 ops/sec 
        Max throughput per process                      =     532.81 ops/sec
        Avg throughput per process                      =     532.81 ops/sec
        Min xfer                                        =     256.00 ops

        Children see throughput for  1 rewriters        =     551.48 ops/sec
        Parent sees throughput for  1 rewriters         =     540.11 ops/sec
        Min throughput per process                      =     551.48 ops/sec 
        Max throughput per process                      =     551.48 ops/sec
        Avg throughput per process                      =     551.48 ops/sec
        Min xfer                                        =     256.00 ops

        Children see throughput for  1 readers          =     469.25 ops/sec
        Parent sees throughput for  1 readers           =     461.11 ops/sec
        Min throughput per process                      =     469.25 ops/sec 
        Max throughput per process                      =     469.25 ops/sec
        Avg throughput per process                      =     469.25 ops/sec
        Min xfer                                        =     256.00 ops

        Children see throughput for 1 re-readers        =     468.12 ops/sec
        Parent sees throughput for 1 re-readers         =     465.54 ops/sec
        Min throughput per process                      =     468.12 ops/sec 
        Max throughput per process                      =     468.12 ops/sec
        Avg throughput per process                      =     468.12 ops/sec
        Min xfer                                        =     256.00 ops



iozone test complete.
```