#include "new_read.hpp"
// #include "open.hpp"
#include <algorithm> // 用于 std::min 和 std::max






// 使用标准库中的 min 和 max 函数
#define MIN(a, b) std::min(a, b)
#define MAX(a, b) std::max(a, b)

// 定义 min_t 宏
#define min_t(type, a, b) ({ \
    type __a = (a); \
    type __b = (b); \
    __a < __b ? __a : __b; \
})

// stripe_pattern 
// common/source/common/storage/striping/Raid0Pattern.h

// struct StripePattern
// {
//    unsigned patternType; // STRIPEPATTERN_...
//    unsigned chunkSize; // must be a power of two (optimizations rely on it)

//    unsigned serialPatternLength; // for (de)serialization 表示序列化后的数据长度。

//    // virtual functions
//    void (*uninit) (StripePattern* sp);

//    // (de)serialization
//    bool (*deserializePattern) (StripePattern* sp, DeserializeCtx* ctx);//用于从给定的反序列化上下文DeserializeCtx中反序列化条带模式。

//    size_t (*getStripeTargetIndex) (StripePattern* sp, int64_t pos);//用于根据数据的位置pos计算应该存储在哪个条带目标上。
//    uint16_t (*getStripeTargetID) (StripePattern* sp, int64_t pos);//用于根据数据的位置pos获取对应的条带目标ID。
//    void (*getStripeTargetIDsCopy) (StripePattern* sp, UInt16Vec* outTargetIDs);//用于将条带目标ID列表复制到提供的UInt16Vec向量中。
//    UInt16Vec* (*getStripeTargetIDs) (StripePattern* sp);//用于获取一个指向条带目标ID列表的指针。
//    UInt16Vec* (*getMirrorTargetIDs) (StripePattern* sp);//用于获取镜像目标ID列表的指针，这通常用于支持数据冗余的条带模式。
//    unsigned (*getMinNumTargets) (StripePattern* sp);//用于获取条带模式所需的最小目标（存储节点）数量。
//    unsigned (*getDefaultNumTargets) (StripePattern* sp);//用于获取条带模式的默认目标（存储节点）数量。
// };

// client_module/source/common/storage/striping/Raid0Pattern.h
// client_module/source/common/storage/striping/Raid0Pattern.c

unsigned StripePattern_getChunkSize(StripePattern* sp)
{   
    if(sp==nullptr)
    {
        std::cout<<"error \n";
    }
   return sp->chunkSize;
}


UInt16Vec* Raid0Pattern_getStripeTargetIDs(StripePattern* sp)
{
   Raid0Pattern* thisCast = (Raid0Pattern*)sp;

   return &thisCast->stripeTargetIDs;
}

// struct UInt16List
// {
//    struct PointerList pointerList;
// };



    #define BEEGFS_BUG_ON_DEBUG(condition, msgStr) \
   do { /* nothing */ } while(0)

uint16_t UInt16Vec_at(UInt16Vec* vec, size_t index)
{
   BEEGFS_BUG_ON_DEBUG(index >= UInt16Vec_length(vec), "Index out of bounds");

   return vec->vecArray[index];
}


size_t Raid0Pattern_getStripeTargetIndex(StripePattern* sp, int64_t pos)
{

   struct Raid0Pattern* p = container_of_v2<Raid0Pattern,StripePattern>(sp, offsetof(Raid0Pattern, stripePattern));

   return (pos / sp->chunkSize) % UInt16Vec_length(&p->stripeTargetIDs);
}


            int64_t StripePattern_getChunkStart(StripePattern* sp, int64_t pos)
            {
               // the code below is an optimization (wrt division) for the following line:
               //    int64_t chunkStart = pos - (pos % sp->chunkSize);

               // "& chunkSize -1" instead of "%", because chunkSize is a power of two
               unsigned posModChunkSize = pos & (sp->chunkSize - 1);//即 pos % chunksize

               int64_t chunkStart = pos - posModChunkSize;//即 此时 

               return chunkStart;
            }

      /**
      * Get the exact file position where the next chunk starts
      */
      int64_t StripePattern_getNextChunkStart(StripePattern* sp, int64_t pos)
      {
         return StripePattern_getChunkStart(sp, pos) + sp->chunkSize;
      }


/**
 * Get the exact file position where the current chunk ends
 */
int64_t StripePattern_getChunkEnd(StripePattern* sp, int64_t pos)
{
   return StripePattern_getNextChunkStart(sp, pos) - 1;
}


//struct iov_iter 替换成 iovec 之后的接口 -------------------------

   //见 new_read.hpp

// BeeGFS_ReadSink -----------------

typedef struct _BeeGFS_ReadSink BeeGFS_ReadSink;
struct _BeeGFS_ReadSink {

  
    struct iovec sanitized_iter;
    /*
      size_t npages; // 我们不需要这个，因为我们不处理页
      void **pages; // 指针数组，指向分配的内存页
      size_t capacity; // 表示总容量
      char *buffer; // 单个连续缓冲区，简化实现
      size_t offset; // 当前偏移量
    
    */
/*
   size_t npages;  // Number of pages currently in use (get_page()) 表示当前使用的页数。这些页是通过 get_page() 函数获取的，通常用于内存管理。
   struct page **pages;  // 0..npages 这是一个指针数组，每个元素都指向一个 struct page 结构体。这些结构体表示内存中的一页，通常用于虚拟内存管理和页缓存。
   struct bio_vec *bvecs;  // 0..npages生物向量是啥 bio_vec 结构体通常用于表示生物（BIO）向量，它描述了一组要进行 I/O 操作的内存页。

   // output value
   struct iov_iter sanitized_iter;//用于描述一组非连续内存区域的迭代器，通常用于处理 I/O 操作，如读取或写入文件时的数据传输。
*/



};



// 初始化读取缓冲区
void beegfs_readsink_init(BeeGFS_ReadSink *readsink, size_t capacity) {
   /*
      readsink->buffer =(char *) malloc(capacity);
      std::cout<<"[beegfs_readsink_init] readsink->buffer = "<<static_cast<void*>(readsink->buffer) <<"\n";
      if (!readsink->buffer) {
         std::cout<<"Failed to allocate memory for readsink \n";
         perror("Failed to allocate memory for readsink");
         exit(EXIT_FAILURE);
      }
      readsink->capacity = capacity;
      readsink->offset = 0;
   
   */
    //

    
}


// 预留读取空间
void beegfs_readsink_reserve(BeeGFS_ReadSink *readsink, struct iovec *iter, size_t maxReadSize) {

readsink->sanitized_iter = *iter;
user_iov_iter_truncate(&readsink->sanitized_iter, maxReadSize);

    /*
    
      if (readsink->offset + maxReadSize > readsink->capacity) {
         fprintf(stderr, "Readsink buffer overflow\n");
         exit(EXIT_FAILURE);
      }
      iter->iov_base = readsink->buffer+ readsink->offset;//readsink->buffer + readsink->offset;

      iter->iov_len = maxReadSize;
      std::cout<<"maxReadSize = "<<maxReadSize<<"\n";

      readsink->offset += maxReadSize;
    */

}
void beegfs_readsink_release(BeeGFS_ReadSink *readsink) {
    /*
    
      if (readsink->buffer) {
         free(readsink->buffer);
         readsink->buffer = nullptr;
      }
      readsink->offset = 0;
      readsink->capacity = 0;
    */
}


// -----------------------------__FhgfsOpsRemoting_getChunkOffset--------------

         /**
            * Checks whether there is only a single bit set in value, in which case value is a power of
            * two.
            *
            * @param value may not be 0 (result is undefined in that case).
            * @return true if only a single bit is set (=> value is a power of two), false otherwise
         */
         bool MathTk_isPowerOfTwo(unsigned value)
         {
            //return ( (x != 0) && !(x & (x - 1) ) ); // this version is compatible with value==0

            return !(value & (value - 1) );
         }

         /**
            * Base 2 logarithm.
            *
            * See log2Int64() for details.
         */
         unsigned MathTk_log2Int32(unsigned value)
         {
            /* __builtin_clz: Count leading zeros - returns the number of leading 0-bits in x, starting
            * at the most significant bit position. If x is 0, the result is undefined. */
            // (note: 8 is bits_per_byte)
            unsigned result = (sizeof(value) * 8) - 1 - __builtin_clz(value); // GCC 内置的一个函数 ，可以正常使用

            return result;
         }

         uint64_t user_do_div_unsigned(uint64_t *n, uint32_t base) {
            uint64_t rem = *n % (uint64_t)base;
            *n = *n / (uint64_t)base;
            return rem;
         }
        int64_t user_do_div(int64_t *n, uint32_t base) {
            int64_t rem = *n % (uint64_t)base;
            *n = *n / (uint64_t)base;
            return rem;
         }

/**
 * Compute the chunk-file offset on a storage server from a given user file position.
 *
 * Note: Make sure that the given stripeNodeIndex is really correct for the given pos.
 */
static int64_t __FhgfsOpsRemoting_getChunkOffset(int64_t pos, unsigned chunkSize, size_t numNodes,
   size_t stripeNodeIndex)
{
   /* the code below is an optimization (wrt division and modulo) of the following three lines:
         int64_t posModChunkSize = (pos % chunkSize);
         int64_t stripeSetStart = pos - posModChunkSize - (stripeNodeIndex*chunkSize);
         return ( (stripeSetStart / numNodes) + posModChunkSize); */

   /* note: "& (chunksize-1) only works as "% chunksize" replacement, because chunksize must be
      a power of two */

   int64_t posModChunkSize = pos & (chunkSize-1);
   int64_t stripeSetStart = pos - posModChunkSize - (stripeNodeIndex*chunkSize);

   int64_t stripeSetStartDivNumNodes;

   // note: if numNodes is a power of two, we can do bit shifting instead of division

   if(MathTk_isPowerOfTwo(numNodes) )
   { // quick path => bit shifting
      stripeSetStartDivNumNodes = stripeSetStart >> MathTk_log2Int32(numNodes);
   }
   else
   { // slow path => division
      // note: do_div(n64, base32) assigns the result to n64 and returns the remainder!
      // (we need do_div to enable this division on 32bit archs)

      stripeSetStartDivNumNodes = stripeSetStart; // will be changed by do_div()
                     //do_div(stripeSetStartDivNumNodes, (unsigned)numNodes); //这个函数是内核中 提供的一个宏
      uint64_t remainder =user_do_div(&stripeSetStartDivNumNodes, (unsigned)numNodes); //这个函数是内核中 提供的一个宏
      // 现在 stripeSetStart 包含了商，remainder 包含了余数
   }

   return (stripeSetStartDivNumNodes + posModChunkSize);
}



//---------------------------- FhgfsOpsCommKit_initFileOpState ----------------------


      // static void commkit_initTargetInfo(struct CommKitTargetInfo* info, uint16_t targetID)
      // {
      //    struct CommKitTargetInfo value = {
      //       .state = CommKitState_PREPARE,
      //       .targetID = targetID,
      //       .useBuddyMirrorSecond = false,
      //       .nodeResult = -FhgfsOpsErr_INTERNAL,
      //    };

      //    *info = value;
      // }

/**
   * Note: Initializes the expectedNodeResult attribute from the size argument
   * Note: defaults to server-side mirroring enabled.
   * 它用于初始化文件操作状态结构体 FileOpState
   * loff_t
 */
void FhgfsOpsCommKit_initFileOpState(FileOpState* state, int64_t offset, size_t size,
   uint16_t targetID)
{
   // *state = (FileOpState) {
      *state =  (FileOpState){
      
      .transmitted = 0,// 初始化已传输字节数为 0
                               
   
      .toBeTransmitted = size,// 初始化待传输字节数为 0
      .totalSize = size,// 设置操作的总大小（从 size 参数初始化）
        .offset = offset,// 设置文件操作的偏移量
      .firstWriteDoneForTarget = false,// 初始化目标首次写入标志为 false
      .receiveFileData = false,
      .expectedNodeResult = size,// 从 size 参数初始化预期节点结果
        
   };
// #ifdef BEEGFS_NVFS
//       .rdmap = NULL,
// #endif
    // For read: RECVHEADER will bump this in each HEADER-DATA loop iteration
                                // For write: PREPARE sets this to totalSize
                                /*
                                    这段注释解释了 toBeTransmitted 字段在读取和写入操作中的不同用途
                                    读取操作：
                                        RECVHEADER：这是一个在读取操作中处理头部信息的阶段，它可能会在每次头部-数据循环迭代中增加 toBeTransmitted 字段的值。
                                        循环迭代：在读取操作中，通常会有一个循环，每次迭代都会处理一个数据块的头部和数据。在每次迭代中，RECVHEADER 阶段可能会调整 toBeTransmitted，以反映剩余需要接收的数据量。
                                    对于写入操作：
                                        PREPARE：这是一个在写入操作中准备阶段，它会设置 toBeTransmitted 字段为 totalSize。
                                        totalSize：这是写入操作需要传输的总数据量。在准备阶段，系统会知道要写入的数据总量，因此会将 toBeTransmitted 设置为这个值，表示开始写入操作时，还有这么多数据需要被传输。
                                */
    // 初始化目标信息
    //在下面这个函数会 state->base = CommKitState_PREPARE,
   commkit_initTargetInfo(&state->base, targetID);
}

//---------------------------------- BitStore_getBit --------------------------

      #define BITSTORE_BLOCK_SIZE         sizeof(bitstore_store_type)   // size of a block in bytes
      #define BITSTORE_BLOCK_BIT_COUNT    (BITSTORE_BLOCK_SIZE * 8)     // size of a block in bits

      typedef long unsigned int bitstore_store_type;
      /**
      * A vector of bits.
      */
      struct BitStore
      {
         unsigned numBits; // max number of bits that this bitstore can hold
         bitstore_store_type lowerBits; // used to avoid overhead of extra alloc for higherBits
         bitstore_store_type* higherBits; // array, which is alloc'ed only when needed
      };


      /**
      * returns the index of the bit block (array value) which contains the searched bit
      */
      unsigned BitStore_getBitBlockIndex(unsigned bitIndex)
      {
         return (bitIndex / BITSTORE_BLOCK_BIT_COUNT);
      }

      /**
      * returns the index in a bit block for the searched bit
      */
      unsigned BitStore_getBitIndexInBitBlock(unsigned bitIndex)
      {
         return (bitIndex % BITSTORE_BLOCK_BIT_COUNT);
      }


      /**
      * User-space test_bit function
      */
      bool user_test_bit(unsigned index, const bitstore_store_type *bitBlock)
      {
         bitstore_store_type mask = 1UL << index;
         return (*bitBlock & mask) != 0;
}

/**
 * Test whether the bit at the given index is set or not.
 *
 * Note: The bit operations in here are atomic.
 */
bool BitStore_getBit(const BitStore* bs, unsigned bitIndex)
{
   unsigned index;
   unsigned indexInBitBlock;

   if(bitIndex >= bs->numBits) 
      return false;

   index = BitStore_getBitBlockIndex(bitIndex);
   indexInBitBlock = BitStore_getBitIndexInBitBlock(bitIndex);

   /*
   test_bit 是 Linux 内核中提供的一个宏，用于测试一个位图中的特定位是否被设置（即是否为1）。这个宏定义在内核的 <linux/bitops.h> 头文件中，并且只能在内核代码中使用，因为它直接操作内核的位图数据结构。
   */
   if (index == 0)
      // return test_bit(indexInBitBlock, &bs->lowerBits);
      return user_test_bit(indexInBitBlock, &bs->lowerBits);

   else
      // return test_bit(indexInBitBlock, &bs->higherBits[index - 1]);
      return user_test_bit(indexInBitBlock, &bs->higherBits[index - 1]);

}

//----------- app --------------------
   // void App_incNumRemoteReads(App* this)
   // {
   //    Mutex_lock(&this->debugCounterMutex);
   //    this->numRemoteReads++;
   //    Mutex_unlock(&this->debugCounterMutex);
   // }



//  
/**
 * Increase current value to "minValue" if it is currently smaller, otherwise leave it as it was.
 *
 * Note: Use this carefully and only in special scenarios, because it could run endless if the
 * atomic value is not monotonically increasing.
 */
// void AtomicInt_max(AtomicInt* this, int minValue)
// {
//    int currentVal = AtomicInt_read(this);
//    for( ; ; )
//    {
//       int swapRes;

//       if(currentVal >= minValue)
//          return; // no need to update it at all

//       swapRes = AtomicInt_compareAndSwap(this, currentVal, minValue);
//       if (swapRes == currentVal)
//          return;

//       currentVal = swapRes; // swap was not successful, update currentVal
//    }
// }