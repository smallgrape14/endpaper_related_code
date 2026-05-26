
```sh
# 并发顺序写多个大文件，并发度 16，每个线程写入 64 个大文件
fio --name=big-write --directory=/mnt/beegfs/test_fio --group_reporting \
  --rw=write \
  --direct=1 \
  --bs=32k \
  --end_fsync=1 \
  --numjobs=16 \
  --nrfiles=16 \
  --size=1M


# 并发顺序读多个大文件，线程数 64，每个线程读 64 个文件
fio --name=big-read-multiple-concurrent --group_reporting \
  --directory=/mnt/beegfs/test_fio \
  --rw=read \
  --direct=1 \
  --bs=32k \
  --numjobs=16 \
  --nrfiles=16 \
  --openfiles=1 \
  --size=32K    \
  --runtime=20


fio --name=big-read-multiple-concurrent --group_reporting \
  --directory=/mnt/beegfs/test_fio \
  --rw=read \
  --direct=1 \
  --bs=32k \
  --numjobs=16 \
  --nrfiles=16 \
  --openfiles=1 \
  --runtime=20 \
  --size=1M


[test_read]
rw=read
bs=4k
size=1G
numjobs=4
runtime=60
time_based
direct=1
filename=/mnt/beegfs/file_32.txt
```