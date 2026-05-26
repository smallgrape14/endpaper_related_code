# 将数据集预处理
import os
import torch
from torchvision import datasets, transforms
from torch.utils.data import DataLoader
from PIL import Image
import argparse

def preprocess_dataset(input_dir, output_dir, transform, batch_size=32, num_workers=4):
    """
    预处理数据集并将结果保存到磁盘。

    参数:
        input_dir (str): 原始数据集的路径。
        output_dir (str): 预处理后数据的保存路径。
        transform (callable): 预处理操作。
        batch_size (int): 数据加载器的批次大小。
        num_workers (int): 数据加载器的工作进程数。
    """
    # 创建输出目录
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # 加载数据集
    dataset = datasets.ImageFolder(input_dir, transform=transforms.Compose([
        transform,
        transforms.ToTensor()
    ]))

    # 创建数据加载器
    data_loader = DataLoader(dataset, batch_size=batch_size, shuffle=False, num_workers=num_workers)

    # 遍历数据集并保存预处理后的图像
    for i, (images, labels) in enumerate(data_loader):
        for j, (image, label) in enumerate(zip(images, labels)):
            # 获取原始文件名
            original_file = dataset.samples[i * batch_size + j][0]
            file_name = os.path.basename(original_file)
            class_name = os.path.basename(os.path.dirname(original_file))

            # 创建类别目录
            class_dir = os.path.join(output_dir, class_name)
            if not os.path.exists(class_dir):
                os.makedirs(class_dir)

            # 保存预处理后的图像
            output_path = os.path.join(class_dir, file_name)
            image = transforms.ToPILImage()(image).convert("RGB")
            image.save(output_path)

        print(f"Processed batch {i + 1}/{len(data_loader)}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Preprocess dataset and save to disk")
    parser.add_argument("--input_dir", type=str, required=True, help="Path to the input dataset")
    parser.add_argument("--output_dir", type=str, required=True, help="Path to save the preprocessed dataset")
    parser.add_argument("--batch_size", type=int, default=32, help="Batch size for processing")
    parser.add_argument("--num_workers", type=int, default=4, help="Number of worker threads for data loading")
    parser.add_argument("--transform", type=str, default="Resize(256)", help="Preprocessing transform to apply")

    args = parser.parse_args()

    # 解析预处理操作
    transform = eval(f"transforms.{args.transform}")

    # 预处理数据集
    preprocess_dataset(args.input_dir, args.output_dir, transform, args.batch_size, args.num_workers)