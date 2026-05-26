# 版本说明：支持多GPU分布式训练
# 0701 修改为支持多GPU分布式训练
import cupy as cp
import numpy as np
import torch
import torch.distributed as dist
import torch.multiprocessing as mp
from torch.utils.data import Dataset, DataLoader
from torch.nn.parallel import DistributedDataParallel as DDP
import torchvision.models as models
import torch.nn as nn
import torch.optim as optim
import posix_ipc
import time
import signal
import os
import csv
import sys
# from torch.profiler import profile, record_function, ProfilerActivity
import torch.profiler
from torch.cuda import nvtx
from torch.optim.lr_scheduler import StepLR
from enum import Enum


# 配置参数
CHECK_DATA_LOADER_RIGHT = 0
MAX_FILE_SIZE = 1 * 1024 * 1024
BATCH_SIZE = 16
EPOCH_NUM = 2
print_freq = 1
NetIO_Block = 1
NetIO_Thread = 16
TOTAL_FILE_NUM = 2600
BATCH_NUM = 32
global_data_loader = 0
One_A_Buf = 520 * 1024 * 1024
Opt_Mode = 1
Open_nvtx = 0

GPUNetIO_On = 1 # 是否开启GPUNetIO，1表示GPUNetIO程序常驻，相同batchsize/blocknum/threadnum时候可以重复运行不同参数的train进程，

import logging

def setup_logger(rank):
    logging.basicConfig(
        level=logging.INFO,
        format=f"[Rank {rank}] %(asctime)s - %(message)s",
        handlers=[
            logging.FileHandler(f"log_rank{rank}.txt"),
            logging.StreamHandler(sys.stdout)
        ]
    )




class FileInfo:
    def __init__(self, data):
        self.file_begin = data[0]
        self.file_size = data[1]
        self.label = data[2]

class SharedMemoryDataset(Dataset):
    def __init__(self, ipc_handle_file, prefetch_idx=0, max_file_path_len=128):
        with open(ipc_handle_file, "rb") as f:
            self.handle_bytes = f.read()
        self.handle = bytes(self.handle_bytes)
        self.memhandler = cp.cuda.runtime.ipcOpenMemHandle(self.handle, cp.cuda.runtime.cudaMemAttachGlobal)
        self.begin_offset = prefetch_idx * One_A_Buf
        self.max_file_path_len = max_file_path_len
        self.file_info_size = max_file_path_len + 3 * np.dtype(np.uint64).itemsize
        self.file_num = self._read_file_num()
        self.file_infos = self._read_file_infos()

    def _read_file_num(self):
        file_num_ptr = cp.cuda.MemoryPointer(cp.cuda.UnownedMemory(self.memhandler, 8, 0), 0 + self.begin_offset)
        file_num = cp.ndarray(shape=(), dtype=np.uint64, memptr=file_num_ptr)
        return int(file_num.get())

    def _read_file_infos(self):
        if Opt_Mode == 3:
            total_meta_size = self.file_num * self.file_info_size
            meta_ptr = cp.cuda.MemoryPointer(
                cp.cuda.UnownedMemory(self.memhandler, total_meta_size, 0),
                8 + self.begin_offset
            )
            self.meta_block = cp.ndarray((total_meta_size,), dtype=np.uint8, memptr=meta_ptr)
            file_infos = []
            for idx in range(self.file_num):
                start = idx * self.file_info_size
                file_begin = self.meta_block[start + self.max_file_path_len:start + self.max_file_path_len + 8].view(np.uint64)[0]
                file_size = self.meta_block[start + self.max_file_path_len + 8:start + self.max_file_path_len + 16].view(np.uint64)[0]
                label = self.meta_block[start + self.max_file_path_len + 16:start + self.max_file_path_len + 24].view(np.uint64)[0]
                file_infos.append(FileInfo((file_begin, file_size, label)))
        elif Opt_Mode == 1:
            file_infos = []
            offset = 8 + self.begin_offset
            for _ in range(self.file_num):
                offset += self.max_file_path_len
                file_begin_ptr = cp.cuda.MemoryPointer(cp.cuda.UnownedMemory(self.memhandler, self.file_info_size, 0), offset)
                file_begin = cp.ndarray(shape=(), dtype=np.uint64, memptr=file_begin_ptr)
                file_begin = int(file_begin.get())
                offset += np.dtype(np.uint64).itemsize

                file_size_ptr = cp.cuda.MemoryPointer(cp.cuda.UnownedMemory(self.memhandler, self.file_info_size, 0), offset)
                file_size = cp.ndarray(shape=(), dtype=np.uint64, memptr=file_size_ptr)
                file_size = int(file_size.get())
                offset += np.dtype(np.uint64).itemsize

                label_ptr = cp.cuda.MemoryPointer(cp.cuda.UnownedMemory(self.memhandler, self.file_info_size, 0), offset)
                label = cp.ndarray(shape=(), dtype=np.uint64, memptr=label_ptr)
                label = int(label.get())
                offset += np.dtype(np.uint64).itemsize
                file_infos.append(FileInfo((file_begin, file_size, label)))
        else:
            print("Error: unknown opt mode")
        return file_infos

    def __len__(self):
        return self.file_num

    def __getitem__(self, idx):
        file_info = self.file_infos[idx]
        data_ptr = cp.cuda.MemoryPointer(
            cp.cuda.UnownedMemory(self.memhandler, MAX_FILE_SIZE - 128, 0),
            self.begin_offset + file_info.file_begin + 128
        )
        data = cp.ndarray(shape=(3, 224, 224), dtype=np.float32, memptr=data_ptr)
        return torch.as_tensor(data, device='cuda'), torch.as_tensor(file_info.label, device='cuda', dtype=torch.long)

    def __del__(self):
        cp.cuda.runtime.ipcCloseMemHandle(self.memhandler)

# 分布式训练初始化函数[1,4](@ref)
def setup(rank, world_size):
    """初始化分布式环境"""
    dist.init_process_group(
        backend='nccl',
        # init_method='env://',
        init_method='tcp://127.0.0.1:3689',
        rank=rank,
        world_size=world_size
    )
    torch.cuda.set_device(rank)

# 分布式训练清理函数[1](@ref)
def cleanup():
    dist.destroy_process_group()

# 消息队列函数（支持多rank）
def init_queues(rank):
    """为每个rank创建独立的消息队列"""
    # send_queue_name = f"/py_to_cpp_rank{rank}"
    send_queue_name = f"/py_to_cpp" #发送队列公用一个，接收队列有各自的

    recv_queue_name = f"/cpp_to_py_rank{rank}"
    
    try:
        mq_send = posix_ipc.MessageQueue(
            send_queue_name,
            # flags=posix_ipc.O_CREAT | posix_ipc.O_EXCL,
            flags=posix_ipc.O_CREAT,
            mode=0o666,
            max_messages=1000,
            max_message_size=100
        )
    except posix_ipc.ExistentialError:
        mq_send = posix_ipc.MessageQueue(send_queue_name)
    
    try:
        mq_recv = posix_ipc.MessageQueue(
            recv_queue_name,
            # flags=posix_ipc.O_CREAT | posix_ipc.O_EXCL,
            flags=posix_ipc.O_CREAT,

            mode=0o666,
            max_messages=1000,
            max_message_size=100
        )
    except posix_ipc.ExistentialError:
        mq_recv = posix_ipc.MessageQueue(recv_queue_name)
    
    return mq_send, mq_recv, send_queue_name, recv_queue_name

def close_queues(mq_send, mq_recv, send_queue_name, recv_queue_name):
    if mq_send:
        mq_send.close()
    if mq_recv:
        mq_recv.close()
    
    try:
        posix_ipc.unlink_message_queue(send_queue_name)
    except posix_ipc.ExistentialError:
        pass
    
    try:
        posix_ipc.unlink_message_queue(recv_queue_name)
    except posix_ipc.ExistentialError:
        pass

def signal_handler(signum, frame):
    print("\nPython: 正在退出...")
    sys.exit(0)

# 精度计算函数（分布式同步）[3](@ref)
def accuracy(output, target, topk=(1,)):
    with torch.no_grad():
        maxk = max(topk)
        batch_size = target.size(0)

        _, pred = output.topk(maxk, 1, True, True)
        pred = pred.t()
        correct = pred.eq(target.view(1, -1).expand_as(pred))

        res = []
        for k in topk:
            correct_k = correct[:k].reshape(-1).float().sum(0, keepdim=True)
            dist.all_reduce(correct_k)  # 同步所有GPU上的正确计数
            res.append(correct_k.mul_(100.0 / (batch_size * dist.get_world_size())))
        return res

class Summary(Enum):
    NONE = 0
    AVERAGE = 1
    SUM = 2
    COUNT = 3
'''
class AverageMeter(object):
    def __init__(self, name, fmt=':f', summary_type=Summary.AVERAGE):
        self.name = name
        self.fmt = fmt
        self.summary_type = summary_type
        self.reset()

    def reset(self):
        self.val = 0
        self.avg = 0
        self.sum = 0
        self.count = 0

    def update(self, val, n=1):
        self.val = val
        self.sum += val * n
        self.count += n
        self.avg = self.sum / self.count

    def all_reduce(self):
        device = torch.device("cuda")
        total = torch.tensor([self.sum, self.count], dtype=torch.float32, device=device)
        dist.all_reduce(total, dist.ReduceOp.SUM)
        self.sum, self.count = total.tolist()
        self.avg = self.sum / self.count

    def __str__(self):
        fmtstr = '{name} {val' + self.fmt + '} ({avg' + self.fmt + '})'
        return fmtstr.format(**self.__dict__)
    
    def summary(self):
        if self.summary_type is Summary.AVERAGE:
            fmtstr = '{name} {avg:.3f}'
        elif self.summary_type is Summary.SUM:
            fmtstr = '{name} {sum:.3f}'
        elif self.summary_type is Summary.COUNT:
            fmtstr = '{name} {count:.3f}'
        else:
            fmtstr = ''
        return fmtstr.format(**self.__dict__)
'''

class AverageMeter(object):
    #Computes and stores the average and current value
    def __init__(self, name, use_accel, fmt=':f', summary_type=Summary.AVERAGE):
        self.name = name
        self.use_accel = use_accel
        self.fmt = fmt
        self.summary_type = summary_type
        self.reset()

    def reset(self):
        self.val = 0
        self.avg = 0
        self.sum = 0
        self.count = 0

    def update(self, val, n=1):
        self.val = val
        self.sum += val * n
        self.count += n
        self.avg = self.sum / self.count

    def all_reduce(self):    
        if use_accel:
            device = torch.accelerator.current_accelerator()
        else:
            device = torch.device("cpu")
        total = torch.tensor([self.sum, self.count], dtype=torch.float32, device=device)
        dist.all_reduce(total, dist.ReduceOp.SUM, async_op=False)
        self.sum, self.count = total.tolist()
        self.avg = self.sum / self.count

    def __str__(self):
        fmtstr = '{name} {val' + self.fmt + '} ({avg' + self.fmt + '})'
        return fmtstr.format(**self.__dict__)
    
    def summary(self):
        fmtstr = ''
        if self.summary_type is Summary.NONE:
            fmtstr = ''
        elif self.summary_type is Summary.AVERAGE:
            fmtstr = '{name} {avg:.3f}'
        elif self.summary_type is Summary.SUM:
            fmtstr = '{name} {sum:.3f}'
        elif self.summary_type is Summary.COUNT:
            fmtstr = '{name} {count:.3f}'
        else:
            raise ValueError('invalid summary type %r' % self.summary_type)
        
        return fmtstr.format(**self.__dict__)


class ProgressMeter(object):
    def __init__(self, num_batches, meters, prefix=""):
        self.batch_fmtstr = self._get_batch_fmtstr(num_batches)
        self.meters = meters
        self.prefix = prefix

    def display(self, batch, rank):
        entries = [self.prefix + self.batch_fmtstr.format(batch)]
        entries += [str(meter) for meter in self.meters]
        print(f'Rank {rank}: ' + '\t'.join(entries))
        
    def display_summary(self, rank):
        entries = [" *"]
        entries += [meter.summary() for meter in self.meters]
        print(f'Rank {rank}: ' + ' '.join(entries))

    def _get_batch_fmtstr(self, num_batches):
        num_digits = len(str(num_batches // 1))
        fmt = '{:' + str(num_digits) + 'd}'
        return '[' + fmt + '/' + fmt.format(num_batches) + ']'

# 分布式训练主函数[1,3,7](@ref)
def train(rank, world_size, batch_size):
    # 在 train() 的开始处加上：
    setup_logger(rank) #可以打印的关键是这个
    logging.info("Begin DataLoader") 
    # 初始化分布式环境[1](@ref)
    setup(rank, world_size)
    if rank == 0:  # 仅在主进程打印
        print("This is train ",flush =True)
    else:
        print(f"Rank {rank} is ready for training.", flush=True)
    # 为每个rank创建独立的IPC句柄文件和消息队列
    ipc_handle_file = f"IPC/ipc_handle_{rank}.bin"
    mq_send, mq_recv, send_queue_name, recv_queue_name = init_queues(rank)
    
    # 注册信号处理
    signal.signal(signal.SIGINT, signal_handler)
    
    # 初始化CSV文件（只在rank0进行）[3](@ref)
    if rank == 0:
        csv_file_path = "IPC/Train/GPUNetIO/result/epoch_time_decomp_dist.csv"
        write_header = not os.path.exists(csv_file_path)
        with open(csv_file_path, mode='a', newline='') as csv_file:
            fieldnames = ['batch_size', 'epoch', 'blocknum', 'threadnum', 'total_time', 'data_loading_time', 'training_time']
            writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
            if write_header:
                writer.writeheader()
        
        avg_csv_file_path = "IPC/Train/GPUNetIO/result/avg_batch_time_decomp_dist.csv"
        write_header = not os.path.exists(avg_csv_file_path)
        with open(avg_csv_file_path, mode='a', newline='') as csv_file:
            fieldnames_batch = ['batch_size', 'blocknum', 'threadnum', 'AVG_data', 'MIN_data', 'MAX_data', 
                              'Median_data', 'p99_data', 'AVG_train', 'MIN_train', 'MAX_train', 
                              'Median_train', 'p99_train']
            writer = csv.DictWriter(csv_file, fieldnames=fieldnames_batch)
            if write_header:
                writer.writeheader()
    
    print(f"Rank {rank} 通知C++端开始读数据（每个rank独立通知）", flush=True)
    
    # 通知C++端开始读数据（每个rank独立通知）
    start_cmd = f"START_TRAINING_{rank}"
    mq_send.send(start_cmd)
    
    print(f"Rank {rank} 等待C++端通知数据已加载完成", flush=True)
    
    # 等待C++端通知数据已加载完成
    msg, _ = mq_recv.receive()
    msg = msg.decode().strip('\x00')
    if msg != f"DATA_LOADED_{rank}":
        print(f"Rank {rank} received unexpected message: {msg}")
        return
    
    print(f"Rank {rank} 创建模型并转移到当前GPU", flush=True)

    # 创建模型并转移到当前GPU
    model = models.resnet18(pretrained=False)
    num_classes = 2
    model.fc = nn.Linear(model.fc.in_features, num_classes)
    model = model.to(rank)
    
    print(f"Rank {rank} 使用DDP包装模型", flush=True)

    # 使用DDP包装模型[1,3](@ref)
    model = DDP(model, device_ids=[rank])
    
    print(f"Rank {rank} 定义损失函数和优化器", flush=True)
    # 定义损失函数和优化器
    criterion = nn.CrossEntropyLoss().to(rank)
    optimizer = optim.SGD(model.parameters(), lr=0.01, momentum=0.9, weight_decay=1e-4)
    scheduler = StepLR(optimizer, step_size=30, gamma=0.1)
    
    # 创建数据集和分布式采样器[3,7](@ref)
    dataset = SharedMemoryDataset(ipc_handle_file, rank)
    # sampler = torch.utils.data.distributed.DistributedSampler(
    #     dataset, 
    #     num_replicas=world_size, 
    #     rank=rank,
    #     shuffle=False
    # )
    train_loader = DataLoader(
        dataset, 
        batch_size=batch_size
        # sampler=sampler,
        # num_workers=0
        # pin_memory=True
    )
    
    batch_data_list = []
    batch_train_list = []
    use_accel=True
    # 训练循环
    for epoch in range(EPOCH_NUM):
        # 设置epoch保证shuffle有效性[3](@ref)
        # sampler.set_epoch(epoch)
        
        # batch_time = AverageMeter('Time', ':2.6f')
        # train_time = AverageMeter('Train', ':2.6f')
        # data_time = AverageMeter('Data', ':2.6f')
        # losses = AverageMeter('Loss', ':.4e')
        # top1 = AverageMeter('Acc@1', ':2.4f')
        # print(f"Rank {rank} 初始化 time ", flush=True)

        batch_time = AverageMeter('Time', use_accel, ':2.6f', Summary.NONE)
        train_time = AverageMeter('Train', use_accel, ':2.6f', Summary.NONE)
        data_time = AverageMeter('Data', use_accel, ':2.6f', Summary.NONE)
        losses = AverageMeter('Loss', use_accel, ':.4e', Summary.NONE)
        top1 = AverageMeter('Acc@1', use_accel, ':2.4f', Summary.NONE)
    
        progress = ProgressMeter(
            len(train_loader),
            [batch_time, data_time, train_time, losses, top1],
            prefix=f"Epoch [{epoch}]"
        )
        print(f"Rank {rank} 初始化 time ", flush=True)

        model.train()
        running_loss = 0.0
        running_corrects = 0
        file_num = 0
        Total_File_Num = TOTAL_FILE_NUM
        
        epoch_train_time = 0
        epoch_data_time = 0
        epoch_total_time = 0
        global global_data_loader
        global_data_loader += 1
        file_num = 0
        while(file_num < Total_File_Num):
            batch_start_time = time.time()

            dataset = SharedMemoryDataset(ipc_handle_file, rank)
            # sampler = torch.utils.data.distributed.DistributedSampler(
            #     dataset, 
            #     num_replicas=world_size, 
            #     rank=rank,
            #     shuffle=False
            # )
            train_loader = DataLoader(
                dataset, 
                batch_size=batch_size,
                shuffle=False
                # sampler=sampler,
                # num_workers=0,
                # pin_memory=True
            )
            
            for i, (images, labels) in enumerate(train_loader):
                
                
                # 确保数据在当前GPU上
                images = images.to(rank, non_blocking=True)
                labels = labels.to(rank, non_blocking=True)
                
                # 数据加载时间计算
                data_end_time = time.time()
                data_time.update(data_end_time - batch_start_time)
                
                # 训练步骤
                train_start_time = time.time()
                outputs = model(images)
                loss = criterion(outputs, labels)
                
                # 计算精度（分布式同步)
                acc1 = accuracy(outputs, labels, topk=(1,))[0]
                losses.update(loss.item(), images.size(0))
                top1.update(acc1[0], images.size(0))
                
                optimizer.zero_grad()
                loss.backward()
                optimizer.step()
                
                # 记录训练时间
                train_end_time = time.time()
                train_time.update(train_end_time - train_start_time)
                batch_time.update(train_end_time - batch_start_time)

                
                # 打印进度（只在rank0）
                if rank == 0 and i % print_freq == 0:
                    progress.display(i + 1, rank)
                
                running_loss += loss.item() * images.size(0)
                _, preds = torch.max(outputs, 1)
                running_corrects += torch.sum(preds == labels.data).item()
                file_num += batch_size
                
                # 记录批次时间

                # batch_num += 1
                if(epoch==EPOCH_NUM-1 and file_num >= Total_File_Num): # 如果是最后一个epoch 且 文件已经全部加载了 就不需要再加载数据了
                        epoch_data_time+=data_end_time-batch_start_time
                        epoch_train_time+=train_end_time-train_start_time
                        epoch_total_time+=train_end_time-batch_start_time
                        break
                else:
                    mq_begin=time.time()
                    mq_send.send(start_cmd)

                    msg, _ = mq_recv.receive()
                    msg = msg.decode().strip('\x00')
                    while( msg != f"DATA_LOADED_{rank}"):
                        print(f"Rank {rank} received unexpected message: {msg}")
                        msg, _ = mq_recv.receive()
                        msg = msg.decode().strip('\x00')
                    
                    mq_end=time.time()
                    batch_idx=int(file_num/batch_size)

                    batch_data_time = mq_end-mq_begin+data_end_time - batch_start_time
                    batch_train_time = train_end_time - train_start_time
                    epoch_data_time += batch_data_time
                    epoch_train_time += batch_train_time
                    epoch_total_time += batch_data_time+batch_train_time
                    batch_data_list.append(batch_data_time)
                    batch_train_list.append(batch_train_time)
                        
                    
            
        # 同步所有GPU[3](@ref)
        # dist.barrier()
        
        # 只在rank0记录日志和保存结果
        if rank == 0:
            epoch_loss = running_loss / file_num
            epoch_acc = running_corrects / file_num
            if rank == 0:  # 仅在主进程打印
                print(f"Epoch {epoch}: Loss={epoch_loss:.4f}, Acc={epoch_acc:.4f}, "
                    f"Data Time={epoch_data_time:.2f}s, Train Time={epoch_train_time:.2f}s",flush=True)
            
            # 写入CSV   
            with open("IPC/Train/GPUNetIO/result/epoch_time_decomp_dist.csv", mode='a', newline='') as csv_file:
                writer = csv.DictWriter(csv_file, fieldnames=[
                    'batch_size', 'epoch', 'blocknum', 'threadnum', 
                    'total_time', 'data_loading_time', 'training_time'
                ])
                writer.writerow({
                    'batch_size': batch_size,
                    'epoch': epoch,
                    'blocknum': NetIO_Block,
                    'threadnum': NetIO_Thread,
                    'total_time': epoch_total_time,
                    'data_loading_time': epoch_data_time,
                    'training_time': epoch_train_time
                })
            
            # 在最后一个epoch保存统计信息
            if epoch == EPOCH_NUM - 1:
                arr_data = np.array(batch_data_list)
                arr_train = np.array(batch_train_list)
                with open("IPC/Train/GPUNetIO/result/avg_batch_time_decomp_dist.csv", mode='a', newline='') as csv_file:
                    writer = csv.DictWriter(csv_file, fieldnames=[
                        'batch_size', 'blocknum', 'threadnum', 'AVG_data', 'MIN_data', 'MAX_data', 
                        'Median_data', 'p99_data', 'AVG_train', 'MIN_train', 'MAX_train', 
                        'Median_train', 'p99_train'
                    ])
                    writer.writerow({
                        'batch_size': batch_size,
                        'blocknum': NetIO_Block,
                        'threadnum': NetIO_Thread,
                        'AVG_data': np.mean(arr_data),
                        'MIN_data': np.min(arr_data),
                        'MAX_data': np.max(arr_data),
                        'Median_data': np.median(arr_data),
                        'p99_data': np.percentile(arr_data, 99),
                        'AVG_train': np.mean(arr_train),
                        'MIN_train': np.min(arr_train),
                        'MAX_train': np.max(arr_train),
                        'Median_train': np.median(arr_train),
                        'p99_train': np.percentile(arr_train, 99)
                    })
        
        # 更新学习率
        scheduler.step()
    
    # 训练结束通知
    # 不结束，这样就不用重复运行GPUNetIO了
    if GPUNetIO_On== 0 :
        mq_send.send("TRAIN_DONE")
    
    # 清理资源
    if GPUNetIO_On == 0:
        close_queues(mq_send, mq_recv, send_queue_name, recv_queue_name)
    cleanup()

# 分布式训练启动函数[1,3](@ref)
# def main(rank, world_size, batch_size=BATCH_SIZE):
#     train(rank, world_size, batch_size)

if __name__ == "__main__":
    # 获取GPU数量
    world_size = torch.cuda.device_count()
    print(f"Using {world_size} GPUs for distributed training.")
    
    # 启动多进程分布式训练[1,4](@ref)
    mp.spawn(
        # main,
        train,
        args=(world_size, BATCH_SIZE),
        nprocs=world_size,
        join=True
    )