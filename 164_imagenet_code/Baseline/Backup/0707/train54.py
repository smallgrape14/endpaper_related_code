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
BATCH_SIZE=256
BATCH_NUM=64
EPOCH_NUM =1
WORKER_NUM=0
Conn_Mode=1 # 0: NFS, 1: BeeGFS, 2: local SataSSD, 3: local NVMe SSD
Open_NVTX=0
Have_Opt=1 # 1 表示采用dataprefetcher 的优化 ，0 表示不要优化
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

# DataPrefetcher (用于自动预取+搬运到GPU)
class DataPrefetcher:
    def __init__(self, loader):
        self.loader = iter(loader)
        self.stream = torch.cuda.Stream()
        self.input = None
        self.label = None
        self.preload()

    def preload(self):
        try:
            self.next_input, self.next_label = next(self.loader)
        except StopIteration:
            self.next_input = None
            self.next_label = None
            return
        with torch.cuda.stream(self.stream):
            self.next_input = self.next_input.cuda(non_blocking=True)
            self.next_label = self.next_label.cuda(non_blocking=True)

    def next(self):
        torch.cuda.current_stream().wait_stream(self.stream)
        input = self.next_input
        label = self.next_label
        if input is not None:
            input.record_stream(torch.cuda.current_stream())
        self.preload()
        return input, label



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
        epoch_train_time =0
        epoch_data_time=0
        epoch_data_whole_time=0 # 包含数据从CPU 加载到GPU 的时间
        epoch_total_time=0
        # 单独计时数据加载时间
        if Open_NVTX==1:
            with nvtx.range("iterate dataloader"):
                # with nvtx.range("data_load"):
                data_load_start = time.time()
                for inputs, labels in train_loader:
                    data_load_end = time.time()
                    with nvtx.range("cpu_to_gpu"):
                        inputs = inputs.cuda()
                        labels = labels.cuda()
                    with nvtx.range("training"):
                        with nvtx.range("zero_grad"):
                            cpu_to_gpu_end=time.time()
                            optimizer.zero_grad()
                        with nvtx.range("model"):
                            outputs = model(inputs)
                        with nvtx.range("criterion"):
                            _, preds = torch.max(outputs, 1)
                            loss = criterion(outputs, labels)
                        with nvtx.range("loss_backward"):
                            loss.backward()
                        with nvtx.range("optimizer_step"):
                            optimizer.step()
                        with nvtx.range("accumulate"):
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
                    # if(batch_idx>=1):
                    #     break

                    print(f"batch {batch_idx}: data time: {data_time:.6f},data whole time: {data_whole_time:.6f}, train time: {train_time:.6f}, total time: {total_time:.6f}")

                    data_load_start = time.time()
        else:
            batch_start = time.time()
            if Have_Opt== 0:
                for inputs, labels in train_loader:
                    data_load_end = time.time()

                    inputs = inputs.cuda(non_blocking=True) # 将张量、模型转换为GPU可用的格式
                    labels = labels.cuda(non_blocking=True)
                    torch.cuda.current_stream().synchronize()
                    cpu_to_gpu_end=time.time()

                    optimizer.zero_grad()
                    #的确数据从CPU-GPU 应该是这里操作的，上面只是对数据做了格式转换 从CPU tensor 转化成 GPU tensor
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
                    # if(batch_idx>=1):
                    #     break

                    # print(f"batch {batch_idx}: data time: {data_time:.6f},data whole time: {data_whole_time:.6f}, train time{train_time:.6f}, total time: {total_time:.6f}")

                    data_load_start = time.time()
            elif Have_Opt == 1:
                prefetcher = DataPrefetcher(train_loader)
                inputs, labels = prefetcher.next()
                while inputs is not None:

                    # inputs = inputs.cuda(non_blocking=True) # 将张量、模型转换为GPU可用的格式
                    # labels = labels.cuda(non_blocking=True)
                    # torch.cuda.current_stream().synchronize()
                    data_end=time.time()
                    data_time.update(data_end - batch_start)
                    #的确数据从CPU-GPU 应该是这里操作的，上面只是对数据做了格式转换 从CPU tensor 转化成 GPU tensor
                    outputs = model(inputs) 
                    loss = criterion(outputs, labels)
                    acc1, acc5 = accuracy(outputs, labels, topk=(1, 2))
                    losses.update(loss.item(), inputs.size(0))
                    top1.update(acc1[0], inputs.size(0))
                    top5.update(acc5[0], inputs.size(0))

                    optimizer.zero_grad()

                    loss.backward()
                    optimizer.step()
                    batch_end = time.time()
                    batch_time.update(batch_end - batch_start)
                    train_time.update(batch_end- data_end) # 记录训练时间
                    
                    if batch_idx % print_freq == 0:
                        progress.display(batch_idx + 1)

                    running_loss += loss.item() * inputs.size(0)
                    _, preds = torch.max(outputs, 1)
                    running_corrects += torch.sum(preds == labels.data)


                    batch_idx+=1
                    # 统计时间，时间分解
                    batch_data_time=data_end - batch_start
                    batch_train_time=batch_end- data_end
                    batch_total_time=batch_end-batch_start

                    epoch_data_time+=batch_data_time
                    epoch_train_time+=batch_train_time
                    epoch_total_time+=batch_total_time
                    # if(batch_idx>=1):
                    #     break

                    # print(f"batch {batch_idx}: data time: {data_time:.6f},data whole time: {data_whole_time:.6f}, train time{train_time:.6f}, total time: {total_time:.6f}")

                    batch_start = time.time()
                    inputs, labels = prefetcher.next()



        epoch_loss = running_loss / len(train_loader.dataset)
        epoch_acc = running_corrects.double() / len(train_loader.dataset)

        print(f'epoch {epoch} Train Loss: {epoch_loss:.6f} Acc: {epoch_acc:.6f},  data time: {epoch_data_time:.6f},train time: {epoch_train_time:.6f},total_time: {epoch_total_time:.6f}')
        print(f" AVG: data time: {1.0*epoch_data_time/batch_idx:.6f},train time: {1.0*epoch_train_time/batch_idx:.6f},total_time: {1.0*epoch_total_time/batch_idx:.6f}")
        # 将时间数据写入 CSV 文件
        # with open(csv_file_path, mode='a', newline='') as csv_file:
        #     writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
        #     writer.writerow({
        #         'batch_size':BATCH_SIZE,
        #         'worker_num':WORKER_NUM,
        #         'prefetch_factor':prefetch_factor,
        #         'epoch': epoch,
        #         'total_time': epoch_total_time,
        #         'data_time': epoch_data_time,
        #         'data_whole_time': epoch_data_whole_time,
        #         'training_time': epoch_train_time
        #     })

        # print(f"训练时间数据已导出到 {csv_file_path} 文件")
        scheduler.step()
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




if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Train a model with different parameters.")
    parser.add_argument("--batch_size", type=int, default=256, help="Batch size for training")
    parser.add_argument("--epochs", type=int, default=10, help="Number of epochs for training")
    parser.add_argument("--workers", type=int, default=0, help="Number of worker threads for data loading")
    parser.add_argument("--conn_mode", type=int, default=0, help="Connection mode: 0 for NFS, 1 for BeeGFS, 2 for local SataSSD, 3 for local NVMe SSD")
    parser.add_argument("--prefetch", type=int, default=0, help="Dataloader prefetch factor ")



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
        csv_file_path = "result/0617/epoch_time_BeeGFS_0617.csv"
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
        fieldnames = ['batch_size','worker_num','prefetch_factor','epoch', 'total_time', 'data_time', 'data_whole_time','training_time']
        writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
        if write_header:
            writer.writeheader()




    # 数据集路径
    # output_dir #替换为你的输出目录
    if Conn_Mode==0:
        output_dir = "/mnt/163_nvme/imagenet_decode"  # 替换为你的输出目录 163 nvme SSD NFS
    elif Conn_Mode==1:
        output_dir = "/mnt/beegfs/test_dir" # 替换为你的输出目录 beegfs 挂载目录
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
            persistent_workers=False)  # 减少进程频繁创建销毁开销
    else:
        train_loader = DataLoader(train_dataset, batch_size=batch_size, shuffle=True,num_workers=WORKER_NUM,drop_last=True)

    # train_loader = DataLoader(train_dataset, batch_size=batch_size, shuffle=True)

    # val_loader = DataLoader(val_dataset, batch_size=batch_size, shuffle=False)   

    # 定义模型
    model = models.resnet18(pretrained=False)
    num_classes = len(train_dataset.class_names)
    model.fc = nn.Linear(model.fc.in_features, num_classes)
    gpu_id=1
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
    optimizer = optim.SGD(model.parameters(),
        lr=0.01, 
        momentum=0.9,
        weight_decay=1e-4)
    scheduler = StepLR(optimizer, step_size=30, gamma=0.1)
    print_freq =1 
    # 训练模型
    print("开始训练")
    # model = train_model(model, criterion, optimizer, train_loader, val_loader, num_epochs=1)
    try:
        train_model(model, criterion, optimizer, train_loader,device,scheduler,print_freq, num_epochs=EPOCH_NUM,prefetch_factor=prefetch)
    except KeyboardInterrupt:
        print("💥 KeyboardInterrupt detected. Exiting cleanly.")

    # 保存模型
    # torch.save(model.state_dict(), 'resnet18_custom_dataset.pth')
    # print("模型已保存")
    print("训练已经结束")

