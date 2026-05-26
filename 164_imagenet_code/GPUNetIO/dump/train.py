import cupy as cp
import torch
from torch.utils.data import Dataset, DataLoader
import numpy as np

class SharedMemoryDataset(Dataset):
    def __init__(self, ipc_handle_file, data_shape, data_type=np.float32):
        self.handle = cp.cuda.runtime.IpcMemHandle()
        with open(ipc_handle_file, "rb") as f:
            self.handle.value = f.read()
        self.ipc_mem_obj = cp.cuda.runtime.IpcMemObject(self.handle)
        self.mapped_ptr = self.ipc_mem_obj.map()
        self.data = cp.ndarray(shape=data_shape, dtype=data_type, memptr=cp.cuda.memory.MemoryPointer(self.mapped_ptr, data_shape[0] * np.dtype(data_type).itemsize))
        self.length = data_shape[0]

    def __len__(self):
        return self.length

    def __getitem__(self, idx):
        # 假设每个样本是一个固定大小的张量
        item_size = np.prod(self.data.shape[1:])  # 计算每个样本的元素数量
        sample = cp.ndarray(shape=self.data.shape[1:], dtype=self.data.dtype, memptr=cp.cuda.memory.MemoryPointer(self.mapped_ptr, idx * item_size * np.dtype(self.data.dtype).itemsize))
        return torch.as_tensor(sample, device='cuda')

def import_shared_memory_and_train():
    ipc_handle_file = "/home/oem/xyp/imagenet_code/backup/GPUNetIO/ipc_handle.bin"
    data_shape = (10, 3, 224, 224)  # 假设数据形状为 (样本数, 通道, 高, 宽)
    
    # 创建自定义数据集
    dataset = SharedMemoryDataset(ipc_handle_file, data_shape)
    
    # 创建DataLoader
    batch_size = 16
    dataloader = DataLoader(dataset, batch_size=batch_size, shuffle=True)
    
    # 定义模型、损失函数和优化器
    model = torch.nn.Linear(10, 2).cuda()
    criterion = torch.nn.MSELoss()
    optimizer = torch.optim.SGD(model.parameters(), lr=0.01)
    
    # 训练循环
    for epoch in range(10):
        model.train()
        for batch in dataloader:
            optimizer.zero_grad()
            outputs = model(batch)
            loss = criterion(outputs, torch.rand_like(outputs))  # 假设标签为随机数据
            loss.backward()
            optimizer.step()
            print(f'Epoch {epoch}, Loss: {loss.item()}')
    
    # 保存模型
    torch.save(model.state_dict(), 'model.pth')
    print("模型已保存")

if __name__ == "__main__":
    import_shared_memory_and_train()