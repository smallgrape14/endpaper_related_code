import argparse
import os
import random
import shutil
import time
import warnings
from enum import Enum

from nvidia.dali import pipeline_def
import nvidia.dali.fn as fn
import nvidia.dali.types as types
from nvidia.dali.plugin.pytorch import DALIGenericIterator
import numpy as np
from torch.utils.data import Dataset, DataLoader
from PIL import Image
from torch.nn import CrossEntropyLoss
from torch.optim import SGD

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
from torch.utils.data import Subset

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
parser.add_argument('--lr', '--learning-rate', default=0.1, type=float,
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


# 定义 DALI 预处理管道,CPU-GPU协作的效率考虑这个 prefetch_queue_depth
@pipeline_def(batch_size=16, num_threads=4, device_id=0,prefetch_queue_depth=2)
def image_processing_pipeline(data_dir,crop_size_x,crop_size_y):
    # 读取文件
    images, labels = fn.readers.file(file_root=data_dir, random_shuffle=True, name="Reader")# 训练集需要随机打乱
    # 将数据移动到 GPU
    # images = fn.copy(images, device="gpu")
    # decode data on the GPU
    images = fn.decoders.image(images, device="mixed", output_type=types.RGB)

    # images = fn.decoders.image(images, device="mixed", output_type=types.RGB,hw_decoder_load=1.0,
    #                                         bytes_per_sample_hint=224 * 224 * 3,  # 显存预分配提示
    #                                         memory_stats=True        # 显存监控
    #                                         )



    # 3. 随机裁剪缩放（等效RandomResizedCrop）
    images = fn.random_resized_crop(
        images,
        device="gpu",
        size=(crop_size_x, crop_size_y),  # 目标尺寸
        random_area=[0.08, 1.0],  # 随机区域面积比例（与PyTorch默认一致）
        random_aspect_ratio=[3/4, 4/3],  # 随机宽高比范围
        interp_type=types.INTERP_LINEAR  # 双线性插值
    )
    # 4. 随机水平翻转（50%概率）
    images = fn.flip(images,device="gpu",horizontal=fn.random.coin_flip(probability=0.5))
    
    # 5. 转换为CHW格式并归一化（类似ToTensor）
    images = fn.transpose(images,device="gpu", perm=[2, 0, 1], output_layout=types.NCHW)  # HWC → CHW
    images = fn.cast(images,device="gpu", dtype=types.FLOAT) / 255.0  # 归一化到[0,1]
    
    # 6. 标准化（与PyTorch一致）
    mean = [0.485, 0.456, 0.406]
    std = [0.229, 0.224, 0.225]
    images = fn.crop_mirror_normalize(images,device="gpu", mean=mean, std=std)

    return images.gpu(), labels.gpu()# 标签也需转移至GPU


@pipeline_def(batch_size=16, num_threads=4, device_id=0,exec_async=False, exec_pipelined=False)
def dali_preprocessing_pipeline_val(data_dir,crop_size_x,crop_size_y):
    # 读取文件
    images, labels = fn.readers.file(file_root=data_dir, random_shuffle=False, name="Reader")
    # 将数据移动到 GPU
    # images = fn.copy(images, device="gpu")
    # decode data on the GPU
    images = fn.decoders.image(images, device="mixed", output_type=types.RGB,hw_decoder_load=1.0)

    # 2. 等比缩放短边至256[1,4](@ref)
    images = fn.resize(
        images,
        device="gpu",
        resize_shorter=256,  # 自动保持宽高比
        interp_type=types.INTERP_LINEAR  # 双线性插值[4](@ref)
    )
    
    # 3. 中心裁剪224x224[8](@ref)
    images = fn.crop(images, crop=(224, 224), crop_pos_x=0.5, crop_pos_y=0.5)

    # 4. 转换为CHW格式并归一化（类似ToTensor）
    images = fn.transpose(images, perm=[2, 0, 1], output_layout=types.NCHW)  # HWC → CHW
    images = fn.cast(images, dtype=types.FLOAT) / 255.0  # 归一化到[0,1]

    # 5. 标准化（与PyTorch的normalize一致）
    mean = [0.485, 0.456, 0.406]
    std = [0.229, 0.224, 0.225]
    images = fn.crop_mirror_normalize(images, mean=mean, std=std)  # 标准化到N(0,1)

    # images = fn.crop_mirror_normalize(
    #     images,
    #     device="gpu",
    #     crop=(224, 224),
    #     crop_pos_x=0.5,  # 水平中心位置
    #     crop_pos_y=0.5,  # 垂直中心位置
    #     dtype=types.FLOAT,  # 转换为浮点Tensor[10](@ref)
    #     output_layout=types.NCHW     # 匹配PyTorch的NCHW格式
    # )
    return images.gpu(), labels.gpu()# 标签也需转移至GPU

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
    if args.pretrained:
        print("=> using pre-trained model '{}'".format(args.arch))
        model = models.__dict__[args.arch](pretrained=True)
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

#加载数据集合的关键代码-------------------
        # 创建自定义数据集并加载到 CPU 内存中
        train_dir = os.path.join(args.data, 'train')
        val_dir = os.path.join(args.data, 'val')

        train_data = DALIGenericIterator(
            [image_processing_pipeline(train_dir, 244,244,batch_size=args.batch_size,num_threads=args.workers)], ['images', 'target'],
            reader_name='Reader'
        )
        val_data = DALIGenericIterator(
            [dali_preprocessing_pipeline_val(val_dir, 244,244,batch_size=args.batch_size,num_threads=args.workers)], ['images', 'target'],
            reader_name='Reader'
        )
        # 检查流水线属性是否生效
        # print(train_pipe.batch_size)   # 应输出32
        # print(train_pipe.num_threads)  # 应输出8
    # 如果启用了评估模式（args.evaluate），直接调用 validate 函数对模型进行评估，并退出函数
    if args.evaluate:
        # validate(val_loader, model, criterion, args)
        validate(val_data, model, criterion, args)

        return
    #@@ 遍历每个 epoch。
    for epoch in range(args.start_epoch, args.epochs):
        if args.distributed: # 如果启用了分布式训练，更新训练采样器的 epoch。
            train_sampler.set_epoch(epoch)

        # train for one epoch
        #@@ 调用 train 函数进行一个 epoch 的训练。
        # train(train_loader, model, criterion, optimizer, epoch, device, args)
        #DALI 预处理的加载模式
        train(train_data, model, criterion, optimizer, epoch, device, args)

        

        # evaluate on validation set
        #@@ 调用 validate 函数对模型进行评估，获取准确率 acc1
        # acc1 = validate(val_loader, model, criterion, args)
        acc1 = validate(val_data, model, criterion, args)

        
        #@@ 更新学习率调度器
        scheduler.step()
        
        # remember best acc@1 and save checkpoint
        #@@ 检查是否是最佳准确率，并更新 best_acc1
        is_best = acc1 > best_acc1
        best_acc1 = max(acc1, best_acc1)

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


def train(train_loader, model, criterion, optimizer, epoch, device, args):
    batch_time = AverageMeter('Time', ':6.3f') # 批处理总时间统计
    data_time = AverageMeter('Data', ':6.3f') # 数据加载时间统计
    losses = AverageMeter('Loss', ':.4e')
    top1 = AverageMeter('Acc@1', ':6.2f')
    top5 = AverageMeter('Acc@5', ':6.2f')
    progress = ProgressMeter( #初始化一个进度条对象 ProgressMeter，用于在训练过程中打印训练进度。
        len(train_loader),
        [batch_time, data_time, losses, top1, top5],
        prefix="Epoch: [{}]".format(epoch))

    # switch to train mode
    model.train()

    end = time.time() # 记录循环开始时刻
    for i, data in enumerate(train_loader): #遍历 train_loader 中的每个批次数据。enumerate 用于获取批次索引 i 和批次数据 (images, target)
        images, target = data[0]['images'], data[0]['target'].squeeze().long()
        
        # 转换为PyTorch张量
        images = images.contiguous()
        target = target.contiguous()
        # measure data loading time
        data_time.update(time.time() - end) # 对应日志中的"Data"列 即数据加载和预处理的时间。

        # move data to the same device as model 
        #将输入图像 images 和目标标签 target 移动到指定的设备（如 GPU）
        #non_blocking 非阻塞模式，允许数据传输与计算并行进行，提高效率。
        # images = images.to(device, non_blocking=True)
        # target = target.to(device, non_blocking=True)

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
        batch_time.update(time.time() - end)
        end = time.time()

        if i % args.print_freq == 0:
            progress.display(i + 1)


def validate(val_loader, model, criterion, args):

    def run_validate(loader, base_progress=0):
        with torch.no_grad():
            end = time.time()
            # for i, (images, target) in enumerate(loader):
            for i, data in enumerate(loader):
                images, target = data[0]['images'], data[0]['target'].squeeze().long()
                # 转换为PyTorch张量
                images = images.contiguous()
                target = target.contiguous()
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