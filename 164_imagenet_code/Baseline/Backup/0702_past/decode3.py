import os
import shutil
import torch
import torchvision
import torchvision.datasets as datasets
import torchvision.transforms as transforms
import numpy as np

# 设置数据集路径
args_data = "/home/oem/xyp/imagenet_test"  # 替换为你的数据集路径
traindir = os.path.join(args_data, 'train')
valdir = os.path.join(args_data, 'val')
output_dir = "/home/oem/xyp/imagenet_test_decode3"  # 替换为你的输出目录

# 确保输出目录存在
os.makedirs(output_dir, exist_ok=True)
train_output_dir = os.path.join(output_dir, 'train')
val_output_dir = os.path.join(output_dir, 'val')
os.makedirs(train_output_dir, exist_ok=True)
os.makedirs(val_output_dir, exist_ok=True)

# 定义标准化参数
normalize = transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])

# 定义训练集和验证集的预处理
train_dataset = datasets.ImageFolder(
    traindir,
    transforms.Compose([
        transforms.RandomResizedCrop(224),
        transforms.RandomHorizontalFlip(),
        transforms.ToTensor(),
        normalize
    ])
)

val_dataset = datasets.ImageFolder(
    valdir,
    transforms.Compose([
        transforms.Resize(256),
        transforms.CenterCrop(224),
        transforms.ToTensor(),
        normalize
    ])
)

# 定义函数，将预处理后的图像导出到指定目录
def export_dataset(dataset, output_dir):
    for i in range(len(dataset)):
        # 获取图像和标签
        img, label = dataset[i]
        # 获取原始图像路径
        original_img_path, _ = dataset.samples[i]
        # 获取类别名称
        class_name = dataset.classes[label]
        # 创建类别目录
        class_dir = os.path.join(output_dir, class_name)
        os.makedirs(class_dir, exist_ok=True)
        # 生成输出文件路径
        filename = os.path.basename(original_img_path)
        output_path = os.path.join(class_dir, filename)
        # 将预处理后的图像保存为 NumPy 文件
        np.save(output_path.replace(".jpg", ".npy"), img.numpy())
        # 或者将预处理后的图像保存为 JPEG 文件
        # torchvision.utils.save_image(img, output_path)

# 导出训练集和验证集
export_dataset(train_dataset, train_output_dir)
export_dataset(val_dataset, val_output_dir)

print("预处理后的图像已导出到", output_dir)