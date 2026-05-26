## [XYP_0626]:新增加 ACC1 和 Acc2 的计算，ACC1 是 top1 准确率，ACC2 是 top2 准确率
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
from torch.cuda import nvtx
from torch.optim.lr_scheduler import StepLR
from enum import Enum

NO_first_batch=1 #表示统计时间（数据加载还是训练）都去掉首个batch（每个epoch都是这样）
BATCH_SIZE=256
BATCH_NUM=64
EPOCH_NUM =1
WORKER_NUM=0
Total_File_num=2600
Conn_Mode=1 # 0: NFS, 1: BeeGFS, 2: local SataSSD, 3: local NVMe SSD
Open_NVTX=0
Open_acc1_2=1 # 是否打开 ACC1 和 ACC2 的计算
Clean_Cache=0 # 是否清理缓存
Modify_Train_End= 1 # 调整train end 的时间戳位置
Close_ACC=1 # 关闭ACC的相关计算
MODE_Train =3 # 1表示用的pytorch的train函数，2表示用的Baseline/Ours 的main.py中的train函数 3:现在正在修改的版本
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

# def train_model(model, criterion, optimizer, train_loader, val_loader, num_epochs=10):
def train_model(model, criterion, optimizer, train_loader,device,scheduler,print_freq=10, num_epochs=10,prefetch_factor=0):
    # 清理页面缓存、目录项和inode
    batch_data_list=[]
    batch_train_list=[]
    batch_todevice_list=[]
    if Clean_Cache == 1:
        os.system('sync; echo 3 > /proc/sys/vm/drop_caches')
    for epoch in range(num_epochs):
        batch_time = AverageMeter('Time', ':2.6f') # 批处理总时间统计
        train_time= AverageMeter('Train', ':2.6f') # 训练时间统计
        data_time = AverageMeter('Data', ':2.6f') # 数据加载时间统计
        losses = AverageMeter('Loss', ':.4e')
        top1 = AverageMeter('Acc@1', ':2.4f')
        top5 = AverageMeter('Acc@5', ':2.4f')
        progress = ProgressMeter( #初始化一个进度条对象 ProgressMeter，用于在训练过程中打印训练进度。
            len(train_loader),
            [batch_time, data_time,train_time, losses, top1, top5],
            prefix="Epoch: [{}]".format(epoch))
    
        # 训练阶段
        model.train()
        running_loss = 0.0
        running_corrects = 0
        file_num=0
        batch_idx=0
        acc1_list=[]
        acc2_list=[]
        epoch_train_time =0
        epoch_data_time=0 # 包含数据从CPU 加载到GPU 的时间
        epoch_data_whole_time=0 
        epoch_total_time=0
        epoch_todevice_time=0 # 数据从CPU 加载到GPU 的时间
        epoch_start_time = time.time()  # 记录 epoch 开始时间
        total_batches=int(Total_File_num/BATCH_SIZE)
        # 单独计时数据加载时间
        if Open_NVTX==1:
            batch_start = time.time()
            with nvtx.range("iterate dataloader"):
                for batch_idx in range(total_batches):
                    with nvtx.range("enumerate_batch"):  # NVTX标记枚举范围
                        inputs, labels = next(iter(train_loader))  # 直接获取下一批次
                # for i, (inputs, labels) in enumerate(train_loader):
                    # data_load_end = time.time()

                    # inputs = inputs.cuda(non_blocking=True) # 将张量、模型转换为GPU可用的格式
                    # labels = labels.cuda(non_blocking=True)
                    todevice_begin =time.time()
                    with nvtx.range("cpu_to_gpu"):
                        inputs = inputs.to(device, non_blocking=True)
                        labels = labels.to(device, non_blocking=True)
                        torch.cuda.current_stream().synchronize()
                    data_end=time.time()
                    
                    data_time.update(data_end - batch_start) # 对应日志中的"Data"列 即数据加载和预处理的时间

                    # optimizer.zero_grad()
                    #的确数据从CPU-GPU 应该是这里操作的，上面只是对数据做了格式转换 从CPU tensor 转化成 GPU tensor
                    with nvtx.range("training"):
                        with nvtx.range("model"):
                            outputs = model(inputs) 
                        with nvtx.range("criterion"):
                            loss = criterion(outputs, labels)
                        with nvtx.range("accuracy"):
                            acc1, acc5 = accuracy(outputs, labels, topk=(1, 5))
                            losses.update(loss.item(), inputs.size(0))
                            top1.update(acc1[0], inputs.size(0))
                            top5.update(acc5[0], inputs.size(0))
                        with nvtx.range("optimizer"):
                            optimizer.zero_grad()
                        with nvtx.range("loss"):
                            loss.backward()
                        with nvtx.range("optimizer_step"):
                            optimizer.step()
                    # if Modify_Train_End == 1: #和main.py一个位置的时间戳
                    batch_end = time.time()
                    batch_time.update(batch_end - batch_start)
                    train_time.update(batch_end- data_end) # 记录训练时间
                    batch_data_list.append(data_end - batch_start)
                    batch_train_list.append(batch_end- data_end)
                    batch_todevice_list.append(data_end-todevice_begin)

                    epoch_data_time+=data_end - batch_start
                    epoch_train_time+=batch_end- data_end
                    epoch_total_time+=batch_end - batch_start
                    epoch_todevice_time+=data_end-todevice_begin
            
                    

                    if batch_idx % print_freq == 0:
                        progress.display(batch_idx + 1)
                    batch_start = time.time()
        else:
            batch_start = time.time()
            # for inputs, labels in train_loader:
            for i, (inputs, labels) in enumerate(train_loader):
                # data_load_end = time.time()

                # inputs = inputs.cuda(non_blocking=True) # 将张量、模型转换为GPU可用的格式
                # labels = labels.cuda(non_blocking=True)
                todevice_begin =time.time()
                inputs = inputs.to(device, non_blocking=True)
                labels = labels.to(device, non_blocking=True)
                torch.cuda.current_stream().synchronize()
                data_end=time.time()
                # if NO_first_batch != 1 or i !=0 :
                data_time.update(data_end - batch_start) # 对应日志中的"Data"列 即数据加载和预处理的时间
                
                outputs = model(inputs) 

                loss = criterion(outputs, labels)

                acc1, acc5 = accuracy(outputs, labels, topk=(1, 2))
                losses.update(loss.item(), inputs.size(0))
                top1.update(acc1[0], inputs.size(0))
                top5.update(acc5[0], inputs.size(0))

                optimizer.zero_grad()
                loss.backward()
                optimizer.step()
                # if Modify_Train_End == 1: #和main.py一个位置的时间戳
                batch_end = time.time()
                batch_time.update(batch_end - batch_start)
                train_time.update(batch_end- data_end) # 记录训练时间
                # if NO_first_batch != 1 or i !=0 :
                batch_data_list.append(data_end - batch_start)
                batch_train_list.append(batch_end- data_end)
                batch_todevice_list.append(data_end-todevice_begin)

                epoch_data_time+=data_end - batch_start
                epoch_train_time+=batch_end- data_end
                epoch_total_time+=batch_end - batch_start
                epoch_todevice_time+=data_end-todevice_begin

                # print("data_time : ",data_end - batch_start)
                if i % print_freq == 0:
                    progress.display(i + 1)
                batch_start = time.time()
                batch_idx+=1
                #调试时候使用只测试50个batch
                # if(batch_idx>50):
                #     break

        # 将时间数据写入 CSV 文件
        with open(csv_file_path, mode='a', newline='') as csv_file:
            writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
            writer.writerow({
                'batch_size':BATCH_SIZE,
                'worker_num':WORKER_NUM,
                'prefetch_factor':prefetch_factor,
                'epoch': epoch,
                'total_time': epoch_total_time,
                'data_time': epoch_data_time,
                'data_todevice_time': epoch_todevice_time,
                'training_time': epoch_train_time
            })
        if epoch == num_epochs-1 :# 只在最后一个epoch的时候，统计这些
            arr_data = np.array(batch_data_list)
            arr_train = np.array(batch_train_list)
            arr_todevice=np.array(batch_todevice_list)
            with open(avg_csv_file_path, mode='a', newline='') as csv_file:
                writer = csv.DictWriter(csv_file, fieldnames=fieldnames_batch)
                writer.writerow({
                    'batch_size':BATCH_SIZE,
                    'worker_num':WORKER_NUM,
                    'prefetch_factor':prefetch_factor,
                    'AVG_data': np.mean(arr_data),
                    'AVG_train': np.mean(arr_train),
                    'AVG_todevice':np.mean(arr_todevice),
                    'MIN_data': np.min(arr_data),
                    'MIN_train': np.min(arr_train),
                    'MIN_todevice':np.min(arr_todevice),
                    'MAX_data': np.max(arr_data),
                    'MAX_train': np.max(arr_train),
                    'MAX_todevice':np.max(arr_todevice),
                    'Median_data': np.median(arr_data),
                    'Median_train': np.median(arr_train),
                    'Median_todevice':np.median(arr_todevice),
                    'p99_data': np.percentile(arr_data,99),
                    'p99_train': np.percentile(arr_train,99),
                    'p99_todevice':np.percentile(arr_todevice,99)
                })
        scheduler.step()

        

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


def train(train_loader, model, criterion, optimizer, epoch, device, print_freq):
    batch_time = AverageMeter('Time', ':2.6f') # 批处理总时间统计
    train_time= AverageMeter('Train', ':2.6f') # 训练时间统计
    data_time = AverageMeter('Data', ':2.6f') # 数据加载时间统计
    losses = AverageMeter('Loss', ':.4e')
    top1 = AverageMeter('Acc@1', ':2.4f')
    top5 = AverageMeter('Acc@5', ':2.4f')
    progress = ProgressMeter( #初始化一个进度条对象 ProgressMeter，用于在训练过程中打印训练进度。
        len(train_loader),
        [batch_time, data_time,train_time, losses, top1, top5],
        prefix="Epoch: [{}]".format(epoch))

    # switch to train mode
    model.train()

    end = time.time() # 记录循环开始时刻
    for i, (images, target) in enumerate(train_loader): #遍历 train_loader 中的每个批次数据。enumerate 用于获取批次索引 i 和批次数据 (images, target)
        
        #将输入图像 images 和目标标签 target 移动到指定的设备（如 GPU）
        #non_blocking 非阻塞模式，允许数据传输与计算并行进行，提高效率。
        images = images.to(device, non_blocking=True)
        target = target.to(device, non_blocking=True)
        torch.cuda.current_stream().synchronize()

        data_end=time.time() # 记录数据加载结束的时刻
        data_time.update(data_end - end) # 对应日志中的"Data"列 即数据加载和预处理的时间
        
        # compute output
        #将输入图像 images 传递给模型，得到模型的输出 output
        output = model(images)
        # 使用损失函数 criterion 计算模型输出与目标标签 target 之间的损失值 loss
        loss = criterion(output, target)

        # measure accuracy and record loss
        acc1, acc2 = accuracy(output, target, topk=(1, 2))
        losses.update(loss.item(), images.size(0))
        top1.update(acc1[0], images.size(0))
        top5.update(acc2[0], images.size(0))

        # compute gradient and do SGD step
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()

        # measure elapsed time
        batch_end=time.time() # 记录批处理结束的时刻
        
        batch_time.update(batch_end - end)
        train_time.update(batch_end- data_end) # 记录训练时间
        end = time.time()

        if i % print_freq == 0:
            progress.display(i + 1)



if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Train a model with different parameters.")
    parser.add_argument("--batch_size", type=int, default=256, help="Batch size for training")
    parser.add_argument("--epochs", type=int, default=10, help="Number of epochs for training")
    parser.add_argument("--workers", type=int, default=0, help="Number of worker threads for data loading")
    parser.add_argument("--conn_mode", type=int, default=0, help="Connection mode: 0 for NFS, 1 for BeeGFS, 2 for local SataSSD, 3 for local NVMe SSD")
    parser.add_argument("--prefetch", type=int, default=0, help="Dataloader prefetch factor ")
    parser.add_argument('--gpu', default=0, type=int,help='GPU id to use.')


    args = parser.parse_args()

    BATCH_SIZE = args.batch_size
    EPOCH_NUM = args.epochs
    WORKER_NUM = args.workers
    Conn_Mode = args.conn_mode
    prefetch=args.prefetch
    print(f"batch_size : {BATCH_SIZE},epoch_num: {EPOCH_NUM},worker_num : {WORKER_NUM},conn_mode : {Conn_Mode},prefetch :{prefetch}")

    # 在训练开始之前，初始化 CSV 文件并写入表头
    if Conn_Mode==0:
        csv_file_path = "whole_epoch_time_decomp_NFS_0525.csv"
    elif Conn_Mode==1:  
        csv_file_path = "result/0806/epoch_time_BeeGFS.csv"
    elif Conn_Mode==2:
        csv_file_path = "whole_epoch_time_decomp_sataSSD_0525.csv"
    elif Conn_Mode==3:
        csv_file_path = "whole_epoch_time_decomp_localNvmeSSD_0525.csv"


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
        fieldnames = ['batch_size','worker_num','prefetch_factor','epoch', 'total_time', 'data_time','training_time', 'data_todevice_time']
        writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
        if write_header:
            writer.writeheader()

    ## 每个batch平均值的记录文件
        # 在训练开始之前，初始化 CSV 文件并写入表头
    avg_csv_file_path = "result/0806/avg_batch_time_BeeGFS.csv"
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
        fieldnames_batch = ['batch_size','worker_num','prefetch_factor',
            'AVG_data','MIN_data','MAX_data', 'Median_data','p99_data',
            'AVG_train','MIN_train','MAX_train','Median_train','p99_train',
            'AVG_todevice','MIN_todevice','MAX_todevice','Median_todevice','p99_todevice']

        writer = csv.DictWriter(csv_file, fieldnames=fieldnames_batch)
        if write_header:
            writer.writeheader()


    # 数据集路径
    # output_dir #替换为你的输出目录
    if Conn_Mode==0:
        output_dir = "/mnt/163_nvme/imagenet_decode"  # 替换为你的输出目录 163 nvme SSD NFS
    elif Conn_Mode==1:
        # output_dir = "/mnt/beegfs/test_dir" # 替换为你的输出目录 beegfs 挂载目录
        # output_dir = "/mnt/beegfs/imagenet_10_decode" # 替换为你的输出目录 beegfs 挂载目录
        output_dir = "/mnt/beegfs/ImageNet_all" 

    elif Conn_Mode==2:  
        output_dir = "/home/oem/xyp/imagenet_decode" #  本地sata SSD
    elif Conn_Mode==3:      
        output_dir = "/mnt/nvme0/imagenet_decode" # 本地 西数 nvme SSD


    


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
    if (WORKER_NUM):
        train_loader = DataLoader(train_dataset,
            batch_size=batch_size, 
            shuffle=True,
            num_workers=WORKER_NUM,
            prefetch_factor=prefetch,
            pin_memory=True,
            drop_last=True,
            persistent_workers=True)  # 减少进程频繁创建销毁开销
    else:
        train_loader = DataLoader(train_dataset, batch_size=batch_size, shuffle=True,num_workers=WORKER_NUM,drop_last=True)

    # train_loader = DataLoader(train_dataset, batch_size=batch_size, shuffle=True)

    # val_loader = DataLoader(val_dataset, batch_size=batch_size, shuffle=False)   

    # 定义模型
    # model = models.resnet18(pretrained=False)
    model = models.resnet18()

    num_classes = len(train_dataset.class_names)
    print("num_classes is ",num_classes)
    model.fc = nn.Linear(model.fc.in_features, num_classes)
    # gpu_id=1
    gpu_id=args.gpu
    print(f"now you are using GPU {gpu_id}")
    torch.cuda.set_device(gpu_id)
    model = model.cuda(gpu_id)
    if torch.cuda.is_available(): #@@ 根据设备可用性，设置 device 变量，用于后续的张量操作。
        device = torch.device('cuda:{}'.format(gpu_id)) 
    else:
        print("没有可用的 GPU，使用 CPU 进行训练")
        device = torch.device("cpu")
    # 定义损失函数和优化器
    # criterion = nn.CrossEntropyLoss()
    criterion = nn.CrossEntropyLoss().to(device)
    # optimizer = optim.SGD(model.parameters(), lr=0.001, momentum=0.9)
    optimizer = optim.SGD(model.parameters(), lr=0.01, momentum=0.9,
        weight_decay=1e-4) #@@ 定义优化器，使用 SGD 优化器，学习率为 0.01，动量为 0.9，权重衰减为 1e-4。

    scheduler = StepLR(optimizer, step_size=30, gamma=0.1) #@@ 定义学习率调度器，每 30 个 epoch 将学习率乘以 0.1。
    print_freq =1 #@@ 打印频率，表示每多少个批次打印一次训练信息
    # 训练模型
    print("开始训练")
    if MODE_Train == 3:
        train_model(model, criterion, optimizer, train_loader,device,scheduler,print_freq, num_epochs=EPOCH_NUM,prefetch_factor=prefetch)
    # elif MODE_Train == 2:
    #     model = train_model(model, criterion, optimizer, train_loader, num_epochs=EPOCH_NUM,prefetch_factor=prefetch)
    elif MODE_Train == 1:

        
        for epoch in range(0, EPOCH_NUM):
        
            #@@ 调用 train 函数进行一个 epoch 的训练。
            train(train_loader, model, criterion, optimizer, epoch, device, print_freq)

            # #@@ 更新学习率调度器
            # scheduler.step()
    

    # 保存模型
    # torch.save(model.state_dict(), 'resnet18_custom_dataset.pth')
    # print("模型已保存")
    print("训练已经结束")
