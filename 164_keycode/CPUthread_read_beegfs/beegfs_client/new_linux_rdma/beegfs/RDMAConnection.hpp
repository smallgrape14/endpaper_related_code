#ifndef RDMA_CONN_HPP
#define RDMA_CONN_HPP
// #ifdef __cplusplus
// 仅 C++ 编译器可见的头文件
#include <atomic>       // C++ 原子操作库
#include <cstddef>      // C++ 标准库（可选，如果不需要可删除）
#include <cstdlib>     // For malloc and free
#include <cstring> // C++ 标准库中的内存操作函数
#include <atomic>
#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <iomanip>
// #endif


// #include <cstdlib>     // For malloc and free
#include <string.h>     // For memset, memcpy, memcmp, and memmove
#include <stddef.h>
#include <stdio.h>
// #include <iostream>
// #include <cstring> // C++ 标准库中的内存操作函数
// #include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>      // for open 系统调用 osflag=O_RDONLY|O_WRONLY
// #include <sys/socket.h> // For socket operations,struct msghdr 
#include <sys/types.h> // for typeof
#include <sys/uio.h> //struct iovec 的定义，它通常与 readv、writev、sendmsg 和 recvmsg
#include <unistd.h>           // for getuid
#include <pthread.h>
// #include <memory> // for unique_ptr 
// #include <linux/sockios.h>  // 包含 SIOCOUTQ 命令
#include <sys/ioctl.h>      // 包含 ioctl 函数声明
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <infiniband/verbs.h>
#include <rdma/rdma_cma.h>
#include <assert.h>
#include <endian.h>
// #include <atomic>
// #include <chrono>
// #include <thread>
// #include <iomanip>
// #include "communicate.h"
// // #include "../open/communicate.h"
// #include "RDMASocket.h"
// #include "IBVSocket.h"
#include "open.hpp"
#include "IBVSocket.hpp"

using namespace std;

// #include <linux/mempool.h>
// #include <netinet/in.h> // For struct in_addr and other IPv4 related definitions
// #include <errno.h>      // For error codes
// #include <time.h>       // For struct timespec and other time related functions
// #include <endian.h>     // For byte order conversions, or <arpa/inet.h>

// ------------------------------ RDMA socket and connect -------------------------------------
// #define RDMASOCKET_DEFAULT_BUF_NUM                    (128) // moved to config
// #define RDMASOCKET_DEFAULT_BUF_SIZE                   (4*1024) // moved to config
// #define RDMASOCKET_DEFAULT_FRAGMENT_SIZE              RDMASOCKET_DEFAULT_BUF_SIZE // moved to config
#define RDMASOCKET_DEFAULT_BUF_NUM                    (256) // moved to config
#define RDMASOCKET_DEFAULT_BUF_SIZE                   (4*1024) // moved to config
#define RDMASOCKET_DEFAULT_FRAGMENT_SIZE              RDMASOCKET_DEFAULT_BUF_SIZE // moved to config
// #define RDMASOCKET_DEFAULT_KEY_TYPE                   RDMAKEYTYPE_UnsafeGlobal
#define RDMASOCKET_DEFAULT_KEY_TYPE                   RDMAKEYTYPE_Register

#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif

#define NULL __null

#define SAFE_FREE(ptr) \
    do { \
        if (ptr) { \
            free(ptr); \
            ptr = NULL; \
        } \
    } while (0)

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

// #define	IFNAMSIZ	16
#define  NICADDRESS_IP_STR_LEN 16

struct iovec_iter;
typedef struct iovec_iter iovec_iter;

typedef struct NicAddressStats
{
   NicAddress nic;
   int        established;
   int        available;
//    Time       used;
//    Time       lastError;
   /**
    * nicValid indicates if the NicAddress can be used for connections.
    * This may be tracking stats for a device that has gone offline.
    */
   bool       nicValid;
}NicAddressStats;


void NicAddressStats_init(NicAddressStats* thisone, NicAddress* nic)
{
   thisone->nic = *nic;
   thisone->established = 0;
   thisone->available = 0;
   thisone->nicValid = true;
   // Time_initZero(&thisone->used);
   // Time_initZero(&thisone->lastError);
}

// inliners
static inline void NicAddress_ipToStr(struct in_addr ipAddr, char* outStr);
static inline bool NicAddress_equals(NicAddress* thisone, NicAddress* other);

/**
 * @param outStr must be at least NICADDRESS_STR_LEN bytes long
 */
void NicAddress_ipToStr(struct in_addr ipAddr, char* outStr)
{
   unsigned char* ipArray = (unsigned char*)&ipAddr.s_addr;

   sprintf(outStr, "%u.%u.%u.%u", ipArray[0], ipArray[1], ipArray[2], ipArray[3]);
}

bool NicAddress_equals(NicAddress* thisone, NicAddress* other)
{
   return (thisone->ipAddr.s_addr == other->ipAddr.s_addr) &&
      (thisone->nicType == other->nicType) &&
      !strncmp(thisone->name, other->name, IFNAMSIZ);
}


#define SOCKET_PEERNAME_LEN   24



typedef uint16_t __be16;


// struct IBVSocket{

// };


/*
 * Socket 是最基础的套接字结构，包含通用的网络套接字信息和操作函数指针 ops
 * PooledSocket 继承并扩展了 Socket，增加了连接池管理的功能，用于管理套接字的复用和生命周期
 * IBVSocket 是 RDMA 通信的底层实现，封装了 RDMA verbs API，负责处理实际的 RDMA 操作，如连接建立、数据传输、事件处理等
 * RDMASocket 继承了 PooledSocket，并包含一个 IBVSocket，作为高层的 RDMA 套接字封装，对外提供统一的接口，屏蔽底层的 RDMA 细节。
 */

typedef struct RDMASocket{
   PooledSocket pooledSocket;   
   IBVSocket ibvsock;
   IBVCommConfig commCfg;
}RDMASocket;


struct DevicePriorityContext
{
   int maxConns;
#ifdef BEEGFS_NVFS
   // index of GPU related to the first page, -1 for none
   int gpuIndex;
#endif
};

typedef struct DevicePriorityContext DevicePriorityContext;

enum RDMAKeyType
{
   RDMAKEYTYPE_UnsafeGlobal = 0,
   RDMAKEYTYPE_UnsafeDMA,
   RDMAKEYTYPE_Register
};


#define IBVSOCKET_CONN_TIMEOUT_MS                  5000
 /* this also includes send completion wait times */
#define IBVSOCKET_COMPLETION_TIMEOUT_MS          300000
#define IBVSOCKET_FLOWCONTROL_ONSEND_TIMEOUT_MS  180000
#define IBVSOCKET_FLOWCONTROL_ONRECV_TIMEOUT_MS  180000
#define IBVSOCKET_SHUTDOWN_TIMEOUT_MS               250
#define IBVSOCKET_POLL_TIMEOUT_MS                 1//10000
#define IBVSOCKET_FLOWCONTROL_MSG_LEN                 1
#define IBVSOCKET_STALE_RETRIES_NUM                 128
#define IBVSOCKET_MIN_SGE                             1
#define IBVSOCKET_MIN_WR                              1

#define IBVSOCKET_RECVT_INFINITE_TIMEOUT_MS     1000000


// -------------------------------------connectByIP-------------------------------------

/**
 * @param buf the buffer to which <IP>:<port> should be written.
 */
void SocketTk_endpointAddrToStrNoAlloc(char* buf, size_t bufLen, struct in_addr ipaddress,
   unsigned short port)
{
   int printRes = snprintf(buf, bufLen, "%pI4:%u", &ipaddress, port);
   if(unlikely( (unsigned)printRes >= bufLen) )
      buf[bufLen-1] = 0; // bufLen exceeded => zero-terminate result
}

#include <sys/time.h>
// 计算当前时间间隔
unsigned int Time_elapsedMS(struct timeval* start)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    unsigned int elapsed = (now.tv_sec - start->tv_sec) * 1000
        + (now.tv_usec - start->tv_usec) / 1000;
    return elapsed;
}

/**
 * Create a new cm_id.
 * This is not only intended for new sockets, but also for stale cm_ids, so this can cleanup/replace
 * existing cm_ids and resets error states.
 */
bool __IBVSocket_createNewID(IBVSocket* _this)
{
   struct rdma_cm_id* new_cm_id = NULL;

   // We need to unconditionally destroy the old CM id.  It is unusable at this point.
   if(_this->cm_id)
   {
      rdma_destroy_id(_this->cm_id);
      _this->cm_id = NULL;
   }

   struct rdma_event_channel *cm_channel;   // 创建一个事件通道
   // cm_channel = rdma_create_event_channel();
   _this->event_channel = rdma_create_event_channel();
   if (!_this->event_channel) {
      fprintf(stderr, "rdma_create_event_channel failed\n");
      return false;
   }

   #if defined(OFED_HAS_NETNS) || defined(rdma_create_id)
      new_cm_id = rdma_create_id(&init_net, __IBVSocket_cmaHandler, _this, RDMA_PS_TCP, IB_QPT_RC);
   #elif defined(OFED_HAS_RDMA_CREATE_QPTYPE)
      new_cm_id = rdma_create_id(__IBVSocket_cmaHandler, _this, RDMA_PS_TCP, IB_QPT_RC);
   #else
      int ret = rdma_create_id(_this->event_channel, &_this->cm_id, _this, RDMA_PS_TCP);
   #endif
   if (ret)
   {
      fprintf(stderr, "rdma_create_id failed: %s\n", strerror(errno));
      rdma_destroy_event_channel(cm_channel);
      return false;
   }

   // if(IS_ERR(new_cm_id) )
   // {
   //    LOG_printf(DEBUG,"rdma_create_id failed. ErrCode: %ld\n", PTR_ERR(new_cm_id) );
   //    return false;
   // }

   _this->cm_id = new_cm_id;

   _this->connState = IBVSOCKETCONNSTATE_UNCONNECTED;
   _this->errState = 0;

   return true;
}

bool IBVBuffer_init(IBVBuffer* buffer, IBVCommContext* ctx, size_t bufLen,
   size_t fragmentLen, enum dma_data_direction dma_dir)
{
   unsigned count;
   unsigned i;

   if (fragmentLen == 0)
      fragmentLen = bufLen;
   count = (bufLen + fragmentLen - 1) / fragmentLen;
   bufLen = MIN(fragmentLen, bufLen);

   buffer->dma_dir = dma_dir;
   buffer->buffers = (char**)malloc(count * sizeof(*buffer->buffers));
   buffer->lists = (struct ibv_sge*)malloc(count * sizeof(*buffer->lists));
   buffer->datasize=(size_t*)malloc(count*sizeof(size_t)); //NOTE@@
   if(!buffer->buffers || !buffer->lists)
      goto fail;

   for(i = 0; i < count; i++)
   {
      buffer->lists[i].lkey = ctx->pd->handle;
      buffer->lists[i].length = bufLen;
      // LOG(DEBUG)<<" ###### bufLen = "<<bufLen<<"\n";
      buffer->buffers[i] = (char*)malloc(bufLen);
      if(unlikely(!buffer->buffers[i]))
      {
         fprintf(stderr, "Failed to allocate buffer size=%zu\n", bufLen);
         goto fail;
      }
      buffer->lists[i].addr = (uintptr_t)buffer->buffers[i];
      
      assert(buffer->lists[i].addr != 0);
   }

   buffer->bufferSize = bufLen;
      LOG(DEBUG)<<" ###### buffer->bufferSize = "<<buffer->bufferSize<<"\n";

   buffer->listLength = count;
   buffer->bufferCount = count;
   return true;

fail:
   IBVBuffer_free(buffer, ctx);
   return false;
}



bool IBVBuffer_initRegistration(IBVBuffer* buffer, IBVCommContext* ctx)
{
   // struct scatterlist* sg;
   int res;
   int i;
   size_t total_size = buffer->bufferCount * buffer->bufferSize;

   // 直接分配总缓冲
   void* buf = malloc(total_size);
   if (buf == NULL)
   {
       fprintf(stderr, "Failed to allocate buffer memory\n");
       goto fail;
   }

   // 注册内存
   buffer->mr = ibv_reg_mr(ctx->pd, buf, total_size, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ);
   if (!(buffer->mr))
   {
      fprintf(stderr, "Failed to register memory region: %s\n", strerror(errno));
      buffer->mr = NULL;
      goto fail;
   }

    // 初始化缓冲区指针数组
   buffer->buffers = (char**)malloc(sizeof(char*) * buffer->bufferCount);
   if (buffer->buffers == NULL)
   {
       fprintf(stderr, "Failed to allocate buffers array\n");
       ibv_dereg_mr(buffer->mr);
       free(buf);
       goto fail;
   }

   // 初始化 ibv_sge 数组
   buffer->lists = (struct ibv_sge*)malloc(buffer->bufferCount * sizeof(struct ibv_sge));
   if (buffer->lists == NULL)
   {
       fprintf(stderr, "Failed to allocate scatter/gather list\n");
       ibv_dereg_mr(buffer->mr);
       free(buffer->buffers);
       free(buf);
       goto fail;
   }

   // 设置缓冲区指针和 ibv_sge
   for (i = 0; i < buffer->bufferCount; ++i)
   {
       buffer->buffers[i] = (char*)buf + i * buffer->bufferSize;

       buffer->lists[i].addr   = (uintptr_t)buffer->buffers[i];
       buffer->lists[i].length = buffer->bufferSize;
       buffer->lists[i].lkey   = buffer->mr->lkey;
   }
   
   return true;

fail:
   if (buffer->mr)
   {
      ibv_dereg_mr(buffer->mr);
      buffer->mr = NULL;
   }
   return false;
}

void IBVBuffer_free(IBVBuffer* buffer, IBVCommContext* ctx)
{
   if(buffer->buffers && buffer->lists)
   {
      unsigned i;
      for(i = 0; i < buffer->bufferCount; i++)
      {
         // if (buffer->lists[i].addr)
         //    ib_dma_unmap_single(ctx->pd->context->device, buffer->lists[i].addr,
         //       buffer->bufferSize, buffer->dma_dir);  // 取消DMA映射，在用户空间不需要手动取消，RDMA库会处理

         if (buffer->buffers[i])
            free(buffer->buffers[i]);
      }
   }

   if (buffer->mr)
      ibv_dereg_mr(buffer->mr);

   if (buffer->buffers)
      free(buffer->buffers);

   if (buffer->lists)
      free(buffer->lists);
}

ssize_t IBVBuffer_fill(IBVBuffer* buffer, iovec_iter* iter)
{
   LOG_printf(DEBUG,"for IBVBuffer_fill\n");
   ssize_t total = 0;
   unsigned i;
   LOG(DEBUG)<<"buffer->bufferCount  = "<<buffer->bufferCount <<"\t";
   LOG(DEBUG)<<"iovec_iter_count(iter) = "<<iovec_iter_count(iter)<<"\n";
   for(i = 0; i < buffer->bufferCount && iovec_iter_count(iter) > 0; i++)
   {
      size_t fragment = MIN(MIN(iovec_iter_count(iter), buffer->bufferSize), 0xFFFFFFFF);
      LOG(DEBUG)<<"fragment = "<<fragment<<"\n";
      if(copy_from_iovec(buffer->buffers[i], fragment, iter) != fragment)
         return -EFAULT;

      buffer->lists[i].length = fragment;
      buffer->listLength = i + 1;

      total += fragment;
      LOG_printf(DEBUG,"for i=%d, buffer data[%d]: %s\n", i, i,buffer->buffers[i]);
      LOG(DEBUG)<<"fragment = "<<fragment<<"\n";
      // for(int j=0;j<fragment;j++)
      // {
      //    // LOG(DEBUG)<<"j = "<<j <<"\n";
      //    LOG(DEBUG)<<buffer->buffers[i][j];
      // }
      LOG(DEBUG)<<"--------\n";
      buffer->datasize[i]=fragment;//NOTE:@@
      LOG(DEBUG)<<"buffer->datasize = "<<buffer->datasize<<"\n";//NOTE:@@
      LOG(DEBUG)<<"\n";
      LOG(DEBUG)<<"strlen(buffer) = "<<strlen(buffer->buffers[i])<<"\n";
      LOG(DEBUG)<<" bufferSize  = "<<buffer->bufferSize<<" \t,bufferCount = "<<buffer->bufferCount<<"\t,listLength="<<buffer->listLength<<"\n";

   }

   LOG_printf(DEBUG,"IBVBuffer_fill over\n");
   return total;
}

// 路由解析完成之后，建立连接前的资源准备
// 为IBVSocket对象创建通信上下文，并初始化通信所需资源
bool __IBVSocket_createCommContext(IBVSocket* _this, struct rdma_cm_id* cm_id,
   IBVCommConfig* commCfg, IBVCommContext** outCommContext)
{
   IBVCommContext* commContext;
   struct ibv_context* dev = cm_id->verbs;
   struct ibv_qp_init_attr qpInitAttr;
   int qpRes;
   unsigned i;
   unsigned fragmentSize;
   unsigned maxSge;
   unsigned maxWr;
   bool globalRkey = (commCfg->keyType == IBVSOCKETKEYTYPE_UnsafeGlobal);

   commContext = (IBVCommContext*)malloc(sizeof(IBVCommContext));
   if(!commContext)
      goto err_cleanup;

   LOG_printf(DEBUG, "Alloc CommContext @ %p\n", commContext);

   // prepare recv and send event notification

   // init_waitqueue_head(&commContext->recvCompWaitQ);
   // init_waitqueue_head(&commContext->sendCompWaitQ);
   pthread_cond_init(&commContext->recvCompCond, (const pthread_condattr_t *)NULL);
   pthread_mutex_init(&commContext->recvCompMutex, (const pthread_mutexattr_t *)NULL);

   pthread_cond_init(&commContext->sendCompCond, (const pthread_condattr_t *)NULL);
   pthread_mutex_init(&commContext->sendCompMutex, (const pthread_mutexattr_t *)NULL);

   // atomic_set(&commContext->recvCompEventCount, 0);
   // atomic_set(&commContext->sendCompEventCount, 0);
   commContext->recvCompEventCount = 0;
   commContext->sendCompEventCount = 0;

   commContext->incompleteSend = {0, false};
   commContext->incompleteRecv = {0, 0, -1, 0};


   // protection domain...
   // IB_PD_UNSAFE_GLOBAL_RKEY is still present as of kernel 6.3.
#ifdef OFED_UNSAFE_GLOBAL_RKEY
   commContext->pd = ibv_alloc_pd(dev, globalRkey? IB_PD_UNSAFE_GLOBAL_RKEY : 0);
#else
   if (globalRkey)
   {
      fprintf(stderr, "Unsafe global rkey not supported on this platform.");
      goto err_cleanup;
   }
   // 分配保护域
   commContext->pd = ibv_alloc_pd(dev);
#endif
   if(!(commContext->pd) )
   {
      fprintf(stderr, "Couldn't allocate PD. ErrCode: %s\n", strerror(errno));

      commContext->pd = NULL;
      goto err_cleanup;
   }
#ifdef OFED_UNSAFE_GLOBAL_RKEY
   if (globalRkey)
      commContext->checkConnRkey = commContext->pd->unsafe_global_rkey;
#endif

   if (commCfg->keyType == IBVSOCKETKEYTYPE_UnsafeDMA)
   {
      // DMA system mem region...

      // (Note: IB spec says:
      //    "The consumer is not allowed to assign remote-write (or remote-atomic) to
      //    a memory region that has not been assigned local-write.")

      // ib_get_dma_mr() goes away in kernel 4.9. Is is still present in MOFED 5.9.
      // If not using the global rkey, then either ib_get_dmr_mr() or an allocated ib_mr
      // needs to be used.
#ifdef OFED_IB_GET_DMA_MR
      commContext->dmaMR = ib_get_dma_mr(commContext->pd,
         IB_ACCESS_LOCAL_WRITE| IB_ACCESS_REMOTE_READ| IB_ACCESS_REMOTE_WRITE);
      if(IS_ERR_OR_NULL(commContext->dmaMR) )
      {
         ibv_print_info("ib_get_dma_mr failed. ErrCode: %ld\n", PTR_ERR(commContext->dmaMR) );
         commContext->dmaMR = NULL;
         goto err_cleanup;
      }
      commContext->checkConnRkey = commContext->dmaMR->rkey;
#else
      fprintf(stderr, "RDMA keyType is dma and ib_get_dma_mr() not supported on this platform.\n");
      goto err_cleanup;
#endif

   }

#ifdef BEEGFS_DEBUG
   fprintf("%s: checkConnRkey = %u\n", __func__, commContext->checkConnRkey);
#endif

   // alloc and register buffers...分配并注册缓存区

   commContext->commCfg = *commCfg;

   commContext->recvBufs = (struct IBVBuffer*)malloc(commCfg->bufNum * sizeof(struct IBVBuffer));
   if(!commContext->recvBufs)
   {
      fprintf(stderr, "couldn't prepare receive buffer list\n");
      goto err_cleanup;
   }
   // LOG(DEBUG)<<"#### commCfg->bufNum = "<<commCfg->bufNum<<"\t"
   // <<"commCfg->bufSize = "<<commCfg->bufSize<<"\t,commCfg->fragmentSize = "<<commCfg->fragmentSize<<"\n";
   //#### commCfg->bufNum = 128   commCfg->bufSize = 4096 ,commCfg->fragmentSize = 4096
   for(i=0; i < commCfg->bufNum; i++)
   {
      if(!IBVBuffer_init(&commContext->recvBufs[i], commContext, commCfg->bufSize,
            commCfg->fragmentSize, DMA_FROM_DEVICE) )
      {
         fprintf(stderr, "couldn't prepare recvBuf #%d\n", i + 1);
         goto err_cleanup;
      }
      bool recvBufRes = IBVBuffer_initRegistration(&commContext->recvBufs[i], commContext);
      if(recvBufRes){
         // LOG_printf(DEBUG,"registration recvBufs[%d] successfully!\n", i);
      }
      else{
         LOG_printf(DEBUG,"registration recvBufs[%d] failed!\n", i);
         goto err_cleanup;
      }
   }

   commContext->sendBufs = (struct IBVBuffer*)malloc(commCfg->bufNum * sizeof(struct IBVBuffer));
   if(!commContext->sendBufs)
   {
      fprintf(stderr, "couldn't prepare send buffer list\n");
      goto err_cleanup;
   }

   for(i=0; i < commCfg->bufNum; i++)
   {
      if(!IBVBuffer_init(&commContext->sendBufs[i], commContext, commCfg->bufSize,
            commCfg->fragmentSize, DMA_TO_DEVICE) )
      {
         fprintf(stderr, "couldn't prepare sendBuf #%d\n", i + 1);
         goto err_cleanup;
      }
       bool recvBufRes = IBVBuffer_initRegistration(&commContext->sendBufs[i], commContext);
      if(recvBufRes){
         // LOG_printf(DEBUG,"registration sendBufs[%d] successfully!\n", i);
      }
      else{
         LOG_printf(DEBUG,"registration sendBufs[%d] failed!\n", i);
         goto err_cleanup;
      }
   }

   if(!IBVBuffer_init(&commContext->checkConBuffer, commContext, sizeof(uint64_t),
         0, DMA_TO_DEVICE) )
   {
      fprintf(stderr, "couldn't alloc dma control memory region\n");
      goto err_cleanup;
   }

   // init flow control v2 (to avoid long receiver-not-ready timeouts)

   /* note: we use -1 because the last buf might not be read by the user (eg during
      nonblockingRecvCheck) and so it might not be immediately available again. */
   commContext->numReceivedBufsLeft = commCfg->bufNum - 1;
   commContext->numSendBufsLeft = commCfg->bufNum - 1;

   // create completion queues... 创建完成队列

   // commContext->recvCQ = __IBVSocket_createCompletionQueue(cm_id->device,
   //    __IBVSocket_recvCompletionHandler, __IBVSocket_cqRecvEventHandler,
   //    _this, commCfg->bufNum);
   commContext->recvCQ = ibv_create_cq(cm_id->verbs, commCfg->bufNum, NULL, NULL, 0);
   if(!(commContext->recvCQ) )
   {
      fprintf(stderr, "couldn't create recv CQ. ErrCode: %s\n", strerror(errno) );
      commContext->recvCQ = NULL;
      goto err_cleanup;
   }

   // note: 1+commCfg->bufNum here for the checkConnection() RDMA read
   // commContext->sendCQ = __IBVSocket_createCompletionQueue(cm_id->device,
   //    __IBVSocket_sendCompletionHandler, __IBVSocket_cqSendEventHandler,
   //    _this, 1+commCfg->bufNum);
   commContext->sendCQ = ibv_create_cq(cm_id->verbs, commCfg->bufNum, NULL, NULL, 0);
   if(!(commContext->sendCQ) )
   {
      fprintf(stderr, "couldn't create send CQ. ErrCode: %s\n", strerror(errno));
      commContext->sendCQ = NULL;
      goto err_cleanup;
   }

   fragmentSize = commCfg->fragmentSize;
   if (fragmentSize == 0)
      fragmentSize = commCfg->bufSize;
   maxSge = MAX(IBVSOCKET_MIN_SGE, commCfg->bufSize / fragmentSize + 1);
   maxWr = MAX(IBVSOCKET_MIN_WR, commCfg->bufNum + 1);

   // note: 1+commCfg->bufNum here for the checkConnection() RDMA read
   memset(&qpInitAttr, 0, sizeof(qpInitAttr) );

   // qpInitAttr.event_handler = __IBVSocket_qpEventHandler;
   qpInitAttr.send_cq = commContext->sendCQ;
   qpInitAttr.recv_cq = commContext->recvCQ;
   qpInitAttr.qp_type = IBV_QPT_RC;
   // qpInitAttr.sq_sig_type = IB_SIGNAL_REQ_WR;
   qpInitAttr.cap.max_send_wr = maxWr;
   qpInitAttr.cap.max_recv_wr = maxWr;
   qpInitAttr.cap.max_send_sge = maxSge;
   qpInitAttr.cap.max_recv_sge = maxSge;
   qpInitAttr.cap.max_inline_data = 0;

   qpRes = rdma_create_qp(cm_id, commContext->pd, &qpInitAttr);
   if(qpRes)
   {
      fprintf(stderr, "couldn't create QP. ErrCode: %d\n", qpRes);
      goto err_cleanup;
   }

   commContext->qp = cm_id->qp;

   // prepare event notification...

   // initial event notification requests
   if(ibv_req_notify_cq(commContext->recvCQ, 0) )
   {
      fprintf(stderr, "couldn't request CQ notification\n");
      goto err_cleanup;
   }

   if(ibv_req_notify_cq(commContext->sendCQ, 0) )
   {
      fprintf(stderr, "couldn't request CQ notification\n");
      goto err_cleanup;
   }

   *outCommContext = commContext;
   return true;


   //  error handling

err_cleanup:
   __IBVSocket_cleanupCommContext(cm_id, commContext);
   *outCommContext = NULL;
   return false;
}

/**
 * Initializes a (local) IBVCommDest.
 */
bool __IBVSocket_initCommDest(IBVCommContext* commContext, IBVCommDest* outDest)
{
   memcpy(outDest->verificationStr, IBVSOCKET_PRIVATEDATA_STR, IBVSOCKET_PRIVATEDATA_STR_LEN);
   // outDest->protocolVersion = cpu_to_le64(IBVSOCKET_PRIVATEDATA_PROTOCOL_VER);   //内核空间用于将 CPU 本地字节序转换为小端字节序
   // outDest->rkey = cpu_to_le32(commContext->checkConnRkey);
   // outDest->vaddr = cpu_to_le64(commContext->checkConBuffer.lists[0].addr);
   // outDest->recvBufNum = cpu_to_le32(commContext->commCfg.bufNum);
   // outDest->recvBufSize = cpu_to_le32(commContext->commCfg.bufSize);
   outDest->protocolVersion = htole64(IBVSOCKET_PRIVATEDATA_PROTOCOL_VER);
   outDest->rkey = htole32(commContext->checkConnRkey);
   outDest->vaddr = htole64(commContext->checkConBuffer.lists[0].addr);
   outDest->recvBufNum = htole32(commContext->commCfg.bufNum);
   outDest->recvBufSize = htole32(commContext->commCfg.bufSize);
#ifdef BEEGFS_DEBUG
   fprintf(stderr, "%s: rkey=%u vaddr=%llu", __func__, outDest->rkey, outDest->vaddr);
#endif
   return true;
}

void __IBVSocket_cleanupCommContext(struct rdma_cm_id* cm_id, IBVCommContext* commContext)
{
   unsigned i;
   struct ibv_device* dev;

   LOG_printf(DEBUG,"Free CommContext @ %p\n", commContext);

   if(!commContext)
      return;

   dev = commContext->pd ? commContext->pd->context->device : NULL;

   if(!dev)
      goto cleanup_no_dev;

   if(cm_id && commContext->qp && cm_id->qp)
      rdma_destroy_qp(cm_id);


   if(commContext->sendCQ)
#ifdef OFED_IB_DESTROY_CQ_IS_VOID
      ibv_destroy_cq(commContext->sendCQ);
#else
   {
      int destroyRes = ibv_destroy_cq(commContext->sendCQ);
      if (unlikely(destroyRes) )
      {
         fprintf(stderr, "sendCQ destroy failed: %d\n", destroyRes);
         // dump_stack();
      }
   }
#endif

   if(commContext->recvCQ)
#ifdef OFED_IB_DESTROY_CQ_IS_VOID
      ibv_destroy_cq(commContext->recvCQ);
#else
   {
      int destroyRes = ibv_destroy_cq(commContext->recvCQ);
      if (unlikely(destroyRes) )
      {
         fprintf(stderr, "recvCQ destroy failed: %d\n", destroyRes);
         // dump_stack();
      }
   }
#endif

   IBVBuffer_free(&commContext->checkConBuffer, commContext);

   for(i=0; i < commContext->commCfg.bufNum; i++)
   {
      if(commContext->recvBufs)
         IBVBuffer_free(&commContext->recvBufs[i], commContext);

      if(commContext->sendBufs)
         IBVBuffer_free(&commContext->sendBufs[i], commContext);
   }

   SAFE_FREE(commContext->recvBufs);
   SAFE_FREE(commContext->sendBufs);

   if(commContext->dmaMR)
      ibv_dereg_mr(commContext->dmaMR);
   if(commContext->pd)
      ibv_dealloc_pd(commContext->pd);

cleanup_no_dev:
   free(commContext);
}

int __IBVSocket_routeResolvedHandler(IBVSocket* _this, struct rdma_cm_id* cm_id,
   IBVCommConfig* commCfg, IBVCommContext** outCommContext)
{
   bool createContextRes;
   struct rdma_conn_param conn_param;

   createContextRes = __IBVSocket_createCommContext(_this, _this->cm_id, commCfg,  &_this->commContext);
   LOG_printf(DEBUG,"createContextRes: %d\n", createContextRes);
   // LOG_printf(DEBUG,"check %d \n", _this->commContext->incompleteSend.numAvailable);
   if(_this->commContext){
      LOG_printf(DEBUG,"success context\n");
   }
   else{
      LOG_printf(DEBUG,"empty context\n");
   }
   if(!createContextRes)
   {
      fprintf(stderr,"creation of CommContext failed\n");
      _this->errState = -1;

      return -EPERM;
   }

   // establish connection...
   if (_this->commContext->checkConnRkey == 0)
   {
      if (!IBVBuffer_initRegistration(&_this->commContext->checkConBuffer, _this->commContext))
      {
         LOG_printf(DEBUG,"_this->errState: %d\n", _this->errState);
         _this->errState = -1;

         return -EPERM;
      }
      _this->commContext->checkConnRkey = _this->commContext->checkConBuffer.mr->rkey;
   }

   if (!__IBVSocket_initCommDest(_this->commContext, &_this->localDest))
   {
      fprintf(stderr,"creation of CommDest failed\n");
      _this->errState = -1;

      return -EPERM;
   }

   memset(&conn_param, 0, sizeof(conn_param) );
#ifdef BEEGFS_NVFS
//$$%
   // conn_param.responder_resources = _this->commContext->pd->device->attrs.max_qp_rd_atom;
   // conn_param.initiator_depth = _this->commContext->pd->device->attrs.max_qp_init_rd_atom;
//$$%
   conn_param.responder_resources = 1;
   conn_param.initiator_depth = 1;
#else
   conn_param.responder_resources = 1;
   conn_param.initiator_depth = 1;
#endif
   conn_param.flow_control = 0;
   conn_param.retry_count = 7; // (3 bits)
   conn_param.rnr_retry_count = 7; // rnr = receiver not ready (3 bits, 7 means infinity)
   conn_param.private_data = &_this->localDest;
   conn_param.private_data_len = sizeof(_this->localDest);
LOG_printf(DEBUG,"try rdma_connect\n");
   int conRes = rdma_connect(_this->cm_id, &conn_param);
   LOG(DEBUG) << "rdma_connecr res = " << conRes << "\n";
   return conRes;
}

bool __IBVSocket_parseCommDest(const void* buf, size_t bufLen, IBVCommDest** outDest)
{
   const IBVCommDest* src = (IBVCommDest*)buf;
   IBVCommDest* dest = NULL;

   // Note: "bufLen < ..." (and not "!="), because there might be some extra padding
   if(!buf || (bufLen < sizeof(*dest) ) )
   {
      LOG_printf(DEBUG,"Bad private data size. length: %zu\n", bufLen);
      return false;
   }

   dest = (IBVCommDest*)malloc(sizeof(struct IBVCommDest));
   if(!dest)
      return false;

   *outDest = dest;
   *dest = *src;

   dest->protocolVersion = le64toh(dest->protocolVersion);
   dest->vaddr = le64toh(dest->vaddr);
   dest->rkey = le32toh(dest->rkey);
   dest->recvBufNum = le32toh(dest->recvBufNum);
   dest->recvBufSize = le32toh(dest->recvBufSize);

   if(memcmp(dest->verificationStr, IBVSOCKET_PRIVATEDATA_STR, IBVSOCKET_PRIVATEDATA_STR_LEN) )
      goto err_cleanup;

   if(dest->protocolVersion != IBVSOCKET_PRIVATEDATA_PROTOCOL_VER)
      goto err_cleanup;

   return true;

err_cleanup:
   free(dest);
   *outDest = NULL;
   return false;
}

int __IBVSocket_postRecv(IBVSocket* _this, IBVCommContext* commContext, size_t bufIndex)
{  
   LOG(DEBUG)<<" into  __IBVSocket_postRecv \n";
   struct ibv_recv_wr wr;
   struct ibv_recv_wr* bad_wr = NULL;
   int postRes;

   commContext->sendBufs[bufIndex].lists[0].length = commContext->commCfg.bufSize;
   LOG(DEBUG)<<"commContext->commCfg.bufSize = "<<commContext->commCfg.bufSize<<"\n";
   LOG(DEBUG)<<"commContext->recvBufs[bufIndex].listLength = "<<commContext->recvBufs[bufIndex].listLength<<"\n";
   LOG(DEBUG)<<"commContext->recvBufs[bufIndex].lists.addr = "<<commContext->recvBufs[bufIndex].lists->addr<<"\n";
   LOG(DEBUG)<<"commContext->recvBufs[bufIndex].lists.length = "<<commContext->recvBufs[bufIndex].lists->length<<"\n";
   LOG(DEBUG)<<"commContext->recvBufs[bufIndex].lists.lkey = "<<commContext->recvBufs[bufIndex].lists->lkey <<"\n";




   wr.next = NULL;
   wr.wr_id = bufIndex;
   wr.sg_list = commContext->recvBufs[bufIndex].lists;
   wr.num_sge = commContext->recvBufs[bufIndex].listLength;
   // LOG(DEBUG)<<" prepare into  ibv_post_recv \n";

   postRes = ibv_post_recv(commContext->qp, &wr, &bad_wr);
   LOG(DEBUG)<<" out of  ibv_post_recv, postRes = "<< postRes<< "\n";

   if(unlikely(postRes) )
   {
      LOG_printf(DEBUG,"ib_post_recv failed. ErrCode: %d\n", postRes);

      return -1;
   }

   return 0;
}

int IBVSocket_registerMr(IBVSocket* _this, void* addr, size_t length, int access)
{
    struct ibv_mr* mr;

    mr = ibv_reg_mr(_this->commContext->pd, addr, length, IBV_ACCESS_LOCAL_WRITE | access);
    if (!mr) {
        fprintf(stderr, "Failed to register memory region: %s\n", strerror(errno));
        return -1;
    }

    // 保存或使用注册得到的 `mr`
    _this->commContext->checkConBuffer.mr = mr;

    return 0;
}



int __IBVSocket_connectedHandler(IBVSocket* _this, struct rdma_cm_event* event)
{
   IBVCommContext* commContext = _this->commContext;
   IBVCommConfig* commCfg;
   int retVal = 0;
   bool parseCommDestRes;
   const void* private_data;
   uint8_t private_data_len;
   int i;

   if (!commContext)
      return -EINVAL;

   commCfg = &commContext->commCfg;
   if (_this->commContext->commCfg.keyType == IBVSOCKETKEYTYPE_Register)
   {
      if(IBVSocket_registerMr(_this, _this->commContext->checkConBuffer.buffers[0], _this->commContext->checkConBuffer.bufferSize, IBV_ACCESS_REMOTE_READ))
      {
         LOG_printf(DEBUG,"register buffer failed\n");
         _this->errState = -1;
         return -EPERM;
      }
   }

   // post initial recv buffers...
   for(i=0; i < commCfg->bufNum; i++)
   {
      if(__IBVSocket_postRecv(_this, commContext, i) )
      {
         LOG_printf(DEBUG,"couldn't post recv buffer with index %d\n", i);
         goto err_invalidateSock;
      }
   }

#if defined BEEGFS_OFED_1_2_API && (BEEGFS_OFED_1_2_API == 1)
   private_data = event->private_data;
   private_data_len = event->private_data_len;
#else // OFED 1.2.5 or higher API
   private_data = event->param.conn.private_data;
   private_data_len = event->param.conn.private_data_len;
#endif

   parseCommDestRes = __IBVSocket_parseCommDest(
      private_data, private_data_len, &_this->remoteDest);
   if(!parseCommDestRes)
   {
      fprintf(stderr, "bad private data received. len: %d\n", private_data_len);

      retVal = -EOPNOTSUPP;
      goto err_invalidateSock;
   }


   return retVal;


err_invalidateSock:
   _this->errState = -1;

   return retVal;
}

int __IBVSocket_cmaHandler(struct rdma_cm_id* cm_id, struct rdma_cm_event* event)
{
   IBVSocket* _this = (IBVSocket*)cm_id->context;
   int retVal = 0;

   if(unlikely(!_this || _this->errState !=0) )
   {
      fprintf(stderr, "cm_id is being torn down. Event: %d\n", event->event);
      return (event->event == RDMA_CM_EVENT_CONNECT_REQUEST) ? -EINVAL : 0;
   }

   pthread_mutex_lock(&_this->cmaMutex);
   if (_this->cm_id != cm_id)
   {
      pthread_mutex_unlock(&_this->cmaMutex);
      return -EINVAL;
   }

   LOG_printf(DEBUG, "rdma event: %i, status: %i\n", event->event, event->status);

   switch(event->event)
   {
      case RDMA_CM_EVENT_ADDR_RESOLVED:
         _this->connState = IBVSOCKETCONNSTATE_ADDRESSRESOLVED;
         break;

      case RDMA_CM_EVENT_ADDR_ERROR:
      case RDMA_CM_EVENT_UNREACHABLE:
         retVal = -ENETUNREACH;
         break;

      case RDMA_CM_EVENT_ROUTE_RESOLVED:
         _this->connState = IBVSOCKETCONNSTATE_ROUTERESOLVED;
         break;

      case RDMA_CM_EVENT_ROUTE_ERROR:
      case RDMA_CM_EVENT_CONNECT_ERROR:
         retVal = -ETIMEDOUT;
         break;

      case RDMA_CM_EVENT_CONNECT_REQUEST:
         // incoming connections not supported => reject all
         #ifdef OFED_RDMA_REJECT_NEEDS_REASON
            rdma_reject(cm_id, NULL, 0, 0);
         #else
            rdma_reject(cm_id, NULL, 0);
         #endif // OFED_RDMA_REJECT_NEEDS_REASON
         break;

      case RDMA_CM_EVENT_CONNECT_RESPONSE:
         retVal = rdma_accept(cm_id, NULL);
         break;

      case RDMA_CM_EVENT_REJECTED:
         retVal = event->status == RDMA_CM_EVENT_REJECTED ? -ESTALE : -ECONNREFUSED;
         break;

      case RDMA_CM_EVENT_ESTABLISHED:
         retVal = __IBVSocket_connectedHandler(_this, event);
         _this->connState = IBVSOCKETCONNSTATE_ESTABLISHED;
         break;

      case RDMA_CM_EVENT_DISCONNECTED:
         rdma_disconnect(cm_id);
         break;

      case RDMA_CM_EVENT_DEVICE_REMOVAL:
         /**
          * Sigh... what to do? There were previous attempts to perform cleanup of the
          * IBVCommContext and return -ENETRESET when this event is encountered.
          * Returning a nonzero value causes RDMA CM to destroy the cm_id and anything
          * belonging to that cm_id must be destroyed here before returning nonzero. There
          * are a lot of race conditions with the worker thread and attempting to implement
          * a locking scheme would require significant redesign due to the use of
          * blocking calls to ib_poll_cq. Ignoring the event appears to allow normal
          * cleanup of resources after the RDMA routines return an error to the caller.
          */
         LOG_printf(DEBUG,"Device has been removed: %s\n", cm_id->verbs->device->name);
         break;

      default:
         fprintf(stderr, "Ignoring RDMA_CMA event: %d\n", event->event);
         break;
   }

   if(unlikely(retVal) )
   {
      if(retVal == -ESTALE)
         _this->connState = IBVSOCKETCONNSTATE_REJECTED_STALE;
      else
         _this->connState = IBVSOCKETCONNSTATE_FAILED;

      _this->errState = -1;
      // free connection resources later. freeing everything here may race with send/recv
      // operations in case of a connection breakage.
      retVal = 0;
   }

   pthread_cond_signal(&_this->eventCond);
   pthread_mutex_unlock(&_this->cmaMutex);   
   // wake_up(&_this->eventWaitQ);

   return retVal;
}

void* rdma_event_loop(void *arg) {
    IBVSocket *sock = (IBVSocket*) arg;
    struct rdma_cm_event *event;

    while (1) {
        if (rdma_get_cm_event(sock->event_channel, &event)) {
            perror("rdma_get_cm_event failed");
            break;
        }
        __IBVSocket_cmaHandler(sock->cm_id, event);
        rdma_ack_cm_event(event);

        if (sock->connState == IBVSOCKETCONNSTATE_ESTABLISHED ||
            sock->connState == IBVSOCKETCONNSTATE_FAILED)
            break;
    }
    return NULL;
}


bool IBVSocket_connectByIP(IBVSocket* _this, struct in_addr ipaddress, unsigned short port,
   IBVCommConfig* commCfg)
{
   struct sockaddr_in sin;
   struct sockaddr_in src;
   // long connTimeoutJiffies = TimeTk_msToJiffiesSchedulable(IBVSOCKET_CONN_TIMEOUT_MS);
   long connTimeoutMS = IBVSOCKET_CONN_TIMEOUT_MS;
   // Time connElapsed;   
   LOG_printf(DEBUG,"---------IBVSocket_connectByIP----------\n");
   struct timeval connElapsed;
   gettimeofday(&connElapsed, NULL);
   _this->commCfg = commCfg;

/*
 * 在需要计算时间差时，获取当前时间，与 connElapsed 进行比较。
 *例如，计算连接耗时：
 *struct timeval now;
 *gettimeofday(&now, NULL);
 *long elapsedMS = (now.tv_sec - connElapsed.tv_sec) * 1000 + (now.tv_usec - connElapsed.tv_usec) / 1000;
 */
   int rc;

   /* note: rejected as stale means remote side still had an old open connection associated with
         our current cm_id. what most likely happened is that the client was reset (i.e. no clean
         disconnect) and our new cm_id after reboot now matches one of the old previous cm_ids.
         => only possible solution seems to be retrying with another cm_id. */
   int numStaleRetriesLeft = IBVSOCKET_STALE_RETRIES_NUM;

   // Time_setToNow(&connElapsed);   // 设置当前时间

   // 创建事件通道
_this->event_channel = rdma_create_event_channel();
if (!_this->event_channel) {
    fprintf(stderr, "rdma_create_event_channel failed: %s\n", strerror(errno));
    return false;
}
// 创建 cm_id，关联到事件通道
if (rdma_create_id(_this->event_channel, &_this->cm_id, _this, RDMA_PS_TCP)) {
    fprintf(stderr, "rdma_create_id failed: %s\n", strerror(errno));
    rdma_destroy_event_channel(_this->event_channel);
    return false;
}
// 启动事件处理线程
pthread_t event_thread;
if (pthread_create(&event_thread, NULL, rdma_event_loop, (void*)_this)) {
    fprintf(stderr, "Failed to create event thread\n");
    rdma_destroy_id(_this->cm_id);
    rdma_destroy_event_channel(_this->event_channel);
    return false;
}


   for( ; ; ) // stale retry loop 失败重试
   {
      // set type of service for this connection
      // #ifdef OFED_HAS_SET_SERVICE_TYPE
         // if (_this->typeOfService)
         //    rdma_set_service_type(_this->cm_id, _this->typeOfService);  //设置服务类型在用户空间没有可替代的直接API，还需要找别的替代方式
      // #endif // OFED_HAS_SET_SERVICE_TYPE

      /* note: the rest of the connect procedure is invoked through the cmaHandler when the
            corresponding asynchronous events arrive => we just have to wait for the connState
            to change here */

      _this->connState = IBVSOCKETCONNSTATE_CONNECTING;

      // resolve IP address ...
      // (async event handler also automatically resolves route on success)

      sin.sin_addr.s_addr = ipaddress.s_addr;
      sin.sin_family = AF_INET;
      sin.sin_port = htons(port);
      // 打印目标地址信息
      LOG_printf(DEBUG,"设置目标地址\n");
      // char dest_ip_str[INET_ADDRSTRLEN];
      // inet_ntop(AF_INET, &(sin.sin_addr), dest_ip_str, INET_ADDRSTRLEN);
      // LOG_printf(DEBUG,"目标 IP 地址：%s，端口：%d\n", dest_ip_str, ntohs(sin.sin_port));

      if (_this->srcIpAddr.s_addr != 0)
      {
         src.sin_addr = _this->srcIpAddr;
         src.sin_family = AF_INET;
         src.sin_port = 0;
      }
      LOG_printf(DEBUG,"设置源地址\n");

      if(rdma_resolve_addr(_this->cm_id, (struct sockaddr*)&src, (struct sockaddr*)&sin, _this->timeoutCfg.connectMS) )
      {
         // fprintf(stderr, "rdma_resolve_addr failed\n");
         fprintf(stderr, "rdma_resolve_addr failed\n");
         goto err_invalidateSock;
      }
      LOG_printf(DEBUG,"解析地址并为RDMA连接创建了通信标识符cm_id\n");
      pthread_mutex_lock(&_this->eventMutex);
      // 使用循环，确保条件成立
      while (_this->connState == IBVSOCKETCONNSTATE_CONNECTING) {
         // 调用条件变量等待函数
         pthread_cond_wait(&_this->eventCond, &_this->eventMutex);
      }
      // 退出临界区，解锁互斥锁
      pthread_mutex_unlock(&_this->eventMutex);
      if(_this->connState != IBVSOCKETCONNSTATE_ADDRESSRESOLVED)
         goto err_invalidateSock;



      if(rdma_resolve_route(_this->cm_id, _this->timeoutCfg.connectMS) )
      {
         fprintf(stderr, "rdma_resolve_route failed\n");
         goto err_invalidateSock;
      }
      LOG_printf(DEBUG,"成功解析路由\n");
      // 在等待队列中休眠直到state不为指定状态
      // wait_event_interruptible(_this->eventWaitQ,
      //    _this->connState != IBVSOCKETCONNSTATE_ADDRESSRESOLVED);
      // 锁定互斥锁，进入临界区
      pthread_mutex_lock(&_this->eventMutex);
      // 使用循环，确保条件成立
      while (_this->connState == IBVSOCKETCONNSTATE_ADDRESSRESOLVED) {
         // 调用条件变量等待函数
         pthread_cond_wait(&_this->eventCond, &_this->eventMutex);
      }
      // 退出临界区，解锁互斥锁
      pthread_mutex_unlock(&_this->eventMutex);
      if(_this->connState != IBVSOCKETCONNSTATE_ROUTERESOLVED)
         goto err_invalidateSock;

      // establish connection...

      // (handler calls rdma_connect() )
      pthread_mutex_lock(&_this->cmaMutex);
      rc = __IBVSocket_routeResolvedHandler(_this, _this->cm_id, commCfg, &_this->commContext);
      pthread_mutex_unlock(&_this->cmaMutex);
      if (rc)
      {
         // fprintf(stderr, "route resolved handler failed\n");
         fprintf(stderr, "route resolved handler failed\n");
         goto err_invalidateSock;
      }

      // wait for async event
      // Note: rdma_connect() can take a very long time (>5m) if the peer's HCA has gone down.
      // wait_event_interruptible_timeout(_this->eventWaitQ,
      //    _this->connState != IBVSOCKETCONNSTATE_ROUTERESOLVED,
      //    connTimeoutJiffies);
      // 获取当前时间并计算超时时间
      struct timespec ts;
      clock_gettime(CLOCK_REALTIME, &ts);

      // 将 connTimeoutMS 加到当前时间上
      ts.tv_sec += connTimeoutMS / 1000;
      ts.tv_nsec += (connTimeoutMS % 1000) * 1000000;

      // 处理进位
      if (ts.tv_nsec >= 1000000000) {
         ts.tv_sec += ts.tv_nsec / 1000000000;
         ts.tv_nsec = ts.tv_nsec % 1000000000;
      }
      LOG_printf(DEBUG,"connState before pthread_cond_timedwait %u\n", _this->connState);
      // 获取互斥锁
      pthread_mutex_lock(&_this->eventMutex);
      // 检查初始条件并等待
      while (_this->connState == IBVSOCKETCONNSTATE_ROUTERESOLVED) {
         int res = pthread_cond_timedwait(&_this->eventCond, &_this->eventMutex, &ts);
         LOG_printf(DEBUG,"waiting establish\n");
         if (res == ETIMEDOUT) {
            // 超时处理
            LOG_printf(DEBUG,"routesolved time out\n");
            break;
         } else if (res != 0) {
            // 其他错误处理
            perror("pthread_cond_timedwait failed");
            pthread_mutex_unlock(&_this->eventMutex);
            goto err_invalidateSock;
         }
         // 如果条件变量被唤醒，需要再次检查条件
      }
      // 释放互斥锁
      pthread_mutex_unlock(&_this->eventMutex);
      LOG_printf(DEBUG,"connState after pthread_cond_timedwait %u\n", _this->connState);

      // test point for failed connections
      if((_this->connState != IBVSOCKETCONNSTATE_ESTABLISHED) &&
         (_this->remapConnectionFailureStatus != 0))
            _this->connState = (IBVSocketConnState_t)_this->remapConnectionFailureStatus;

      // check if cm_id was reported as stale by remote side
      if(_this->connState == IBVSOCKETCONNSTATE_REJECTED_STALE)
      {
         bool createIDRes;

         if(!numStaleRetriesLeft)
         { // no more stale retries left
            if(IBVSOCKET_STALE_RETRIES_NUM) // did we have any retries at all
               LOG_printf(DEBUG,"Giving up after %d stale connection retries\n",
                  IBVSOCKET_STALE_RETRIES_NUM);

            goto err_invalidateSock;
         }

         // printk_fhgfs_connerr(KERN_INFO, "Stale connection detected. Retrying with a new one...\n");
         fprintf(stderr, "Stale connection detected. Retrying with a new one...\n");

         // We need to clean up the commContext created in the routeResolvedHandler because
         // the next time through the loop it will get recreated.  If this is the final try,
         // then we don't need it anymore.
         pthread_mutex_lock(&_this->cmaMutex);
         __IBVSocket_cleanupCommContext(_this->cm_id, _this->commContext);
         _this->commContext = (IBVCommContext*)NULL;
         createIDRes = __IBVSocket_createNewID(_this);
         pthread_mutex_unlock(&_this->cmaMutex);
         if(!createIDRes)
            goto err_invalidateSock;

         numStaleRetriesLeft--;
         continue;
      }

      if(_this->connState != IBVSOCKETCONNSTATE_ESTABLISHED)
      {
      fprintf(stderr, "Failed after %d stale connection retries, elapsed = %u\n",
         IBVSOCKET_STALE_RETRIES_NUM - numStaleRetriesLeft, Time_elapsedMS(&connElapsed));
         goto err_invalidateSock;
      }

      // connected

      if(numStaleRetriesLeft != IBVSOCKET_STALE_RETRIES_NUM)
      {
         fprintf(stderr,"Succeeded after %d stale connection retries, elapsed = %u\n",
            IBVSOCKET_STALE_RETRIES_NUM - numStaleRetriesLeft, Time_elapsedMS(&connElapsed));
      }

      return true;
   }


err_invalidateSock:
   // If we have a comm context, we need to delete it since we can't use it. We set an error state
   // on the socket first, so we stop accepting callbacks that would access the commContext that is
   // in the process of being destroyed.
   _this->errState = -1;
   // Mutex_lock(&_this->cmaMutex);
   pthread_mutex_lock(&_this->cmaMutex);
   __IBVSocket_cleanupCommContext(_this->cm_id, _this->commContext);
   _this->commContext = (IBVCommContext*)NULL;
   // Mutex_unlock(&_this->cmaMutex);
   pthread_mutex_unlock(&_this->cmaMutex);
   return false;
}

//--------------------------------- 流速控制 --------------------------------

/**
 * @return pointer to static buffer with human readable string for a wc status code
 */
const char* __IBVSocket_wcStatusStr(int wcStatusCode)
{
   switch(wcStatusCode)
   {
      case IBV_WC_WR_FLUSH_ERR:
         return "work request flush error";
      case IBV_WC_RETRY_EXC_ERR:
         return "retries exceeded error";
      case IBV_WC_RESP_TIMEOUT_ERR:
         return "response timeout error";
      default:
         return "<undefined>";
   }
}

int IBVSocket_checkConnection(IBVSocket* _this)
{
   IBVCommContext* commContext = _this->commContext;
#ifdef OFED_SPLIT_WR
# define rdma_of(wr) (wr)
# define wr_of(wr) (wr.wr)
   struct ib_rdma_wr wr;
#else
# define rdma_of(wr) (wr.wr.rdma)
# define wr_of(wr) (wr)
   struct ibv_send_wr wr;
#endif
   struct ibv_send_wr* bad_wr = NULL;
   int postRes;
   int waitRes;

   int timeoutMS = _this->timeoutCfg.completionMS;
   unsigned numWaitWrites = 0;
   unsigned numWaitReads = 1;

   rdma_of(wr).remote_addr = _this->remoteDest->vaddr;
   rdma_of(wr).rkey = _this->remoteDest->rkey;

   wr_of(wr).wr_id      = 0;
   wr_of(wr).sg_list    = commContext->checkConBuffer.lists;
   wr_of(wr).num_sge    = commContext->checkConBuffer.listLength;
   wr_of(wr).opcode     = IBV_WR_RDMA_READ;
   wr_of(wr).send_flags = IBV_SEND_SIGNALED;
   wr_of(wr).next       = NULL;

   postRes = ibv_post_send(commContext->qp, &wr_of(wr), &bad_wr);//原来那个不知道为啥的发送包是在这里呀
   if(unlikely(postRes) )
   {
      fprintf(stderr, "ibv_post_send() failed. ErrCode: %d\n", postRes);

      goto error;
   }

   waitRes = __IBVSocket_waitForTotalSendCompletion(_this,
      &commContext->incompleteSend.numAvailable, &numWaitWrites, &numWaitReads, timeoutMS);

   if(unlikely(waitRes <= 0) )
      goto error;

   commContext->incompleteSend.forceWaitForAll = false;

   return 0;

#undef rdma_of
#undef wr_of

error:
   _this->errState = -1;
   return -1;
}

/**
 * @return 1 on available data, 0 on timeout, <0 on error
 * 等待RDMA的接收完成事件。
 * 通过轮询等待事件队列上的事件来确定是否有新的接收数据，并在有数据可用时返回。
 */
int __IBVSocket_waitForRecvCompletionEvent(IBVSocket* _this, int timeoutMS, struct ibv_wc* outWC)
{
   LOG(DEBUG)<<"go into __IBVSocket_waitForRecvCompletionEvent \n";
   IBVCommContext* commContext = _this->commContext;
   long waitRes;
   int numEvents = 0;
   int checkRes;

   // special quick path: other than in the userspace version of this method, we only need the
   //    quick path when timeoutMS==0, because then we might have been called from a special
   //    context, in which we don't want to sleep
   LOG(DEBUG)<<"timeoutMS = "<<timeoutMS<<"\n";
   LOG(DEBUG)<<"outWC->status = "<<outWC->status<<"\n";
   if(!timeoutMS)
      return ibv_poll_cq(commContext->recvCQ, 1, outWC);

   auto start = std::chrono::steady_clock::now();
   int cnt=0;
   int timeoutUS=timeoutMS*1000;//XYP_0317
   int timeoutCfg_pollUS=2;//XYP_0317
   // while(timeoutMS != 0)//XYP_0317
   while(timeoutUS != 0)//XYP_0317
   {
      /* note: we use pollTimeoutMS to check the conn every few secs (otherwise we might
         wait for a very long time in case the other side disconnected silently) */

      // int pollTimeoutMS = MIN(_this->timeoutCfg.pollMS, timeoutMS);//XYP_0317
      
      int pollTimeoutUS =MIN(timeoutCfg_pollUS, timeoutMS*1000);//XYP_0317
      cnt++;
      // LOG_printf(DEBUG,"------------%d--------------\n",cnt);
      // LOG(DEBUG)<<"pollTimeoutMS = "<<pollTimeoutMS<<"\n";//XYP_0317
      LOG(DEBUG)<<"pollTimeoutUS = "<<pollTimeoutUS<<"\n";//XYP_0317

      numEvents = ibv_poll_cq(commContext->recvCQ, 1, outWC);
      LOG(DEBUG)<<"outWC->status = "<<outWC->status<<"\n";

      LOG(DEBUG)<<"numEvents = "<<numEvents<<"\n";
      if (numEvents > 0) {
         return numEvents;  // 收到事件
      }

      if(likely(numEvents) )
      { // we got something
         LOG(DEBUG)<<" we got something \n";
         return numEvents;
      }

      // timeout
      LOG(DEBUG)<<"prepare go into IBVSocket_checkConnection \n";
      // checkRes = IBVSocket_checkConnection(_this);//$$$% 因为这里发送包 ，我把这个先注释了
      LOG(DEBUG)<<"checkRes = "<<checkRes<<"\n";
      if(checkRes < 0)
         return -ECONNRESET;
      
      // 使用 sleep_for 实现超时等待
      // std::this_thread::sleep_for(std::chrono::milliseconds(pollTimeoutMS));//XYP_0317
      std::this_thread::sleep_for(std::chrono::microseconds(pollTimeoutUS));//XYP_0317

      // timeoutMS -= pollTimeoutMS;//XYP_0317
      timeoutUS -= pollTimeoutUS;//XYP_0317

      // LOG(DEBUG)<<"timeoutMS ="<<timeoutMS<<"\n";//XYP_0317
      LOG(DEBUG)<<"timeoutUS ="<<timeoutUS<<"\n";//XYP_0317

      LOG(DEBUG)<<" 进入下一次循环 \n";
   } // end of for-loop

   return 0;
}


/**
 * Receive work completion.
 *
 * Note: contains flow control.
 *
 * @param timeoutMS 0 for non-blocking
 * @return 1 on success, 0 on timeout, <0 on error
 */
int __IBVSocket_recvWC(IBVSocket* _this, int timeoutMS, struct ibv_wc* outWC)
{
   LOG(DEBUG)<<" start __IBVSocket_recvWC \n";
   IBVCommContext* commContext = _this->commContext;
   int waitRes;
   size_t bufIndex;
   LOG(DEBUG)<<" prepare go into  __IBVSocket_waitForRecvCompletionEvent \n";
   LOG(DEBUG)<<"outWC->status = "<<outWC->status<<"\n";

   waitRes = __IBVSocket_waitForRecvCompletionEvent(_this, timeoutMS, outWC);//卡在这里
   LOG(DEBUG)<<"waitRes ="<<waitRes<<"\n";
   if(waitRes <= 0)
   { // (note: waitRes==0 can often happen, because we call this with timeoutMS==0)

      if(unlikely(waitRes < 0) )
      {
         if(waitRes != -EINTR)
         { // only print message if user didn't press "<CTRL>-C"
            fprintf(stderr, "retrieval of completion event failed. result: %d\n", waitRes);
         }
      }

      return waitRes;
   }

   // we got something...
   LOG(DEBUG)<<"we got something... \n";

   if(unlikely(outWC->status != IBV_WC_SUCCESS) )//进入的是这个分支
   {
      fprintf(stderr, "%s: Connection error (wc_status: %d; msg: %s)\n",
         "IBVSocket (recv work completion)", (int)outWC->status,
         __IBVSocket_wcStatusStr(outWC->status) );
      // return -1;//$$%
   }

   bufIndex = outWC->wr_id;
   LOG(DEBUG)<<"bufIndex = "<<bufIndex<<"\n";

   if(unlikely(bufIndex >= commContext->commCfg.bufNum) )
   {
      fprintf(stderr, "Completion for unknown/invalid wr_id %d\n", (int)outWC->wr_id);
      return -1;
   }

   // receive completed

   // flow control

   if(unlikely(__IBVSocket_flowControlOnRecv(_this, _this->timeoutCfg.flowRecvMS) ) )
   {
      fprintf(stderr, "got an error from flowControlOnRecv().\n");
      // return -1;//$$$%
   }

   return 1;
}


/**
 * Intention: Avoid IB rnr by waiting for control msg when (almost) all peer bufs are used up.
 *
 * @timeoutMS may be 0 for non-blocking operation, otherwise typically
 * IBVSOCKET_FLOWCONTROL_ONSEND_TIMEOUT_MS
 * @return >0 on success, 0 on timeout (waiting for flow control packet from peer), <0 on error
 */
int __IBVSocket_flowControlOnSendWait(IBVSocket* _this, int timeoutMS)
{
   IBVCommContext* commContext = _this->commContext;

   struct ibv_wc wc;
   int recvRes;
   size_t bufIndex;
   int postRecvRes;

   LOG(DEBUG) << "numSendLeft " << commContext->numSendBufsLeft << "\n";
   if(commContext->numSendBufsLeft)
   {
      LOG(DEBUG)<<"// flow control not triggered yet 发送缓冲区未满，不需要控制 \n";
      return 1; // flow control not triggered yet 发送缓冲区未满，不需要控制

   }

   recvRes = __IBVSocket_recvWC(_this, timeoutMS, &wc);
   if(recvRes <= 0)
      return recvRes;

   bufIndex = wc.wr_id;

   if(unlikely(wc.byte_len != IBVSOCKET_FLOWCONTROL_MSG_LEN) )
   { // error (bad length)
      fprintf(stderr, "received flow control packet length mismatch %d\n", (int)wc.byte_len);
      return -1;
   }

   postRecvRes = __IBVSocket_postRecv(_this, _this->commContext, bufIndex);
   if(postRecvRes)
      return -1;

   // note: numSendBufsLeft is reset during recvWC() (if it actually received a packet)

   return 1;
}

/**
 * @param oldSendCount old sendCompEventCount
 * @return 1 on available data, 0 on timeout, -1 on error
 * 等待RDMA的发送完成事件
 * 在发送数据后使用，确保 RDMA 的 sendCompEventCount（发送完成事件计数器）更新，这表示数据已经成功发送并得到了确认
 */
int __IBVSocket_waitForSendCompletionEvent(IBVSocket* _this, int oldSendCount, int timeoutMS)
{
   IBVCommContext* commContext = _this->commContext;
   long waitRes;

   while(timeoutMS != 0)
   {
      // Note: We use pollTimeoutMS to check the conn every few secs (otherwise we might
      //    wait for a very long time in case the other side disconnected silently)
      int pollTimeoutMS = MIN(_this->timeoutCfg.pollMS, timeoutMS);
      // long pollTimeoutJiffies = TimeTk_msToJiffiesSchedulable(pollTimeoutMS);

      // // wait_event_timeout 在内核空间中等待指定的时间间隔，监测 sendCompWaitQ 事件队列中的条件是否满足；条件是 commContext->sendCompEventCount 是否不等于 oldSendCount，意味着有新的发送完成事件发生
      // waitRes = wait_event_timeout(commContext->sendCompWaitQ,
      //    atomic_read(&commContext->sendCompEventCount) != oldSendCount, pollTimeoutJiffies);

      // // fatal_signal_pending(current) 检测到终止信号
      // if(unlikely(waitRes == -ERESTARTSYS || fatal_signal_pending(current)))
      // { // signal pending
      //    fprintf(stderr, "wait for sendCompEvent ended by pending signal\n");

      //    return -1;
      // }

      // 检查发送完成事件计数器是否更新
        if (commContext->sendCompEventCount.load() != oldSendCount) {
            return 1; // 事件到达，发送完成
        }

      // 等待指定时间，以轮询方式检查事件
      std::this_thread::sleep_for(std::chrono::milliseconds(pollTimeoutMS));

      // timeout
      timeoutMS -= pollTimeoutMS;
   }

   return 0;
}

/**
 * Intention: Avoid IB rnr by sending control msg when (almost) all our recv bufs are used up to
 * show that we got our new recv bufs ready.
 *
 * @timeoutMS don't set this to 0, we really need to wait here sometimes
 * @return 0 on success, -1 on error
 */
int __IBVSocket_flowControlOnRecv(IBVSocket* _this, int timeoutMS)
{
   IBVCommContext* commContext = _this->commContext;

   // we received a packet, so peer has received all of our currently pending data => reset counter
   commContext->numSendBufsLeft = commContext->commCfg.bufNum - 1; /* (see
      createCommContext() for "-1" reason) */

   // send control packet if recv counter expires...

   #ifdef BEEGFS_DEBUG
      if(!commContext->numReceivedBufsLeft)
         fprintf(stderr, "BUG: numReceivedBufsLeft underflow!\n");
   #endif // BEEGFS_DEBUG

   commContext->numReceivedBufsLeft--;

   if(!commContext->numReceivedBufsLeft)
   {
      size_t currentBufIndex;
      int postRes;

      if(commContext->incompleteSend.forceWaitForAll ||
         (commContext->incompleteSend.numAvailable == commContext->commCfg.bufNum) )
      { // wait for all (!) incomplete sends

         /* note: it's ok that all send bufs are used up, because it's possible that we do a lot of
            recv without the user sending any data in between (so the bufs were actually used up by
            flow control). */
         unsigned numWaitWrites = 0;
         unsigned numWaitReads = 0;

         int waitRes = __IBVSocket_waitForTotalSendCompletion(_this,
            &commContext->incompleteSend.numAvailable, &numWaitWrites, &numWaitReads, timeoutMS);
         if(waitRes <= 0)
         {
            fprintf(stderr, "problem encountered during waitForTotalSendCompletion(). ErrCode: %d\n",
               waitRes);
            return -1;
         }

         commContext->incompleteSend.forceWaitForAll = false;
      }

      currentBufIndex = commContext->incompleteSend.numAvailable;

      commContext->incompleteSend.numAvailable++; /* inc'ed before postSend() for conn checks */

      commContext->sendBufs[currentBufIndex].lists[0].length = IBVSOCKET_FLOWCONTROL_MSG_LEN;
      commContext->sendBufs[currentBufIndex].listLength = 1;

      postRes = __IBVSocket_postSend(_this, currentBufIndex);
      if(unlikely(postRes) )
      {
         commContext->incompleteSend.numAvailable--;
         return -1;
      }


      // note: numReceivedBufsLeft is reset during postSend() flow control
   }

   return 0;
}

/**
 * Called after sending a packet to update flow control counters.
 *
 * Intention: Avoid IB rnr by waiting for control msg when (almost) all peer bufs are used up.
 *
 * Note: This is only one part of the on-send flow control. The other one is
 * _flowControlOnSendWait().
 */
void __IBVSocket_flowControlOnSendUpdateCounters(IBVSocket* _this)
{
   IBVCommContext* commContext = _this->commContext;

   // we sent a packet, so we received all currently pending data from the peer => reset counter
   commContext->numReceivedBufsLeft = commContext->commCfg.bufNum - 1; /* (see
      createCommContext() for "-1" reason) */

   #ifdef BEEGFS_DEBUG

   if(!commContext->numSendBufsLeft)
      fprintf(stderr, "BUG: numSendBufsLeft underflow!\n");

   #endif

   commContext->numSendBufsLeft--;
}

/**
 * @param numSendElements also used as out-param to return the remaining number
 * @param timeoutMS 0 for non-blocking; this is a soft timeout that is reset after each received
 * completion
 * @return 1 if all completions received, 0 if completions missing (in case you wanted non-blocking)
 * or -1 in case of an error.
 */
// 等待 RDMA 发送队列中所有发送操作的完成
/*numSendElements：发送操作的剩余元素计数指针。函数通过减少该计数器来判断所有发送操作是否已完成。
numWriteElements：写操作的剩余元素计数指针。
numReadElements：读操作的剩余元素计数指针。
timeoutMS：等待完成事件的超时时间（毫秒）*/
int __IBVSocket_waitForTotalSendCompletion(IBVSocket* _this,
   unsigned* numSendElements, unsigned* numWriteElements, unsigned* numReadElements, int timeoutMS)
{
   IBVCommContext* commContext = _this->commContext;
   int numElements;
   int waitRes;
   int oldSendCount;
   int i;
   size_t bufIndex;
   struct ibv_wc wc[2];

   do
   {
      oldSendCount = commContext->sendCompEventCount.load();
      LOG(DEBUG)<< "oldSendout: " << oldSendCount << "\n";

      numElements = ibv_poll_cq(commContext->sendCQ, 2, wc); // 轮询发送完成队列，最多获取两个工作完成（wc）
      LOG_printf(DEBUG,"__IBVSocket_waitForTotalSendCompletion ibv_pool_cq returns %d\n", numElements);
      if(unlikely(numElements < 0) )
      {
         fprintf(stderr, "bad ib_poll_cq result: %d\n", numElements);

         return -1;
      }
      else
      if(!numElements)
      { // no completions available yet => wait
         if(!timeoutMS)
            return 0;

         waitRes = __IBVSocket_waitForSendCompletionEvent(_this, oldSendCount, timeoutMS);
         LOG_printf(DEBUG,"in __IBVSocket_waitForTotalSendCompletion : __IBVSocket_waitForSendCompletionEvent returns %d\n", waitRes);
         if(likely(waitRes > 0) )
            continue;

         return waitRes;
      }

      // we got something...

      // for each completion element
      for(i=0; i < numElements; i++)
      {
         if(unlikely(wc[i].status != IBV_WC_SUCCESS) )
         {
            fprintf(stderr, "%s: Connection error (wc_status: %d; msg: %s)\n",
               "IBVSocket (wait for total send completion)", (int)(wc[i].status),
               __IBVSocket_wcStatusStr(wc[i].status) );

            return -1;
         }

         switch(wc[i].opcode)
         {
            case IBV_WC_SEND:
            {
               bufIndex = wc[i].wr_id;

               if(unlikely(bufIndex >= commContext->commCfg.bufNum) )
               {
                  fprintf(stderr, "bad send completion wr_id 0x%x\n", (int)wc[i].wr_id);

                  return -1;
               }

               if(likely(*numSendElements) )
                  (*numSendElements)--;
               else
               {
                  fprintf(stderr, "received bad/unexpected send completion\n");

                  return -1;
               }

            } break;

            case IBV_WC_RDMA_READ:
            {
               if(unlikely(wc[i].wr_id != 0) )
               {
                  fprintf(stderr, "bad read completion wr_id 0x%x\n", (int)wc[i].wr_id);

                  return -1;
               }

               if(likely(*numReadElements) )
                  (*numReadElements)--;
               else
               {
                  fprintf(stderr, "received bad/unexpected RDMA read completion\n");

                  return -1;
               }
            } break;

            default:
            {
               fprintf(stderr, "received bad/unexpected completion opcode %d\n", wc[i].opcode);

               return -1;
            } break;

         } // end of switch

      } // end of for-loop

   } while(*numSendElements || *numWriteElements || *numReadElements);

   return 1;
}


/**
 * Note: contains flow control.
 *
 * @return 0 on success, -1 on error
 */
int __IBVSocket_postSend(IBVSocket* _this, size_t bufIndex)
{
   LOG(DEBUG)<<" go into __IBVSocket_postSend \n";
   IBVCommContext* commContext = _this->commContext;
   /*
   //以下是修改前的代码 wsy_source_code
      struct ibv_send_wr wr;
      struct ibv_send_wr* bad_wr = NULL;
      int postRes;
      memset(&wr, 0, sizeof(wr));

      wr.wr_id      = bufIndex;
      wr.sg_list    = commContext->sendBufs[bufIndex].lists;

            LOG(DEBUG)<<"list.addr = "<<commContext->sendBufs[bufIndex].lists->addr<<"\t ,list.length ="<<commContext->sendBufs[bufIndex].lists->length
            <<"\t, list.lkey = "<<commContext->sendBufs[bufIndex].lists->lkey<<"\n";
            for(int i=0;i<48;i++)
            {
               LOG(DEBUG)<<reinterpret_cast<char*>(commContext->sendBufs[bufIndex].lists->addr)[i];
            }
            LOG(DEBUG)<<"\n lists.addr \n"; //这个内容是正确的
            
      wr.num_sge    = commContext->sendBufs[bufIndex].listLength;
      wr.opcode     = IBV_WR_SEND;
      wr.send_flags = IBV_SEND_SIGNALED;
      wr.next       = NULL;

      postRes = ibv_post_send(commContext->qp, &wr, &bad_wr);//返回值是正确的，为什么没有抓到包？
   */
//   struct ibv_send_wr wr;
    struct ibv_send_wr sr;

   struct ibv_send_wr* bad_wr = NULL;
   int postRes;
   // memset(&wr, 0, sizeof(wr));
    memset(&sr, 0, sizeof(sr));


   // wr.wr_id      = bufIndex;
   sr.wr_id      = bufIndex;
   LOG(DEBUG)<<"bufIndex = "<<bufIndex<<"\n";

   // wr.sg_list    = commContext->sendBufs[bufIndex].lists;
   // 发送"Hello World"
   //  char *testbuffer = "Hello World";
   //  size_t buffer_size = strlen(testbuffer) + 1;
   //          for(int i=0;i<48;i++)
   //          {
   //             LOG(DEBUG)<<reinterpret_cast<char*>(commContext->sendBufs[bufIndex].lists->addr)[i];
   //          }
   //          LOG(DEBUG)<<"\n lists.addr \n"; //这个内容是正确的
   //          LOG(DEBUG)<<"strlen = "<<strlen(reinterpret_cast<char*>(commContext->sendBufs[bufIndex].lists->addr))<<"\n";
   size_t buffer_size=commContext->sendBufs[bufIndex].datasize[0];
   LOG(DEBUG)<<" commContext->sendBufs[bufIndex].datasize[0] , buffer_size = "<<buffer_size<<"\n";
   char *testbuffer = (char *)malloc(buffer_size);
   if (!testbuffer) {
      fprintf(stderr, "Failed to allocate memory\n");
      return -1;
   }
   LOG(DEBUG)<<"Successfully to allocate memory\n ";
   // strcpy(testbuffer, commContext->sendBufs[bufIndex].buffers[0]); //1206修改成下面这个
   memcpy(testbuffer,commContext->sendBufs[bufIndex].buffers[0],buffer_size);

   // for(int j=0;j<buffer_size;j++)
   // {
   //    // LOG(DEBUG)<<commContext->sendBufs[bufIndex].buffers[0][j];
   //    LOG(DEBUG)<<testbuffer[j];

   // }
   LOG(DEBUG)<<"\n lists.addr \n"; //这个内容是正确的
    struct ibv_mr *mr;
    mr = ibv_reg_mr(commContext->pd, testbuffer, buffer_size, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ);
    if (!mr) {
        fprintf(stderr, "Failed to register memory region\n");
        ibv_destroy_qp(commContext->qp);
        rdma_destroy_id(_this->cm_id );
        rdma_destroy_event_channel(_this->event_channel);
        ibv_dealloc_pd(commContext->pd);
        ibv_close_device(_this->cm_id->verbs);
        return -1;
    }
    LOG_printf(DEBUG,"Memory region registered successfully.\n");

   struct ibv_sge sge;
    sge.addr = (uintptr_t)testbuffer;
    sge.length = buffer_size;
    sge.lkey = mr->lkey;
    sr.sg_list = &sge;


        
         
   // wr.num_sge    = commContext->sendBufs[bufIndex].listLength;
    sr.num_sge = 1;

   // wr.opcode     = IBV_WR_SEND;
    sr.opcode = IBV_WR_SEND;

   // wr.send_flags = IBV_SEND_SIGNALED;
    sr.send_flags = IBV_SEND_SIGNALED;

   // wr.next       = NULL;
    sr.next = NULL;

   // postRes = ibv_post_send(commContext->qp, &wr, &bad_wr);//返回值是正确的，为什么没有抓到包？

   postRes = ibv_post_send(commContext->qp, &sr, &bad_wr);//返回值是正确的，为什么没有抓到包？
   LOG(DEBUG)<<"postRes = "<<postRes<<"\n";
   if(unlikely(postRes) )
   {
      fprintf(stderr, "ib_post_send() failed. ErrCode: %d\n", postRes);

      return -1;
   }

   // flow control
   __IBVSocket_flowControlOnSendUpdateCounters(_this);

   LOG(DEBUG)<<" out of __IBVSocket_postSend \n";

   return 0;
}









//------------------------------------send--------------------------------
// 检查发送缓冲区是否可用
int __IBVSocket_nonblockingSendCheck(IBVSocket* _this)
{
   IBVCommContext* commContext = _this->commContext;
   int flowControlRes;
   int waitRes;

   unsigned numWaitWrites = 0;
   unsigned numWaitReads = 0;
   int timeoutMS = 0;

   if(unlikely(_this->errState) )
      return -1;

   // check whether we have a pending on-send flow control packet that needs to be received first
   flowControlRes = __IBVSocket_flowControlOnSendWait(_this, 0);
   LOG_printf(DEBUG,"in __IBVSocket_nonblockingSendCheck: __IBVSocket_flowControlOnSendWait returns %d \n", flowControlRes);
   if(unlikely(flowControlRes < 0) ){
      // goto err_invalidateSock;
      fprintf(stderr, "invalidating connection\n");

      _this->errState = -1;
      return -1;
   }

   if(!flowControlRes)
      return flowControlRes;

   LOG_printf(DEBUG,"--------------onblockingSendCheck----------- \n");
   struct IBVIncompleteSend send = _this->commContext->incompleteSend;
   LOG(DEBUG) << "IBVIncompleteSend:\n";
   LOG(DEBUG) << "  numAvailable: " << send.numAvailable << "\n";
   LOG(DEBUG) << "  forceWaitForAll: " << (send.forceWaitForAll ? "true" : "false") << "\n";
   LOG(DEBUG) << "commCfg.bufNum"<< commContext->commCfg.bufNum << "\n";

   if(!commContext->incompleteSend.forceWaitForAll &&
      (commContext->incompleteSend.numAvailable < commContext->commCfg.bufNum) )
      return 1;

   commContext->incompleteSend.forceWaitForAll = true; // always setting saves an "if" below

   // we have to wait for completions before we can send...

   waitRes = __IBVSocket_waitForTotalSendCompletion(_this,
      &commContext->incompleteSend.numAvailable, &numWaitWrites, &numWaitReads, timeoutMS);
   LOG_printf(DEBUG,"in __IBVSocket_nonblockingSendCheck : __IBVSocket_waitForTotalSendCompletion returns %d \n", waitRes);
   if(unlikely(waitRes < 0) )
      goto err_invalidateSock;

   if(waitRes > 0)
      commContext->incompleteSend.forceWaitForAll = false; // no more completions peding

   return waitRes;

err_invalidateSock:
   fprintf(stderr, "invalidating connection\n");

   _this->errState = -1;
   return -1;
}


ssize_t IBVSocket_send(IBVSocket* _this, iovec_iter* iter, int flags)
{
   LOG_printf(DEBUG,"IBVSocket_senf begin. list iter data\n");
   for(int i=0;i<iter->iovcnt;i++){
      LOG_printf(DEBUG,"%s\n", iter->iov[i].iov_base);
   }
   LOG(DEBUG)<<"iter = \n";
   LOG(DEBUG)<<"iter->total_len = "<<iter->total_len<<"\n";
   // for(int i=0;i<iter->total_len;i++){
   // LOG(DEBUG)<<static_cast<char*>(iter->iov[0].iov_base)[i];
   // }
   LOG(DEBUG)<<"\n";
   IBVCommContext* commContext = _this->commContext;   // 通信上下文
  
   struct IBVIncompleteRecv recv = _this->commContext->incompleteRecv;
   LOG(DEBUG) << "IBVIncompleteRecv:\n";
   LOG(DEBUG) << "  isAvailable: " << recv.isAvailable << "\n";
   LOG(DEBUG) << "  completedOffset: " << recv.completedOffset << "\n";
   LOG(DEBUG) << "  bufIndex: " << recv.bufIndex << "\n";
   LOG(DEBUG) << "  totalSize: " << recv.totalSize << "\n";
   struct IBVIncompleteSend send = _this->commContext->incompleteSend;
   LOG(DEBUG) << "IBVIncompleteSend:\n";
   LOG(DEBUG) << "  numAvailable: " << send.numAvailable << "\n";
   LOG(DEBUG) << "  forceWaitForAll: " << (send.forceWaitForAll ? "true" : "false") << "\n";
   int flowControlRes;                   
   size_t currentBufIndex;
   iovec_iter source = *iter;
   int postRes;
   size_t postedLen = 0;
   ssize_t currentPostLen;
   int waitRes;

   unsigned numWaitWrites = 0;
   unsigned numWaitReads = 0;
   int timeoutMS = _this->timeoutCfg.completionMS;

   if(unlikely(_this->errState) )
      return -1;

   // handle flags
   if(flags & MSG_DONTWAIT)  // 非阻塞发送
   { // send only as much as we can without blocking

      // note: we adapt the bufLen variable as necessary here for simplicity
      LOG_printf(DEBUG,"非阻塞发送\n");
      int checkSendRes;
      size_t bufNumLeft;
      size_t bufLenLeft;

      checkSendRes = __IBVSocket_nonblockingSendCheck(_this);   // 检查发送缓冲区是否可用
      LOG_printf(DEBUG,"in IBVSocket_send: __IBVSocket_nonblockingSendCheck returns %d \n", checkSendRes);
      if(!checkSendRes)
      { // we can't send non-blocking at the moment, caller shall try again later
         LOG_printf(DEBUG,"--------发送缓冲区不可用---------\n");
         return -EAGAIN;
      }
      else
      if(unlikely(checkSendRes < 0) )
         goto err_invalidateSock;

      // buffers available => adapt bufLen (if necessary)
      LOG_printf(DEBUG,"--------send buffer is available---------\n");

      bufNumLeft = MIN(commContext->commCfg.bufNum - commContext->incompleteSend.numAvailable,
         commContext->numSendBufsLeft);
      LOG_printf(DEBUG,"bufNum = %d, bufNumleft = %d\n", commContext->commCfg.bufNum, bufNumLeft);
      bufLenLeft = bufNumLeft * commContext->commCfg.bufSize;

      iovec_iter_truncate(&source, bufLenLeft);
   }

   // send data cut in buf-sized pieces...

   do  //将数据按照缓冲区大小分片发送
   {
      flowControlRes = __IBVSocket_flowControlOnSendWait(_this,
         _this->timeoutCfg.flowSendMS);    // 等待发送缓冲区可用
      if(unlikely(flowControlRes <= 0) )
         goto err_invalidateSock;

      // note: we only poll for completed sends if forced or after we used up all (!) available bufs

      if(commContext->incompleteSend.forceWaitForAll ||
         (commContext->incompleteSend.numAvailable == commContext->commCfg.bufNum) )
      { // wait for all (!) incomplete sends  // 等待所有未完成的发送完成
         waitRes = __IBVSocket_waitForTotalSendCompletion(_this,
            &commContext->incompleteSend.numAvailable, &numWaitWrites, &numWaitReads, timeoutMS);
         if(waitRes <= 0)
            goto err_invalidateSock;

         commContext->incompleteSend.forceWaitForAll = false;
      }


      currentBufIndex = commContext->incompleteSend.numAvailable;   // 获取当前可用的发送缓冲区索引
      LOG_printf(DEBUG,"currentBufIndex=%d\n",currentBufIndex);

      currentPostLen = IBVBuffer_fill(&commContext->sendBufs[currentBufIndex], &source);   // 数据从 source 拷贝到发送缓冲区
      // for(int j=0;j<48;j++)
      // {
      //    LOG(DEBUG)<<commContext->sendBufs[currentBufIndex].buffers[0][j];
      // }
      LOG(DEBUG)<<"\n after IBVBuffer_fill \n";
      if(currentPostLen < 0)
         goto err_fault;

      commContext->incompleteSend.numAvailable++; /* inc'ed before postSend() for conn checks */

      postRes = __IBVSocket_postSend(_this, currentBufIndex);
      LOG(DEBUG)<<"after __IBVSocket_postSend, postRes ="<<postRes<<"\n";
      if(unlikely(postRes) )
      {
         commContext->incompleteSend.numAvailable--;
         goto err_invalidateSock;
      }

      postedLen += currentPostLen;
   } while(iovec_iter_count(&source));

   iovec_iter_advance(iter, postedLen);
   return (ssize_t)postedLen;

err_invalidateSock:
   _this->errState = -1;
   return -ECOMM;

err_fault:
   _this->errState = -1;
   return -EFAULT;
}

//------------------------------------receive--------------------------------

/**
 * @return <0 on error, 0 if recv would block, >0 if recv would not block
 */
int __IBVSocket_receiveCheck(IBVSocket* _this, int timeoutMS)
{
   IBVCommContext* commContext = _this->commContext;
   struct ibv_wc wc;
   int flowControlRes;
   int recvRes;

   if(unlikely(_this->errState) )
      return -1;

   if(commContext->incompleteRecv.isAvailable)
      return 1;

   // check whether we have a pending on-send flow control packet that needs to be received first
   flowControlRes = __IBVSocket_flowControlOnSendWait(_this, timeoutMS);
   LOG(DEBUG)<<"flowControlRes =  "<<flowControlRes<<"\n";// 1
   if(unlikely(flowControlRes < 0) )
   {
      fprintf(stderr, "got an error from flowControlOnSendWait(). ErrCode: %d\n",
         flowControlRes);
      goto err_invalidateSock;
   }

   if(!flowControlRes)
      return 0;

   // recv one packet (if available) and add it as incompleteRecv
   LOG(DEBUG)<<"recv one packet (if available) and add it as incompleteRecv \n";
   recvRes = __IBVSocket_recvWC(_this, timeoutMS, &wc);//卡在这里
   LOG(DEBUG)<<"recvRes ="<<recvRes<<"\n";//-1
   if(unlikely(recvRes < 0) )
   {
      fprintf(stderr, "got an error from __IBVSocket_recvWC(). ErrCode: %d\n", recvRes);
      goto err_invalidateSock;
   }

   if(!recvRes)
      return 0;

   // we got something => prepare to continue later

   commContext->incompleteRecv.totalSize = wc.byte_len;
   commContext->incompleteRecv.bufIndex = wc.wr_id;
   commContext->incompleteRecv.completedOffset = 0;
   commContext->incompleteRecv.isAvailable = 1;

   return 1;

err_invalidateSock:
   fprintf(stderr, "invalidating connection\n");
   _this->errState = -1;
   return -1;
}

/**
 * Continues an incomplete former recv() by returning immediately available data from the
 * corresponding buffer.
 */
ssize_t __IBVSocket_recvContinueIncomplete(IBVSocket* _this, struct iovec_iter* iter)
{
   LOG(DEBUG)<<"go into __IBVSocket_recvContinueIncomplete \n";
   IBVCommContext* commContext = _this->commContext;
   struct IBVIncompleteRecv* recv = &commContext->incompleteRecv;
   size_t bufIndex = recv->bufIndex;//xyp_0317
   // size_t bufIndex = 0; //xyp_0317
   struct IBVBuffer* buffer = &commContext->recvBufs[bufIndex];
   // buffer->bufferSize=4096;//xyp_0317

   size_t copyRes = 0;
   ssize_t total = 0;
   LOG(DEBUG)<<"_this->errState = "<<_this->errState<<"\n";
   LOG(DEBUG)<<"recv->completedOffset = "<<recv->completedOffset<<"\n";
   LOG(DEBUG)<<"recv->totalSize = "<<recv->totalSize<<"\n";
   if(unlikely(_this->errState) )//$$%
      return -1;
   LOG(DEBUG)<<"iter->total_len = "<<iter->total_len<<"\n";
   

   LOG(DEBUG)<<"buffer->bufferSize = "<<buffer->bufferSize<<"\n";


   while(iovec_iter_count(iter) > 0 && recv->totalSize != recv->completedOffset)
   {
      unsigned page = recv->completedOffset / buffer->bufferSize;
      unsigned offset = recv->completedOffset % buffer->bufferSize;
      unsigned fragment = MIN(MIN(iovec_iter_count(iter), buffer->bufferSize - offset),
         recv->totalSize - recv->completedOffset);
      // LOG(DEBUG)<<"fragment = "<<fragment<<"\n";
      // LOG(DEBUG)<<"page = "<<page<<"\t, offset = "<<offset<<"\n";
      copyRes = copy_to_iovec(buffer->buffers[page] + offset, fragment, iter);
      // LOG(DEBUG)<<"buffer = \n";
      // for(int j=0;j<fragment;j++)
      // {
      //    LOG(DEBUG)<<std::hex << std::setw(2) << std::setfill('0') << (int)(buffer->buffers[page] + offset)[j]<<" ";
      // }
      // LOG(DEBUG) << std::dec << std::endl; // 打印完毕后切换回十进制格式
      // LOG(DEBUG)<<"\n buffer end \n";

      if(copyRes != fragment)
      {
         copyRes = 0;
         break;
      }

      total += fragment;

      recv->completedOffset += fragment;
   }

   if(recv->completedOffset == recv->totalSize)
   {
      int postRes;

      commContext->incompleteRecv.isAvailable = 0;
      LOG(DEBUG)<<" prepare go into __IBVSocket_postRecv \n ";
      LOG(DEBUG)<<"bufIndex : "<<bufIndex<<"\n";
      postRes = __IBVSocket_postRecv(_this, _this->commContext, bufIndex);
      LOG(DEBUG)<<" out of  __IBVSocket_postRecv \n ";
      LOG(DEBUG)<<"postRes : "<<postRes<<"\n";

      if(unlikely(postRes) )
         goto err_invalidateSock;
   }

   if(!copyRes)
      goto err_fault;

   return total;

err_invalidateSock:
   LOG_printf(DEBUG,"invalidating connection\n");

err_fault:
   _this->errState = -1;
   return -EFAULT;
}



/**
 * @return number of received bytes on success, -ETIMEDOUT on timeout, -ECOMM on error
 */
ssize_t IBVSocket_recvT(IBVSocket* _this, struct iovec_iter* iter, int flags, int timeoutMS)
{  
   LOG(DEBUG)<<"go into IBVSocket_recvT \n";
   int checkRes;
   int wait = timeoutMS < 0 ? IBVSOCKET_RECVT_INFINITE_TIMEOUT_MS : timeoutMS;
   LOG(DEBUG)<<"wait = "<<wait<<"\n";//wait == timeoutMS
   
   do {
      checkRes = __IBVSocket_receiveCheck(_this, wait);//卡在这里
      LOG(DEBUG)<<"checkRes = "<<checkRes<<"\n";
   } while (checkRes == 0 && timeoutMS < 0);

   if(checkRes < 0)
      return -ECOMM;

   if(checkRes == 0)
      return -ETIMEDOUT;
   


   return __IBVSocket_recvContinueIncomplete(_this, iter);
}





//----------------------------------ops--------------------------------------

bool _RDMASocket_connectByIP(Socket* thisone, struct in_addr ipaddress, unsigned short port)
{
   // note: does not set the family type to the one of this socket.

   RDMASocket* thisCast = (RDMASocket*)thisone;

   bool connRes;
   connRes = IBVSocket_connectByIP(&thisCast->ibvsock, ipaddress, port, &thisCast->commCfg);

   if(!connRes)
   {
      // note: this message would flood the log if hosts are unreachable on the primary interface

      // char* ipStr = SocketTk_ipaddrToStr(ipaddress);
      // printk_fhgfs(KERN_WARNING, "RDMASocket failed to connect to %s.\n", ipStr);
      // kfree(ipStr);

      return false;
   }

   // connected
   // LOG_printf(DEBUG,"-------------------成功完成IBVSocket_connectByIP--------------\n");

   // set peername if not done so already (e.g. by connect(hostname) )
   if(!thisone->peername[0])
   {
      SocketTk_endpointAddrToStrNoAlloc(thisone->peername, SOCKET_PEERNAME_LEN, ipaddress, port);
      thisone->peerIP = ipaddress;
   }

   return true;
}


ssize_t _RDMASocket_recvT(Socket* thisone, iovec* iter, int flags, int timeoutMS)
{
   RDMASocket* thisCast = (RDMASocket*)thisone;

   ssize_t retVal;
   iovec_iter iter1;
   iovec_iter_init(&iter1, iter, 1);
   LOG(DEBUG)<<"go into _RDMASocket_recvT \n";
   timeoutMS=5000;//
   retVal = IBVSocket_recvT(&thisCast->ibvsock, &iter1, flags, timeoutMS);
   unsigned char *ptr = (unsigned char *)iter1.iov->iov_base; // 将 void* 转换为 unsigned char* 以按字节访问
   //  for (size_t i = 0; i < iter1.iov->iov_len; ++i) {
   //      LOG_printf(DEBUG,"%02x ", ptr[i]); // 以十六进制格式打印每个字节
   //      if ((i + 1) % 16 == 0) {
   //          LOG_printf(DEBUG,"\n"); // 每16个字节换行，为了可读性
   //      }
   //  }
   
   //    std:cout<<"\n end \n";
   LOG(DEBUG)<<"[_RDMASocket_recvT] retVal : "<<retVal<<" \n";
   return retVal;
}

ssize_t _RDMASocket_sendto(Socket* thisone, iovec* iter, int iovcnt, int flags,
   fhgfs_sockaddr_in *to)
{
   RDMASocket* thisCast = (RDMASocket*)thisone;

   ssize_t retVal;
   iovec_iter iter1;
   iovec_iter_init(&iter1, iter, 1);

   retVal = IBVSocket_send(&thisCast->ibvsock, &iter1, flags);
   LOG(DEBUG)<<"out of IBVSocket_send \n";
   LOG(DEBUG)<<"retVal = "<<retVal<<"\n";
   return retVal;
}

static const struct SocketOps rdmaOps = {
   // .uninit = _RDMASocket_uninit,

   .connectByIP = _RDMASocket_connectByIP,
   // .bindToAddr = _RDMASocket_bindToAddr,
   // .listen = _RDMASocket_listen,
   // .shutdown = _RDMASocket_shutdown,
   // .shutdownAndRecvDisconnect = _RDMASocket_shutdownAndRecvDisconnect,

   .sendto = _RDMASocket_sendto,
   .recvT = _RDMASocket_recvT,
};


// void _PooledSocket_init(PooledSocket* psock, NicAddrType_t nicType){  

//    Socket* sock=(Socket*)psock;
//    memset(sock, 0, sizeof(*sock) );//$$ 头文件

//    sock->sockType = NICADDRTYPE_RDMA;
//    sock->boundPort = -1;

//    psock->available = false;
//    psock->hasActivity = true; // initially active to avoid immediate disconnection
//    psock->closeOnRelease = false;
//    psock->expireTimeStart.tv_sec = 0;
//    psock->expireTimeStart.tv_nsec = 0;
//    psock->nicType = nicType;
//    psock->pool = nullptr;
//    psock->poolElem = nullptr;
// }

void _PooledSocket_uninit(Socket* thisone){

}

IBVSocketKeyType RDMASocket_toIBVSocketKeyType(RDMAKeyType keyType)
{
   switch (keyType)
   {
   case RDMAKEYTYPE_UnsafeDMA:
      return IBVSOCKETKEYTYPE_UnsafeDMA;
   case RDMAKEYTYPE_Register:
      return IBVSOCKETKEYTYPE_Register;
   default:
      return IBVSOCKETKEYTYPE_UnsafeGlobal;
   }
}

bool IBVSocket_init(IBVSocket* _this, struct in_addr srcIpAddr, NicAddressStats* nicStats)
{
   memset(_this, 0, sizeof(*_this) );

   _this->connState = IBVSOCKETCONNSTATE_UNCONNECTED;

   _this->timeoutCfg.connectMS = IBVSOCKET_CONN_TIMEOUT_MS;
   _this->timeoutCfg.completionMS = IBVSOCKET_COMPLETION_TIMEOUT_MS;
   _this->timeoutCfg.flowSendMS = IBVSOCKET_FLOWCONTROL_ONSEND_TIMEOUT_MS;
   _this->timeoutCfg.flowRecvMS = IBVSOCKET_FLOWCONTROL_ONRECV_TIMEOUT_MS;
   _this->timeoutCfg.pollMS = IBVSOCKET_POLL_TIMEOUT_MS;

   _this->typeOfService = 0;
   _this->srcIpAddr = srcIpAddr;
   _this->nicStats = nicStats;

   // init_waitqueue_head(&_this->eventWaitQ);   
    // 初始化条件变量和互斥锁
   pthread_cond_init(&_this->eventCond, (const pthread_condattr_t *)NULL);
   pthread_mutex_init(&_this->eventMutex, (const pthread_mutexattr_t *)NULL);

   // Mutex_init(&_this->cmaMutex);
   // 初始化互斥锁
   pthread_mutex_init(&_this->cmaMutex, (const pthread_mutexattr_t *)NULL);
   return __IBVSocket_createNewID(_this);
}


// 获取RDMA连接
bool RDMASocket_init(RDMASocket* thisoneone, struct in_addr src, NicAddressStats* nicStats)
{
   Socket* thisoneBase = (Socket*)thisoneone;

   // init super class
   _PooledSocket_init( (PooledSocket*)thisoneone, NICADDRTYPE_RDMA);

   thisoneBase->ops = (SocketOps*)&rdmaOps;

   // normal init part

   thisoneBase->sockType = NICADDRTYPE_RDMA;

   thisoneone->commCfg.bufNum = RDMASOCKET_DEFAULT_BUF_NUM;
   thisoneone->commCfg.bufSize = RDMASOCKET_DEFAULT_BUF_SIZE;
   thisoneone->commCfg.fragmentSize = RDMASOCKET_DEFAULT_FRAGMENT_SIZE;
   thisoneone->commCfg.keyType = RDMASocket_toIBVSocketKeyType(RDMASOCKET_DEFAULT_KEY_TYPE);

   if(!IBVSocket_init(&thisoneone->ibvsock, src, nicStats) )
      goto err_ibv;
   // if(!thisoneone->ibvsock.commContext){
   //    LOG_printf(DEBUG,"empty commContext!!!! error \n");
   // }
   // LOG_printf(DEBUG,"after RDMASocket_init %d\n", thisoneone->ibvsock.commContext->incompleteSend.numAvailable);
   return true;

err_ibv:
   _PooledSocket_uninit(&thisoneone->pooledSocket.socket);
   return false;
}

RDMASocket* RDMASocket_construct(struct in_addr src, NicAddressStats *nicStats)
{
   RDMASocket* ssock = (RDMASocket*)malloc(sizeof(RDMASocket));

   if(!ssock ||
      !RDMASocket_init(ssock, src, nicStats) )
   {
      free(ssock);
      return (RDMASocket*)NULL;
   }

   return ssock;
}

// // communication 的第一阶段，准备连接
// static bool __commkit_prepare_generic_try_connect(CommKitContext* context, struct CommKitTargetInfo* info)
// {
//    struct in_addr srcAddr;
//    NicAddress* nicAddr = (NicAddress*)malloc(sizeof(NicAddress));
//    NicAddressStats* srcRdma = (NicAddressStats*)malloc(sizeof(NicAddressStats));

//    const char* LocalRdmaIP = "192.168.0.204";       //本地dell02的RDMA IP,   网口名为 enp130s0np0

//    if (inet_pton(AF_INET, LocalRdmaIP, &srcAddr) <= 0) {    // 将点分十进制的IP地址转换为用于网络传输的数值格式。
//       perror("Invalid IP address");
//       return -1;
//    }
//    LOG_printf(DEBUG,"查看本主机IP情况 LocalIP %s and tranfer to %u\n", LocalRdmaIP, srcAddr.s_addr);
//    nicAddr->ipAddr = srcAddr;
//    nicAddr->nicType = NICADDRTYPE_RDMA;
//    strncpy(nicAddr->name, "enp130s0np0", IFNAMSIZ);   //本地 RDMA 端口名
// #ifdef BEEGFS_RDMA
//    LOG_printf(DEBUG,"-------------检测RDMA设备-----------------\n");
//    struct ibv_device **dev_list = (ibv_device**)NULL;
//    int num_devices = 0;

//    dev_list = ibv_get_device_list(&num_devices);    //获取RDMA列表
//    if (!dev_list) {
//       perror("Failed to get IB devices list");
//       return -1;
//    }
//    LOG_printf(DEBUG,"Found %d RDMA device(s):\n", num_devices);
//     for (int i = 0; i < num_devices; ++i) {
//         LOG_printf(DEBUG,"  Device %d: %s\n", i, ibv_get_device_name(dev_list[i]));
//     }
   
//    struct ibv_device *ib_dev = (ibv_device*)NULL;
//    for (int i = 0; i < num_devices; ++i) {
//     if (strcmp(ibv_get_device_name(dev_list[i]), "rocep130s0") == 0) {       //RDMA设备名来寻找设备对象， 用 ibv_devices 或 ls /sys/class/infiniband 查看
//         ib_dev = dev_list[i];
//         break;
//       } 
//    }

//    if (!ib_dev) {
//     fprintf(stderr, "RDMA device rocep130s0 not found\n");
//     ibv_free_device_list(dev_list);
//     return -1;
//    }
//    LOG_printf(DEBUG,"获取RDMA设备rocep130s0成功\n");

//    nicAddr->ibdev = ib_dev;  // 若有实际的 ibv_device 可填充
//    ibv_free_device_list(dev_list);
// #endif

//    NicAddressStats_init(srcRdma, nicAddr);

//    info->socket = (Socket*)RDMASocket_construct(srcAddr, srcRdma);  //构造一个RDMA socket
//    if(!info->socket)
//    { // no conn available => error or didn't want to wait
//      LOG_printf(DEBUG,"----------------获取RDMA socket失败!----------------------\n");
//      info->state = CommKitState_CLEANUP;
//      return false;
//    }
//    else{
//      LOG_printf(DEBUG,"----------------获取RDMA socket成功!----------------------\n");
//    }

// // 5. 根据IP尝试建立连接
//    struct in_addr destAddr;
//    const char* RemoteRdmaIP = "192.168.0.203";   //存储服务器dell01的RDMA IP,网口名 enp152s0np0

//    if (inet_pton(AF_INET, RemoteRdmaIP, &destAddr) <= 0) {
//     perror("Invalid remote IP address");
//     return -1;
//    }
//    LOG_printf(DEBUG,"查看远端主机IP情况 RemoteIP %s and tranfer to %u\n", RemoteRdmaIP, destAddr.s_addr);

//    printCurrentTime();
//    unsigned short port = 8003;    //存储服务器的port
//    LOG_printf(DEBUG,"----------------开始根据IP与远端建立RDMA连接-----------------------\n");
//    bool connectRes = info->socket->ops->connectByIP(info->socket, destAddr, port);
//    printCurrentTime();
//    if(connectRes){
//       LOG_printf(DEBUG,"尝试通过IP连接 连接成功 \n");
//    }
//    else
//    {
//       LOG_printf(DEBUG,"尝试通过IP连接 连接失败 \n");
//    }
//    printCurrentTime();

//    LOG_printf(DEBUG,"----------------开始通过RDMA向远端发送数据-----------------------\n");
//    printCurrentTime();
//    // 定义 iovec 数组
//    char buffer1[] = "HELLOWA, ";
//    char buffer2[] = "dell01!WA";
//    struct iovec iov[2];
//    iov[0].iov_base = buffer1;
//    iov[0].iov_len = strlen(buffer1);
//    iov[1].iov_base = buffer2;
//    iov[1].iov_len = strlen(buffer2);
//    // 初始化 iovec_iter
//    iovec_iter iter;
//    iovec_iter_init(&iter, iov, 2);

//    int sendRes = info->socket->ops->sendto(info->socket, &iter, MSG_DONTWAIT, NULL);
//    LOG_printf(DEBUG," return %d \n", sendRes);
//    if( sendRes < 0 ){
//       LOG_printf(DEBUG,"尝试发送数据 发送失败\n");
//    }
//    else{
//       LOG_printf(DEBUG,"尝试发送数据 发送成功\n");
//    }
//    printCurrentTime();

//    // printCurrentTime();
//    // LOG_printf(DEBUG,"----------------开始验证接收数据-----------------------\n");




// // 6. 准备头部信息
//    // info->headerSize = context->ops->prepareHeader(context, info);
//    // if(info->headerSize == 0)
//    // {
//    //    LOG_printf(DEBUG,"准备头部信息失败!\n");
//    //    info->state = CommKitState_CLEANUP;
//    //    goto cleanup;
//    // }
//    // else{
//    //    LOG_printf(DEBUG,"准备头部信息成功!\n");
//    // }

//    context->numAcquiredConns++;

//    info->state = CommKitState_SENDHEADER;

//    return true;

// cleanup:
//    info->state = CommKitState_CLEANUP;
//    return false;

// error:
//    free(info->headerBuffer);
//    info->headerBuffer = (char*)NULL;
//    return false;
// }
#endif
