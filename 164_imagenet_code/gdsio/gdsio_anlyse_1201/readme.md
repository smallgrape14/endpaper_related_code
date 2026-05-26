
```sh

生成对应的test dir
# 写
/usr/local/cuda-12.4/gds/tools/gdsio -D /mnt/beegfs/gdsio_0728 -w 256 -d 0 -I 1 -x 0 -s 64MB -i 1M
/usr/local/cuda-12.4/gds/tools/gdsio -D /mnt/beegfs/gdsio_0728/dir_64MB -w 256 -d 0 -I 0 -x 0 -s 64M -i 1M >> debug.txt

```