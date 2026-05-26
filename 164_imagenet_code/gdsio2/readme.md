## 0916 gds 测试文档

- /usr/local/cuda-11.7/gds/tools/gdsio --help
写
/usr/local/cuda-11.7/gds/tools/gdsio -D /mnt/beegfs/gdsio_0728 -w 1 -d 0 -I 1 -x 0 -s 1K -i 1K
/usr/local/cuda-11.7/gds/tools/gdsio -D ./ -w 256 -d 0 -I 1 -x 0 -s 1K -i 1K
 /usr/local/cuda-11.7/gds/tools/gdsio -D /mnt/beegfs/gdsio_0728/dir1/ -w 1024 -d 0 -I 1 -x 1 -s 32K -i 32K



/usr/local/cuda-11.7/gds/tools/gdsio -D /mnt/beegfs/gdsio_0728/dir_64MB -w 256 -d 0 -I 1 -x 1 -s 64MB -i 1M



读
/usr/local/cuda-11.7/gds/tools/gdsio -D ./ -w 1 -d 0 -I 0 -x 0 -s 1K -i 1K
/usr/local/cuda-11.7/gds/tools/gdsio -D /mnt/beegfs/gdsio_0728/dir1 -w 1 -d 0 -I 0 -x 0 -s 1K -i 1K
/usr/local/cuda-11.7/gds/tools/gdsio -D /mnt/beegfs/gdsio_0728 -w 1 -d 0 -I 0 -x 0 -s 64M -i 1K


/usr/local/cuda-11.7/gds/tools/gdsio -f /mnt/beegfs/gdsio_0728/gdsio.0 -w 1 -d 0 -I 0 -x 0 -s 64M -i 1K


```sh
gdsio version :1.6
Usage [using config file]: gdsio rw-sample.gdsio
Usage [using cmd line options]:/usr/local/cuda-11.7/gds/tools/gdsio 
         -f <file name>
         -D <directory name>
         -d <gpu_index (refer nvidia-smi)>
         -n <numa node>
         -w <number of threads for a job>
         -s <file size(K|M|G)>
         -o <start offset(K|M|G)>
         -i <io_size(K|M|G)> <min_size:max_size:step_size>
         -p <enable nvlinks> 
         -b <skip bufregister> 
         -V <verify IO>
         -x <xfer_type> [0(GPU_DIRECT), 1(CPU_ONLY), 2(CPU_GPU), 3(CPU_ASYNC_GPU), 4(CPU_CACHED_GPU), 5(GPU_DIRECT_ASYNC), 6(GPU_BATCH)]
         -B <batch size>
         -I <(read) 0|(write)1| (randread) 2| (randwrite) 3>
         -T <duration in seconds>
         -k <random_seed> (number e.g. 3456) to be used with random read/write> 
         -U <use unaligned(4K) random offsets>
         -R <fill io buffer with random data>
         -F <refill io buffer with random data during each write>
         -P <rdma url>
         -J <per job statistics>

xfer_type:
0 - Storage->GPU (GDS)
1 - Storage->CPU
2 - Storage->CPU->GPU
3 - Storage->CPU->GPU_ASYNC
4 - Storage->PAGE_CACHE->CPU->GPU
5 - Storage->GPU_ASYNC
6 - Storage->GPU_BATCH

Note:
read test (-I 0) with verify option (-V) should be used with files written (-I 1) with -V option
read test (-I 2) with verify option (-V) should be used with files written (-I 3) with -V option, using same random seed (-k),
same number of threads(-w), offset(-o), and data size(-s)
write test (-I 1/3) with verify option (-V) will perform writes followed by read

```