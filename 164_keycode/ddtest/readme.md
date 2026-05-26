
```sh
# 512B
time dd if=/mnt/beegfs/file_1.txt of=/dev/null bs=8k count=1000000
# 32KB
time dd if=/mnt/beegfs/file_32.txt of=/dev/null bs=32k count=1000000
time dd if=/mnt/beegfs/file_32.txt of=/dev/null bs=32k count=500
time dd if=/mnt/beegfs/file_32.txt of=/dev/null bs=32k count=256




```