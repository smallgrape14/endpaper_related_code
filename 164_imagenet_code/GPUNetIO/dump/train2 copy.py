import cupy as cp
import torch
from torch.utils.data import Dataset, DataLoader
import numpy as np

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
        item_size = np.prod(self.data_shape) * np.dtype(self.data_type).itemsize
        mem_size = num_samples * item_size/1024
        print(f"data_shape={data_shape};\t num_samples={num_samples}; \t data_type={data_type};\t mem_size={mem_size}KB")
        self.mem_ptr=cp.cuda.MemoryPointer(cp.cuda.UnownedMemory(self.memhandler,mem_size, 0), 0)


    def __len__(self):
        return self.num_samples

    def __getitem__(self, idx):
        item_size = np.prod(self.data_shape) * np.dtype(self.data_type).itemsize
        offset = idx * item_size
        print(f"{idx}th File  offset is {offset};")
        sample_ptr = cp.cuda.memory.MemoryPointer(self.mem_ptr.mem, offset)
        sample = cp.ndarray(shape=self.data_shape, dtype=self.data_type, memptr=sample_ptr)
        return torch.as_tensor(sample, device='cuda')

def import_shared_memory_and_train():
    ipc_handle_file = "/home/oem/xyp/imagenet_code/backup/GPUNetIO/ipc_handle.bin"
    data_shape = (3, 224, 224)  # 假设每个样本的形状为 (通道, 高, 宽)
    num_samples = 10  # 假设有10个样本
    
    # 创建自定义数据集
    dataset = SharedMemoryDataset(ipc_handle_file, data_shape, num_samples)
    
    # 创建DataLoader
    batch_size = 1
    dataloader = DataLoader(dataset, batch_size=batch_size, shuffle=False)
    
    # 打印数据集和数据加载器的信息
    print(f"Dataset length: {len(dataset)}")
    print(f"Dataloader length: {len(dataloader)}")
    
    # 遍历数据加载器并打印每个批次的数据形状
    for i, batch in enumerate(dataloader):
        print(f"Batch {i}: Shape = {batch.shape}")
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