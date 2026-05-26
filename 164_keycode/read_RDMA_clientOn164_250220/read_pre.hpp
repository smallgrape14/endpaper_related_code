#include <asm-generic/socket.h>
#include <infiniband/verbs.h>
#include <rdma/rdma_cma.h>
#include <pthread.h>
#include <stddef.h> // 包含 offsetof 的头文件
#include <cstddef> // 包含 offsetof 的头文件
#include <stdio.h>   // 用于 printf，如果需要的话
#include <sys/uio.h> // 包含iovec的定义
#include <atomic>
#include <unistd.h>
#include "RDMAConnection.hpp"

// struct ReadLocalFileRDMAMsg;
// typedef struct ReadLocalFileRDMAMsg ReadLocalFileRDMAMsg;

// static inline void ReadLocalFileRDMAMsg_init(ReadLocalFileRDMAMsg* this);
// static inline void ReadLocalFileRDMAMsg_initFromSession(ReadLocalFileRDMAMsg* this,
//    NumNodeID clientNumID, const char* fileHandleID, uint16_t targetID, PathInfo* pathInfoPtr,
//    unsigned accessFlags, int64_t offset, int64_t count, RdmaInfo *rdmap);

// virtual functions
extern void ReadLocalFileRDMAMsg_serializePayload(NetMessage* msg, SerializeCtx* ctx);
struct user_scatterlist {
    uint64_t address;   // 64位地址
    uint64_t length;    // 长度
    uint64_t offset;    // 偏移
};
typedef uint64_t dma_addr_t;
struct scatterlist {
	unsigned long	page_link;  // 指示物理页
	unsigned int	offset;   // 页中偏移
	unsigned int	length;    // 数据长度
	dma_addr_t	dma_address;  // dma地址
#ifdef CONFIG_NEED_SG_DMA_LENGTH
	unsigned int	dma_length;
#endif
};

// struct RdmaInfo
// {
//    size_t              sg_count;
//    size_t              dma_count;
//    uint32_t            tag;
//    uint64_t            key;
//    // struct ib_device   *device;
//    struct ibv_device   *device;


//    void               *pages;
//    struct user_scatterlist *sglist;
//    struct user_scatterlist *dmalist;
// };
struct RdmaInfo
{
   size_t              sg_count;
   size_t              dma_count;
   uint32_t            tag;
   uint64_t            key;
   struct ibv_device   *device;
   void               *pages;
   // struct user_scatterlist *sglist;
   // struct user_scatterlist *dmalist;
   struct scatterlist *sglist;
   struct scatterlist *dmalist;
};
typedef struct RdmaInfo RdmaInfo;
typedef struct RdmaInfo RdmaInfo;
         // static RdmaInfo * RdmaInfo_map(const struct iov_iter *iter, Socket *socket,enum dma_data_direction dma_dir);
static RdmaInfo * RdmaInfo_map(const struct iovec *iter, Socket *socket,enum dma_data_direction dma_dir);

// RdmaInfo* RdmaInfo_mapRead(const struct iov_iter *iter, Socket *socket)
RdmaInfo* RdmaInfo_mapRead(const struct iovec *iter, Socket *socket)

{
   return RdmaInfo_map(iter, socket, DMA_FROM_DEVICE);
}

static inline NicAddrType_t Socket_getSockType(Socket* sock)
{
   return sock->sockType;
}
bool RDMASocket_isRkeyGlobal(RDMASocket* sock)
{
   std::cout<<"sock->commCfg.keyType = "<<sock->commCfg.keyType<<"\n";
   return sock->commCfg.keyType != IBVSOCKETKEYTYPE_Register;
}
// 定义错误码的最大值
#define MAX_ERRNO 4095

// 模拟内核中的 ERR_PTR 宏
void *ERR_PTR(long error) {
    return (void *) error;
}

// 模拟内核中的 PTR_ERR 宏
long PTR_ERR(const void *ptr) {
    return (long) ptr;
}

// 模拟内核中的 IS_ERR 宏
int IS_ERR(const void *ptr) {
    return (unsigned long)ptr >= (unsigned long)-MAX_ERRNO;
}
// struct ib_device* RDMASocket_getDevice(RDMASocket *sock)
struct ibv_device* RDMASocket_getDevice(RDMASocket *sock)
{
   return IBVSocket_getDevice(&sock->ibvsock);
}

unsigned RDMASocket_getRkey(RDMASocket *sock)
{
   return IBVSocket_getRkey(&sock->ibvsock);
}


size_t user_iov_iter_npages(const struct iovec *iov, size_t nr_segs) {
    size_t total_length = 0;
    size_t pages = 0;
    size_t page_size = sysconf(_SC_PAGESIZE);

    for (size_t i = 0; i < nr_segs; ++i) {
        total_length += iov[i].iov_len;
    }

    pages = (total_length + page_size - 1) / page_size;

    return pages;
}

// static int RdmaInfo_iovToSglist(const struct iovec *iov, size_t iovcnt, user_scatterlist *sglist, int max_sg) {
static int RdmaInfo_iovToSglist(const struct iovec *iov, size_t iovcnt, user_scatterlist *sglist) {

    size_t sg_idx = 0;
    size_t total_length = 0;

    // Calculate the total length of all iov entries
    for (size_t i = 0; i < iovcnt; i++) {
        total_length += iov[i].iov_len;
    }

    // Initialize sglist with the iov data
   //  for (size_t i = 0; i < iovcnt && sg_idx < max_sg; i++) {
    for (size_t i = 0; i < iovcnt; i++) {
        user_scatterlist *sg = &sglist[sg_idx++];
        sg->address = (uint64_t)(uintptr_t)iov[i].iov_base;
        sg->length = iov[i].iov_len;
        sg->offset = 0; // In user space, we don't have a concept of page offset like in kernel space
    }

    // If there are more iov entries than the maximum number of scatter/gather entries,
    // we need to return an error or handle it accordingly.

   //  if (sg_idx >= max_sg) {
   //      fprintf(stderr, "Error: Not enough space in sglist for all iov entries.\n");
   //      return -1;
   //  }

    return sg_idx; // Return the number of scatter/gather entries set up
}

// 模拟DMA映射函数
// int nvfs_dma_map_sg_attrs(void *dma_device, user_scatterlist *sglist, int sg_count,int dma_dir, int attrs) {
int nvfs_dma_map_sg_attrs( user_scatterlist *sglist, int sg_count) 
{

    // 模拟检查属性
   //  if (attrs & DMA_ATTR_NO_WARN) {
   //      // 没有警告
   //      std::cout<<"// 没有警告 \n";
   //  }

    // 模拟遍历scatter-gather列表
    uint64_t total_length = 0;
    for (int i = 0; i < sg_count; i++) {
        total_length += sglist[i].length;
    }

    printf("Total length to be mapped: %lu\n", total_length);

    // 在用户态，我们不能实际进行DMA映射，所以我们只是模拟这个过程
    // 这里可以添加代码来处理数据传输，例如通过mmap和read/write系统调用

    return total_length; // 返回映射的总长度
}

#define NVFS_IO_ERR                    -1
#define NVFS_CPU_REQ                   -2
#define RDMA_MAX_DMA_COUNT   64
// 获取下一个 user_scatterlist 条目的宏
#define sg_next(sg) ((sg) + 1)

// 将 user_scatterlist 条目合并到一个新的列表中
int RdmaInfo_coalesceSglist(user_scatterlist *sglist, user_scatterlist *dmalist, int count) {
    user_scatterlist *sgp = sglist;
    user_scatterlist *dmap = dmalist;
    uint64_t dma_ba = sgp->address;
    uint64_t dma_la = sgp->length;

    // 处理第一个条目
    *dmap++ = *sgp++;

    // 遍历剩余的条目
    for (int i = 1; i < count; i++, sgp++) {
        uint64_t sg_ba = sgp->address;
        uint64_t sg_la = sgp->length;

        // 检查当前条目是否与前一个条目连续
        if (dma_ba + dma_la == sg_ba) {
            // 合并条目
            dma_la += sg_la;
        } else {
            // 不连续，保存前一个条目的信息并开始新的条目
            dmap->address = sg_ba;
            dmap->length = sg_la;
            dmap++;
        }

        // 更新合并的物理地址和长度
        dma_ba = sg_ba;
        dma_la = dma_la;
    }

    // 返回合并后的条目数量
    return dmap - dmalist;
}


/*
 * RdmaInfo_map - Map GPU buffers for RDMA operations.
 * @iter: iov_iter
 * @socket: RDMA capable socket struct.
 * @dma_dir: read (DMA_FROM_DEVICE) or write (DMA_TO_DEVICE)
 * @returns RdmaInfo struct
 */
static RdmaInfo * RdmaInfo_map(const struct iovec *iter, Socket *socket,enum dma_data_direction dma_dir)
{
   //@@12_9
      // iovec_iter* iter;
      // iter = (iovec_iter*)malloc(sizeof(iovec_iter));
      // iovec_iter_init(iter, &vecState->data, 1);

      size_t totalSize =iter->iov_len;
      char* totalbuffer = (char*)malloc(totalSize);

      // 假设你实现了类似 iov_iter_copy_to_buffer 的函数来将分散的数据拷贝到连续缓冲
      // iov_iter_copy_to_buffer(&vecState->data, buffer, totalSize);
      // @@@ 这里只传输一个所以不需要这个, 后面需要完善如果有多个的情况！！！！
      cout << "data = " << iter->iov_base << "\n";
      printf("state->data = %s\n", (char*)iter->iov_base);
      memcpy(totalbuffer, iter->iov_base, totalSize);
      cout << "buffer =";
      for(int i=0;i<totalSize;i++){
         cout << totalbuffer[i];
      }
      cout << "\n";
      printf("buffer printf = %s\n", totalbuffer);

      // cout << "data = " << iter->iov_base << "\n";
      // memcpy(totalbuffer, iter->iov_base, totalSize);
      // cout << "buffer =";
      // for(int i=0;i<totalSize;i++){
      //    cout << totalbuffer[i];
      // }
      // cout << "\n";

      RDMASocket* rs = (RDMASocket*)socket;
      struct ibv_mr *mr = ibv_reg_mr(rs->ibvsock.commContext->pd, totalbuffer, totalSize, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE);
      if (!mr) {
         fprintf(stderr, "Failed to register memory region\n");
         // 错误处理
         ibv_dereg_mr(mr);
         free(totalbuffer);
         return nullptr;
      }
      rdma_recv_buffer=totalbuffer;
      rdma_recv_len=totalSize;

      RdmaInfo *rdmap0 = (RdmaInfo*)malloc(sizeof(RdmaInfo));
      memset(rdmap0, 0, sizeof(RdmaInfo));
      rdmap0->sg_count = 1;
      rdmap0->dma_count = 1;  // 这里假设只有一块连续内存
      rdmap0->tag = 0;
      rdmap0->key = mr->rkey; // or mr->lkey，根据你的使用场景
      std::cout<<"mr->rkey ="<<mr->rkey<<"\n";
      rdmap0->device = rs->ibvsock.commContext->pd->context->device;
      if(!rdmap0->device){
         cout << "device = NULL\n";
      }
      rdmap0->pages = NULL;
      // 保存内存起始地址和长度信息，以便后续RDMA读写时使用
      rdmap0->sglist = (struct scatterlist*)malloc(rdmap0->sg_count * sizeof(struct scatterlist));
      rdmap0->dmalist = (struct scatterlist*)malloc(rdmap0->dma_count * sizeof(struct scatterlist));
      if(!rdmap0->sglist || !rdmap0->dmalist)
      {
         fprintf(stderr, "failed to allocate sglist or dmalist\n");
         free(totalbuffer);
         free(rdmap0->sglist);
         free(rdmap0->dmalist);
         free(rdmap0);
         return nullptr;
      }
      rdmap0->dmalist[0].dma_address = (dma_addr_t)(uintptr_t)totalbuffer;
      rdmap0->dmalist[0].length = totalSize;
      rdmap0->dmalist[0].offset = 0;
      rdmap0->dmalist[0].page_link = 0;

      // sglist也同理，如果需要也可填充同样的信息或为空
      rdmap0->sglist[0].dma_address = (dma_addr_t)(uintptr_t)totalbuffer;
      rdmap0->sglist[0].length = totalSize;
      rdmap0->sglist[0].offset = 0;
      rdmap0->sglist[0].page_link = 0;

      return rdmap0;
}
/*
// static RdmaInfo * RdmaInfo_map(const struct iov_iter *iter, Socket *socket,enum dma_data_direction dma_dir)
static RdmaInfo * RdmaInfo_map(const struct iovec *iter, Socket *socket,enum dma_data_direction dma_dir)
{
   // RdmaInfo           *rdmap = new RdmaInfo();
   // return rdmap;
   //
   // Fill in the rdma info.
   //
   // rdmap->dma_count = dma_count;
   // rdmap->sg_count = sg_count;
   // rdmap->tag = 0x00000000;
   // rdmap->device = device;
   // rdmap->key = key;
   // rdmap->sglist = sglist;
   // rdmap->dmalist = dmalist;

   RdmaInfo           *rdmap;
   RDMASocket         *rs;
   // struct ib_device   *device;
   struct ibv_device   *device;


   // struct scatterlist *sglist;
   // struct scatterlist *dmalist;
   struct user_scatterlist *sglist;
   struct user_scatterlist *dmalist;
   int         status = 0;
   int         sg_count;
   int         dma_count;
   int         count;
   unsigned    npages;
   unsigned    key;
   std::cout<<"Socket_getSockType(socket) = "<<Socket_getSockType(socket)<<"\n";
   if (Socket_getSockType(socket) != NICADDRTYPE_RDMA)
      return (RdmaInfo*)(ERR_PTR(-EINVAL));

   rs = (RDMASocket*) socket;
   if (!RDMASocket_isRkeyGlobal(rs))
   {
      // printk_fhgfs(KERN_ERR, "ERROR: rkey type is not compatible with GDS\n");
      printf("ERROR: rkey type is not compatible with GDS\n");

      // return (RdmaInfo*)(ERR_PTR(-EINVAL));//$$%
   }

   // npages = 1 + iov_iter_npages(iter, INT_MAX);
   int iovcnt=1;
   npages = 1 + user_iov_iter_npages(iter, iovcnt);
   std::cout<<"npages = "<<npages<<"\n";

   //
   // Allocate the scatterlist.
   //
   rdmap = (RdmaInfo*)malloc(sizeof(RdmaInfo));      //kzalloc(sizeof(RdmaInfo), GFP_ATOMIC);
   sglist =(user_scatterlist*)malloc(npages * sizeof(user_scatterlist));      // kzalloc(npages * sizeof(struct scatterlist), GFP_ATOMIC);
   dmalist =(user_scatterlist*)malloc(npages * sizeof(user_scatterlist));     // kzalloc(npages * sizeof(struct scatterlist), GFP_ATOMIC);
   if (!rdmap || !sglist || !dmalist)
   {
      // printk_fhgfs(KERN_ERR, "%s: no memory for scatterlist\n", __func__);
      printf("%s: no memory for scatterlist\n", __func__);
      free(rdmap);
        free(sglist);
        free(dmalist);
      status = -ENOMEM;
      goto error_return;
   }

   //
   // Populate the scatterlist from the iov_iter.
   //
   sg_count = RdmaInfo_iovToSglist(iter,iovcnt,sglist);
   std::cout<<"sg_count = "<<sg_count<<"\n";
   if (sg_count < 0)
   {
      // printk_fhgfs(KERN_ERR, "%s: can't convert iov_iter to scatterlist\n", __func__);
      printf("%s: can't convert iov_iter to scatterlist\n", __func__);

      status = -EIO;
      goto error_return;
   }

   //
   // DMA map all of the pages.
   //
   device = RDMASocket_getDevice(rs);
   key = RDMASocket_getRkey(rs);

   // count = nvfs_ops->nvfs_dma_map_sg_attrs(device->dma_device, sglist, sg_count,
   //    dma_dir, DMA_ATTR_NO_WARN);
   // count = nvfs_dma_map_sg_attrs(device->dma_device, sglist, sg_count,dma_dir, DMA_ATTR_NO_WARN);
   // count = nvfs_dma_map_sg_attrs(sglist, sg_count);//$$%
   count = sg_count;//nvfs_dma_map_sg_attrs(sglist, sg_count);//$$%

   std::cout<<"count = "<<count <<"\t sg_count = "<<sg_count<<"\n";
   if (count != sg_count)
   {
      std::cout<<"count != sg_count \n";
      if (count == NVFS_CPU_REQ)
      {
         // printk_fhgfs(KERN_ERR, "%s: NVFS_CPU_REQ\n", __func__);
         printf("%s: NVFS_CPU_REQ\n", __func__);

         status = 0;
      }
      else if (count == NVFS_IO_ERR)
      {
         printf("%s: can't DMA map mixed CPU/GPU pages\n", __func__);
         status = -EINVAL;
      }
      else
      {
         printf("%s: unknown error returned from NVFS (%d)\n", __func__, count);
         status = -EIO;
      }
      goto error_return;
   }

   //
   // Coalesce the scatterlist.
   //
   dma_count = RdmaInfo_coalesceSglist(sglist, dmalist, count);
   std::cout<<"dma_count = "<<dma_count<<"\n";
   if (dma_count > RDMA_MAX_DMA_COUNT)//>64
   {
      // printk_fhgfs(KERN_ERR, "%s: too many DMA elements count=%d max=%d\n", __func__,
      //    dma_count, RDMA_MAX_DMA_COUNT);
      printf("%s: too many DMA elements count=%d max=%d\n", __func__,
         dma_count, RDMA_MAX_DMA_COUNT);
      status = -EIO;
      goto error_return;
   }

   //
   // Fill in the rdma info.
   //
   rdmap->dma_count = dma_count;
   rdmap->sg_count = sg_count;
   rdmap->tag = 0x00000000;
   rdmap->device = device;
   rdmap->key = key;
   rdmap->sglist = sglist;
   rdmap->dmalist = dmalist;

// #ifdef BEEGFS_DEBUG_RDMA
//    RdmaInfo_dumpIovIter(iter);
//    RdmaInfo_dumpSgtable("MAP", rdmap->dmalist, rdmap->dma_count);
//    RdmaInfo_dumpRdmaInfo(rdmap);
// #endif // BEEGFS_DEBUG_RDMA 

   return rdmap;

error_return:
                  // if (sglist)
                  // {
                  //    RdmaInfo_putPages(sglist, sg_count);
                  //    kfree(sglist);
                  // }
                  // if (dmalist)
                  //    kfree(dmalist);
                  // if (rdmap)
                  //    kfree(rdmap);
    if (sglist) {
        free(sglist);
    }
    if (dmalist) {
        free(dmalist);
    }
    if (rdmap) {
        free(rdmap);
    }
   return (RdmaInfo*)((status == 0) ? NULL : ERR_PTR(status));
   
}

*/
#define NETMSGTYPE_ReadLocalFileRDMA               3039
#define NETMSGTYPE_ReadLocalFileRDMAResp           3040

struct ReadLocalFileRDMAMsg
{
   NetMessage netMessage;

   int64_t offset;
   int64_t count;
   unsigned accessFlags;
   NumNodeID clientNumID;
   const char* fileHandleID;
   unsigned fileHandleIDLen;
   PathInfo* pathInfoPtr;
   uint16_t targetID;
   RdmaInfo *rdmap;
};

extern const struct NetMessageOps ReadLocalFileRDMAMsg_Ops;

void ReadLocalFileRDMAMsg_init(ReadLocalFileRDMAMsg* msg)
{
   NetMessage_init(&msg->netMessage, NETMSGTYPE_ReadLocalFileRDMA, &ReadLocalFileRDMAMsg_Ops);
}

/**
 * @param sessionID just a reference, so do not free it as long as you use this object!
 */
void ReadLocalFileRDMAMsg_initFromSession(ReadLocalFileRDMAMsg* msg,
   NumNodeID clientNumID, const char* fileHandleID, uint16_t targetID, PathInfo* pathInfoPtr,
   unsigned accessFlags, int64_t offset, int64_t count, RdmaInfo *rdmap)
{
   ReadLocalFileRDMAMsg_init(msg);

   msg->clientNumID = clientNumID;

   msg->fileHandleID = fileHandleID;
   msg->fileHandleIDLen = strlen(fileHandleID);

   msg->targetID = targetID;

   msg->pathInfoPtr = pathInfoPtr;

   msg->accessFlags = accessFlags;

   msg->offset = offset;
   msg->count = count;

   msg->rdmap = rdmap;
}

const struct NetMessageOps ReadLocalFileRDMAMsg_Ops = {
   .serializePayload   = ReadLocalFileRDMAMsg_serializePayload,
//    .deserializePayload = _NetMessage_deserializeDummy,
//    .processIncoming = NetMessage_processIncoming,
   .getSupportedHeaderFeatureFlagsMask = NetMessage_getSupportedHeaderFeatureFlagsMask,
};
// static inline void RdmaInfo_serialize(SerializeCtx* ctx, RdmaInfo *rdmap)
// {
//    int i = 0;
//    // struct user_scatterlist *sgp;


//    if (rdmap != NULL)
//    {
//       Serialization_serializeUInt64(ctx, rdmap->dma_count);

//       if (rdmap->dma_count > 0)
//       {
//          Serialization_serializeUInt(ctx, rdmap->tag);
//          Serialization_serializeUInt64(ctx, rdmap->key);
//          for (i = 0, sgp = rdmap->dmalist; i < rdmap->dma_count; i++, sgp++)
//          {
//             Serialization_serializeUInt64(ctx, (uint64_t)sgp->address);
//             Serialization_serializeUInt64(ctx, (uint64_t)sgp->length);
//             Serialization_serializeUInt64(ctx, (uint64_t)sgp->offset);
//          }
//       }
//    }
//    else
//    {
//       Serialization_serializeUInt64(ctx, 0ull);
//    }
// }
#define sg_dma_address(sg)      ((sg)->dma_address)
static inline void RdmaInfo_serialize(SerializeCtx* ctx, RdmaInfo *rdmap)
{
   int i = 0;
   struct scatterlist *sgp;


   if (rdmap != NULL)
   {
      Serialization_serializeUInt64(ctx, rdmap->dma_count);

      if (rdmap->dma_count > 0)
      {
         Serialization_serializeUInt(ctx, rdmap->tag);
         Serialization_serializeUInt64(ctx, rdmap->key);
         for (i = 0, sgp = rdmap->dmalist; i < rdmap->dma_count; i++, sgp++)
         {
            // Serialization_serializeUInt64(ctx, (uint64_t)sgp->address);
            Serialization_serializeUInt64(ctx, sg_dma_address(sgp));

            Serialization_serializeUInt64(ctx, (uint64_t)sgp->length);
            Serialization_serializeUInt64(ctx, (uint64_t)sgp->offset);
         }
      }
   }
   else
   {
      Serialization_serializeUInt64(ctx, 0ull);
   }
}
void ReadLocalFileRDMAMsg_serializePayload(NetMessage* msg, SerializeCtx* ctx)
{
   ReadLocalFileRDMAMsg* thisCast = (ReadLocalFileRDMAMsg*)msg;

   // offset
   Serialization_serializeInt64(ctx, thisCast->offset);

   // count
   Serialization_serializeInt64(ctx, thisCast->count);

   // accessFlags
   Serialization_serializeUInt(ctx, thisCast->accessFlags);

   // fileHandleID
   Serialization_serializeStrAlign4(ctx, thisCast->fileHandleIDLen, thisCast->fileHandleID);

   // clientNumID
   NumNodeID_serialize(ctx, &thisCast->clientNumID);

   // pathInfo
   PathInfo_serialize(ctx, thisCast->pathInfoPtr);

   // targetID
   Serialization_serializeUShort(ctx, thisCast->targetID);

   // RDMA info
   RdmaInfo_serialize(ctx, thisCast->rdmap);
}

/**
 * Test flag. (For convenience and readability.)
 *
 * @return true if given flag is set.
 */
inline bool NetMessage_isMsgHeaderFeatureFlagSet(NetMessage* thisone, unsigned flag)
{
   return (thisone->msgHeader.msgFeatureFlags & flag) != 0;
}
