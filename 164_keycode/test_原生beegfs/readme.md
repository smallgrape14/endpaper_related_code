## 0312 执行过程

### 读FashionMNIST 数据集时间
```sh
make
./test /mnt/beegfs/FashionMNIST out.csv
# 加上了读取文件输出到指定文件 result/outfile/out_ 验证文件读取的正确性
./test1 /mnt/beegfs/FashionMNIST out1.csv

# 统计平均时间
./avg out.csv
./avg out1.csv
```

### 读 image_net 数据集时间
```sh
# 测试 ImageNet  文件位于 /mnt/beegfs/image
xyp@dell02:/mnt/beegfs$ ls image | wc -l
20580
# 
make
./test /mnt/beegfs/image out_imagenet.csv
# 测试脚本 ，循环运行20次 ./test /mnt/beegfs/image out_imagenet.csv
./run.sh 
#统计平均值
./avg out_imagenet.csv
# 结果：
# 平均经过时间: 18.688269 秒

# 时间分解 分解 open read 的时间 这里读是4KB 一个IO 的读
./test_decom /mnt/beegfs/image_32KB out_image_32KB.csv

# 时间分解 分解 open read 的时间 这里读是先获取文件的大小， 一个文件只发出一个IO请求的读
./test_decom_all /mnt/beegfs/image_32KB out_image_32KB_all.csv

./test_decom_all /mnt/beegfs/test_dir/train/n02415577 out_image_0514.csv

```


```sh
# 抓包文件分析
6.001204

# 54 33 file length
6.001425
6.001493

6.001503


```