import os
import numpy as np
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import Dataset, DataLoader
import torchvision.models as models
from torchvision import transforms
from PIL import Image
import csv
import time
import argparse  # 导入 argparse 模块

BATCH_SIZE=256
BATCH_NUM=64
EPOCH_NUM =10
WORKER_NUM=0

class CustomDataset(Dataset):
    def __init__(self, root_dir, transform=None):
        self.root_dir = root_dir
        self.transform = transform
        self.class_names = os.listdir(root_dir)#获取根目录下所有子目录的名称，假设每个子目录代表一个类别，将这些类别名称保存在 class_names 列表中
        self.image_paths = [] #初始化两个空列表，分别用于存储图像文件的路径和对应标签。
        self.labels = []
        # print(f"class_names is {self.class_names}")
        for label, class_name in enumerate(self.class_names):
            class_dir = os.path.join(root_dir, class_name)
            for filename in os.listdir(class_dir):
                if filename.endswith(('.npy', '.jpg', '.jpeg', '.png')):
                    image_path = os.path.join(class_dir, filename)
                    self.image_paths.append(image_path)
                    self.labels.append(label)

    def __len__(self):
        # print(f"data set len is {len(self.image_paths)}")

        return len(self.image_paths)
        # return 512

    def __getitem__(self, idx):
        # print(f"Getting item at index: {idx}")
        image_path = self.image_paths[idx]
        label = self.labels[idx]

        # 如果是 NumPy 文件
        if image_path.endswith('.npy'):
            image = np.load(image_path)
            image = torch.from_numpy(image).float()
        # 如果是图像文件
        else:
            image = Image.open(image_path)
            if self.transform:
                image = self.transform(image)

        return image, label




# 在训练开始之前，初始化 CSV 文件并写入表头
csv_file_path = "epoch_time_decomp_Baseline.csv"
# 检查文件是否存在并且是否有表头
write_header = not os.path.exists(csv_file_path)
if not write_header:
    with open(csv_file_path, mode='r', newline='') as csv_file:
        reader = csv.reader(csv_file)
        try:
            header = next(reader)
            write_header = len(header) == 0 or header[0] != 'batch_size'
        except StopIteration:
            write_header = True

with open(csv_file_path, mode='a', newline='') as csv_file:
    fieldnames = ['batch_size','worker_num','epoch', 'total_time', 'data_time', 'data_whole_time','training_time']
    writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
    if write_header:
        writer.writeheader()




# 数据集路径
# output_dir = "/home/oem/xyp/imagenet_test_decode3"  # 替换为你的输出目录
# output_dir = "/mnt/beegfs/imagenet_decode"  # 替换为你的输出目录
output_dir = "/mnt/beegfs/test_dir"  # 替换为你的输出目录


train_output_dir = os.path.join(output_dir, 'train')
# val_output_dir = os.path.join(output_dir, 'val')

# 定义数据预处理
train_transform = transforms.Compose([
    transforms.ToTensor(),
    transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])
])

# val_transform = transforms.Compose([
#     transforms.ToTensor(),
#     transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])
# ])

# 创建数据集
train_dataset = CustomDataset(train_output_dir, transform=train_transform)
# val_dataset = CustomDataset(val_output_dir, transform=val_transform)

# 创建数据加载器
batch_size = BATCH_SIZE
# train_loader = DataLoader(train_dataset, batch_size=batch_size, shuffle=True,num_workers=WORKER_NUM)
train_loader = DataLoader(train_dataset, batch_size=batch_size, shuffle=True)

# val_loader = DataLoader(val_dataset, batch_size=batch_size, shuffle=False)   

# 定义模型
model = models.resnet18(pretrained=False)
num_classes = len(train_dataset.class_names)
model.fc = nn.Linear(model.fc.in_features, num_classes)
model = model.cuda()

# 定义损失函数和优化器
criterion = nn.CrossEntropyLoss()
optimizer = optim.SGD(model.parameters(), lr=0.001, momentum=0.9)

# def train_model(model, criterion, optimizer, train_loader, val_loader, num_epochs=10):
def train_model(model, criterion, optimizer, train_loader, num_epochs=10):

    for epoch in range(num_epochs):
        # 训练阶段
        model.train()
        running_loss = 0.0
        running_corrects = 0
        file_num=0
        batch_idx=0
        epoch_start_time = time.time()  # 记录 epoch 开始时间
        epoch_train_time =0
        epoch_data_time=0
        epoch_data_whole_time=0 # 包含数据从CPU 加载到GPU 的时间
        epoch_total_time=0
        # 单独计时数据加载时间
        data_load_start = time.time()
        for inputs, labels in train_loader:
            data_load_end = time.time()

            inputs = inputs.cuda()
            labels = labels.cuda()
            cpu_to_gpu_end=time.time()

            optimizer.zero_grad()

            outputs = model(inputs)
            _, preds = torch.max(outputs, 1)
            loss = criterion(outputs, labels)

            loss.backward()
            optimizer.step()

            running_loss += loss.item() * inputs.size(0)
            running_corrects += torch.sum(preds == labels.data)
            # print(f"file num {file_num}")
            # print(f"images Shape = {inputs.shape}, Label = {labels}")

            # file_num=file_num+batch_size
            # print(f"file num {file_num}")
            train_end = time.time()
            
            batch_idx+=1
            # 统计时间，时间分解
            data_time=data_load_end-data_load_start
            data_whole_time=cpu_to_gpu_end-data_load_start
            train_time=train_end-cpu_to_gpu_end
            total_time=train_end-data_load_start
            epoch_data_time+=data_time
            epoch_data_whole_time+=data_whole_time
            epoch_train_time+=train_time
            epoch_total_time+=total_time
            # print(f"batch {batch_idx}: data time: {data_time:.6f},data whole time: {data_whole_time:.6f}, train time{train_time:.6f}, total time: {total_time:.6f}")

            data_load_start = time.time()




        epoch_loss = running_loss / len(train_loader.dataset)
        epoch_acc = running_corrects.double() / len(train_loader.dataset)

        print(f'epoch {epoch} Train Loss: {epoch_loss:.6f} Acc: {epoch_acc:.6f}, data time: {epoch_data_time:.6f}, data whole time: {epoch_data_whole_time:.6f},train time: {epoch_train_time:.6f},total_time: {epoch_total_time:.6f}')
        # 将时间数据写入 CSV 文件
        with open(csv_file_path, mode='a', newline='') as csv_file:
            writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
            writer.writerow({
                'batch_size':BATCH_SIZE,
                'worker_num':WORKER_NUM,
                'epoch': epoch,
                'total_time': epoch_total_time,
                'data_time': epoch_data_time,
                'data_whole_time': epoch_data_whole_time,
                'training_time': epoch_train_time
            })

        # print(f"训练时间数据已导出到 {csv_file_path} 文件")
        
        # 验证阶段
        # model.eval()
        # running_loss = 0.0
        # running_corrects = 0

        # with torch.no_grad():
        #     for inputs, labels in val_loader:
        #         inputs = inputs.cuda()
        #         labels = labels.cuda()

        #         outputs = model(inputs)
        #         _, preds = torch.max(outputs, 1)
        #         loss = criterion(outputs, labels)

        #         running_loss += loss.item() * inputs.size(0)
        #         running_corrects += torch.sum(preds == labels.data)

        # epoch_loss = running_loss / len(val_loader.dataset)
        # epoch_acc = running_corrects.double() / len(val_loader.dataset)

        # print(f'Val Loss: {epoch_loss:.4f} Acc: {epoch_acc:.4f}')
        

    return model

# 训练模型
# model = train_model(model, criterion, optimizer, train_loader, val_loader, num_epochs=1)
model = train_model(model, criterion, optimizer, train_loader, num_epochs=EPOCH_NUM)


# 保存模型
# torch.save(model.state_dict(), 'resnet18_custom_dataset.pth')
# print("模型已保存")
print("训练已经结束")