## 0916 gds 测试文档

- /usr/local/cuda-11.7/gds/tools/gdsio --help
写
/usr/local/cuda-11.7/gds/tools/gdsio -D /mnt/beegfs/gdsio_0728 -w 1 -d 0 -I 1 -x 0 -s 1K -i 1K
/usr/local/cuda-11.7/gds/tools/gdsio -D ./ -w 256 -d 0 -I 1 -x 0 -s 1K -i 1K
 /usr/local/cuda-11.7/gds/tools/gdsio -D /mnt/beegfs/gdsio_0728/dir1/ -w 1024 -d 0 -I 0 -x 1 -s 32K -i 32K


/usr/local/cuda-11.7/gds/tools/gdsio -D /mnt/beegfs/gdsio_0728 -w 1 -d 0 -I 1 -x 0 -s 2G -i 1G

/usr/local/cuda-11.7/gds/tools/gdsio -D /mnt/nvme0/gdstest -w 1 -d 0 -I 1 -x 0 -s 1G -i 1G

/usr/local/cuda-12.4/gds/tools/gdsio -D /home/oem/xyp/imagenet_code/gdsio/data/dir -w 1 -d 0 -I 1 -x 0 -s 2M -i 2M



读
/usr/local/cuda-11.7/gds/tools/gdsio -D ./ -w 1 -d 0 -I 0 -x 0 -s 1K -i 1K
/usr/local/cuda-11.7/gds/tools/gdsio -D /mnt/beegfs/gdsio_0728/dir1 -w 1 -d 0 -I 0 -x 0 -s 1K -i 1K
/usr/local/cuda-11.7/gds/tools/gdsio -D /mnt/beegfs/gdsio_0728 -w 1 -d 0 -I 0 -x 0 -s 64M -i 1K


/usr/local/cuda-11.7/gds/tools/gdsio -f /mnt/beegfs/gdsio_0728/gdsio.0 -w 1 -d 0 -I 0 -x 0 -s 64M -i 1K

/usr/local/cuda-11.7/gds/tools/gdsio -f /mnt/beegfs/gdsio_0728/gdsio.0 -w 1 -d 0 -I 0 -x 0 -s 64M -i 1M



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


## 1126

### BEEGFS


### LOCAL NVME SSD
```sh
(base) oem@node9:/mnt/nvme0/gdstest$ cat cufile.log 
 26-11-2025 19:51:31:692 [pid=795751 tid=795751] ERROR  cufio-fs:199 NVMe Driver not registered with nvidia-fs!!!
 26-11-2025 19:51:31:693 [pid=795751 tid=795751] ERROR  cufio-fs:199 NVMe Driver not registered with nvidia-fs!!!
 26-11-2025 19:51:31:693 [pid=795751 tid=795751] NOTICE  cufio-fs:441 dumping volume attributes: DEVNAME:/dev/nvme1n1p2,ID_FS_TYPE:ext4,ID_FS_USAGE:filesystem,ext4_journal_mode:ordered,fsid:4f503588a466ef940x,numa_node:-1,queue/logical_block_size:4096,wwid:eui.0050431d0b000001,
 26-11-2025 19:51:31:693 [pid=795751 tid=795751] NOTICE  cufio:293 cuFileHandleRegister GDS not supported or disabled by config, using cuFile posix read/write with compat mode enabled
(base) oem@node9:/mnt/nvme0/gdstest$ 

## 并且程序所在目录下的 cufile.log  ----------------

 26-11-2025 19:55:10:906 [pid=799371 tid=799371] ERROR  cufio-fs:199 NVMe Driver not registered with nvidia-fs!!!
 26-11-2025 19:55:10:907 [pid=799371 tid=799371] ERROR  cufio-fs:199 NVMe Driver not registered with nvidia-fs!!!
 26-11-2025 19:55:10:907 [pid=799371 tid=799371] NOTICE  cufio-fs:441 dumping volume attributes: DEVNAME:/dev/nvme1n1p2,ID_FS_TYPE:ext4,ID_FS_USAGE:filesystem,ext4_journal_mode:ordered,fsid:4f503588a466ef940x,numa_node:-1,queue/logical_block_size:4096,wwid:eui.0050431d0b000001,
 26-11-2025 19:55:10:907 [pid=799371 tid=799371] NOTICE  cufio:293 cuFileHandleRegister GDS not supported or disabled by config, using cuFile posix read/write with compat mode enabled
 26-11-2025 19:55:12:956 [pid=799450 tid=799450] ERROR  cufio-fs:199 NVMe Driver not registered with nvidia-fs!!!
 26-11-2025 19:55:12:957 [pid=799450 tid=799450] ERROR  cufio-fs:199 NVMe Driver not registered with nvidia-fs!!!
 26-11-2025 19:55:12:957 [pid=799450 tid=799450] NOTICE  cufio-fs:441 dumping volume attributes: DEVNAME:/dev/nvme1n1p2,ID_FS_TYPE:ext4,ID_FS_USAGE:filesystem,ext4_journal_mode:ordered,fsid:4f503588a466ef940x,numa_node:-1,queue/logical_block_size:4096,wwid:eui.0050431d0b000001,
 26-11-2025 19:55:12:957 [pid=799450 tid=799450] NOTICE  cufio:293 cuFileHandleRegister GDS not supported or disabled by config, using cuFile posix read/write with compat mode enabled
 26-11-2025 19:55:15:3 [pid=799530 tid=799530] ERROR  cufio-fs:199 NVMe Driver not registered with nvidia-fs!!!
 26-11-2025 19:55:15:4 [pid=799530 tid=799530] ERROR  cufio-fs:199 NVMe Driver not registered with nvidia-fs!!!
 26-11-2025 19:55:15:4 [pid=799530 tid=799530] NOTICE  cufio-fs:441 dumping volume attributes: DEVNAME:/dev/nvme1n1p2,ID_FS_TYPE:ext4,ID_FS_USAGE:filesystem,ext4_journal_mode:ordered,fsid:4f503588a466ef940x,numa_node:-1,queue/logical_block_size:4096,wwid:eui.0050431d0b000001,
 26-11-2025 19:55:15:4 [pid=799530 tid=799530] NOTICE  cufio:293 cuFileHandleRegister GDS not supported or disabled by config, using cuFile posix read/write with compat mode enabled
 26-11-2025 19:55:22:661 [pid=799707 tid=799707] ERROR  cufio-fs:199 NVMe Driver not registered with nvidia-fs!!!
 26-11-2025 19:55:22:662 [pid=799707 tid=799707] ERROR  cufio-fs:199 NVMe Driver not registered with nvidia-fs!!!
 26-11-2025 19:55:22:662 [pid=799707 tid=799707] NOTICE  cufio-fs:441 dumping volume attributes: DEVNAME:/dev/nvme1n1p2,ID_FS_TYPE:ext4,ID_FS_USAGE:filesystem,ext4_journal_mode:ordered,fsid:4f503588a466ef940x,numa_node:-1,queue/logical_block_size:4096,wwid:eui.0050431d0b000001,
 26-11-2025 19:55:22:662 [pid=799707 tid=799707] NOTICE  cufio:293 cuFileHandleRegister GDS not supported or disabled by config, using cuFile posix read/write with compat mode enabled


```



### 由于 
```sh
sudo apt install nvidia-gds-12-4
将会同时安装下列软件：
  cuda-toolkit-12-4-config-common gds-tools-12-4 libcufile-12-4 libcufile-dev-12-4
  nvidia-fs nvidia-fs-dkms
建议安装：
  mlnx-ofed-all
下列【新】软件包将被安装：
  cuda-toolkit-12-4-config-common gds-tools-12-4 libcufile-12-4 libcufile-dev-12-4
  nvidia-gds-12-4
下列软件包将被升级：
  nvidia-fs nvidia-fs-dkms
升级了 2 个软件包，新安装了 5 个软件包，要卸载 0 个软件包，有 209 个软件包未被升级。
需要下载 42.4 MB 的归档。
解压缩后会消耗 90.6 MB 的额外空间。
您希望继续执行吗？ [Y/n] y
rmmod: ERROR: Module nvidia_fs is in use
dpkg: 警告: 旧的 nvidia-fs-dkms 软件包 pre-removal 脚本 子进程返回错误状态 1
dpkg: 现在尝试使用新软件包所带的脚本...
rmmod: ERROR: Module nvidia_fs is in use
dpkg: 处理归档 /tmp/apt-dpkg-install-nY3P8S/4-nvidia-fs-dkms_2.25.7-1_amd64.deb (--unpack)时出错：
 新的 nvidia-fs-dkms 软件包 pre-removal 脚本 子进程返回错误状态 1
Error! DKMS tree already contains: nvidia-fs-2.17.3
You cannot add the same module/version combo more than once.

*************************************************************************
*** Reboot your computer and verify that the NVIDIA filesystem driver ***
*** can be loaded.                                                    ***
*************************************************************************

准备解压 .../5-nvidia-fs_2.25.7-1_amd64.deb  ...
正在解压 nvidia-fs (2.25.7-1) 并覆盖 (2.17.3-1) ...
正在选中未选择的软件包 nvidia-gds-12-4。
准备解压 .../6-nvidia-gds-12-4_12.4.1-1_amd64.deb  ...
正在解压 nvidia-gds-12-4 (12.4.1-1) ...
在处理时有错误发生：
 /tmp/apt-dpkg-install-nY3P8S/4-nvidia-fs-dkms_2.25.7-1_amd64.deb
E: Sub-process /usr/bin/dpkg returned an error code (1)

```




#### debug
```sh
/usr/local/cuda-12.4/gds/tools/gdsio_verify -f /mnt/nvme0/gdstest/gdsio.0 -n 1 -m 0 -s 4k -o 0 -d 0 -t 0

/usr/local/cuda-12.4/gds/tools/gdsio -f /mnt/nvme0/gdstest/gdsio.0 -d 0 -w 1 -s 4k -x 0 -i 4K -I 1 -V

/usr/local/cuda-12.4/gds/tools/gdsio -f /mnt/nvme0/gdstest/gdsio.0 -d 0 -w 1 -s 4k -x 1 -i 4K -I 1 -V

/usr/local/cuda-12.4/gds/tools/gdsio -f /mnt/nvme0/gdstest/gdsio.0 -d 0 -w 1 -s 4k -x 2 -i 4K -I 1 -V


/usr/local/cuda-12.4/gds/tools/gdsio -f /home/oem/xyp/imagenet_code/gdsio/data/dir/gdsio.0 -d 0 -w 1 -s 4k -x 0 -i 4K -I 1 -V




### 1.
root@node9:/mnt/nvme0/gdstest# /usr/local/cuda-12.4/gds/tools/gdsio_verify -f /mnt/nvme0/gdstest/gdsio.0 -n 1 -m 0 -s 4k -o 0 -d 0 -t 0
gpu index :0,file :/mnt/nvme0/gdstest/gdsio.0, gpu buffer alignment :0, gpu buffer offset :0, gpu devptr offset :0, file offset :0, io_requested :4096, io_chunk_size :4096, bufregister :true, sync :0, nr ios :1, 
fsync :0, 
Batch mode: 0
Data Verification Success
## 2.
root@node9:/mnt/nvme0/gdstest# /usr/local/cuda-12.4/gds/tools/gdsio -f /mnt/nvme0/gdstest/gdsio.0 -d 0 -w 1 -s 4k -x 0 -i 4K -I 1 -V
IoType: WRITE XferType: GPUD Threads: 1 DataSetSize: 4/4(KiB) IOSize: 4(KiB) Throughput: 0.001974 GiB/sec, Avg_Latency: 0.000000 usecs ops: 1 total_time 0.001932 secs
Verifying data 
IoType: READ XferType: GPUD Threads: 1 DataSetSize: 4/4(KiB) IOSize: 4(KiB) Throughput: 0.001221 GiB/sec, Avg_Latency: 0.000000 usecs ops: 1 total_time 0.003125 secs




```

26-11-2025 21:35:11:539 [pid=815918 tid=815918] ERROR  cufio-fs:199 NVMe Driver not registered with nvidia-fs!!!
 26-11-2025 21:35:11:540 [pid=815918 tid=815918] ERROR  cufio-fs:199 NVMe Driver not registered with nvidia-fs!!!
 26-11-2025 21:35:11:540 [pid=815918 tid=815918] NOTICE  cufio-fs:441 dumping volume attributes: DEVNAME:/dev/nvme1n1p2,ID_FS_TYPE:ext4,ID_FS_USAGE:filesystem,ext4_journal_mode:ordered,fsid:4f503588a466ef940x,numa_node:-1,queue/logical_block_size:4096,wwid:eui.0050431d0b000001,
 26-11-2025 21:35:11:540 [pid=815918 tid=815918] NOTICE  cufio:293 cuFileHandleRegister GDS not supported or disabled by config, using cuFile posix read/write with compat mode enabled
 26-11-2025 21:35:11:661 [pid=815918 tid=815918] ERROR  cufio-fs:199 NVMe Driver not registered with nvidia-fs!!!
 26-11-2025 21:35:11:661 [pid=815918 tid=815918] NOTICE  cufio-fs:441 dumping volume attributes: DEVNAME:/dev/nvme1n1p2,ID_FS_TYPE:ext4,ID_FS_USAGE:filesystem,ext4_journal_mode:ordered,fsid:4f503588a466ef940x,numa_node:-1,queue/logical_block_size:4096,wwid:eui.0050431d0b000001,
 26-11-2025 21:35:11:661 [pid=815918 tid=815918] NOTICE  cufio:293 cuFileHandleRegister GDS not supported or disabled by config, using cuFile posix read/write with compat mode enabled

 ```sh
nvme error-log /dev/nvme1n1p2 > debug.txt

nvme id-ns /dev/nvme1n1p2 -H > debug2.txt

 ```