## 250506 说明主要代码功能

```sh
主要是Baseline BeeGFS-Client 加载数据的

1. decode / resize 等预处理 对整个imagenet数据集进行预处理，预处理后导出到一个目录下去
    python3 decode3.py

2. 从目标目录 加载数据集 做AI训练  (优先用main.py 因为是官方的代码，比较标准，各种参数都便于修改)
    sudo python3 main.py -a resnet18 /home/oem/xyp/imagenet_test_decode3 -j 1 --epochs 1 -b 16 --gpu 0
    sudo python3 train3.py
```