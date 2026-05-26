
## 环境部署
```sh
sudo pip install torchvision
sudo pip install torch
# nvidia-smi 查看cuda 版本 安装对应版本的dali
sudo pip install nvidia-dali-cuda120
    #遇到Error :ERROR: Could not find a version that satisfies the requirement nvidia-nvcomp-cu11; extra == "all" (from nvidia-nvimgcodec-cu11[all]<0.6.0,>=0.5.0->nvidia-dali-cuda110) (from versions: none)
    #ERROR: No matching distribution found for nvidia-nvcomp-cu11; extra == "all" (from nvidia-nvimgcodec-cu11[all]<0.6.0,>=0.5.0->nvidia-dali-cuda110)
pip install --extra-index-url https://pypi.nvidia.com nvidia-dali-cuda120
#cuda12x 不行，也不能两个版本都安装
pip install cupy-cuda11x 
```

## 训练代码解析

```sh

main : 解析参数
    ==> man_worker
        ==> 1. creadte model选择预训练或创建一个新的的模型实例
            model = models.__dict__[args.arch](pretrained=True) 
        ==> 2. set_device 选择模型使用GPU device，将模型移动到指定的 GPU 上
            torch.cuda.set_device(args.gpu)
            model = model.cuda(args.gpu)
        ==> 3. 根据设备可用性，设置 device 变量，用于后续的张量操作
            device = torch.device('cuda:{}'.format(args.gpu)) 
        ==> 4. define loss function (criterion), optimizer, and learning rate scheduler
            ==> 4.1 define loss function
                criterion = nn.CrossEntropyLoss().to(device) #@@ 定义交叉熵损失函数，并将其移动到指定设备。
            ==> 4.2 使用随机梯度下降（SGD）优化器，设置学习率、动量和权重衰减
                optimizer = torch.optim.SGD(model.parameters(), args.lr,
                                momentum=args.momentum,
                                weight_decay=args.weight_decay)
            ==> 4.3 定义学习率调度器，每 30 个 epoch 将学习率乘以 0.1。
                 scheduler = StepLR(optimizer, step_size=30, gamma=0.1) 


```

## 常规AI 训练运行指令
```sh
# 注意root 下运行
python main.py -a resnet18 /mnt/beegfs/imagenet -j 1 --epochs 1 -b 16 --gpu 0

python main_nopreprocess.py -a resnet18 /mnt/beegfs/imagenet_test -j 1 --epochs 1 -b 16 --gpu 0
python main_nopreprocess.py -a resnet18 /mnt/beegfs/imagenet -j 1 --epochs 1 -b 16 --gpu 0


# 非beegfs测试
python main_nopreprocess.py -a resnet18 /home/oem/xyp/imagenet_test -j 1 --epochs 1 -b 16 --gpu 0

# 非beegfs 且预先解码到存储盘上的测试
python main_nopreprocess.py -a resnet18 /home/oem/xyp/imagenet_test_decode -j 1 --epochs 1 -b 16 --gpu 0

python main_nopreprocess.py -a resnet18 /home/oem/xyp/imagenet_test_decode2 -j 1 --epochs 1 -b 16 --gpu 0


## 其他查询相关指令
#nvidia-smi -L  # 列出所有GPU及其ID

```



## Decode 
```sh
decode 解码 指令

python3 decode2.py -i /home/oem/xyp/imagenet_test/train/n07711569 -o /home/oem/xyp/imagenet_test_decode2/train/n07711569

python3 decode2.py -i /home/oem/xyp/imagenet_test/val/n07711569 -o /home/oem/xyp/imagenet_test_decode2/val/n07711569

# 对预处理后的文件接入AI 训练 
sudo python main.py -a resnet18 /home/oem/xyp/imagenet_test_decode3 -j 1 --epochs 1 -b 16 --gpu 0

## 真正作用的代码是这个---------------------------------------------------------------------------------------------------------------
# 在decode 目录下的处理逻辑
# decode 解码以及其他的预处理操作之后，将数据以numpy的形式导出到一个目标目录下
sudo python3 decode3.py

# 从目标目录下读取数据，进行训练
sudo python3 train3.py
sudo python3 main.py -a resnet18 /home/oem/xyp/imagenet_test_decode3 -j 1 --epochs 1 -b 16 --gpu 0
```

## 如何判断数据集的加载是正确的
```sh
总体表现
损失下降趋势：训练损失和验证损失都在逐渐下降，表明模型在不断学习和优化。
准确率上升趋势：训练准确率和验证准确率都在逐渐上升，表明模型的分类性能在不断提升。
无过拟合现象：训练准确率和验证准确率的变化趋势较为接近，没有出现训练准确率很高但验证准确率很低的情况，表明模型没有出现明显的过拟合现象。
```

## 解析 训练前对数据的预处理操作，到底是在干什么，为什么需要预处理，预处理之后的数据是什么样的

```sh 

整理下目前试过的一些思路：

首先是decode 是CPU/GPU混合的，如果我要把GPUNetIO接进来就得先解码后存到盘里，再GPUNetIO发起文件访问，然后通过IPC共享显存，给训练进程，也就是我得构造一个自定义的dataloader （参考下 FashionMNIST训练时候的数据加载逻辑）

首先我想预先decode 之后存储在盘内，然后加载到GPU内存中，然后剩余的预处理操作（也可以预先做了），然后直接接入AI 训练，(相当于直接把预处理剔除出去) 先这样做


# 梳理下 GPUNetIO 数据加载的逻辑 
现在 GPUNetIO 发起的文件读逻辑，相当于需要实现 正常训练时候 dataloader的功能

1.首先是 dataset 生成两个关键的列表，包含整个数据集
    image_Path  图像的路径
    和对应的label label 从 0~n-1 n 是类别的数量，同一个目录下的图像都属于同一个类，即数据集根目录下有多少个子目录就有多少个类 
    






```


## Reference

Imagenet DataSet Setup : https://github.com/lmb-freiburg/ovqa
DALI: https://docs.nvidia.com/deeplearning/dali/user-guide/docs/examples/index.html

## AI 提示词

```sh
我的目的是JPEG解码后存放在对应的目录下，通过GDS 直接加载到GPU显存中，然后用DALI做预处理 比如翻转裁剪 归一化，然后pytorch 训练resnet 完整代码应该怎么做 

```
## Error 记录 
### Error 1: root 权限时候 安装dali 时候报错 

- 注意事项
```sh
root 和非 root 看到的 cuda 版本不一样
一个是 12.4 一个是 11.4


pip install --extra-index-url https://pypi.nvidia.com nvidia-dali-cuda120

pip install --extra-index-url https://pypi.nvidia.com nvidia-dali-cuda110

```
```sh
pip install nvidia-pyindex
pip install nvidia-dali-cuda120

pip install --extra-index-url https://pypi.nvidia.com nvidia-dali-cuda120

pip install --extra-index-url https://pypi.nvidia.com --upgrade nvidia-dali-cuda120

pip install "nvidia-dali-cuda120[all]" --extra-index-url https://pypi.nvidia.com
# 报错内容是
ERROR: Could not find a version that satisfies the requirement nvidia-nvcomp-cu12; extra == "all" (from nvidia-nvimgcodec-cu12[all]<0.6.0,>=0.5.0->nvidia-dali-cuda120[all]) (from versions: none)
ERROR: No matching distribution found for nvidia-nvcomp-cu12; extra == "all" (from nvidia-nvimgcodec-cu12[all]<0.6.0,>=0.5.0->nvidia-dali-cuda120[all])



No matching distribution found for nvidia-nvcomp-cu12
```
- 解决方案
```sh

pip install --extra-index-url https://pypi.nvidia.com --upgrade nvidia-dali-cuda120

# 安装NVIDIA包索引工具（网页9）
pip install nvidia-pyindex
sudo python -m pip install --upgrade pip

pip install --extra-index-url https://pypi.ngc.nvidia.com nvidia-nvcomp-cu12

pip install --trusted-host pypi.ngc.nvidia.com --extra-index-url https://pypi.ngc.nvidia.com nvidia-nvcomp-cu12

```
### Error2： 运行python 代码显示缺少什么模块之类的时候

```sh
pip list | grep <模块名称>

也可以 sudo / 非root 权限下 运行都试一试

sudo/ python/ python3  xx.py
```