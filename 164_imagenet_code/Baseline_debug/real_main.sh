#!/bin/bash

# sudo python3 real_main.py -a resnet18 /mnt/nvme0/FlexGV_Test/ImageNet-1K -j 0 --epochs 1 -b 512 --gpu 0 > real_main_out.txt
sudo python3 real_main.py -a resnet18 /mnt/nvme0/FlexGV_Test/ImageNet-1K -j 16 --epochs 25 -b 512 --gpu 0 > real/out_epoch25_w16_b512.txt

# sudo python3 real_main.py -a resnet18 /home/oem/xyp/imagenet_test -j 0 --epochs 1 -b 512 --gpu 0 > real_main_out.txt


# sudo python3 real_main2.py -a resnet18 /mnt/nvme0/FlexGV_Test/ImageNet-1K -j 0 --epochs 1 -b 512 --gpu 0 > real_main_out.txt
