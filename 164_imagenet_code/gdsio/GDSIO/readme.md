## 1127 DEBUG
现在的问题是：164client上nvme SSD 盘不能GDS，会报错，查看了盘的挂载方式也是 ordered模式
```sh
cufile.log的报错提示：(/etc/cufile.json中开启兼容模式时候的报错)
26-11-2025 21:07:09:13 [pid=809108 tid=809108] ERROR  cufio-fs:199 NVMe Driver not registered with nvidia-fs!!!
26-11-2025 21:07:09:14 [pid=809108 tid=809108] ERROR  cufio-fs:199 NVMe Driver not registered with nvidia-fs!!!
26-11-2025 21:07:09:14 [pid=809108 tid=809108] NOTICE  cufio-fs:441 dumping volume attributes: DEVNAME:/dev/nvme1n1p2,ID_FS_TYPE:ext4,ID_FS_USAGE:filesystem,ext4_journal_mode:ordered,fsid:4f503588a466ef940x,numa_node:-1,queue/logical_block_size:4096,wwid:eui.0050431d0b000001,
26-11-2025 21:07:09:14 [pid=809108 tid=809108] NOTICE  cufio:293 cuFileHandleRegister GDS not supported or disabled by config, using cuFile posix read/write with compat mode enabled
cufile.log的报错提示：(/etc/cufile.json中关闭兼容模式时候的报错)

```
