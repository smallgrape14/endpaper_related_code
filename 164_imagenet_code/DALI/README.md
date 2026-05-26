## 250506 说明 

```sh
main.py 是源码 传统的AI 训练 代码



main_DALI.py 是最新的代码 ： DALI 进行预处理操作的AI 训练代码


main_nopreprocess.py 是将预处理操作由DALI 执行的AI 训练代码： 这个版本用于调试

## 运行示例
sudo python3 main.py -a resnet18 /mnt/beegfs/imagenet -j 1 --epochs 1 -b 16 --gpu 0
sudo python3 main_DALI.py -a resnet18 /home/oem/xyp/imagenet_test -j 1 --epochs 1 -b 16 --gpu 0
## PS 
README.md ： 这个是源码的文件，程序代码运行参数指定 可以参考这个文件
```
## 0717 DALI 处理说明


