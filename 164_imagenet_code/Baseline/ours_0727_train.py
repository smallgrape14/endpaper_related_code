# 版本说明，这个版本是和GPUNetIO 交互的AI train版本
# 0620 对目前的这个版本进行相关的优化工作，优化数据加载时间
# 164docker GPUNetIO 用的数据加载代码
import cupy as cp
import numpy as np
import torch
from torch.utils.data import Dataset, DataLoader
import numpy as np
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
# pip install torch_tb_profiler 需要安装这个库
from torch.cuda import nvtx
from torch.optim.lr_scheduler import StepLR
from enum import Enum
CHECK_DATA_LOADER_RIGHT=0 # 这个变量是用来检查数据加载器是否正确的，如果为1，则会导出数据到output目录下，方便检查数据加载器是否正确
MAX_FILE_SIZE=1*1024*1024
NUM_Classes = 1000 #1000 #10 #00 #1000
TOTAL_FILE_NUM=1281167 #12947 #12947 #12947 #2600 #1281167
GPUNetIO_On=1 #表示是否GPUNETIO程序常驻
Max_Batch=20
#-------需要频繁修改的参数------------
BATCH_SIZE=16
EPOCH_NUM =2
print_freq=1
NetIO_Block=1 # 4
NetIO_Thread=1
# --------------------------------

BATCH_NUM=32

global_data_loader=0
One_A_Buf=520*1024*1024 #1030*1024*1024 #520*1024*1024     # 520MB 的缓冲区大小，假设存储每个batch的数据缓冲区大小，支持最大batchsize=512
Opt_Mode= 1 # opt_mode=3 _read_file_infos中是批量获取元数据，opt_mode=1 _read_file_infos中是非批量获取元数据，现在性能好的是mode=1

Open_nvtx = 0 # 是否开启nvtx标记，0表示不开启，1表示开启
class FileInfo:
    def __init__(self, data):
        # self.file_path = data[0]#@@--begin
        # self.file_begin = data[1]
        # self.file_size = data[2]
        # self.label = data[3] #label在__init__中被定义为公共属性（非双下划线开头），外部可直接访问
        self.file_begin = data[0]
        self.file_size = data[1]
        self.label = data[2] #la

class SharedMemoryDataset(Dataset):
    def __init__(self, ipc_handle_file,prefetch_idx=0, max_file_path_len=128):
        with open(ipc_handle_file, "rb") as f:
            self.handle_bytes = f.read()
        self.handle = bytes(self.handle_bytes)
        self.memhandler = cp.cuda.runtime.ipcOpenMemHandle(self.handle, cp.cuda.runtime.cudaMemAttachGlobal)
        self.begin_offset=prefetch_idx*One_A_Buf # 预取索引
        self.max_file_path_len = max_file_path_len 
        self.file_info_size = max_file_path_len + 3 * np.dtype(np.uint64).itemsize
        self.file_num = self._read_file_num()
        self.file_infos = self._read_file_infos()


    def _read_file_num(self):
        # 读取文件数量
        file_num_ptr = cp.cuda.MemoryPointer(cp.cuda.UnownedMemory(self.memhandler, 8, 0), 0+self.begin_offset)
        #[XYP_0620]
        file_num = cp.ndarray(shape=(), dtype=np.uint64, memptr=file_num_ptr)
        file_num = int(file_num.get())
        # print("file_num :",file_num)
        return file_num


        
   
    def _read_file_infos(self):
        if Opt_Mode == 3 :
            #批量读取文件元数据到显存
            # 计算元数据总大小
            total_meta_size = self.file_num * self.file_info_size
            
            # 创建显存映射的元数据块 
            meta_ptr = cp.cuda.MemoryPointer(
                cp.cuda.UnownedMemory(self.memhandler, total_meta_size, 0),
                # 4 + self.begin_offset  # 跳过文件数量字段
                8 + self.begin_offset  # 跳过文件数量字段 #[XYP_0620]

            )
            self.meta_block = cp.ndarray(
                (total_meta_size,), 
                dtype=np.uint8, 
                memptr=meta_ptr
            )
            
            # 解析元数据 
            file_infos = []
            for idx in range(self.file_num):
                start = idx * self.file_info_size
                end = start + self.file_info_size
                
                file_begin = self.meta_block[start+self.max_file_path_len:start+self.max_file_path_len+8].view(np.uint64)[0]
                file_size = self.meta_block[start+self.max_file_path_len+8:start+self.max_file_path_len+16].view(np.uint64)[0]
                label = self.meta_block[start+self.max_file_path_len+16:start+self.max_file_path_len+24].view(np.uint64)[0]
                print(f"{idx} th File :  file_begin = {file_begin}; \t file_size = {file_size}; \t label = {label}")
                
                file_infos.append(FileInfo((file_begin, file_size, label)))
        elif Opt_Mode == 1:
            file_infos = []
            offset = 8+self.begin_offset  # uint32_t的大小是4字节 表示的是文件数量  [XYP_0620] --> 改成 8B
            file_idx=0
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
                # print(f"{file_idx} label = {label}")
                file_infos.append(FileInfo((file_begin, file_size, label)))##@@--begin

                file_idx=file_idx+1
        else:
            print("Error : unkown opt mode ")
        return file_infos
    
    def __len__(self):
        return self.file_num

    def __getitem__(self, idx):
        
        file_info = self.file_infos[idx]
        data_ptr = cp.cuda.MemoryPointer(cp.cuda.UnownedMemory(self.memhandler, MAX_FILE_SIZE-128, 0),self.begin_offset+file_info.file_begin+128) # 偏移128B 的原因是因为 128B 是 numpy 格式数据的头部字段长度
        data = cp.ndarray(shape=(3, 224, 224), dtype=np.float32, memptr=data_ptr)

        # if CHECK_DATA_LOADER_RIGHT==1 :
        #     # 导出数据到文件
        #     # file_path = file_info.file_path
        #     export_file_name = f"IPC/Train/GPUNetIO/output/{idx}"  # 使用索引和文件名作为新文件名

        #     # export_file_name = f"output/{file_path.split('/')[-1]}"  # 使用索引和文件名作为新文件名
        #     with open(export_file_name, "wb") as f:
        #         f.write(data.get().tobytes())
        #     # 获取实际写入的字节数
        #     actual_bytes_written = os.path.getsize(export_file_name)
        #     print(f"Exported data from index {idx} to file: {export_file_name}, byte is {actual_bytes_written}")


        return torch.as_tensor(data, device='cuda'), torch.as_tensor(file_info.label, device='cuda',dtype=torch.long)

    #新增在SharedMemoryDataset类中
    # def __del__(self):
    #     cp.cuda.runtime.ipcCloseMemHandle(self.memhandler)


SEND_QUEUE = "/py_to_cpp"
RECV_QUEUE = "/cpp_to_py"

# 全局消息队列对象
mq_send = None
mq_recv = None

def signal_handler(signum, frame):
    print("\nPython: 正在退出...")
    # cleanup()
    sys.exit(0)

def init_queues():
    global mq_send, mq_recv
    try:
        # 发送队列（Python→C++）
        mq_send = posix_ipc.MessageQueue(
            SEND_QUEUE,
            flags=posix_ipc.O_CREAT | posix_ipc.O_EXCL,  # 强制创建新队列
            mode=0o666,
            max_messages=1000,
            max_message_size=100
        )
    except posix_ipc.ExistentialError:
        # 队列已存在，直接打开
        mq_send = posix_ipc.MessageQueue(SEND_QUEUE)
    
    # 接收队列（C++→Python）同理
    try:
        mq_recv = posix_ipc.MessageQueue(
            RECV_QUEUE,
            flags=posix_ipc.O_CREAT | posix_ipc.O_EXCL,
            mode=0o666,
            max_messages=1000,
            max_message_size=100
        )
    except posix_ipc.ExistentialError:
        mq_recv = posix_ipc.MessageQueue(RECV_QUEUE)

def clean_queue():
    global mq_send, mq_recv
    
    # 先关闭描述符
    if mq_send:
        mq_send.close() # 关闭操作释放进程级别的资源，但不影响其他进程使用该队列
        mq_send = None
    if mq_recv:
        mq_recv.close()
        mq_recv = None
    
    # 后删除队列（无需检查存在性）
    try:
        posix_ipc.unlink_message_queue(SEND_QUEUE) # unlink 逻辑中已经包含存在性检查
    except posix_ipc.ExistentialError:
        pass
    try:
        posix_ipc.unlink_message_queue(RECV_QUEUE)
    except posix_ipc.ExistentialError:
        pass

def accuracy(output, target, topk=(1,)):
    """Computes the accuracy over the k top predictions for the specified values of k"""
    with torch.no_grad():
        maxk = max(topk)
        batch_size = target.size(0)

        _, pred = output.topk(maxk, 1, True, True)
        pred = pred.t()
        correct = pred.eq(target.view(1, -1).expand_as(pred))

        res = []
        for k in topk:
            correct_k = correct[:k].reshape(-1).float().sum(0, keepdim=True)
            res.append(correct_k.mul_(100.0 / batch_size))
        return res

class Summary(Enum):
    NONE = 0
    AVERAGE = 1
    SUM = 2
    COUNT = 3

class AverageMeter(object):
    """Computes and stores the average and current value"""
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
        if torch.cuda.is_available():
            device = torch.device("cuda")
        elif torch.backends.mps.is_available():
            device = torch.device("mps")
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

    def display(self, batch):
        entries = [self.prefix + self.batch_fmtstr.format(batch)]
        entries += [str(meter) for meter in self.meters]
        print('\t'.join(entries))
        
    def display_summary(self):
        entries = [" *"]
        entries += [meter.summary() for meter in self.meters]
        print(' '.join(entries))

    def _get_batch_fmtstr(self, num_batches):
        num_digits = len(str(num_batches // 1))
        fmt = '{:' + str(num_digits) + 'd}'
        return '[' + fmt + '/' + fmt.format(num_batches) + ']'


def import_shared_memory_and_train(_batch_size=16):
    
    # 在训练开始之前，初始化 CSV 文件并写入表头
    csv_file_path = "IPC/Train/GPUNetIO/result/epoch_time_decomp_0702_singleGPU.csv"
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
        fieldnames = ['batch_size','epoch','blocknum','threadnum', 'total_time', 'data_loading_time', 'training_time']
        writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
        if write_header:
            writer.writeheader()
    
    ## 每个batch平均值的记录文件
        # 在训练开始之前，初始化 CSV 文件并写入表头
    avg_csv_file_path = "IPC/Train/GPUNetIO/result/avg_batch_time_decomp_0702_singleGPU.csv"
    # 检查文件是否存在并且是否有表头
    write_header = not os.path.exists(avg_csv_file_path)
    if not write_header:
        with open(avg_csv_file_path, mode='r', newline='') as csv_file:
            reader = csv.reader(csv_file)
            try:
                header = next(reader)
                write_header = len(header) == 0 or header[0] != 'batch_size'
            except StopIteration:
                write_header = True

    with open(avg_csv_file_path, mode='a', newline='') as csv_file:
        # fieldnames_batch = ['batch_size','blocknum','threadnum','AVG_data', 'AVG_train','MIN_data','MIN_train','Median_data','Median_train','p99_data','p99_train']
        fieldnames_batch = ['batch_size','blocknum','threadnum','AVG_data','MIN_data','MAX_data', 'Median_data','p99_data','AVG_train','MIN_train','MAX_train','Median_train','p99_train']

        writer = csv.DictWriter(csv_file, fieldnames=fieldnames_batch)
        if write_header:
            writer.writeheader()
    # ipc_handle_file_0 = "/doca_devel/164_docker_2.9/samples/原doca_gpunetio_ReadRDMAMsg_Multi-QP_0605_接入DALI预处理/gpunetio_rdma_client_server_write/IPC/ipc_handle_0.bin"
    # ipc_handle_file_1 = "/doca_devel/164_docker_2.9/samples/原doca_gpunetio_ReadRDMAMsg_Multi-QP_0605_接入DALI预处理/gpunetio_rdma_client_server_write/IPC/ipc_handle_1.bin"
    ipc_handle_file_0 = "IPC/ipc_handle_0.bin"
    ipc_handle_file_1 = "IPC/ipc_handle_1.bin"
    
    MSG_START_TRAINING = "START_TRAINING"
    MSG_START_TRAINING_0 = "START_TRAINING_0"
    MSG_START_TRAINING_1 = "START_TRAINING_1"

    MSG_DATA_LOADED_0 = "DATA_LOADED_0"
    MSG_DATA_LOADED_1 = "DATA_LOADED_1"
    MSG_DATA_LOADED = "DATA_LOADED"
    # MSG_BATCH_DONE = "BATCH_DONE"
    MSG_TRAIN_DONE = "TRAIN_DONE"

    list_data=[]
    list_data.append(MSG_DATA_LOADED_0)
    list_data.append(MSG_DATA_LOADED_1)
    list_train=[]
    list_train.append(MSG_START_TRAINING_0)
    list_train.append(MSG_START_TRAINING_1)


    # 初始化消息队列
    init_queues()
    
    # 注册信号处理
    signal.signal(signal.SIGINT, signal_handler)
    # 通知C++端开始读数据
    print("Python: Sending START_TRAINING command.")
    mq_send.send(MSG_START_TRAINING_0)
    mq_send.send(MSG_START_TRAINING_1)


    # 等待C++端通知数据已加载完成
    msg, _ = mq_recv.receive()
    msg = msg.decode().strip('\x00')
    

    if msg in list_data: #检查 msg 是否在 list_data 中

        print(f"Python: Received MSG {msg}.")

        # 模拟训练一个批次
        cur_batch_idx=list_data.index(msg) # 获取当前批次索引
        print(f"Python: Current batch index is {cur_batch_idx}.")

        # 定义模型、损失函数和优化器
        # model = models.resnet18(pretrained=False)
        model = models.resnet18()

        num_classes = NUM_Classes   #len(dataset.num_classes) # todolist 增加目录类别时需要修改这个值 
        model.fc = nn.Linear(model.fc.in_features, num_classes)

        gpu_id=0 #换成GPU0
        torch.cuda.set_device(gpu_id)
        model = model.cuda(gpu_id)
        if torch.cuda.is_available(): #@@ 根据设备可用性，设置 device 变量，用于后续的张量操作。
            device = torch.device('cuda:{}'.format(gpu_id)) 
        else:
            print("[ERROR] 没有可用的 GPU，使用 CPU 进行训练")
            device = torch.device("cpu")
        
        # 定义损失函数和优化器
        # criterion = nn.CrossEntropyLoss()
        criterion = nn.CrossEntropyLoss().to(device)
        optimizer = optim.SGD(model.parameters(), lr=0.01, momentum=0.9,
            weight_decay=1e-4)
            # weight_decay=1e-5)

        scheduler = StepLR(optimizer, step_size=30, gamma=0.1)
        # scheduler = StepLR(optimizer, step_size=15, gamma=0.5)


        batch_data_list=[]
        batch_train_list=[]

        # 训练循环
        num_epochs=EPOCH_NUM
        dataset = SharedMemoryDataset(ipc_handle_file_0,cur_batch_idx)
        train_loader = DataLoader(dataset, batch_size=_batch_size, shuffle=True,drop_last=True)
        for epoch in range(num_epochs):
            batch_time = AverageMeter('Time', ':2.6f') # 批处理总时间统计
            train_time= AverageMeter('Train', ':2.6f') # 训练时间统计
            data_time = AverageMeter('Data', ':2.6f') # 数据加载时间统计
            losses = AverageMeter('Loss', ':.4e')
            top1 = AverageMeter('Acc@1', ':2.4f')
            # top5 = AverageMeter('Acc@5', ':2.4f')
            progress = ProgressMeter( #初始化一个进度条对象 ProgressMeter，用于在训练过程中打印训练进度。
                len(train_loader),
                # [batch_time, data_time,train_time, losses, top1, top5],
                [batch_time, data_time,train_time, losses, top1],
                prefix="Epoch: [{}]".format(epoch))


            model.train()
            running_loss = 0.0
            running_corrects = 0
            file_num=0
            Total_File_Num=TOTAL_FILE_NUM #_batch_size*BATCH_NUM  #*64
            
            # data_loading_start_time = time.time()  # 记录数据加载开始时间
            epoch_train_time =0
            epoch_data_time=0
            epoch_total_time=0
            avg_data_time=0
            avg_train_time=0
            avg_total_time=0
            batch_num=0
            acc1_list=[]
            acc2_list=[]
            global global_data_loader
            global_data_loader+=1
            epoch_start_time = time.time()  # 记录 epoch 开始时间
            if Open_nvtx == 0 :
                while(file_num < Total_File_Num):
                    batch_start_time =time.time()

                    dataset = SharedMemoryDataset(ipc_handle_file_0,cur_batch_idx)

                    train_loader = DataLoader(dataset, batch_size=_batch_size, shuffle=True,drop_last=True)


                    iteration_time_begin=time.time()
                    # for images, labels  in train_loader:
                    for i, (images, labels) in enumerate(train_loader):

                        transfer_time_begin=time.time()
                        labels = labels.cuda(non_blocking=True)
                        torch.cuda.current_stream().synchronize()

                        batch_train_begin=time.time()
                        data_time.update(batch_train_begin - batch_start_time)

                        
                        outputs = model(images)
                        loss = criterion(outputs, labels) 

                        acc1, acc2 = accuracy(outputs, labels, topk=(1, 2))
                        losses.update(loss.item(), images.size(0))
                        top1.update(acc1[0], images.size(0))
                        # top5.update(acc2[0], images.size(0))

                        optimizer.zero_grad()
                        loss.backward()
                        optimizer.step()
                        batch_train_end=time.time()

                        batch_time.update(batch_train_end - batch_start_time)
                        train_time.update(batch_train_end- batch_train_begin) # 记录训练时间

                        if i % print_freq == 0:
                            progress.display(i + 1)

                        running_loss += loss.item() * images.size(0)
                        _, preds = torch.max(outputs, 1)
                        running_corrects += torch.sum(preds == labels.data)
                        file_num=file_num+_batch_size
                    
                    batch_num+=1
                    # 通知C++端开始读数据
                    if(epoch==num_epochs-1 and file_num >= Total_File_Num or batch_num >Max_Batch): # 如果是最后一个epoch 且 文件已经全部加载了 就不需要再加载数据了
                        epoch_data_time+=batch_train_begin-batch_start_time
                        epoch_train_time+=batch_train_end-batch_train_begin
                        epoch_total_time+=batch_train_end-batch_start_time
                        break
                    else:
                        mq_begin=time.time()
                        mq_send.send(list_train[cur_batch_idx])
                        # 等待C++端通知数据已加载完成
                        msg, _ = mq_recv.receive()
                        msg = msg.decode().strip('\x00')
                        while(msg not in list_data): # 检查 msg 是否在 list_data 中
                            msg, _ = mq_recv.receive()
                            msg = msg.decode().strip('\x00')
                    
                        cur_batch_idx=list_data.index(msg) # 切换到下一个批次
                        mq_end=time.time()
                        batch_idx=int(file_num/_batch_size)
                        # if(batch_idx<10):
                        #     print(f"batch{batch_idx}: iteration_time : {(transfer_time_begin-iteration_time_begin):.6f} ; transfer_time :{(batch_train_begin-transfer_time_begin):.6f}, percent: {1.0*(batch_train_begin-transfer_time_begin)/(batch_train_begin-batch_start_time):.2f}; data_time_iteration:{(batch_train_begin-batch_start_time):.6f},percent{(1.0*(batch_train_begin-batch_start_time)/(batch_data_end-batch_train_end+batch_train_begin-batch_start_time)):.3f} data time: {(batch_data_end-batch_train_end+batch_train_begin-batch_start_time):.6f}, train time:{(batch_train_end-batch_train_begin):.6f}")
                        

                        epoch_data_time+=mq_end-mq_begin+batch_train_begin-batch_start_time
                        epoch_train_time+=batch_train_end-batch_train_begin
                        epoch_total_time+=mq_end-mq_begin+batch_train_begin-batch_start_time+batch_train_end-batch_train_begin
                        batch_data_list.append(mq_end-mq_begin+batch_train_begin-batch_start_time)
                        batch_train_list.append(batch_train_end-batch_train_begin)
                        
            else:
                while(file_num < Total_File_Num):
                    batch_start_time =time.time()
                    with nvtx.range("dataset"):
                        dataset = SharedMemoryDataset(ipc_handle_file_0,cur_batch_idx)
                        train_loader = DataLoader(dataset, batch_size=_batch_size, shuffle=True,drop_last=True)#num_workers=4 pin_memory=True 这些参数都不能用

                        iteration_time_begin=time.time()
                    with nvtx.range("iterate dataloader"):
                        for images, labels  in train_loader:
                        # for batch_idx, (images, labels) in enumerate(train_loader):
                            with nvtx.range("cpu_to_gpu"):
                                transfer_time_begin=time.time()
                                labels = labels.cuda()
                            with nvtx.range("training"):
                                batch_train_begin=time.time()

                                optimizer.zero_grad()
                                outputs = model(images)
                                
                                _, preds = torch.max(outputs, 1)
                                loss = criterion(outputs, labels) 

                                loss.backward()
                                optimizer.step()
                                running_loss += loss.item() * images.size(0)
                                running_corrects += torch.sum(preds == labels.data)
                                file_num=file_num+_batch_size
                                batch_train_end=time.time()
                    
                    # 通知C++端开始读数据
                    if(epoch==num_epochs-1 and file_num >= Total_File_Num): # 如果是最后一个epoch 且 文件已经全部加载了 就不需要再加载数据了
                        epoch_data_time+=batch_train_begin-batch_start_time
                        epoch_train_time+=batch_train_end-batch_train_begin
                        epoch_total_time+=batch_train_end-batch_start_time
                        break
                    else:
                        # print(f"epoch {epoch} Python: Sending START_TRAINING command.")
                        with nvtx.range("info_mq_communicate"):
                            mq_send.send(list_train[cur_batch_idx])
                            # print(f"epoch {epoch} Python: Sent DATA message {list_train[cur_batch_idx]}.")
                            # 等待C++端通知数据已加载完成
                            msg, _ = mq_recv.receive()
                            msg = msg.decode().strip('\x00')
                            while(msg not in list_data): # 检查 msg 是否在 list_data 中
                                msg, _ = mq_recv.receive()
                                msg = msg.decode().strip('\x00')
                        
                            cur_batch_idx=list_data.index(msg) # 切换到下一个批次
                            batch_data_end=time.time()
                            batch_idx=int(file_num/_batch_size)
                        # if(batch_idx<10):
                        #     print(f"batch{batch_idx}: iteration_time : {(transfer_time_begin-iteration_time_begin):.6f} ; transfer_time :{(batch_train_begin-transfer_time_begin):.6f}, percent: {1.0*(batch_train_begin-transfer_time_begin)/(batch_train_begin-batch_start_time):.2f}; data_time_iteration:{(batch_train_begin-batch_start_time):.6f},percent{(1.0*(batch_train_begin-batch_start_time)/(batch_data_end-batch_train_end+batch_train_begin-batch_start_time)):.2f} data time: {(batch_data_end-batch_train_end+batch_train_begin-batch_start_time):.6f}, train time:{(batch_train_end-batch_train_begin):.6f}")

                        epoch_data_time+=batch_data_end-batch_train_end+batch_train_begin-batch_start_time
                        epoch_train_time+=batch_train_end-batch_train_begin
                        epoch_total_time+=batch_data_end-batch_start_time
                        # avg_data_time+=batch_data_end-batch_train_end+batch_train_begin-batch_start_time
                        # avg_train_time+=batch_train_end-batch_train_begin
                        # avg_total_time+=batch_data_end-batch_start_time
                        # batch_num+=1
                        # print(f"epoch {epoch} Python: Received DATA message_LOADED.")
                            

            # epoch_loss = running_loss / Total_File_Num
            # epoch_acc = running_corrects.double() / Total_File_Num

            scheduler.step()

            # [XYP_0625]
            epoch_loss = running_loss / file_num
            epoch_acc = running_corrects.double() / file_num
            
            print(f"epoch {epoch} Train Loss: {epoch_loss:.6f}, Acc: {epoch_acc:.6f} , data time: {epoch_data_time:.6f},train time: {epoch_train_time:.6f},total_time: {epoch_total_time:.6f}")
            print(f" AVG: data time: {1.0*epoch_data_time/batch_num:.6f},train time: {1.0*epoch_train_time/batch_num:.6f},total_time: {1.0*epoch_total_time/batch_num:.6f}")
            # 将时间数据写入 CSV 文件
            with open(csv_file_path, mode='a', newline='') as csv_file:
                writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
                writer.writerow({
                    'batch_size':BATCH_SIZE,
                    'epoch': epoch,
                    'blocknum':NetIO_Block,
                    'threadnum':NetIO_Thread,
                    'total_time': epoch_total_time,
                    'data_loading_time': epoch_data_time,
                    'training_time': epoch_train_time
                })
            if epoch == num_epochs-1 :# 只在最后一个epoch的时候，统计这些
                arr_data = np.array(batch_data_list)
                arr_train = np.array(batch_train_list)
                with open(avg_csv_file_path, mode='a', newline='') as csv_file:
                    writer = csv.DictWriter(csv_file, fieldnames=fieldnames_batch)
                    writer.writerow({
                        'batch_size':BATCH_SIZE,
                        'blocknum':NetIO_Block,
                        'threadnum':NetIO_Thread,
                        'AVG_data': np.mean(arr_data),
                        'AVG_train': np.mean(arr_train),
                        'MIN_data': np.min(arr_data),
                        'MIN_train': np.min(arr_train),
                        'MAX_data': np.max(arr_data),
                        'MAX_train': np.max(arr_train),
                        'Median_data': np.median(arr_data),
                        'Median_train': np.median(arr_train),
                        'p99_data': np.percentile(arr_data,99),
                        'p99_train': np.percentile(arr_train,99)
                    })
            # print(f"训练时间数据已导出到 {csv_file_path} 文件")
        if GPUNetIO_On == 0:
            # 通知C++端本次训练结束
            print("Python: Sending TRAIN_DONE command.")
            mq_send.send(MSG_TRAIN_DONE)
        # 保存模型
        # torch.save(model.state_dict(), 'model.pth')
        # print("模型已保存")
        print("训练已完成")
    else:
        print(f"未知消息: {msg}")
    # 清理
    if GPUNetIO_On == 0:
        clean_queue()
    # cleanup()


if __name__ == "__main__":
    global_data_loader=0

    import_shared_memory_and_train(_batch_size=BATCH_SIZE)