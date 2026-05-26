#ifndef OPENTK_IBVSOCKET_H_
#define OPENTK_IBVSOCKET_H_

#ifdef __cplusplus
// 仅 C++ 编译器可见的头文件
#include <atomic>       // C++ 原子操作库
// #include <cstddef>      // C++ 标准库（可选，如果不需要可删除）
// #include <cstdlib>     // For malloc and free
// #include <cstring> // C++ 标准库中的内存操作函数
// #include <atomic>

#endif

// #include <common/Common.h>
// #include <common/toolkit/Random.h>
// #include <linux/in.h>
// #include <linux/inet.h>
// #include <linux/sched.h>
// #include <linux/types.h>
// #include <linux/wait.h>
// #include <net/sock.h>
// #include <net/inet_common.h>
// #include <asm/atomic.h>
// #include <os/iov_iter.h>
#include <sys/types.h>
// #include <atomic>
#include <pthread.h>
#include <stdint.h>
#include <infiniband/verbs.h>
#include <rdma/rdma_cma.h>
#include <netinet/in.h>
// #include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef __must_check
#define __must_check __attribute__((warn_unused_result))
#endif

#define IBVSOCKET_PRIVATEDATA_STR            "fhgfs0 " // must be exactly(!!) 8 bytes long
#define IBVSOCKET_PRIVATEDATA_STR_LEN        8
#define IBVSOCKET_PRIVATEDATA_PROTOCOL_VER   1



struct ib_device;
struct ib_mr;

struct IBVIncompleteRecv;
typedef struct IBVIncompleteRecv IBVIncompleteRecv;
struct IBVIncompleteSend;
typedef struct IBVIncompleteSend IBVIncompleteSend;

struct IBVCommContext;
typedef struct IBVCommContext IBVCommContext;

struct IBVCommDest;
typedef struct IBVCommDest IBVCommDest;

struct IBVTimeoutConfig;
typedef struct IBVTimeoutConfig IBVTimeoutConfig;

struct IBVSocket; // forward declaration
typedef struct IBVSocket IBVSocket;

struct IBVCommConfig;
typedef struct IBVCommConfig IBVCommConfig;

struct NicAddressStats;
typedef struct NicAddressStats NicAddressStats;

enum IBVSocketKeyType
{
   IBVSOCKETKEYTYPE_UnsafeGlobal = 0,
   IBVSOCKETKEYTYPE_UnsafeDMA,
   IBVSOCKETKEYTYPE_Register
};
typedef enum IBVSocketKeyType IBVSocketKeyType;

// construction/destruction
extern __must_check bool IBVSocket_init(IBVSocket* _this, struct in_addr srcIpAddr, NicAddressStats* nicStats);
extern void IBVSocket_uninit(IBVSocket* _this);

// static
extern bool IBVSocket_rdmaDevicesExist(void);

// methods
extern bool IBVSocket_connectByIP(IBVSocket* _this, struct in_addr ipaddress,
   unsigned short port, IBVCommConfig* commCfg);
extern bool IBVSocket_bindToAddr(IBVSocket* _this, struct in_addr ipAddr,
   unsigned short port);
extern bool IBVSocket_listen(IBVSocket* _this);
extern bool IBVSocket_shutdown(IBVSocket* _this);

extern ssize_t IBVSocket_recvT(IBVSocket* _this, struct iovec_iter* iter, int flags,
   int timeoutMS);
extern ssize_t IBVSocket_send(IBVSocket* _this, struct iovec_iter* iter, int flags);

extern int IBVSocket_checkConnection(IBVSocket* _this);

extern unsigned long IBVSocket_poll(IBVSocket* _this, short events, bool finishPoll);

// getters & setters
extern void IBVSocket_setTimeouts(IBVSocket* _this, int connectMS,
   int completionMS, int flowSendMS, int flowRecvMS, int pollMS);
extern void IBVSocket_setTypeOfService(IBVSocket* _this, int typeOfService);
extern void IBVSocket_setConnectionFailureStatus(IBVSocket* _this, unsigned value);
extern struct in_addr IBVSocket_getSrcIpAddr(IBVSocket* _this);

// Only access members of NicAddressStats when the owner NodeConnPool mutex is held.
// OK to access "nic" without holding mutex.
extern NicAddressStats* IBVSocket_getNicStats(IBVSocket* _this);

extern unsigned IBVSocket_getRkey(struct IBVSocket* sock);
// extern struct ib_device* IBVSocket_getDevice(struct IBVSocket* sock);
extern struct ibv_device* IBVSocket_getDevice(struct IBVSocket* sock);

extern int IBVSocket_registerMr(IBVSocket* _this, struct ib_mr* mr, int access);



struct IBVTimeoutConfig
{
   int connectMS;
   int completionMS;
   int flowSendMS;
   int flowRecvMS;
   int pollMS;
};

struct IBVCommConfig
{
   unsigned bufNum; // number of available buffers
   unsigned bufSize; // total size of each buffer
   /**
    * IBVBuffer can allocate the buffer in multiple memory regions. This
    * is to allow allocation of large buffers without requiring the
    * buffer to be entirely contiguous. A value of 0 means that the
    * buffer should not be fragmented.
    */
   unsigned fragmentSize; // size of buffer fragments
   IBVSocketKeyType keyType; // Which type of rkey for RDMA
};

// #ifdef BEEGFS_RDMA


// #include <rdma/ib_verbs.h>
// #include <rdma/rdma_cm.h>
// #include <rdma/ib_cm.h>
// #include <common/threading/Mutex.h>
// #include "IBVBuffer.h"

#ifndef IBVSOCKET_H
#define IBVSOCKET_H
enum IBVSocketConnState
{
   IBVSOCKETCONNSTATE_UNCONNECTED=0,
   IBVSOCKETCONNSTATE_CONNECTING=1,
   IBVSOCKETCONNSTATE_ADDRESSRESOLVED=2,
   IBVSOCKETCONNSTATE_ROUTERESOLVED=3,
   IBVSOCKETCONNSTATE_ESTABLISHED=4,
   IBVSOCKETCONNSTATE_FAILED=5,
   IBVSOCKETCONNSTATE_REJECTED_STALE=6
};
typedef enum IBVSocketConnState IBVSocketConnState_t;
#endif

extern bool __IBVSocket_createNewID(IBVSocket* _this);
extern bool __IBVSocket_createCommContext(IBVSocket* _this, struct rdma_cm_id* cm_id,
   IBVCommConfig* commCfg, IBVCommContext** outCommContext);
extern void __IBVSocket_cleanupCommContext(struct rdma_cm_id* cm_id, IBVCommContext* commContext);

extern bool __IBVSocket_initCommDest(IBVCommContext* commContext, IBVCommDest* outDest);
extern bool __IBVSocket_parseCommDest(const void* buf, size_t bufLen, IBVCommDest** outDest);

extern int __IBVSocket_receiveCheck(IBVSocket* _this, int timeoutMS);
extern int __IBVSocket_nonblockingSendCheck(IBVSocket* _this);

extern int __IBVSocket_postRecv(IBVSocket* _this, IBVCommContext* commContext, size_t bufIndex);
extern int __IBVSocket_postSend(IBVSocket* _this, size_t bufIndex);
extern int __IBVSocket_recvWC(IBVSocket* _this, int timeoutMS, struct ib_wc* outWC);

extern int __IBVSocket_flowControlOnRecv(IBVSocket* _this, int timeoutMS);
extern void __IBVSocket_flowControlOnSendUpdateCounters(IBVSocket* _this);
extern int __IBVSocket_flowControlOnSendWait(IBVSocket* _this, int timeoutMS);

extern int __IBVSocket_waitForRecvCompletionEvent(IBVSocket* _this, int timeoutMS,
   struct ib_wc* outWC);
extern int __IBVSocket_waitForSendCompletionEvent(IBVSocket* _this, int oldSendCount,
   int timeoutMS);
extern int __IBVSocket_waitForTotalSendCompletion(IBVSocket* _this,
   unsigned* numSendElements, unsigned* numWriteElements, unsigned* numReadElements, int timeoutMS);

extern ssize_t __IBVSocket_recvContinueIncomplete(IBVSocket* _this, struct iov_iter* iter);

extern int __IBVSocket_cmaHandler(struct rdma_cm_id* cm_id, struct rdma_cm_event* event);
extern void __IBVSocket_cqSendEventHandler(struct ib_event* event, void* data);
extern void __IBVSocket_sendCompletionHandler(struct ib_cq* cq, void* cq_context);
extern void __IBVSocket_cqRecvEventHandler(struct ib_event* event, void* data);
extern void __IBVSocket_recvCompletionHandler(struct ib_cq* cq, void* cq_context);
extern void __IBVSocket_qpEventHandler(struct ib_event* event, void* data);
extern int __IBVSocket_routeResolvedHandler(IBVSocket* _this, struct rdma_cm_id* cm_id,
   IBVCommConfig* commCfg, IBVCommContext** outCommContext);
extern int __IBVSocket_connectedHandler(IBVSocket* _this, struct rdma_cm_event *event);

extern struct ib_cq* __IBVSocket_createCompletionQueue(struct ib_device* device,
            ibv_comp_channel comp_handler, void (*event_handler)(struct ib_event *, void *),
            void* cq_context, int cqe);

extern const char* __IBVSocket_wcStatusStr(int wcStatusCode);



struct IBVIncompleteRecv
{
   int                  isAvailable;
   int                  completedOffset;
   int bufIndex;
   int totalSize;
};

struct IBVIncompleteSend
{
   unsigned             numAvailable;
   bool           forceWaitForAll; // true if we received only some completions and need
                                         //    to wait for the rest before we can send more data
};

enum dma_data_direction {
    DMA_BIDIRECTIONAL = 0,
    DMA_TO_DEVICE = 1,
    DMA_FROM_DEVICE = 2,
    DMA_NONE = 3,
};
typedef enum dma_data_direction dma_data_direction;

typedef struct IBVBuffer
{
   char** buffers;   // 指向每个缓冲区的指针数组
   struct ibv_sge* lists;   
   struct ibv_mr* mr;     // 内存区域

   size_t bufferSize;
   unsigned bufferCount;
   size_t* datasize;//NOTE
   unsigned listLength;
   enum dma_data_direction dma_dir;
}IBVBuffer;

struct IBVCommContext
{
   struct ibv_pd*             pd; // protection domain//$$%
   // struct ib_pd*             pd; // protection domain
   struct ibv_mr*             dmaMR; // system DMA MR. Not supported on all platforms.
   std::atomic<int>                 recvCompEventCount; // incremented on incoming event notification
   // wait_queue_head_t：用于管理等待队列的头部，通常用于线程等待事件并唤醒。
   // wait_queue_head_t         recvCompWaitQ; // for recvCompEvents
   // wait_queue_t：等待队列中的条目，表示一个等待的线程。
   // wait_queue_t              recvWait;
   pthread_cond_t            recvCompCond; // 接收完成条件变量
   pthread_mutex_t           recvCompMutex; // 接收完成互斥锁
   bool                recvWaitInitialized; // true if init_wait was called for the thread

   std::atomic<int>                  sendCompEventCount; // incremented on incoming event notification
   // 用于实现内核中的等待队列，线程可以在等待队列上休眠，直到某个事件发生。
   // wait_queue_head_t         sendCompWaitQ; // for sendCompEvents
   // wait_queue_t              sendWait;
   pthread_cond_t            sendCompCond; // 发送完成条件变量
   pthread_mutex_t           sendCompMutex; // 发送完成互斥锁
   bool                sendWaitInitialized; // true if init_wait was called for the thread

   struct ibv_cq*             recvCQ; // recv completion queue
   struct ibv_cq*             sendCQ; // send completion queue
   struct ibv_qp*             qp; // send+recv queue pair

   IBVCommConfig             commCfg;
   struct IBVBuffer*         sendBufs;
   struct IBVBuffer*         recvBufs;
   struct IBVBuffer          checkConBuffer;
   unsigned                  numReceivedBufsLeft; // flow control v2 to avoid IB rnr timeout
   unsigned                  numSendBufsLeft; // flow control v2 to avoid IB rnr timeout

   IBVIncompleteRecv         incompleteRecv;
   IBVIncompleteSend         incompleteSend;
   uint32_t                       checkConnRkey;
};

#pragma pack(push, 1)
// Note: Make sure this struct has the same size on all architectures (because we use
//    sizeof(IBVCommDest) for private_data during handshake)
struct IBVCommDest
{
   char                 verificationStr[IBVSOCKET_PRIVATEDATA_STR_LEN];
   uint64_t             protocolVersion;
   uint64_t             vaddr;
   unsigned             rkey;
   unsigned             recvBufNum;
   unsigned             recvBufSize;
};
#pragma pack(pop)


struct IBVSocket
{
   // wait_queue_head_t             eventWaitQ; // used to wait for connState change during connect
   pthread_cond_t                eventCond;     // 条件变量，用于等待 connState 的变化
   pthread_mutex_t               eventMutex;    // 互斥锁，保护 connState 和条件变量
   struct rdma_event_channel*    event_channel;
   IBVCommConfig* commCfg; // 添加此成员，用于保存通信配置

   struct rdma_cm_id*            cm_id;
   struct in_addr                srcIpAddr;

   IBVCommDest                   localDest;
   IBVCommDest*                  remoteDest;

   IBVCommContext*               commContext;

   int                           errState; // 0 = <no error>; -1 = <unspecified error>

   volatile IBVSocketConnState_t connState;

   int                           typeOfService;
   unsigned                      remapConnectionFailureStatus;
   NicAddressStats*              nicStats;  // Owned by a NodeConnPool instance. Do not access
                                            // members without locking the NodeConnPool mutex.
                                            // Possibly NULL.
   IBVTimeoutConfig              timeoutCfg;
   // Mutex：内核中的互斥锁，用于保护共享数据结构，实现互斥访问
   // Mutex                              cmaMutex;
   pthread_mutex_t                         cmaMutex;  // used to manage concurrency of cm_id and commContext
                                            // with __IBVSocket_cmaHandler
};


unsigned IBVSocket_getRkey(struct IBVSocket *sock)
{
   return sock->commContext->checkConnRkey;
}

// struct ib_device* IBVSocket_getDevice(struct IBVSocket* sock)
struct ibv_device* IBVSocket_getDevice(struct IBVSocket* sock)

{
   // return sock->commContext->pd->device;
   return sock->commContext->pd->context->device;

}

// struct IBVSocket
// {
//    /* empty structs are not allowed, so until this kludge can go, add a dummy member */
//    unsigned:0;
// };


// #endif

#endif /*OPENTK_IBVSOCKET_H_*/




#ifndef IBVBuffer_h_aMQFNfzrjbEHDOcv216fi
#define IBVBuffer_h_aMQFNfzrjbEHDOcv216fi

// #include <common/Common.h>
// #ifdef BEEGFS_RDMA

// #include <rdma/ib_verbs.h>
// #include <rdma/rdma_cm.h>
// #include <rdma/ib_cm.h>

// #include <os/iov_iter.h>


struct IBVBuffer;
typedef struct IBVBuffer IBVBuffer;

struct IBVCommContext;
struct IBVSocket;


extern bool IBVBuffer_init(IBVBuffer* buffer, struct IBVCommContext* ctx, size_t bufLen,
   size_t fragmentLen, enum dma_data_direction dma_dir);
/**
 * Prepare the instance to use its internal ib_mr. This is only needed for buffers used
 * with RDMA READ/WRITE and when not using a global rkey. This may be called before
 * the connection is established. Once the connection has been established,
 * the registration must be completed via a call to IBVSocket_registerMr().
 */
extern bool IBVBuffer_initRegistration(IBVBuffer* buffer, struct IBVCommContext* ctx);
extern void IBVBuffer_free(IBVBuffer* buffer, struct IBVCommContext* ctx);
extern ssize_t IBVBuffer_fill(IBVBuffer* buffer, struct iov_iter* iter);


// #endif

#endif

