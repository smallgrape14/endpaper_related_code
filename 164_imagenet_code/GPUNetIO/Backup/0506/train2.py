import cupy as cp
import torch
from torch.utils.data import Dataset, DataLoader
import numpy as np
import torchvision.models as models
import torch.nn as nn
import torch.optim as optim
class SharedMemoryDataset(Dataset):
    def __init__(self, ipc_handle_file, data_shape, num_samples, data_type=np.float32):
        with open(ipc_handle_file, "rb") as f:
            self.handle_bytes = f.read()
        self.handle = bytes(self.handle_bytes)
        self.memhandler = cp.cuda.runtime.ipcOpenMemHandle(self.handle, cp.cuda.runtime.cudaMemAttachGlobal)
        # MAX_FILE_INFOS_NUM = 10
        # MAX_FILE_PATH_LEN = 512
        # file_num_offset = 0
        # file_num_size = 4 
        # file_info_offset = file_num_offset + file_num_size
        # file_info_size = 528
        self.data_shape = data_shape
        self.num_samples = num_samples
        self.data_type = data_type
        self.num_classes=1
        self.labels = []
        self.labels.append(0) # 目前只有一个类

        item_size = np.prod(self.data_shape) * np.dtype(self.data_type).itemsize
        mem_size = num_samples * item_size/1024
        print(f"data_shape={data_shape};\t num_samples={num_samples}; \t data_type={data_type};\t mem_size={mem_size}KB")
        self.mem_ptr=cp.cuda.MemoryPointer(cp.cuda.UnownedMemory(self.memhandler,mem_size, 0), 0)


    def __len__(self):
        return self.num_samples

    def __getitem__(self, idx):
        item_size = np.prod(self.data_shape) * np.dtype(self.data_type).itemsize
        offset = idx * item_size
        # print(f"{idx}th File  offset is {offset};")
        sample_ptr = cp.cuda.memory.MemoryPointer(self.mem_ptr.mem, offset)
        sample = cp.ndarray(shape=self.data_shape, dtype=self.data_type, memptr=sample_ptr)
        # Todo : label 的获取需要对 memptr 对应的offset 进行解析
        label = 0
        return torch.as_tensor(sample, device='cuda'),label

def import_shared_memory_and_train():
    ipc_handle_file = "/home/oem/xyp/imagenet_code/backup/GPUNetIO/ipc_handle.bin"
    data_shape = (3, 224, 224)  # 假设每个样本的形状为 (通道, 高, 宽)
    num_samples = 10  # 假设有10个样本
    
    # 创建自定义数据集
    dataset = SharedMemoryDataset(ipc_handle_file, data_shape, num_samples)
    
    # 创建DataLoader
    batch_size = 1
    train_loader = DataLoader(dataset, batch_size=batch_size, shuffle=False)
    
    # # 打印数据集和数据加载器的信息
    print(f"Dataset length: {len(dataset)}")
    print(f"train_loader length: {len(train_loader)}")
    
    # # 遍历数据加载器并打印每个批次的数据形状
    # for i, batch in enumerate(train_loader):
    #     print(f"Batch {i}: Shape = {batch.shape}")
    # 定义模型、损失函数和优化器
    model = models.resnet18(pretrained=False)
    num_classes =1 #len(dataset.num_classes)
    model.fc = nn.Linear(model.fc.in_features, num_classes)
    model = model.cuda()
    # 定义损失函数和优化器
    criterion = nn.CrossEntropyLoss()
    optimizer = optim.SGD(model.parameters(), lr=0.001, momentum=0.9)
    
    # 训练循环
    num_epochs=1
    for epoch in range(num_epochs):
        model.train()
        running_loss = 0.0
        running_corrects = 0
        for images, labels  in train_loader:
            labels = labels.cuda()
            optimizer.zero_grad()
            outputs = model(images)
            
            _, preds = torch.max(outputs, 1)
            loss = criterion(outputs, labels) 

            loss.backward()
            optimizer.step()
            running_loss += loss.item() * images.size(0)
            running_corrects += torch.sum(preds == labels.data)
            # print(f'Epoch {epoch}, Loss: {loss.item()}')
        
        epoch_loss = running_loss / len(train_loader.dataset)
        epoch_acc = running_corrects.double() / len(train_loader.dataset)

        print(f'Train Loss: {epoch_loss:.6f} Acc: {epoch_acc:.6f}')
    
    # 保存模型
    # torch.save(model.state_dict(), 'model.pth')
    # print("模型已保存")
    print("训练已完成")
if __name__ == "__main__":
    import_shared_memory_and_train()