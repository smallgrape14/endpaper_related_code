# 加载预处理后的numpy文件，没有预处理的操作
import argparse
import os
import random
import shutil
import time
import warnings
from enum import Enum
import psutil
import numpy as np
from PIL import Image

import torch
import torch.backends.cudnn as cudnn
import torch.distributed as dist
import torch.multiprocessing as mp
import torch.nn as nn
import torch.nn.parallel
import torch.optim
import torch.utils.data
import torch.utils.data.distributed
import torchvision.datasets as datasets
import torchvision.models as models
import torchvision.transforms as transforms
from torch.optim.lr_scheduler import StepLR
from torch.utils.data import Subset, Dataset, DataLoader
import csv
from torch.cuda import nvtx
# 每个 worker 初始化时打印 PID 和 CPU
# def worker_init_fn(worker_id):
#     pid = os.getpid()
#     try:
#         # 查看当前 CPU 核心（Linux only）
#         with open(f"/proc/{pid}/stat") as f:
#             cpu = f.read().split()[38]
#     except Exception:
#         cpu = "N/A"
#     print(f"[Worker {worker_id}] PID: {pid}, CPU: {cpu}")
from functools import partial
def worker_init_fn(worker_id, gpu_id):
    pid = os.getpid()

    # 获取当前 CPU
    try:
        with open(f"/proc/{pid}/stat") as f:
            cpu = f.read().split()[38]
    except Exception:
        cpu = "N/A"

    # 获取当前 GPU（如果用到了 GPU）
    # if torch.cuda.is_available():
    #     try:
    #         # 当前默认 GPU ID
    #         gpu_id = torch.cuda.current_device()
    #         gpu_name = torch.cuda.get_device_name(gpu_id)
    #     except Exception:
    #         gpu_id = "N/A"
    #         gpu_name = "N/A"
    # else:
    #     gpu_id = "CPU_ONLY"
    #     gpu_name = "No GPU"

    print(f"[Worker {worker_id}] PID: {pid}, CPU: {cpu}, GPU ID: {gpu_id}")
Open_opt=0 # 表示打开相关的优化
Include_CPUTOGPU= 1 # 表示 data time 包含 CPU copy to GPU Time
Open_DEBUG =1
Open_NVTX=0
Total_File_num=2600 #[todo list]
model_names = sorted(name for name in models.__dict__
    if name.islower() and not name.startswith("__")
    and callable(models.__dict__[name]))

parser = argparse.ArgumentParser(description='PyTorch ImageNet Training')
parser.add_argument('data', metavar='DIR', nargs='?', default='imagenet',
                    help='path to dataset (default: imagenet)')
parser.add_argument('-a', '--arch', metavar='ARCH', default='resnet18',
                    choices=model_names,
                    help='model architecture: ' +
                        ' | '.join(model_names) +
                        ' (default: resnet18)')
parser.add_argument('-j', '--workers', default=4, type=int, metavar='N',
                    help='number of data loading workers (default: 4)')
parser.add_argument('--epochs', default=90, type=int, metavar='N',
                    help='number of total epochs to run')
parser.add_argument('--start-epoch', default=0, type=int, metavar='N',
                    help='manual epoch number (useful on restarts)')
parser.add_argument('-b', '--batch-size', default=256, type=int,
                    metavar='N',
                    help='mini-batch size (default: 256), this is the total '
                         'batch size of all GPUs on the current node when '
                         'using Data Parallel or Distributed Data Parallel')
##[XYP_0626]调整学习率默认是 0.1 --> 0.01 
# parser.add_argument('--lr', '--learning-rate', default=0.1, type=float,
parser.add_argument('--lr', '--learning-rate', default=0.01, type=float,
                    metavar='LR', help='initial learning rate', dest='lr')
parser.add_argument('--momentum', default=0.9, type=float, metavar='M',
                    help='momentum')
parser.add_argument('--wd', '--weight-decay', default=1e-4, type=float,
                    metavar='W', help='weight decay (default: 1e-4)',
                    dest='weight_decay')
parser.add_argument('-p', '--print-freq', default=10, type=int,
                    metavar='N', help='print frequency (default: 10)')
parser.add_argument('--resume', default='', type=str, metavar='PATH',
                    help='path to latest checkpoint (default: none)')
parser.add_argument('-e', '--evaluate', dest='evaluate', action='store_true',
                    help='evaluate model on validation set')
parser.add_argument('--pretrained', dest='pretrained', action='store_true',
                    help='use pre-trained model')
parser.add_argument('--world-size', default=-1, type=int,
                    help='number of nodes for distributed training')
parser.add_argument('--rank', default=-1, type=int,
                    help='node rank for distributed training')
parser.add_argument('--dist-url', default='tcp://224.66.41.62:23456', type=str,
                    help='url used to set up distributed training')
parser.add_argument('--dist-backend', default='nccl', type=str,
                    help='distributed backend')
parser.add_argument('--seed', default=None, type=int,
                    help='seed for initializing training. ')
parser.add_argument('--gpu', default=None, type=int,
                    help='GPU id to use.')
parser.add_argument('--multiprocessing-distributed', action='store_true',
                    help='Use multi-processing distributed training to launch '
                         'N processes per node, which has N GPUs. This is the '
                         'fastest way to use PyTorch for either single node or '
                         'multi node data parallel training')
parser.add_argument('--dummy', action='store_true', help="use fake data to benchmark")

best_acc1 = 0

class CustomDataset(Dataset):
    def __init__(self, root_dir, transform=None):
        self.root_dir = root_dir
        self.transform = transform
        self.class_names = os.listdir(root_dir)
        self.image_paths = []
        self.labels = []

        for label, class_name in enumerate(self.class_names):
            class_dir = os.path.join(root_dir, class_name)
            for filename in os.listdir(class_dir):
                if filename.endswith(('.npy', '.jpg', '.jpeg', '.png')):
                    image_path = os.path.join(class_dir, filename)
                    self.image_paths.append(image_path)
                    self.labels.append(label)

    def __len__(self):
        return len(self.image_paths)

    def __getitem__(self, idx):
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


def main():
    args = parser.parse_args()

    if args.seed is not None:
        random.seed(args.seed)
        torch.manual_seed(args.seed)
        cudnn.deterministic = True
        cudnn.benchmark = False
        warnings.warn('You have chosen to seed training. '
                      'This will turn on the CUDNN deterministic setting, '
                      'which can slow down your training considerably! '
                      'You may see unexpected behavior when restarting '
                      'from checkpoints.')

    if args.gpu is not None:
        warnings.warn('You have chosen a specific GPU. This will completely '
                      'disable data parallelism.')

    if args.dist_url == "env://" and args.world_size == -1:
        args.world_size = int(os.environ["WORLD_SIZE"])

    args.distributed = args.world_size > 1 or args.multiprocessing_distributed

    # ngpus_per_node = 1
    if torch.cuda.is_available():
        ngpus_per_node = torch.cuda.device_count()
        if ngpus_per_node == 1 and args.dist_backend == "nccl":
            warnings.warn("nccl backend >=2.5 requires GPU count>1, see https://github.com/NVIDIA/nccl/issues/103 perhaps use 'gloo'")
    else:
        ngpus_per_node = 1

    if args.multiprocessing_distributed:
        # Since we have ngpus_per_node processes per node, the total world_size
        # needs to be adjusted accordingly
        args.world_size = ngpus_per_node * args.world_size
        # Use torch.multiprocessing.spawn to launch distributed processes: the
        # main_worker process function
        mp.spawn(main_worker, nprocs=ngpus_per_node, args=(ngpus_per_node, args))
    else:
        # Simply call main_worker function
        main_worker(args.gpu, ngpus_per_node, args)


def main_worker(gpu, ngpus_per_node, args):
    global best_acc1 #用于记录训练过程中最佳的准确率
    args.gpu = gpu
    pid = os.getpid()
    proc = psutil.Process(pid)
    cpu_num = proc.cpu_num()

    print(f"GPU: {gpu} 进程 PID: {pid}")
    print(f"GPU: {gpu} 当前运行 CPU: {cpu_num}")

    if args.gpu is not None:
        print("Use GPU: {} for training".format(args.gpu))

    if args.distributed:
        if args.dist_url == "env://" and args.rank == -1:
            args.rank = int(os.environ["RANK"])
        if args.multiprocessing_distributed:
            # For multiprocessing distributed training, rank needs to be the
            # global rank among all the processes
            args.rank = args.rank * ngpus_per_node + gpu
        dist.init_process_group(backend=args.dist_backend, init_method=args.dist_url,
                                world_size=args.world_size, rank=args.rank)
    #@@ create model
    if Open_opt == 1:
        num_classes = 2  # 获取实际类别数[todo] 这个先硬编码吧
        # num_classes = len(train_dataset.class_names)  # 获取实际类别数[todo] 这个先硬编码吧
        print(f"num_classes :{num_classes}")
    if args.pretrained:
        print("=> using pre-trained model '{}'".format(args.arch))
        model = models.__dict__[args.arch](pretrained=True)
        if Open_opt == 1:
            # 关键修复2：修改最后一层适配实际类别数
            if hasattr(model, 'fc'):  # ResNet系列
                in_features = model.fc.in_features
                model.fc = nn.Linear(in_features, num_classes)
            elif hasattr(model, 'classifier'):  # DenseNet/VGG系列
                if isinstance(model.classifier, nn.Sequential):
                    in_features = model.classifier[-1].in_features
                    model.classifier[-1] = nn.Linear(in_features, num_classes)
                else:
                    in_features = model.classifier.in_features
                    model.classifier = nn.Linear(in_features, num_classes)
    else:
        print("=> creating model '{}'".format(args.arch))
        model = models.__dict__[args.arch]()

    # 检查是否支持 CUDA 或 Metal Performance Shaders (MPS)。如果不支持，则使用 CPU，并提示训练速度会很慢。
    if not torch.cuda.is_available() and not torch.backends.mps.is_available():
        print('using CPU, this will be slow')
    elif args.distributed:
        # For multiprocessing distributed, DistributedDataParallel constructor
        # should always set the single device scope, otherwise,
        # DistributedDataParallel will use all available devices.
        if torch.cuda.is_available():
            if args.gpu is not None:
                torch.cuda.set_device(args.gpu)
                model.cuda(args.gpu)
                # When using a single GPU per process and per
                # DistributedDataParallel, we need to divide the batch size
                # ourselves based on the total number of GPUs of the current node.
                args.batch_size = int(args.batch_size / ngpus_per_node)
                args.workers = int((args.workers + ngpus_per_node - 1) / ngpus_per_node)
                model = torch.nn.parallel.DistributedDataParallel(model, device_ids=[args.gpu])
            else:
                model.cuda()
                # DistributedDataParallel will divide and allocate batch_size to all
                # available GPUs if device_ids are not set
                model = torch.nn.parallel.DistributedDataParallel(model)
    elif args.gpu is not None and torch.cuda.is_available(): #@@ 如果指定了 GPU 编号且支持 CUDA，则将模型移动到指定的 GPU 上。
        torch.cuda.set_device(args.gpu)
        model = model.cuda(args.gpu)
    elif torch.backends.mps.is_available():
        device = torch.device("mps")
        model = model.to(device)
    else:
        # DataParallel will divide and allocate batch_size to all available GPUs
        if args.arch.startswith('alexnet') or args.arch.startswith('vgg'):
            model.features = torch.nn.DataParallel(model.features)
            model.cuda()
        else:
            model = torch.nn.DataParallel(model).cuda()

    if torch.cuda.is_available(): #@@ 根据设备可用性，设置 device 变量，用于后续的张量操作。
        if args.gpu:
            device = torch.device('cuda:{}'.format(args.gpu)) 
        else:
            device = torch.device("cuda")
    elif torch.backends.mps.is_available():
        device = torch.device("mps")
    else:
        device = torch.device("cpu")
    #@@ define loss function (criterion), optimizer, and learning rate scheduler
    criterion = nn.CrossEntropyLoss().to(device) #@@ 定义交叉熵损失函数，并将其移动到指定设备。

    #@@ 使用随机梯度下降（SGD）优化器，设置学习率、动量和权重衰减
    # print(f"args.lr = {args.lr}")
    optimizer = torch.optim.SGD(model.parameters(), args.lr,
                                momentum=args.momentum,
                                weight_decay=args.weight_decay)
    
    """Sets the learning rate to the initial LR decayed by 10 every 30 epochs"""
    scheduler = StepLR(optimizer, step_size=30, gamma=0.1) #@@ 定义学习率调度器，每 30 个 epoch 将学习率乘以 0.1。
    
    # optionally resume from a checkpoint
    #@@ 加载检查点文件，恢复模型状态、优化器状态和学习率调度器状态。
    if args.resume:
        if os.path.isfile(args.resume):
            print("=> loading checkpoint '{}'".format(args.resume))
            if args.gpu is None:
                checkpoint = torch.load(args.resume)
            elif torch.cuda.is_available():
                # Map model to be loaded to specified single gpu.
                loc = 'cuda:{}'.format(args.gpu)
                checkpoint = torch.load(args.resume, map_location=loc)
            args.start_epoch = checkpoint['epoch']
            best_acc1 = checkpoint['best_acc1']
            if args.gpu is not None:
                # best_acc1 may be from a checkpoint from a different GPU
                best_acc1 = best_acc1.to(args.gpu)
            model.load_state_dict(checkpoint['state_dict'])
            optimizer.load_state_dict(checkpoint['optimizer'])
            scheduler.load_state_dict(checkpoint['scheduler'])
            print("=> loaded checkpoint '{}' (epoch {})"
                  .format(args.resume, checkpoint['epoch']))
        else:
            print("=> no checkpoint found at '{}'".format(args.resume))


    #@@ Data loading code
    if args.dummy: #@@ 如果启用了虚拟数据（args.dummy），使用 datasets.FakeData 生成虚拟训练和验证数据集。
        print("=> Dummy data is used!")
        train_dataset = datasets.FakeData(1281167, (3, 224, 224), 1000, transforms.ToTensor())
        val_dataset = datasets.FakeData(50000, (3, 224, 224), 1000, transforms.ToTensor())
    else: #@@ 从指定路径加载训练和验证数据集。
        '''
        traindir = os.path.join(args.data, 'train')
        valdir = os.path.join(args.data, 'val')
        
        #@@ 加载预处理后的数据集
        train_dataset = datasets.ImageFolder(
            traindir,
            transforms.Compose([
                transforms.ToTensor(),  # 只需要将图像转换为 Tensor
            ]))
        val_dataset = datasets.ImageFolder(
            valdir,
            transforms.Compose([
                transforms.ToTensor(),  # 只需要将图像转换为 Tensor
            ]))
        '''
        # 数据集路径
        # output_dir = "/home/oem/xyp/imagenet_test_decode3"  # 替换为你的输出目录
        # train_output_dir = os.path.join(output_dir, 'train')
        # val_output_dir = os.path.join(output_dir, 'val')

        train_output_dir = os.path.join(args.data, 'train')
        val_output_dir = os.path.join(args.data, 'val')

        # 定义数据预处理
        train_transform = transforms.Compose([
            transforms.ToTensor(),
            transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])
        ])

        val_transform = transforms.Compose([
            transforms.ToTensor(),
            transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])
        ])
        print("@@ 非dummy数据集加载")
        # 创建数据集
        train_dataset = CustomDataset(train_output_dir, transform=train_transform)
        val_dataset = CustomDataset(val_output_dir, transform=val_transform)
    # 如果启用了分布式训练，创建分布式采样器以确保每个进程只处理数据集的一部分。否则，采样器为 None
    if args.distributed:
        train_sampler = torch.utils.data.distributed.DistributedSampler(train_dataset) #DistributedSampler自动划分数据子集到不同进程
        val_sampler = torch.utils.data.distributed.DistributedSampler(val_dataset, shuffle=False, drop_last=True) #训练集默认启用shuffle，验证集禁用以保证评估一致性，丢弃不完整批次，避免分布式同步问题
    else:
        train_sampler = None
        val_sampler = None

    #@@ 创建训练和验证的数据加载器，设置批量大小、是否打乱数据、工作进程数和是否使用 PinMemory
    '''
    train_loader = torch.utils.data.DataLoader(
        train_dataset, batch_size=args.batch_size,
        shuffle=(train_sampler is None),    # 采样器存在时禁用shuffle
        num_workers=args.workers,           # 多进程加载
        pin_memory=True,                    ## 加速GPU数据传输
        sampler=train_sampler)

    val_loader = torch.utils.data.DataLoader(
        val_dataset, batch_size=args.batch_size, shuffle=False, # 验证集无需shuffle
        num_workers=args.workers, pin_memory=True, sampler=val_sampler)
    '''
    # train_loader = DataLoader(train_dataset, batch_size=args.batch_size, shuffle=True,num_workers=args.workers,pin_memory=True,prefetch_factor=1,drop_last=True)
    if args.workers > 0:
        train_loader = DataLoader(train_dataset,
            batch_size=args.batch_size, 
            shuffle=True,
            num_workers=args.workers,
            pin_memory=True,
            prefetch_factor=1,
            persistent_workers=True, # 持久化工作线程
            worker_init_fn=partial(worker_init_fn, gpu_id=gpu),
            drop_last=True)
    else:
        train_loader = DataLoader(train_dataset,
            batch_size=args.batch_size, 
            shuffle=True,
            num_workers=args.workers,
            pin_memory=True,
            # prefetch_factor=1,
            # persistent_workers=True, # 持久化工作线程
            drop_last=True)
    # train_loader = DataLoader(train_dataset, batch_size=args.batch_size, shuffle=True,num_workers=args.workers,pin_memory=True,drop_last=True)
    val_loader = DataLoader(val_dataset, batch_size=args.batch_size, shuffle=False)   
    # 如果启用了评估模式（args.evaluate），直接调用 validate 函数对模型进行评估，并退出函数
    if args.evaluate:
        validate(val_loader, model, criterion, args)
        return
    
    batch_data_list = []
    batch_train_list = []
    
    #@@ 遍历每个 epoch。
    for epoch in range(args.start_epoch, args.epochs):
        if args.distributed: # 如果启用了分布式训练，更新训练采样器的 epoch。
            train_sampler.set_epoch(epoch)

        # train for one epoch
        #@@ 调用 train 函数进行一个 epoch 的训练。
        train(train_loader, model, criterion, optimizer, epoch, device, args,batch_data_list, batch_train_list)

        # evaluate on validation set
        #@@ 调用 validate 函数对模型进行评估，获取准确率 acc1
        # acc1 = validate(val_loader, model, criterion, args)
        
        #@@ 更新学习率调度器
        scheduler.step()
        
        # remember best acc@1 and save checkpoint
        #@@ 检查是否是最佳准确率，并更新 best_acc1
        # is_best = acc1 > best_acc1
        # best_acc1 = max(acc1, best_acc1)

        # 如果是主进程（或非多进程分布式训练），保存检查点
        # if not args.multiprocessing_distributed or (args.multiprocessing_distributed
        #         and args.rank % ngpus_per_node == 0):
        #     save_checkpoint({
        #         'epoch': epoch + 1,
        #         'arch': args.arch,
        #         'state_dict': model.state_dict(),
        #         'best_acc1': best_acc1,
        #         'optimizer' : optimizer.state_dict(),
        #         'scheduler' : scheduler.state_dict()
        #     }, is_best)
    #统计：
    arr_data = np.array(batch_data_list)
    arr_train = np.array(batch_train_list)


    # avg_csv_file_path = "result/avg_batch_time_dist0707.csv"
    # write_header = not os.path.exists(avg_csv_file_path)
    # with open(avg_csv_file_path, mode='a', newline='') as csv_file:
    #     fieldnames_batch = ['batch_size',  'AVG_data', 'MIN_data', 'MAX_data', 
    #                         'Median_data', 'p99_data', 'AVG_train', 'MIN_train', 'MAX_train', 
    #                         'Median_train', 'p99_train']
    #     writer = csv.DictWriter(csv_file, fieldnames=fieldnames_batch)
    #     if write_header:
    #         writer.writeheader()

    if gpu == 0: # 只在主进程（GPU 0）写入统计结果
        if Open_DEBUG == 1:
            avg_csv_file_path = "result/avg_batch_time_dist0707_debug.csv"
        else: 
            avg_csv_file_path = "result/avg_batch_time_dist0707.csv"
        write_header = not os.path.exists(avg_csv_file_path)
        with open(avg_csv_file_path, mode='a', newline='') as csv_file:
            fieldnames_batch = ['batch_size','worker_num', 'AVG_data', 'MIN_data', 'MAX_data', 
                                'Median_data', 'p99_data', 'AVG_train', 'MIN_train', 'MAX_train', 
                                'Median_train', 'p99_train']
            writer = csv.DictWriter(csv_file, fieldnames=fieldnames_batch)
            if write_header:
                writer.writeheader()
        with open(avg_csv_file_path, mode='a', newline='') as csv_file:
            writer = csv.DictWriter(csv_file, fieldnames=fieldnames_batch)
            writer.writerow({
                'batch_size': args.batch_size,
                'worker_num': args.workers,
                # 'blocknum': NetIO_Block,
                # 'threadnum': NetIO_Thread,
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


def train(train_loader, model, criterion, optimizer, epoch, device, args,batch_data_list, batch_train_list):
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
    batch_idx=0
    total_batches=int(Total_File_num/args.batch_size)
    end = time.time() # 记录循环开始时刻
    if Open_NVTX == 1:
        with nvtx.range("iterate dataloader"):
            for batch_idx in range(total_batches):
                with nvtx.range("enumerate_batch"):  # NVTX标记枚举范围
                    images, target = next(iter(train_loader))
                with nvtx.range("cpu_to_gpu"):
                    images = images.to(device, non_blocking=True)
                    target = target.to(device, non_blocking=True)
                    torch.cuda.current_stream().synchronize()
                data_end=time.time() # 记录数据加载结束的时刻
                data_time.update(data_end - end) # 对应日志中的"Data"列 即数据加载和预处理的时间
                batch_data_list.append(data_end - end)
                train_begin=time.time() # 记录训练开始的时刻
                # compute output
                with nvtx.range("training"):
                    #将输入图像 images 传递给模型，得到模型的输出 output
                    with nvtx.range("model"):
                        output = model(images)
                    # 使用损失函数 criterion 计算模型输出与目标标签 target 之间的损失值 loss
                    with nvtx.range("criterion"):
                        loss = criterion(output, target)
                    with nvtx.range("accuracy"):
                        # measure accuracy and record loss
                        acc1, acc5 = accuracy(output, target, topk=(1, 5))
                        losses.update(loss.item(), images.size(0))
                        top1.update(acc1[0], images.size(0))
                        top5.update(acc5[0], images.size(0))
                    # compute gradient and do SGD step
                    with nvtx.range("optimizer"):
                        optimizer.zero_grad()
                    with nvtx.range("loss"):
                        loss.backward()
                    with nvtx.range("optimizer_step"):
                        optimizer.step()

                # measure elapsed time
                batch_end=time.time() # 记录批处理结束的时刻
                batch_time.update(batch_end- train_begin+data_end-end)
                train_time.update(batch_end- train_begin) # 记录训练时间
                batch_train_list.append(batch_end- train_begin)
                
                end = time.time()
                
                if batch_idx % args.print_freq == 0:
                    progress.display(batch_idx + 1)
    else:
        for i, (images, target) in enumerate(train_loader): #遍历 train_loader 中的每个批次数据。enumerate 用于获取批次索引 i 和批次数据 (images, target)
            
            #将输入图像 images 和目标标签 target 移动到指定的设备（如 GPU）
            #non_blocking 非阻塞模式，允许数据传输与计算并行进行，提高效率。
            images = images.to(device, non_blocking=True)
            target = target.to(device, non_blocking=True)
            torch.cuda.current_stream().synchronize()

            data_end=time.time() # 记录数据加载结束的时刻
            data_time.update(data_end - end) # 对应日志中的"Data"列 即数据加载和预处理的时间
            batch_data_list.append(data_end - end)
            train_begin=time.time() # 记录训练开始的时刻
            # compute output
            #将输入图像 images 传递给模型，得到模型的输出 output
            output = model(images)
            # 使用损失函数 criterion 计算模型输出与目标标签 target 之间的损失值 loss
            loss = criterion(output, target)

            # measure accuracy and record loss
            acc1, acc5 = accuracy(output, target, topk=(1, 5))
            losses.update(loss.item(), images.size(0))
            top1.update(acc1[0], images.size(0))
            top5.update(acc5[0], images.size(0))

            # compute gradient and do SGD step
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()

            # measure elapsed time
            batch_end=time.time() # 记录批处理结束的时刻
            batch_time.update(batch_end- train_begin+data_end-end)
            train_time.update(batch_end- train_begin) # 记录训练时间
            batch_train_list.append(batch_end- train_begin)
            
            end = time.time()

            

            if i % args.print_freq == 0:
                progress.display(i + 1)

def validate(val_loader, model, criterion, args):

    def run_validate(loader, base_progress=0):
        with torch.no_grad():
            end = time.time()
            for i, (images, target) in enumerate(loader):
                i = base_progress + i
                if args.gpu is not None and torch.cuda.is_available():
                    images = images.cuda(args.gpu, non_blocking=True)
                if torch.backends.mps.is_available():
                    images = images.to('mps')
                    target = target.to('mps')
                if torch.cuda.is_available():
                    target = target.cuda(args.gpu, non_blocking=True)

                # compute output
                output = model(images)
                loss = criterion(output, target)

                # measure accuracy and record loss
                acc1, acc5 = accuracy(output, target, topk=(1, 5))
                losses.update(loss.item(), images.size(0))
                top1.update(acc1[0], images.size(0))
                top5.update(acc5[0], images.size(0))

                # measure elapsed time
                batch_time.update(time.time() - end)
                end = time.time()

                if i % args.print_freq == 0:
                    progress.display(i + 1)

    batch_time = AverageMeter('Time', ':6.3f', Summary.NONE)
    losses = AverageMeter('Loss', ':.4e', Summary.NONE)
    top1 = AverageMeter('Acc@1', ':6.2f', Summary.AVERAGE)
    top5 = AverageMeter('Acc@5', ':6.2f', Summary.AVERAGE)
    progress = ProgressMeter(
        len(val_loader) + (args.distributed and (len(val_loader.sampler) * args.world_size < len(val_loader.dataset))),
        [batch_time, losses, top1, top5],
        prefix='Test: ')

    # switch to evaluate mode
    model.eval()

    run_validate(val_loader)
    if args.distributed:
        top1.all_reduce()
        top5.all_reduce()

    if args.distributed and (len(val_loader.sampler) * args.world_size < len(val_loader.dataset)):
        aux_val_dataset = Subset(val_loader.dataset,
                                 range(len(val_loader.sampler) * args.world_size, len(val_loader.dataset)))
        aux_val_loader = torch.utils.data.DataLoader(
            aux_val_dataset, batch_size=args.batch_size, shuffle=False,
            num_workers=args.workers, pin_memory=True)
        run_validate(aux_val_loader, len(val_loader))

    progress.display_summary()

    return top1.avg


def save_checkpoint(state, is_best, filename='checkpoint.pth.tar'):
    torch.save(state, filename)
    if is_best:
        shutil.copyfile(filename, 'model_best.pth.tar')

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


if __name__ == '__main__':
    main()