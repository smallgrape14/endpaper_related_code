
#include <pthread.h>
// #include <stdatomic.h>
#include <stddef.h> // 包含 offsetof 的头文件
#include <cstddef> // 包含 offsetof 的头文件
#include <stdio.h>   // 用于 printf，如果需要的话
#include <sys/uio.h> // 包含iovec的定义
#include <atomic>

//$$% ---------------------------
// #include "open.hpp"
// #include "RDMAConnection.hpp"
#include "read_pre.hpp"

//$$% ---------------------------


//------------------------(1) ioInfo 初始化  --------------------------


        struct RemotingIOInfo
        {
            //OpenFileRespMsg 中有的 

                    // const char* fileHandleID;@@ 我替换成下面的 NOTE：
            char* fileHandleID;

            struct StripePattern* pattern;
            PathInfo* pathInfo;

                //其他 
            unsigned accessFlags; // OPENFILE_ACCESS_... flags

            // pointers to fhgfsInode->fileHandles[handleType]...

            bool* needsAppendLockCleanup; // (note: can be NULL in some cases, implies "false")
                        

            unsigned userID;     // only used in storage server write message
            unsigned groupID;    // only used in storage server write message

            //注释 跳过以下
                        // AtomicInt* UInt16List;
                        // BitStore* firstWriteDone;
                        // App* app;
                        // #ifdef BEEGFS_NVFS
                        //     bool nvfs;
                        // #endif
        };

        // 初始化PathInfo结构体的函数
        void PathInfo_init(PathInfo* pathInfo) {
            if (pathInfo == nullptr) {
                // 如果传入的指针是nullptr，那么不能进行初始化
                std::cout<<" 如果传入的指针是nullptr，那么不能进行初始化"<<endl;
                return;
            }

            // 使用memset来将整个结构体初始化为0
            std::memset(pathInfo, 0, sizeof(PathInfo));

            // 由于union的特性，只需要初始化一个成员即可
            // 这里我们初始化_flags成员，其他成员将自动被初始化为0
            pathInfo->_flags = 1;//PATHINFO_FEATURE_ORIG;//PATHINFO_FEATURE_ORIG;//@@ PATHINFO_FEATURE_ORIG;//1

            // 如果需要设置特定的值，可以在这里进行设置
            // 例如，设置flags
            pathInfo->flags = 1;//PATHINFO_FEATURE_ORIG;//PATHINFO_FEATURE_ORIG;//@@ PATHINFO_FEATURE_ORIG;  PATHINFO_FEATURE_INLINED;

            // 设置origParentUID或origParentEntryID，
            pathInfo->origParentUID = 29999;//@@ root UID  stat mnt/beegfs 是 0： stat mnt/beegfs/test_1201 :1000
            pathInfo->_origParentUID = 29999;//@@
            pathInfo->origParentEntryID = "0-67B6EDE2-A2";//@@ root 是 根目录的entryid
            
            pathInfo->_origParentEntryID = "0-67B6EDE2-A2";//@@
            // 162 server 上：test_1201 目录的ID 是 0-674C7122-1 
            // 162 server 上 ：test_1202 是0-674D5ED6-1
            // 162 server 上 ：test_1217 是0-674D5ED6-1

            
            //Note: 以上这六个参数和 open获取的信息是一致的
        }

// void initStripePattern(StripePattern* pattern, unsigned patternType, unsigned chunkSize, unsigned serialPatternLength) {
struct Raid0Pattern
{
   StripePattern stripePattern;

   UInt16Vec stripeTargetIDs;
   unsigned defaultNumTargets;
};

            void PointerList_init(PointerList* plist)
            {
                plist->head = nullptr;
                plist->tail = nullptr;
                plist->length = 0;
            }

            void UInt16List_init(UInt16List* list)
            {
                PointerList_init( (PointerList*)list);
            }

        void UInt16Vec_init(UInt16Vec* vec)
        {
            UInt16List_init( (UInt16List*)vec);

            vec->vecArrayLen = 4;
            vec->vecArray = (uint16_t*)malloc(vec->vecArrayLen * sizeof(uint16_t) );
        }

            size_t PointerList_length(const PointerList* pl)
            {
                return pl->length;
            }

        static inline size_t UInt16List_length(UInt16List* list)
        {
            return PointerList_length( (PointerList*)list);
        }

    size_t UInt16Vec_length(UInt16Vec* vec)
    {
        return UInt16List_length( (UInt16List*)vec);
    }



      static inline void __PointerList_addTail(PointerList* plist, PointerListElem* elem)
      {
         if(plist->length)
         { // elements exist => replace tail
            elem->prev = plist->tail;
            plist->tail->next = elem;
            plist->tail = elem;
         }
         else
         { // no elements exist yet
            plist->head = elem;
            plist->tail = elem;

            elem->prev = nullptr;
         }

         elem->next = nullptr;
         plist->length++;
      }

      void PointerList_addTail(PointerList* plist, void* valuePointer)
      {
         PointerListElem* elem = (PointerListElem*)malloc(sizeof(PointerListElem) );
         elem->valuePointer = valuePointer;
         __PointerList_addTail(plist, elem);
      }

      void PointerList_append(PointerList* plist, void* valuePointer)
      {
         PointerList_addTail(plist, valuePointer);
      }

      void UInt16List_append(UInt16List* list, uint16_t value)
      {
         /* cast value directly to pointer type here to store value directly in the pointer variable
            without allocating extra mem */
         PointerList_append( (PointerList*)list, (void*)(size_t)value);
      }

void UInt16Vec_append(UInt16Vec* vec, uint16_t value)
{
   size_t newListLen;

   UInt16List_append( (UInt16List*)vec, value);

   newListLen = UInt16List_length( (UInt16List*)vec);

   // check if we have enough buffer space for new elem

   if(newListLen > vec->vecArrayLen)
   { // double vector array size: alloc new, copy values, delete old, switch to new
      uint16_t* newVecArray = (uint16_t*)malloc(vec->vecArrayLen * sizeof(uint16_t) * 2);
      memcpy(newVecArray, vec->vecArray, vec->vecArrayLen * sizeof(uint16_t) );

      free(vec->vecArray);

      vec->vecArrayLen = vec->vecArrayLen * 2;
      vec->vecArray = newVecArray;
   }

   // add value to last array elem (determine last used index based on list length)

   (vec->vecArray)[newListLen-1] = value;
}



void StripePattern_init(StripePattern* pattern) {
    if (pattern == NULL) {
        std::cout<<"StripePattern_init error \n";
        return; // 或者可以设置一个错误码
    }
     // 使用memset来将整个结构体初始化为0
    std::memset(pattern, 0, sizeof(StripePattern));
    pattern->patternType = 1;//#define STRIPEPATTERN_Raid0        1
    pattern->chunkSize = 512*1024;//512KB
    pattern->serialPatternLength = 12;//Note:TODO 这个序列化长度，不知道哪里会用到 12B
}
void Raid0_StripePattern_init(Raid0Pattern* pattern)
{
    if(pattern==nullptr)
    {
        std::cout<<"[Raid0_StripePattern_init] error \n";
    }
    std::memset(pattern, 0, sizeof(Raid0Pattern));
    //(1)
    StripePattern_init(&(pattern->stripePattern));
    
    // (2)init attribs
   UInt16Vec_init(&pattern->stripeTargetIDs);
   size_t len= UInt16Vec_length(&pattern->stripeTargetIDs);
   std::cout<<"[Raid0_StripePattern_init] len = "<<len<<"\n";
    // (3)
    int targetnum=1;//@@ todo
   pattern->defaultNumTargets=targetnum;//@@ todo默认为1
   // append 
   uint16_t targetNumID[10];//1913;//@@ todo 217 server :1913 
   // 162server : 3 4 
   targetNumID[0]=81;//@@ 162server: 250303 更新之后就是 81了
   //224 server :重新配置 241206更新 targetID 为 1201
   targetNumID[1]=4;//@@

   for(int i=0;i<targetnum ;i++)
   {
     UInt16Vec_append(&pattern->stripeTargetIDs,targetNumID[i]);

   }
   std::cout<<"[Raid0_StripePattern_init] after append, len = "<<len<<"\n";


}
        // 初始化 RemotingIOInfo 结构体的函数
        void initRemotingIOInfo(RemotingIOInfo* info) {
            if (info != NULL) {
                memset(info, 0, sizeof(RemotingIOInfo)); // 将整个结构体清零

                // 初始化字段到默认值
                        //info->fileHandleID = ;//"75196CA8#0-67282B84-BE";//@@从 open 函数 中获取
                info->fileHandleID=(char*)malloc(50);
                // strcpy(info->fileHandleID,"837976248685");
                //XYP_MODIFY 
                // strcpy(info->fileHandleID,"filehandleID_xyp");//@@直接拷贝 open 返回的信息

                strcpy(info->fileHandleID,filehandleID_xyp);//@@直接拷贝 open 返回的信息
                //std::cout<<"[initRemotingIOInfo] info->fileHandleID = "<<info->fileHandleID<<"\n";
                
                // StripePattern* pattern=new StripePattern ();
                
                Raid0Pattern* pattern=new Raid0Pattern ();
                Raid0_StripePattern_init(pattern);
                info->pattern=&(pattern->stripePattern);
                //std::cout<<"info->pattern = "<<info->pattern<<"\n";
                if(pattern==nullptr|| info->pattern==nullptr)
                {
                    std::cout<<"error \n";
                }
                

                
                info->accessFlags = 1; // #define OPEN_ACCESS_READ  1          1
                PathInfo* pathinfo=new PathInfo ();
                PathInfo_init(pathinfo);//@@
                //std::cout<<"$$$ pathinfo->flag = "<<pathinfo->flags<<"\t, _flag = "<<pathinfo->_flags<<"\n";
                info->pathInfo = pathinfo; // OpenFileRespMsg 中 有 PathInfo pathInfo;

                
                info->needsAppendLockCleanup = nullptr;
                info->userID = 0;
                info->groupID = 0;
                // 初始化其他字段...
                //std::cout<<"初始化其他字段...\n";
             //std::cout<<"ioInfo->pattern = "<<info->pattern<<"\n";

            }
            else
            {
                std::cout<<"info ==NUll \n ";
            }
        }

//---------------------(2) 初始化 statelist -------------------------------

        // 初始化链表头的函数
        static inline void INIT_LIST_HEAD(list_head_t *list) {
            list->next = list;
            list->prev = list;
        }

        // 判断链表是否为空
         int list_empty(list_head_t *head) {
            return head->next == head;
         }

        // 将节点添加到链表尾部的函数
        static inline void list_add_tail(list_head_t *new_node, list_head_t *head) {
            new_node->next = head;
            new_node->prev = head->prev;
            head->prev->next = new_node;
            head->prev = new_node;
        }
        // 在链表头部添加元素
      void list_add_head(list_head_t *elem, list_head_t *head) {
         elem->next = head->next;
         elem->prev = head;
         head->next->prev = elem;
         head->next = elem;
      }

        // 从链表中移除元素
    //   void list_del(list_head_t *elem) {
    //     // if (elem == nullptr || elem->next == nullptr || elem->prev == nullptr) {
    //     // std::cerr << "Error: Attempt to delete an already deleted list element." << std::endl;
    //     // return;
    //     // }
    //     std::cout<<"1-----------\n";
    //      elem->next->prev = elem->prev;
    //     std::cout<<"2-----------\n";

    //      elem->prev->next = elem->next;
    //     std::cout<<"3-----------\n";

    //      elem->next = nullptr;
    //     std::cout<<"4-----------\n";

    //      elem->prev = nullptr;
    //     std::cout<<"5-----------\n";

    //   }
// 从链表尾部移除元素的函数
static inline void list_del_tail(list_head_t *head) {
    // 检查链表是否为空
    if (list_empty(head)) {
        std::cerr << "Error: Attempt to delete from an empty list." << std::endl;
        return;
    }
    
    // 获取尾节点，即 head->prev 指向的节点
    list_head_t *tail = head->prev;
    
    // 确保 tail 节点不是头节点，否则列表为空
    if (tail == head) {
        std::cerr << "Error: The list is empty." << std::endl;
        return;
    }
    
    // 将 tail 节点的前一个节点（即 head->prev 的前一个节点，也就是倒数第二个节点）的 next 指向 head
    tail->prev->next = head;
    
    // 将 head 的 prev 指向 tail 的前一个节点，完成链表的断开
    head->prev = tail->prev;
    
    // 将 tail 节点的 next 和 prev 设置为 nullptr，避免野指针
    tail->next = nullptr;
    tail->prev = nullptr;
}


      // 统计链表长度的函数
int list_count(list_head_t *head) {
    int count = 0;
    if(head==nullptr)
    {
        std::cout<<"head==nullptr \n";
        return 0;
    }
    list_head_t *tmp = head->next; // 从头节点的下一个节点开始，因为头节点不计入长度
    if(tmp==nullptr)
    {
        std::cout<<"head->next==nullptr \n";
        return 0;
    }
std::cout<<static_cast<void*>(tmp)<<"\n";
    // 遍历链表直到回到头节点
    while (tmp != head) {
        count++;
        tmp = tmp->next;
    }

    return count;
}

    // 从链表中获取容器结构体的指针
        // #define container_of(ptr, type, member) ({                      \
        //         const typeof( ((type *)0)->member ) *__mptr = (ptr);      \
        //         (type *)( (char *)__mptr - offsetof(type,member) );})

        // // 获取链表的第一个元素
        // #define list_first_entry(ptr, type, member) \
        //     container_of((ptr)->next, type, member)

        // // 获取链表中的元素
        // #define list_entry(ptr, type, member) \
        //     container_of(ptr, type, member)


    template <typename StructType, typename MemberType>
    StructType* container_of_v2(MemberType* ptr, ptrdiff_t offset) {
        return reinterpret_cast<StructType*>(
            reinterpret_cast<char*>(ptr) - offset
        );
    }

// 定义 list_first_entry 模板函数
// template <typename StructType, typename MemberType>
// inline StructType* list_first_entry(MemberType* ptr) {
//     // static_assert(std::is_convertible<typename std::remove_pointer<MemberType>::type*, StructType*>::value,
//     //               "MemberType must be a member of StructType");
//     return container_of_v2<StructType, typename std::remove_pointer<MemberType>::type>(
//         ptr, OffsetHelper<StructType, typename std::remove_pointer<MemberType>::type>::offset()
//     );
// }
// 定义 list_first_entry 模板函数
// template <typename StructType, typename MemberType>
// inline StructType* list_first_entry(struct list_head* ptr, MemberType StructType::* member) {
//     return container_of_v2<StructType, MemberType>(ptr, offsetof(StructType, member));
// }
// template <typename StructType, typename MemberType>
// inline StructType* list_first_entry(MemberType* ptr, StructType, MemberType StructType::* member) {
//     return container_of_v2<StructType, MemberType>(ptr, offsetof(StructType, member));
// }



        enum CommKitState
        {
            CommKitState_PREPARE,
            CommKitState_SENDHEADER,
            CommKitState_SENDDATA,
            CommKitState_RECVHEADER,
            CommKitState_RECVDATA,
            CommKitState_SOCKETINVALIDATE,
            CommKitState_CLEANUP,
            CommKitState_RETRYWAIT,
            CommKitState_DONE,
        };

        // 假设的红黑树节点结构体
                // typedef struct rb_node {
                //     struct rb_node *rb_parent; // 父节点
                //     int rb_color;             // 节点颜色，红或黑
                //     struct rb_node *rb_right; // 右子节点
                //     struct rb_node *rb_left;  // 左子节点
                // } rb_node;
        // 定义红黑树节点结构体
        typedef struct rb_node {
            struct rb_node *parent;
            struct rb_node *child[2]; // 0 表示左孩子，1 表示右孩子
            int color; // 0 表示红色，1 表示黑色
        } rb_node_t;

        // 红黑树的根节点宏定义
        #define RB_ROOT ((rb_node_t *)0)

        // 初始化红黑树元素的函数
        static inline void RB_tree_init(rb_node_t *rbTreeElement) {
            rbTreeElement->parent = RB_ROOT;
            rbTreeElement->child[0] = RB_ROOT;
            rbTreeElement->child[1] = RB_ROOT;
            rbTreeElement->color = 0; // 红色
        }

        struct Node
        {
            char* id; // string ID, generated locally on each node; not thread-safe (not meant to be changed)
            NumNodeID numID; // numeric ID, assigned by mgmtd server store (unused for clients)

            NodeType nodeType; // set by NodeStore::addOrUpdate()
            char* nodeIDWithTypeStr; // for log messages (initially NULL, alloc'ed when needed)

            // NodeConnPool* connPool;//@@
            unsigned short portUDP;

            Time lastHeartbeatT; // last heartbeat receive time

            bool isActive; // for internal use by the NodeStore only

            // Mutex mutex; //换成下面这个
            pthread_mutex_t mutex;

            Condition changeCond; // for last heartbeat time only

                    // struct kref references;
            // int references; //@@不知道后来会怎么用
            std::atomic<int> references;

            /* used by node tree */ //@@不知道后来会怎么用
            struct {
                struct rb_node rbTreeElement;
            } _nodeTree;
            
        };

        /*
            这段代码定义了一个名为 CommKitTargetInfo 的结构体，
            它用于存储与特定目标节点相关的通信状态和信息。这个结构体是通信框架中的一个关键组件，用于管理节点间的通信过程。
        */
        struct CommKitTargetInfo
        {
            struct list_head targetInfoList;//用于将多个 CommKitTargetInfo 结构体链接在一起，形成一个链表。

            // (set to _PREPARE in the beginning, assigned by the state-creator)
            enum CommKitState state; // the current stage of the individual communication process
            //表示当前通信过程的阶段，如准备、发送、接收等。


            // used by GenericResponse handler
            //用于记录与响应处理相关的日志信息，例如是否尝试重新连接、是否记录了间接通信错误等。
            struct {
                unsigned peerTryAgain:1;
                unsigned indirectCommError:1;
                unsigned indirectCommErrorNoRetry:1;
            } logged;

            // assigned by the state-creator
            char* headerBuffer; // for serialization//用于存储要发送到目标节点的头部信息，通常用于序列化操作。
            uint16_t targetID; //表示目标节点的唯一标识符。
            uint16_t selectedTargetID; // either targetID or the buddy, if useBuddyMirrorSecond表示实际要通信的目标节点的ID，可能是 targetID 或者是辅助镜像节点的ID，取决于是否使用辅助镜像。
            bool useBuddyMirrorSecond; // if buddy mirroring, this msg goes to secondary指示是否使用辅助镜像节点进行通信。

            // set by _PREPARE handler
            Node* node; // target-node reference 指向目标节点的引用，包含节点的详细信息。
            Socket* socket; // target-node connection 指向目标节点的套接字连接。
            unsigned headerSize;//表示头部信息的大小。

            // error if negative, other set by specialized actions
            int64_t nodeResult;//存储节点操作的结果，如果是负数表示错误。
        };

        static void commkit_initTargetInfo(struct CommKitTargetInfo* info, uint16_t targetID)
        {
             CommKitTargetInfo value = {
                .state = CommKitState_PREPARE,
                .targetID = targetID,
                .useBuddyMirrorSecond = false,
                .nodeResult = -FhgfsOpsErr_INTERNAL,
            };

            *info = value;
            // std::cout<<"[commkit_initTargetInfo] info->targetID = "<<info->targetID<<"\n";
        }

        // 遍历链表并打印每个节点
        void print_list(list_head_t *head) {
            // CommKitTargetInfo *info;
            // list_for_each_entry(info, head, targetInfoList) {
            //     printf("Node data: %p\n", info);
            // }
            list_head_t *pos;
            CommKitTargetInfo *info;
            int cnt=0;
            for (pos = head->next; pos != head&& pos!=nullptr; pos = pos->next) {
                // 使用 container_of 函数从链表节点中获取包含它的结构体指针
                if(pos==nullptr)
                {
                    break;
                }
                
                info = container_of_v2<CommKitTargetInfo, list_head_t>(pos, offsetof(CommKitTargetInfo, targetInfoList));
               
                //std::cout << "Node data: " << static_cast<void*>(info) << std::endl;
                //std::cout<<"info->state = "<<info->state<<"\n";
                 //std::cout<<"info->targetID = "<<info->targetID<<"\n";
                //std::cout<<"------------------\n";
            }
        }

void statelist_init(list_head_t* stateList)
{
   
   INIT_LIST_HEAD(stateList);
                           
   // 创建并初始化 CommKitTargetInfo 结构体，考虑多个target 时候，需要考虑 块大小 和 文件关系 到底会在 targetklist中如何存放 会比较复杂
   int targetnum=1;
   uint16_t targetIDlist[targetnum];
   targetIDlist[0]=1913;//1911;//1911;//@@ target ID // 162 ubuntu: 3 // 217 dell01 : 1911

    for (int i = 0; i < targetnum; ++i) {
        //  CommKitTargetInfo targetInfo;
         


                //  std::cout<<"targetIDlist[i] = "<<targetIDlist[i]<<"\n";
        // commkit_initTargetInfo(&targetInfo, targetIDlist[i]);
                 //CommKitTargetInfo * targetInfo; targetInfo->targetID 
        // list_add_tail(&targetInfo.targetInfoList, stateList);
        // std::cout<<"info->targetID = "<<targetInfo.targetID<<"\n";

        CommKitTargetInfo* targetInfo =new CommKitTargetInfo();
        commkit_initTargetInfo(targetInfo, targetIDlist[i]);
        list_add_tail(&(targetInfo->targetInfoList), stateList);
        // std::cout<<"info->targetID = "<<targetInfo->targetID<<"\n";

    }
            // 遍历链表并打印每个节点
    print_list(stateList);
}

//-------------------------------(3) readfile_nextIter

struct CommKitContext;
typedef struct CommKitContext CommKitContext;

static unsigned __commkit_readfile_prepareHeader(CommKitContext* context,struct CommKitTargetInfo* info);
static int __commkit_readfile_recvdata(CommKitContext* context, struct CommKitTargetInfo* info);

struct CommKitContextOps
{

 unsigned (*prepareHeader)(CommKitContext*, struct CommKitTargetInfo*);

  // returns >0 for successful send, 0 to advance to next state, or negative error code
   int (*sendData)(CommKitContext*, struct CommKitTargetInfo*);

   // return >= for success, 0 to advance, or negative beegfs error code
   int (*recvHeader)(CommKitContext*, struct CommKitTargetInfo*);

   // return >= for success, 0 to advance, or negative beegfs error code
   int (*recvData)(CommKitContext*, struct CommKitTargetInfo*);

   int retryFlags;

 //-----------
  /*
   enum CKTargetBadAction (*selectedTargetBad)(CommKitContext*, struct CommKitTargetInfo*,
      const CombinedTargetState*);

  

  

   
   

   void (*printSendDataDetails)(CommKitContext*, struct CommKitTargetInfo*);
   void (*printSocketDetails)(CommKitContext*, struct CommKitTargetInfo*);

   
   const char* logContext;
  */
};

static const struct CommKitContextOps readfileOps = {
   .prepareHeader = __commkit_readfile_prepareHeader,
   .sendData=nullptr,
   .recvHeader=nullptr,
   .recvData = __commkit_readfile_recvdata,

//    .printSocketDetails = __commkit_readfile_printSocketDetails,

//    .retryFlags = CK_RETRY_BUDDY_FALLBACK,
//    .logContext = "readfileV2 (communication)",
};

static const struct CommKitContextOps writefileOps = {
   // .prepareHeader = __commkit_writefile_prepareHeader,
   // .sendData = __commkit_writefile_sendData,
   // .recvHeader = __commkit_writefile_recvHeader,

   // .printSendDataDetails = __commkit_writefile_printSendDataDetails,
   // .printSocketDetails = __commkit_writefile_printSocketDetails,

   // .retryFlags = CK_RETRY_LOOP_EAGAIN,
   // .logContext = "writefile (communication)",
};




struct FileOpState
{
   struct CommKitTargetInfo base; 

   // data for spefic modes (will be assigned by the corresponding modes)
   size_t transmitted; // how much data has been transmitted already
   size_t toBeTransmitted; // how much data has to be transmitted
   size_t totalSize; // how much data was requested

   // data for all modes

   // (assigned by the state-creator)
                                 // struct iov_iter data;
                                 // loff_t offset; // target-node local offset
   //替换成下面的
   struct iovec data; // 使用 iovec 代替 iov_iter
   int64_t offset; // 目标节点的本地偏移量

   bool firstWriteDoneForTarget; /* true if a chunk was previously written to this target in
                                          this session; used for the session check */
   bool receiveFileData;         /* if false, receive the int64 fragment length, else the fragment */

   // result data
   int64_t expectedNodeResult; // the amount of data that we wanted to read

    // #ifdef BEEGFS_NVFS
    RdmaInfo* rdmap;//$$%
    // #endif
};

struct FileOpVecState {
   FileOpState base;

   // struct iov_iter data;
   struct iovec data;//RDMA传的消息

};
#define MAX_POLL_SOCKS 10 //$$%
// 定义链表结构体
struct PollState {
    list_head_t list;
    // 可以添加其他成员
    struct pollfd fds[MAX_POLL_SOCKS];   // 用于存储 pollfd 结构的数组 //$$%
   nfds_t nfds;   //当前添加到 fds 数组中的套接字数量 //$$%
};

struct CommKitContext
{
   const struct CommKitContextOps* ops;

//    App* app;
//    Logger* log;
   void* private_data; //@@ 

   RemotingIOInfo* ioInfo;

   struct list_head* targetInfoList;

   unsigned numRetryWaiters; // number of states with a communication error
   unsigned numDone; // number of finished states

   unsigned numAcquiredConns; // number of acquired conns to decide waiting or not (avoids deadlock)
   unsigned numUnconnectable; // states that didn't get a conn this round (reset to 0 in each round)
   unsigned numBufferless; // states that couldn't acquire a header buffer

   bool pollTimedOut;
   bool pollTimeoutLogged;
   bool connFailedLogged;

   PollState pollState;
   unsigned numPollSocks;

   unsigned currentRetryNum; // number of used up retries (in case of comm errors)
   unsigned maxNumRetries; // 0 if infinite number of retries enabled
    // #ifdef BEEGFS_NVFS
       int gpudRc;//$$%
    // #endif
};

static void readfile_nextIter(CommKitContext* context, FileOpState* state)//表示当前文件操作的状态，可能包含操作的结果、数据指针等信息。
{  //获取完整的状态结构体：
   struct FileOpVecState* vecState = container_of_v2<FileOpVecState,FileOpState>(state,offsetof(FileOpVecState, base));
   //复制数据指针：这样做的目的可能是为了确保 state 结构体包含最新的数据指针，以便在迭代的下一个阶段使用。
   state->data = vecState->data;
}

struct ReadfileIterOps
{
   void (*nextIter)(CommKitContext*, FileOpState*);
   void (*prepare)(CommKitContext*, FileOpState*);
};

//--------------------- 初始化 轮询状态  PollState_init(&context.pollState)


static inline void PollState_init(PollState* state)
{
   INIT_LIST_HEAD(&state->list);
}

static inline void PollState_addSocket(PollState* state, Socket* socket, short events)
{
   list_add_tail(&socket->poll._list, &state->list);
   socket->poll._events = events;
   socket->poll.revents = 0;
}

// 3. 准备通信 __commkit_prepare_generic-----------------------------------------------------------------------

//------@@ 3. 准备通信 __commkit_prepare_generic-----------------------------------------------------------------------

    // 3.1 为头部缓冲区分配内存

    static void* allocHeaderBuffer(size_t size, bool allowWait) {
        void* buffer;
        if (allowWait) {
            buffer = malloc(size); // 如果允许等待，使用 malloc 分配内存
        } else {
            buffer = malloc(size); // 在用户态程序中，malloc 总是可以等待的，所以这里的逻辑可能需要根据实际情况调整
        }
        return buffer;
    }

    static void freeHeaderBuffer(void* header) {
        free(header); // 使用 free 释放内存
    }

    // 初始化一个 Node 结构体的函数
    Node* Node_init(NodeType nodeType, const char* id, uint16_t numID, unsigned short portUDP) {
        Node* node = (Node*)malloc(sizeof(Node));
        if (node == NULL) {
            return nullptr; // 内存分配失败
        }

        // 初始化字符串 ID
        node->id = strdup(id);
        if (node->id == nullptr) {
            free(node);
            return nullptr; // 内存分配失败
        }

        // 初始化数字 ID
        node->numID.value = numID;

        // 设置节点类型
        node->nodeType = nodeType;

        // 初始化日志消息用的 ID，初始为 NULL
        node->nodeIDWithTypeStr = nullptr;

        // 初始化连接池，假设为 NULL 或者分配内存
            //  node->connPool = NULL;

        // 设置 UDP 端口
        node->portUDP = portUDP;

        // 初始化最后一次心跳接收时间为 0
                // node->lastHeartbeatT = 0;
                // 将 node->lastHeartbeatT 设置为 0
        node->lastHeartbeatT.tv_sec = 0;
        node->lastHeartbeatT.tv_nsec = 0;

        // 初始化活跃状态为 false
        node->isActive = true;//false;

        // 初始化互斥锁和条件变量，这里需要具体的初始化函数
                // Mutex_init(&node->mutex);
        pthread_mutex_init(&node->mutex, nullptr);
        Condition_init(&node->changeCond);

        // 初始化引用计数为 1
        // node->references = 1;
        node->references.store(1, std::memory_order_relaxed);

                // 初始化红黑树元素，通常设置为 RB_ROOT
                // node->rbTreeElement = RB_ROOT;

        // 初始化红黑树元素，通常设置为 RB_ROOT
        RB_tree_init(&(node->_nodeTree.rbTreeElement));
            

        return node;
    }



    /**
     * This is a helper to have only one call for the typical targetMapper->getNodeID() and following
     * referenceNode() calls.
     *
     * Note: remember to call releaseNode().
     *
     * @param targetMapper where to resolve the given targetID
     * @param outErr will be set to FhgfsOpsErr_UNKNOWNNODE, _UNKNOWNTARGET, _SUCCESS (may be NULL)
     * @return NULL if targetID is not mapped or if the mapped node does not exist in the store.
     */
    // Node* NodeStoreEx_referenceNodeByTargetID(NodeStoreEx* this, uint16_t targetID,TargetMapper* targetMapper, FhgfsOpsErr* outErr)
    Node* NodeStoreEx_referenceNodeByTargetID()
    {
        //连targetid 都不用传
        NumNodeID nodeID;
        nodeID.value=8;//@@ 162 存储节点ID=1 查询得知 // 217 dell01 : ID=191 ； 162 server :ID=1 ;224:xfusion3 241206 更新为120
        // 假设的节点类型、ID 和端口
            NodeType nodeType = NODETYPE_Storage;
            const char* id ="ubuntu-PowerEdge-R750xa" ;//@@ 162： "ubuntu-PowerEdge-R750xa"; 217： "dell01"
            uint16_t nodenumID = 8;//@@ 存储节点的NodeID   217： 191 ； 162： 1 ;224:xfusion3 241206 更新为120
            unsigned short portUDP = 8003;//@@

        Node* node= Node_init(nodeType, id, nodenumID, portUDP);
        if (node == NULL) {
                fprintf(stderr, "Failed to initialize storage node\n");
                return nullptr;
            }
            return node;
                    /*
                    nodeID = TargetMapper_getNodeID(targetMapper, targetID);
        

                    if(!nodeID.value)
                    {
                       // SAFE_ASSIGN(outErr, FhgfsOpsErr_UNKNOWNTARGET);
                       printf("nodeID.value ==0 \n");
                       return NULL;
                    }

                    node = NodeStoreEx_referenceNode(this, nodeID);
        

                    if(!node)
                    {
                       SAFE_ASSIGN(outErr, FhgfsOpsErr_UNKNOWNNODE);
                       return NULL;
                    }

                    SAFE_ASSIGN(outErr, FhgfsOpsErr_SUCCESS);
                    */
    
    }



//---------------准备信息 头部 __commkit_readfile_prepareHeader--------------------------------------------------------
// 3.4 准备头部信息------------------------------------------------------------------------

//(1) ReadLocalFileV2Msg_initFromSession

   #define NETMSGTYPE_ReadLocalFileV2                 3019

   struct ReadLocalFileV2Msg
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
   };
void ReadLocalFileV2Msg_serializePayload(NetMessage* nm, SerializeCtx* ctx);
   const struct NetMessageOps ReadLocalFileV2Msg_Ops = {
      .serializePayload   = ReadLocalFileV2Msg_serializePayload,
    //   .deserializePayload = _NetMessage_deserializeDummy,
   //    .processIncoming = NetMessage_processIncoming,
   //    .getSupportedHeaderFeatureFlagsMask = NetMessage_getSupportedHeaderFeatureFlagsMask,
   };

void ReadLocalFileV2Msg_serializePayload(NetMessage* nm, SerializeCtx* ctx)
{
    //std::cout<<"go into ReadLocalFileV2Msg_serializePayload \n";
   ReadLocalFileV2Msg* thisCast = (ReadLocalFileV2Msg*)nm;
   //std::cout<<"---------- 1 --------------\n";

   // offset
   //std::cout<<"thisCast->offset = "<<thisCast->offset<<"\n";//-1585904400
   Serialization_serializeInt64(ctx, thisCast->offset);
   //std::cout<<"---------- 2 --------------\n";


   // count
   Serialization_serializeInt64(ctx, thisCast->count);
   //std::cout<<"---------- 3 --------------\n";


   // accessFlags
   Serialization_serializeUInt(ctx, thisCast->accessFlags);
   //std::cout<<"---------- 4 --------------\n";



   // fileHandleID
   Serialization_serializeStrAlign4(ctx, thisCast->fileHandleIDLen, thisCast->fileHandleID);
   //std::cout<<"---------- 5 --------------\n";


   // clientNumID
   NumNodeID_serialize(ctx, &thisCast->clientNumID);
   //std::cout<<"---------- 6 --------------\n";


   // pathInfo
   PathInfo_serialize(ctx, thisCast->pathInfoPtr);
   
   //std::cout<<"---------- 7 --------------\n";


   // targetID
   Serialization_serializeUShort(ctx, thisCast->targetID);
   //std::cout<<"out of ReadLocalFileV2Msg_serializePayload \n";
}

   void ReadLocalFileV2Msg_init(ReadLocalFileV2Msg* rmsg)
   {
     //std::cout<<"go into ReadLocalFileV2Msg_init \n";
      NetMessage_init(&rmsg->netMessage, NETMSGTYPE_ReadLocalFileV2, &ReadLocalFileV2Msg_Ops);
   }


   /**
    * @param sessionID just a reference, so do not free it as long as you use this object!
    */
   void ReadLocalFileV2Msg_initFromSession(ReadLocalFileV2Msg* rmsg,
      NumNodeID clientNumID, const char* fileHandleID, uint16_t targetID, PathInfo* pathInfoPtr,
      unsigned accessFlags, int64_t offset, int64_t count)
   {
    //std::cout<<"go into ReadLocalFileV2Msg_initFromSession \n";
      ReadLocalFileV2Msg_init(rmsg);
      //std::cout<<"---------------------11-------\n";

      rmsg->clientNumID = clientNumID;

      rmsg->fileHandleID = fileHandleID;
      rmsg->fileHandleIDLen = strlen(fileHandleID);
      //std::cout<<"---------------------12-------\n";


      rmsg->targetID = targetID;

      rmsg->pathInfoPtr = pathInfoPtr;

      rmsg->accessFlags = accessFlags;

      rmsg->offset = offset;
      rmsg->count = count;
   }

   //(2)

   /**
    * @param targetID this has to be an actual targetID (not a groupID).
    */
   void NetMessage_setMsgHeaderTargetID(NetMessage* nm, uint16_t targetID)
   {
      nm->msgHeader.msgTargetID = targetID;
   }

                // static inline void beegfs_iov_iter_clear(struct iov_iter *iter)
    static inline void beegfs_iov_iter_clear(struct iovec *iter)
    {
        iter->iov_base = NULL; // 将基地址设置为NULL
        iter->iov_len = 0;    // 将长度设置为0
    }
            #define READLOCALFILEMSG_FLAG_SESSION_CHECK       1 /* if session check infos should be done */

   #define READLOCALFILEMSG_FLAG_DISABLE_IO          2 /* disable read syscall for net bench */
    #define BEEGFS_COMMKIT_MSGBUF_SIZE         4096
/**
 * Add another flag without clearing the previously set flags.
 *
 * Note: The receiver will reject this message if it doesn't know the given feature flag.
 */
void NetMessage_addMsgHeaderFeatureFlag(NetMessage* nm, unsigned flag)
{
   nm->msgHeader.msgFeatureFlags |= flag;
}
// ------------ __commkit_readfile_prepareHeader  准备头部信息------------------------------------------------------------------------

static unsigned __commkit_readfile_prepareHeader(CommKitContext* context,struct CommKitTargetInfo* info)//准备头部消息
{   
    std::cout<<"go into __commkit_readfile_prepareHeader \n";
                                    //    Node* localNode = App_getLocalNode(context->app);
    NumNodeID localNodeNumID ;
    localNodeNumID.value=2;         //@@ Node_getNumID(localNode);//this->numID;
    // 162 sever 对应的164 client ID =12 // 224 xfusion3 [ID: 6]  //217 dell01 [ID: 5]


   FileOpState* currentState = container_of_v2<FileOpState,CommKitTargetInfo>(info, offsetof(FileOpState, base));
//    currentState->offset=0;//这个参数为什么怎么改也不影响结果
    //std::cout<<"输出 currentState 的参数列表 ：\n";
   {
    //  std::cout<<"transmitted = "<<currentState->transmitted<<"\n";
    //  std::cout<<"toBeTransmitted = "<<currentState->toBeTransmitted<<"\n";
    //  std::cout<<"totalSize = "<<currentState->totalSize<<"\n";  //@@ 321 是哪里来的
    //  std::cout<<"offset = "<<currentState->offset<<"\n";        //@@
    //  std::cout<<"firstWriteDoneForTarget = "<<currentState->firstWriteDoneForTarget<<"\n";
    //  std::cout<<"receiveFileData = "<<currentState->receiveFileData<<"\n";
    //  std::cout<<"expectedNodeResult = "<<currentState->expectedNodeResult<<"\n";//数值奇怪
    //  std::cout<<"data = ";
    //  for (size_t i = 0; i < sizeof(int64_t); ++i) {
    //     // std::cout << std::hex << static_cast<int>(((char *)(currentState->data.iov_base))[i]) << " ";
    //     std::cout << std::hex << static_cast<int>((((char *)currentState->data.iov_base))[i]) << " ";

    //     }
      //std::cout<<"\n iter->iov_len ="<<currentState->data.iov_len<<"\n";//数值奇怪
    //  std::cout<<"transmitted = "<<currentState->transmitted<<"\n";
    //  std::cout<<"transmitted = "<<currentState->transmitted<<"\n";


   }
   //std::cout<<"------------------1--------------------\n";
   struct ReadfileIterOps* ops = (ReadfileIterOps*)(context->private_data); 
   //std::cout<<"------------------2--------------------\n";

   NetMessage *netMessage = nullptr;
   ReadLocalFileV2Msg readV2Msg;//消息类型
//    ReadLocalFileRDMAMsg readRDMAMsg;//消息类型 $$%

   //跳过 下面这个RDMA 分支
   std::cout<<" currentState->offset = "<< currentState->offset<<"\n";//这个参数也是正确的

   std::cout<<" currentState->totalSize = "<<currentState->totalSize<<"\n";//这个参数是正确的
    //$$%
    // #ifdef BEEGFS_NVFS
    std::cout<<"go into BEEGFS_NVFS \n";
    ReadLocalFileRDMAMsg readRDMAMsg;

    currentState->rdmap = NULL;
    std::cout<<"context->gpudRc = "<<context->gpudRc<<"\n";
    if (context->gpudRc > 0)
    {   
      
//    FileOpState* currentState = container_of_v2<FileOpState,CommKitTargetInfo>(info, offsetof(FileOpState, base));
        std::cout<<"go out 1 \n";

        struct FileOpVecState* vecState = container_of_v2<FileOpVecState,FileOpState>(currentState, offsetof(FileOpVecState, base));
        // struct FileOpVecState* vecState = container_of(currentState, struct FileOpVecState, base);
        currentState->rdmap = RdmaInfo_mapRead(&vecState->data, info->socket);
        std::cout<<"currentState->rdmap = "<<currentState->rdmap<<"\n";
    }
    if (context->gpudRc < 0 || IS_ERR(currentState->rdmap))
    {
        int st = currentState->rdmap != NULL? PTR_ERR(currentState->rdmap) : context->gpudRc;
        info->state = CommKitState_CLEANUP;
        info->nodeResult = (st == -ENOMEM)? -FhgfsOpsErr_OUTOFMEM : -FhgfsOpsErr_INVAL;
        currentState->rdmap = NULL;
        std::cout<<"go out 2 \n";
        return 0;
    }

    // prepare message
    std::cout<<"// prepare message \n";

    if (!currentState->rdmap)
    // #endif //  BEEGFS_NVFS
   {
   std::cout<<"----prepare message -2--------------------\n";

      ReadLocalFileV2Msg_initFromSession(&readV2Msg, localNodeNumID,
         context->ioInfo->fileHandleID, info->targetID, context->ioInfo->pathInfo, //
         context->ioInfo->accessFlags, currentState->offset, currentState->totalSize);

      netMessage = &readV2Msg.netMessage;
   }
   //跳过 下面这个RDMA 分支 //$$%
    // #ifdef BEEGFS_NVFS
    else
    {
   std::cout<<"----prepare message -4--------------------\n";

        ReadLocalFileRDMAMsg_initFromSession(&readRDMAMsg, localNodeNumID,
            context->ioInfo->fileHandleID, info->targetID, context->ioInfo->pathInfo,
            context->ioInfo->accessFlags, currentState->offset, currentState->totalSize,
            currentState->rdmap);

        netMessage = &readRDMAMsg.netMessage;
    }
    // #endif // BEEGFS_NVFS
    
   std::cout<<"----prepare message -5--------------------\n";

   NetMessage_setMsgHeaderTargetID(netMessage, info->selectedTargetID);
// NetMessage_addMsgHeaderFeatureFlag(netMessage, READLOCALFILEMSG_FLAG_SESSION_CHECK);
   //std::cout<<"------------------4--------------------\n";
    //std::cout<<"currentState->firstWriteDoneForTarget = "<<currentState->firstWriteDoneForTarget<<"\n";
    /*
        //先选择不使用 会话检查
                //    Config* cfg = App_getConfig(context->app);
                //    if (currentState->firstWriteDoneForTarget && cfg->sysSessionChecksEnabled)// this->sysSessionChecksEnabled;会话检查
                //       NetMessage_addMsgHeaderFeatureFlag(netMessage, READLOCALFILEMSG_FLAG_SESSION_CHECK);//this->msgHeader.msgFeatureFlags |= flag;
        //选择 不使用buddy mirror
                //    if(context->ioInfo->pattern->patternType == STRIPEPATTERN_BuddyMirror)
                //    {
                //       NetMessage_addMsgHeaderFeatureFlag(netMessage, READLOCALFILEMSG_FLAG_BUDDYMIRROR);

                //       if(info->useBuddyMirrorSecond)
                //          NetMessage_addMsgHeaderFeatureFlag(netMessage,READLOCALFILEMSG_FLAG_BUDDYMIRROR_SECOND);
                //    }
        //直接选择不禁用磁盘 IO ，注释
                //    if(context->app->netBenchModeEnabled )//用于控制是否启用网络基准测试模式。这个变量可以通过 procfs 接口进行修改，以禁用服务器端的磁盘读写操作。
                //       NetMessage_addMsgHeaderFeatureFlag(netMessage, READLOCALFILEMSG_FLAG_DISABLE_IO);
    */
      
   //只有对要发送的header进行序列化的操作
   NetMessage_serialize(netMessage, info->headerBuffer, BEEGFS_COMMKIT_MSGBUF_SIZE);

   //std::cout<<"------------------5--------------------\n";

   currentState->transmitted = 0;
//    currentState->toBeTransmitted = 0;
   currentState->receiveFileData = false;
   beegfs_iov_iter_clear(&currentState->data);
   //std::cout<<"------------------6--------------------\n";


   if(ops->prepare)//null
     {
         ops->prepare(context, currentState);  // 这个prepare 的具体实现在哪里
   //std::cout<<"------------------7--------------------\n";

     }
   //std::cout<<"------------------8--------------------\n";


   return NetMessage_getMsgLength(netMessage);
}

//RDMA 

static bool __commkit_prepare_generic(CommKitContext* context, struct CommKitTargetInfo* info)
{   
    std::cout<<"go into __commkit_prepare_generic \n";
    FileOpState* currentState = container_of_v2<FileOpState,CommKitTargetInfo>(info,offsetof(FileOpState, base));

    // std::cout<<"1----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";
    //std::cout<<"ioinfo->state ="<<info->state<<"\n";
                           /*
                            TargetMapper* targetMapper = App_getTargetMapper(context->app);//this->targetMapper;
                            NodeStoreEx* storageNodes = App_getStorageNodes(context->app);//this->storageNodes;
                            NodeConnPool* connPool;
                            #ifdef BEEGFS_NVFS
                                FileOpState* currentState = container_of(info, struct FileOpState, base);
                                struct iov_iter* data = NULL;
                            #endif
                                DevicePriorityContext devPrioCtx =
                                {
                                    .maxConns = 0,
                                                        // #ifdef BEEGFS_NVFS
                                                        //       .gpuIndex = -1,
                                                        // #endif
                                };
    

                            #ifdef BEEGFS_NVFS
                                // only set data if this is a storage call, NVFS ops are available and the
                                // iov is an ITER_IOVEC.
                                if (context->ioInfo && context->ioInfo->nvfs)
                                {
                                    struct FileOpVecState* vs = container_of(currentState, struct FileOpVecState, base);
                                    if (beegfs_iov_iter_is_iovec(&vs->data))
                                        data = &vs->data;
                                }
                            #endif
                           */

// 如果已经获取了连接，则不等待
   bool allowWaitForConn = !context->numAcquiredConns;// numAcquiredConns=0 则这个变量的值是true // don't wait if we got at least
    // one conn already (this is important to avoid a deadlock between racing commkit processes)

   info->socket = nullptr;
   info->nodeResult = -FhgfsOpsErr_COMMUNICATION;
   info->selectedTargetID = info->targetID;//statelist 中存储的信息（1）

// 为头部缓冲区分配内存
//std::cout<<"--为头部缓冲区分配内存 \n";
                         //info->headerBuffer = allocHeaderBuffer(allowWaitForConn ? GFP_NOFS : GFP_NOWAIT);
    size_t bufferSize = 1024; // 假设我们需要分配的缓冲区大小
    info->headerBuffer= (char *)allocHeaderBuffer(bufferSize, allowWaitForConn);
    if (info->headerBuffer == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        context->numBufferless += 1;
        printf("分配头部缓冲区失败\n");

        return false;
    }

    // std::cout<<"2----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";

// 选择正确的目标ID并获取目标状态

   if(!context->ioInfo) // 进入这个分支
     {
         info->selectedTargetID = info->targetID;
         //std::cout<<"info->targetID = "<<info->targetID<<"\n";
      cout << "设置target ID = " << info->selectedTargetID << "\n";

     }

                            /*
                                else if(StripePattern_getPatternType(context->ioInfo->pattern) == STRIPEPATTERN_BuddyMirror)
                                { // given targetID refers to a buddy mirror group
                                    MirrorBuddyGroupMapper* mirrorBuddies = App_getStorageBuddyGroupMapper(context->app);

                                    info->selectedTargetID = info->useBuddyMirrorSecond ?
                                        MirrorBuddyGroupMapper_getSecondaryTargetID(mirrorBuddies, info->targetID) :
                                        MirrorBuddyGroupMapper_getPrimaryTargetID(mirrorBuddies, info->targetID);

                                    if(unlikely(!info->selectedTargetID) )
                                    { // invalid mirror group ID
                                        Logger_logErrFormatted(context->log, context->ops->logContext,
                                            "Invalid mirror buddy group ID: %hu", info->targetID);
                                        info->nodeResult = -FhgfsOpsErr_UNKNOWNTARGET;
                                        goto cleanup;
                                    }
                                }
                            */

// check target state 我认为可以跳过 ，想办法获取 target 的状态 ，确认他是正常运行的即可
                            /*
                                
                                TargetStateStore* stateStore = App_getTargetStateStore(context->app);//this->targetStateStore;
                                CombinedTargetState targetState;
                                bool getStateRes = TargetStateStore_getState(stateStore, info->selectedTargetID,
                                    &targetState);

                                if(unlikely( !getStateRes ||
                                    (targetState.reachabilityState == TargetReachabilityState_OFFLINE) ||
                                    ( context->ioInfo &&
                                        (StripePattern_getPatternType(context->ioInfo->pattern) ==
                                        STRIPEPATTERN_BuddyMirror) &&
                                        (targetState.consistencyState != TargetConsistencyState_GOOD) ) ) )
                                { // unusable target state, retry details will be handled in retry handler
                                    int targetAction = CK_SKIP_TARGET;

                                    info->state = CommKitState_CLEANUP;

                                    if(context->ops->selectedTargetBad)
                                        targetAction = context->ops->selectedTargetBad(context, info, &targetState);

                                    if(targetAction == CK_SKIP_TARGET)
                                        goto error;
                                }
                            */
// 获取目标节点引用
// get the target-node reference

                                // //传入参数的值确定
                                // storageNodes //用于的到 node
                                // info->selectedTargetID=3;// beegfs-ctl 查询得知
                                // //targetMapper 可以不用 
                                // //info->node 通过storageNodes 得到 或者直接根据结构体的参数一一赋值 选择后者
                                // FhgfsOpsErr resolveErr;
                                // info->node = NodeStoreEx_referenceNodeByTargetID(storageNodes,info->selectedTargetID, targetMapper, &resolveErr);//通过ID 获取节点
   
   info->node = NodeStoreEx_referenceNodeByTargetID();
   if(!info->node) 
   { // unable to resolve targetID
      info->nodeResult = -1;//负数表示错误 //-resolveErr;
      printf("[__commkit_prepare_generic] 获取目标节点引用失败 unable to resolve targetID  \n");
      goto cleanup;
   }

    // std::cout<<"3----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";

// 从连接池中获取 socket ，建立连接 
                                    /*   
                                     connPool = Node_getConnPool(info->node);
                                        #ifdef BEEGFS_NVFS
                                        // perform first test for GPUD
                                        context->gpudRc = 0;// 对 GPUD 进行首次测试

                                        if (data)
                                            context->gpudRc = RdmaInfo_detectNVFSRequest(&devPrioCtx, data);
                                        #endif
                                            info->socket = NodeConnPool_acquireStreamSocketEx(connPool, allowWaitForConn, &devPrioCtx);
                                        if(!info->socket)
                                        { // no conn available => error or didn't want to wait
                                        // 没有可用连接 => 错误或不想等待
                                            if(likely(!allowWaitForConn) )
                                            { // just didn't want to wait => keep stage and try again later
                                            // 只是没有想等待 => 保持状态并稍后重试
                                                Node_put(info->node);
                                                info->node = NULL;

                                                context->numUnconnectable++;
                                                goto error;
                                            }
                                            else
                                            { // connection error// 连接错误
                                                if(!context->connFailedLogged)
                                                { // no conn error logged yet
                                                    if (fatal_signal_pending(current))
                                                    Logger_logFormatted(context->log, Log_DEBUG, context->ops->logContext,
                                                        "Connect to server canceled by pending signal: %s",
                                                        Node_getNodeIDWithTypeStr(info->node) );
                                                    else
                                                    Logger_logFormatted(context->log, Log_WARNING, context->ops->logContext,
                                                        "Unable to connect to server: %s",
                                                        Node_getNodeIDWithTypeStr(info->node) );
                                                }

                                                context->connFailedLogged = true;
                                                goto cleanup;
                                            }
                                        }
                                    */
            //NodeConnPool_acquireStreamSocketEx(connPool, allowWaitForConn, &devPrioCtx); 替换成如下 
    {
        printf("--------- 1.创建 socket -------------\n");
        
        struct in_addr srcAddr;
        NicAddress* nicAddr = (NicAddress*)malloc(sizeof(NicAddress));
        NicAddressStats* srcRdma = (NicAddressStats*)malloc(sizeof(NicAddressStats));

        const char* LocalRdmaIP = "192.168.3.212";       //本地dell02的RDMA IP,   网口名为 enp130s0np0
                                                        //@@本地 dell01 的RDMA IP : 192.168.0.203 ,   网口名为 enp152s0np0
                                                        // 164 Client: ens5f0np0 192.168.5.209 mlx5_0
                                                        //164 client ens4f0np0 192.168.3.212 mlx5_2

                                                        

        if (inet_pton(AF_INET, LocalRdmaIP, &srcAddr) <= 0) {    // 将点分十进制的IP地址转换为用于网络传输的数值格式。
            perror("Invalid IP address");
            return -1;
        }
        printf("查看本主机IP情况 LocalIP %s and tranfer to %u\n", LocalRdmaIP, srcAddr.s_addr);
        nicAddr->ipAddr = srcAddr;
        nicAddr->nicType = NICADDRTYPE_RDMA;
        strncpy(nicAddr->name, "ens4f0np0", IFNAMSIZ);  //@@本地 RDMA 端口名 dell02 : enp130s0np0
                                                        //dell 01: enp152s0np0
                                                        //164 client ens5f0np0 192.168.5.209 mlx5_0
                                                        //164 client ens4f0np0 192.168.3.212 mlx5_2

 /*
    #ifdef BEEGFS_RDMA
        printf("-------------检测RDMA设备-----------------\n");
        struct ibv_device **dev_list = (ibv_device**)NULL;
        int num_devices = 0;

        dev_list = ibv_get_device_list(&num_devices);    //获取RDMA列表
        if (!dev_list) {
            perror("Failed to get IB devices list");
            return -1;
        }
        printf("Found %d RDMA device(s):\n", num_devices);
            for (int i = 0; i < num_devices; ++i) {
                printf("  Device %d: %s\n", i, ibv_get_device_name(dev_list[i]));
            }
        
        struct ibv_device *ib_dev = (ibv_device*)NULL;
        for (int i = 0; i < num_devices; ++i) {
            if (strcmp(ibv_get_device_name(dev_list[i]), "rocep130s0") == 0) {       //RDMA设备名来寻找设备对象， 用 ibv_devices 或 ls /sys/class/infiniband 查看
                ib_dev = dev_list[i];
                break;
            } 
        }

        if (!ib_dev) {
            fprintf(stderr, "RDMA device rocep130s0 not found\n");
            ibv_free_device_list(dev_list);
            return -1;
        }
        printf("获取RDMA设备rocep130s0成功\n");//NOTE：这一句就没有输出

        nicAddr->ibdev = ib_dev;  // 若有实际的 ibv_device 可填充
        ibv_free_device_list(dev_list);
    #endif
 */

        NicAddressStats_init(srcRdma, nicAddr);

        info->socket = (Socket*)RDMASocket_construct(srcAddr, srcRdma);  //构造一个RDMA socket
        if(!info->socket)
        { // no conn available => error or didn't want to wait
            printf("----------------获取RDMA socket失败!----------------------\n");
            info->state = CommKitState_CLEANUP;
            return false;
        }
        else{
            printf("----------------获取RDMA socket成功!----------------------\n");
        }

        printf("--------- RDMA 没有对应的这个阶段 设置 socket 接收缓冲区-------------\n");
        

    // std::cout<<"4----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";

        printf("------------------- 2. 尝试通过IP连接-------------------\n");
        struct in_addr destAddr;
        const char* RemoteRdmaIP = "192.168.3.216";   //@@存储服务器xfusion3的RDMA IP 192.168.0.218,网口名 ens2np0
                                                        // 162 server 的 RDMA IP : 192.168.5.210 , ens4f0np0
                                                        // 162 server 的 RDMA IP : 192.168.3.216 , ens4f1np1


        if (inet_pton(AF_INET, RemoteRdmaIP, &destAddr) <= 0) {
            perror("Invalid remote IP address");
            return -1;
        }
        printf("查看远端主机IP情况 RemoteIP %s and tranfer to %u\n", RemoteRdmaIP, destAddr.s_addr);

        unsigned short port = 8003;    //存储服务器的port
        printf("----------------开始根据IP与远端建立RDMA连接-----------------------\n");
        bool connectRes = info->socket->ops->connectByIP(info->socket, destAddr, port);
        if(connectRes){
            printf("尝试通过IP连接 连接成功 \n");
        }
        else
        {
            printf("尝试通过IP连接 连接失败 \n");
        }



        // 启用TCP Keepalive
        printf("------------------- RDMA 没有这个阶段  启用TCP Keepalive-------------------\n");
        

        // 这是 服务端的认证配置
        printf("-------------------   服务端的认证配置 -------------------\n");

        bool isflag= __NodeConnPool_applySocketOptionsConnected(info->socket);
        std::cout<<"[read 服务端认证配置已经完成]\n";
        if(isflag==true)
        {
            std::cout<<"__NodeConnPool_applySocketOptionsConnected 成功 ！！\n";

        }
        else
        {
            std::cout<<"__NodeConnPool_applySocketOptionsConnected 失败 ！！\n";

        }
    }
    // std::cout<<"6----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";

// 准备头部信息,
std::cout<<"--------- 准备头部信息 \n";
   info->headerSize = context->ops->prepareHeader(context, info);//
   if(info->headerSize == 0)
    {
        std::cout<<"[__commkit_prepare_generic ] info->headerSize == 0 \n";
        goto cleanup;
    }

   context->numAcquiredConns++;
   // 更新状态为发送头部
   //std::cout<<"---- 更新状态为发送头部----\n";
   info->state = CommKitState_SENDHEADER;
    // std::cout<<"7----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";

    //std::cout<<"ioinfo->state ="<<info->state<<"\n";
//std::cout<<"out of  __commkit_prepare_generic \n";

   return true;

cleanup:
// 释放头部缓冲区
   info->state = CommKitState_CLEANUP;
   return false;

error:
   freeHeaderBuffer(info->headerBuffer);
   info->headerBuffer = nullptr;
   return false;
   
}





// 原 TCP 的函数 
// static bool __commkit_prepare_generic(CommKitContext* context, struct CommKitTargetInfo* info)
// {   
//     // std::cout<<"go into __commkit_prepare_generic \n";
//     FileOpState* currentState = container_of_v2<FileOpState,CommKitTargetInfo>(info,offsetof(FileOpState, base));

//     // std::cout<<"1----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";
//     //std::cout<<"ioinfo->state ="<<info->state<<"\n";
//                            /*
//                             TargetMapper* targetMapper = App_getTargetMapper(context->app);//this->targetMapper;
//                             NodeStoreEx* storageNodes = App_getStorageNodes(context->app);//this->storageNodes;
//                             NodeConnPool* connPool;
//                             #ifdef BEEGFS_NVFS
//                                 FileOpState* currentState = container_of(info, struct FileOpState, base);
//                                 struct iov_iter* data = NULL;
//                             #endif
//                                 DevicePriorityContext devPrioCtx =
//                                 {
//                                     .maxConns = 0,
//                                                         // #ifdef BEEGFS_NVFS
//                                                         //       .gpuIndex = -1,
//                                                         // #endif
//                                 };
    

//                             #ifdef BEEGFS_NVFS
//                                 // only set data if this is a storage call, NVFS ops are available and the
//                                 // iov is an ITER_IOVEC.
//                                 if (context->ioInfo && context->ioInfo->nvfs)
//                                 {
//                                     struct FileOpVecState* vs = container_of(currentState, struct FileOpVecState, base);
//                                     if (beegfs_iov_iter_is_iovec(&vs->data))
//                                         data = &vs->data;
//                                 }
//                             #endif
//                            */

// // 如果已经获取了连接，则不等待
//    bool allowWaitForConn = !context->numAcquiredConns;// numAcquiredConns=0 则这个变量的值是true // don't wait if we got at least
//     // one conn already (this is important to avoid a deadlock between racing commkit processes)

//    info->socket = nullptr;
//    info->nodeResult = -FhgfsOpsErr_COMMUNICATION;
//    info->selectedTargetID = info->targetID;//statelist 中存储的信息（1）

// // 为头部缓冲区分配内存
// //std::cout<<"--为头部缓冲区分配内存 \n";
//                          //info->headerBuffer = allocHeaderBuffer(allowWaitForConn ? GFP_NOFS : GFP_NOWAIT);
//     size_t bufferSize = 1024; // 假设我们需要分配的缓冲区大小
//     info->headerBuffer= (char *)allocHeaderBuffer(bufferSize, allowWaitForConn);
//     if (info->headerBuffer == NULL) {
//         fprintf(stderr, "Memory allocation failed\n");
//         context->numBufferless += 1;
//         printf("分配头部缓冲区失败\n");

//         return false;
//     }

//     // std::cout<<"2----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";

// // 选择正确的目标ID并获取目标状态

//    if(!context->ioInfo) // 进入这个分支
//      {
//          info->selectedTargetID = info->targetID;
//          //std::cout<<"info->targetID = "<<info->targetID<<"\n";
//       cout << "设置target ID = " << info->selectedTargetID << "\n";

//      }

//                             /*
//                                 else if(StripePattern_getPatternType(context->ioInfo->pattern) == STRIPEPATTERN_BuddyMirror)
//                                 { // given targetID refers to a buddy mirror group
//                                     MirrorBuddyGroupMapper* mirrorBuddies = App_getStorageBuddyGroupMapper(context->app);

//                                     info->selectedTargetID = info->useBuddyMirrorSecond ?
//                                         MirrorBuddyGroupMapper_getSecondaryTargetID(mirrorBuddies, info->targetID) :
//                                         MirrorBuddyGroupMapper_getPrimaryTargetID(mirrorBuddies, info->targetID);

//                                     if(unlikely(!info->selectedTargetID) )
//                                     { // invalid mirror group ID
//                                         Logger_logErrFormatted(context->log, context->ops->logContext,
//                                             "Invalid mirror buddy group ID: %hu", info->targetID);
//                                         info->nodeResult = -FhgfsOpsErr_UNKNOWNTARGET;
//                                         goto cleanup;
//                                     }
//                                 }
//                             */

// // check target state 我认为可以跳过 ，想办法获取 target 的状态 ，确认他是正常运行的即可
//                             /*
                                
//                                 TargetStateStore* stateStore = App_getTargetStateStore(context->app);//this->targetStateStore;
//                                 CombinedTargetState targetState;
//                                 bool getStateRes = TargetStateStore_getState(stateStore, info->selectedTargetID,
//                                     &targetState);

//                                 if(unlikely( !getStateRes ||
//                                     (targetState.reachabilityState == TargetReachabilityState_OFFLINE) ||
//                                     ( context->ioInfo &&
//                                         (StripePattern_getPatternType(context->ioInfo->pattern) ==
//                                         STRIPEPATTERN_BuddyMirror) &&
//                                         (targetState.consistencyState != TargetConsistencyState_GOOD) ) ) )
//                                 { // unusable target state, retry details will be handled in retry handler
//                                     int targetAction = CK_SKIP_TARGET;

//                                     info->state = CommKitState_CLEANUP;

//                                     if(context->ops->selectedTargetBad)
//                                         targetAction = context->ops->selectedTargetBad(context, info, &targetState);

//                                     if(targetAction == CK_SKIP_TARGET)
//                                         goto error;
//                                 }
//                             */
// // 获取目标节点引用
// // get the target-node reference

//                                 // //传入参数的值确定
//                                 // storageNodes //用于的到 node
//                                 // info->selectedTargetID=3;// beegfs-ctl 查询得知
//                                 // //targetMapper 可以不用 
//                                 // //info->node 通过storageNodes 得到 或者直接根据结构体的参数一一赋值 选择后者
//                                 // FhgfsOpsErr resolveErr;
//                                 // info->node = NodeStoreEx_referenceNodeByTargetID(storageNodes,info->selectedTargetID, targetMapper, &resolveErr);//通过ID 获取节点
   
//    info->node = NodeStoreEx_referenceNodeByTargetID();
//    if(!info->node) 
//    { // unable to resolve targetID
//       info->nodeResult = -1;//负数表示错误 //-resolveErr;
//       printf("[__commkit_prepare_generic] 获取目标节点引用失败 unable to resolve targetID  \n");
//       goto cleanup;
//    }

//     // std::cout<<"3----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";

// // 从连接池中获取 socket ，建立连接 
//                                     /*   
//                                      connPool = Node_getConnPool(info->node);
//                                         #ifdef BEEGFS_NVFS
//                                         // perform first test for GPUD
//                                         context->gpudRc = 0;// 对 GPUD 进行首次测试

//                                         if (data)
//                                             context->gpudRc = RdmaInfo_detectNVFSRequest(&devPrioCtx, data);
//                                         #endif
//                                             info->socket = NodeConnPool_acquireStreamSocketEx(connPool, allowWaitForConn, &devPrioCtx);
//                                         if(!info->socket)
//                                         { // no conn available => error or didn't want to wait
//                                         // 没有可用连接 => 错误或不想等待
//                                             if(likely(!allowWaitForConn) )
//                                             { // just didn't want to wait => keep stage and try again later
//                                             // 只是没有想等待 => 保持状态并稍后重试
//                                                 Node_put(info->node);
//                                                 info->node = NULL;

//                                                 context->numUnconnectable++;
//                                                 goto error;
//                                             }
//                                             else
//                                             { // connection error// 连接错误
//                                                 if(!context->connFailedLogged)
//                                                 { // no conn error logged yet
//                                                     if (fatal_signal_pending(current))
//                                                     Logger_logFormatted(context->log, Log_DEBUG, context->ops->logContext,
//                                                         "Connect to server canceled by pending signal: %s",
//                                                         Node_getNodeIDWithTypeStr(info->node) );
//                                                     else
//                                                     Logger_logFormatted(context->log, Log_WARNING, context->ops->logContext,
//                                                         "Unable to connect to server: %s",
//                                                         Node_getNodeIDWithTypeStr(info->node) );
//                                                 }

//                                                 context->connFailedLogged = true;
//                                                 goto cleanup;
//                                             }
//                                         }
//                                     */
//             //NodeConnPool_acquireStreamSocketEx(connPool, allowWaitForConn, &devPrioCtx); 替换成如下 
//     {
//         printf("--------- 创建 socket -------------\n");
//         info->socket=(Socket*)StandardSocket_construct(PF_INET, SOCK_STREAM, 0);
//         if(info->socket==nullptr) //如果获取套接字失败，记录错误并返回通信错误。
//         { 
//             printf(" 获取套接字失败 \n");
//             return false;
//         }
//         else
//         {
//             printf(" 获取套接字成功 \n");
            
//         }


//         printf("--------- 设置 socket 接收缓冲区-------------\n");
//         int tcpbufLen;
//         socklen_t tcpbufLenSize = sizeof(tcpbufLen);
//         if (getsockopt(((StandardSocket*) (info->socket))->sock, SOL_SOCKET, SO_RCVBUF, &tcpbufLen, &tcpbufLenSize) < 0) {
//             std::cerr << "Failed to get socket receive buffer size" << std::endl;
//             return false;
//         }
//         else
//         std::cout<<"get socket receive buffer size = "<<tcpbufLen<<"\n";

//         int bufSize =tcpbufLen;//8192*70;
//                                 // if (bufSize > 0)// 如果缓冲区大小大于0，则设置套接字的接收缓冲区大小。
//                                 //    StandardSocket_setSoRcvBuf(stdSock, bufSize);// 这里会设置ssock->sock参数，很可能和上面跳过的内核函数有关
//         if(bufSize>0)
//         {
//             int val = bufSize;
//             if (setsockopt(((StandardSocket*) (info->socket))->sock, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val)) < 0) {
//                     std::cerr << "Failed to set socket receive buffer size" << std::endl;
//                     return false;
//             }
//             else
//                 std::cout<<"Succesfully set socket receive buffer size = "<<val<<"\n";
//         }

//     // std::cout<<"4----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";

//         printf("-------------------  尝试通过IP连接-------------------\n");
//         unsigned short port=8003; //@@ 8005 meta 8003 storage
//         NicAddress nicAddr[11]; //@@
//         initNicAddressArray_storage(nicAddr);//@@
//         bool connectRes;
//         connectRes = info->socket->ops->connectByIP((info->socket), nicAddr[2].ipAddr, port);//@@v  217: nicAddr[10];  162: nicAddr[4] 164TCP 用这个[3]
//         if(connectRes)
//         {
//             printf(" 尝试通过IP连接 连接成功 \n");

//         }
//         else
//         {
//             printf(" 尝试通过IP连接 连接失败 \n");
//         }

//         // 启用TCP Keepalive
//         printf("-------------------   启用TCP Keepalive-------------------\n");
//         int opt = 1;
//         if (setsockopt(((StandardSocket*)info->socket)->sock, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) < 0) {
//         perror("setsockopt failed");
//         exit(EXIT_FAILURE);
//         }

//         // 设置TCP Keepalive参数
//         int keepalive_time = 30; // 30秒
//         int keepalive_intvl = 5; // 每5秒发送一次Keepalive
//         int keepalive_probes = 3; // 最多发送3次Keepalive

//         if (setsockopt(((StandardSocket*)info->socket)->sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepalive_time, sizeof(keepalive_time)) < 0) {
//             perror("setsockopt TCP_KEEPIDLE failed");
//             exit(EXIT_FAILURE);
//         }

//         if (setsockopt(((StandardSocket*)info->socket)->sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepalive_intvl, sizeof(keepalive_intvl)) < 0) {
//             perror("setsockopt TCP_KEEPINTVL failed");
//             exit(EXIT_FAILURE);
//         }

//         if (setsockopt(((StandardSocket*)info->socket)->sock, IPPROTO_TCP, TCP_KEEPCNT, &keepalive_probes, sizeof(keepalive_probes)) < 0) {
//             perror("setsockopt TCP_KEEPCNT failed");
//             exit(EXIT_FAILURE);
//         }
//     // std::cout<<"5----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";

//         // 现在，TCP连接将保持活跃状态
//         printf("Connection established and Keepalive is enabled.\n");

//         // 这是 服务端的认证配置
//         printf("-------------------   服务端的认证配置 -------------------\n");

//         bool isflag= __NodeConnPool_applySocketOptionsConnected(info->socket);
//         std::cout<<"[read 服务端认证配置已经完成]\n";
//         if(isflag==true)
//         {
//             std::cout<<"__NodeConnPool_applySocketOptionsConnected 成功 ！！\n";

//         }
//         else
//         {
//             std::cout<<"__NodeConnPool_applySocketOptionsConnected 失败 ！！\n";

//         }
//     }
//     // std::cout<<"6----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";

// // 准备头部信息,
// std::cout<<"--------- 准备头部信息 \n";
//    info->headerSize = context->ops->prepareHeader(context, info);//
//    if(info->headerSize == 0)
//     {
//         std::cout<<"[__commkit_prepare_generic ] info->headerSize == 0 \n";
//         goto cleanup;
//     }

//    context->numAcquiredConns++;
//    // 更新状态为发送头部
//    //std::cout<<"---- 更新状态为发送头部----\n";
//    info->state = CommKitState_SENDHEADER;
//     // std::cout<<"7----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";

//     //std::cout<<"ioinfo->state ="<<info->state<<"\n";
// //std::cout<<"out of  __commkit_prepare_generic \n";

//    return true;

// cleanup:
// // 释放头部缓冲区
//    info->state = CommKitState_CLEANUP;
//    return false;

// error:
//    freeHeaderBuffer(info->headerBuffer);
//    info->headerBuffer = nullptr;
//    return false;
   
// }

// --------------

// 辅助函数，用于计算结构体的完整指针
static inline CommKitTargetInfo* get_container_of(list_head_t *ptr) {
    return (CommKitTargetInfo *)((char *)ptr - offsetof(CommKitTargetInfo, targetInfoList));
}

// // 遍历链表的函数
// void list_for_each_entry(CommKitTargetInfo **pos, list_head_t *head) {
//     *pos = get_container_of(head->next);
//     while (&(*pos)->targetInfoList != head) {
//         // 处理每个 info
//         printf("Processing info\n");

//         *pos = get_container_of((*pos)->targetInfoList.next);
//     }
// }

// Lifted from Linux 5.10
#if __has_attribute(__fallthrough__)
#define BEEGFS_FALLTHROUGH __attribute__((__fallthrough__))
#else
#define BEEGFS_FALLTHROUGH do {} while (0) /* FALLTHROUGH */
#endif


//------------------- 发送头部信息的函数  __commkit_sendheader_generic -------------------------------------

static void __commkit_sendheader_generic(CommKitContext* context,struct CommKitTargetInfo* info)
{
    //std::cout<<"go into __commkit_sendheader_generic \n";
   ssize_t sendRes;
    //不模拟超时错误 ，直接发送消息
                //    if(BEEGFS_SHOULD_FAIL(commkit_sendheader_timeout, 1) )
                //       sendRes = -ETIMEDOUT;
                //    else
                //    sendRes = Socket_send_kernel(info->socket, info->headerBuffer, info->headerSize, 0);
    
    fhgfs_sockaddr_in *to=nullptr;
    struct iovec iter;
    iter.iov_base=info->headerBuffer;
    iter.iov_len=info->headerSize;
    int iovcnt=1;
                // sendRes=info->socket->ops->sendto(info->socket, info->headerBuffer,  info->headerSize,0, to);
    sendRes=info->socket->ops->sendto(info->socket, &iter,  iovcnt,0, to);//发送头部消息字段


    //std::cout<<"[__commkit_sendheader_generic] 完成 sendto \n";
   if(sendRes != info->headerSize) 
   {
    //   Logger_logFormatted(context->log, Log_WARNING, context->ops->logContext,
    //      "Failed to send message to %s: %s", Node_getNodeIDWithTypeStr(info->node),
    //      Socket_getPeername(info->socket) );
        printf("[__commkit_sendheader_generic] sendRes != info->headerSize \n");
      info->state = CommKitState_SOCKETINVALIDATE;
      return;
   }

   info->state = CommKitState_SENDDATA; //state=2
   //std::cout<<"[] info->state ="<<  info->state<<"\n";
   info->headerSize = 0;
    //std::cout<<"out of  __commkit_sendheader_generic \n";
   //std::cout<<"[] info->state ="<<  info->state<<"\n";
      // 检查套接字是否正常
   int sockfd = ((StandardSocket*)info->socket)->sock;
   std::cout << "Socket file descriptor: " << sockfd << std::endl;


}

//---------------------- __commkit_add_socket_pollstate -------------------------------

static void __commkit_add_socket_pollstate(CommKitContext* context,struct CommKitTargetInfo* info, short pollEvents)
{   
    //std::cout<<"go into __commkit_add_socket_pollstate \n";
    //std::cout<<"[] info->state = "<<info->state<<"\n";
   PollState_addSocket(&context->pollState, info->socket, pollEvents);
   context->numPollSocks++;
    //std::cout<<"[] info->state = "<<info->state<<"\n";
    //std::cout<<"out of  __commkit_add_socket_pollstate \n";


}



//---------------------------------- __commkit_recvdata_generic ----------------------

            bool Node_getIsActive(Node* node)
            {
                bool isActive;

                            // Mutex_lock(&node->mutex);
                size_t mutex_res= pthread_mutex_lock(&node->mutex);

                if (mutex_res != 0) {
                        // 处理错误
                        perror("[Node_getIsActive] Mutex_lock failed");
                        exit(EXIT_FAILURE);
                }

                isActive = node->isActive;

                         // Mutex_unlock(&node->mutex);
                mutex_res= pthread_mutex_unlock(&node->mutex);

                if (mutex_res != 0) {
                        // 处理错误
                        perror("[Node_getIsActive] Mutex_unlock failed");
                        exit(EXIT_FAILURE);
                }

                return isActive;
            }


            static bool __commkit_prepare_io(CommKitContext* context, struct CommKitTargetInfo* info,int events)
            {   
                //std::cout<<"go into __commkit_prepare_io \n";
                             // if (fatal_signal_pending(current) || !Node_getIsActive(info->node))
                if (!Node_getIsActive(info->node))
                {
                    info->state = CommKitState_SOCKETINVALIDATE;
                    std::cout<<"[__commkit_prepare_io] 1.Node_getIsActive(info->node)==0 \n";
                    return false;
                }

                // check for a poll() timeout or error (all states have to be cancelled in that case)
                        // if(context->pollTimedOut || BEEGFS_SHOULD_FAIL(commkit_polltimeout, 1) )
                // 跳过 上面的
                if( context->pollTimedOut )
                {
                    info->nodeResult = -FhgfsOpsErr_COMMUNICATION;
                    info->state = CommKitState_SOCKETINVALIDATE;
                    std::cout<<"[__commkit_prepare_io] 2.context->pollTimedOut \n";
                    return false;
                }

            // // 检查 socket 是否已经关闭
            //     if (info->socket->is_closed()) { // 假设您有一个方法来检查 socket 是否关闭
            //         std::cerr << "[__commkit_prepare_io]4. Error: socket is closed" << std::endl;
            //         return false;
            //     }

                //std::cout<<"info->socket->poll.revents = "<<info->socket->poll.revents<<"\n"; //0 实际发生的事件是 
                //std::cout<<"info->socket->poll._events = "<<info->socket->poll._events<<"\n"; //1 要监听的事件是 POLLIN：有数据可读。这是最常见的事件，用于检查 socket 是否有数据可以被读取。
                //std::cout<<"预期的事件是 events = "<<events <<"\n"; //POLLIN =1 
                if(!(info->socket->poll.revents & events) )
                {
                    std::cout<<"[__commkit_prepare_io] 3.prepare go into __commkit_add_socket_pollstate \n";
                    __commkit_add_socket_pollstate(context, info, events);
                    return false;
                }

                return true;
            }



            /**
             * Receive with timeout.
             *
             * @return -ETIMEDOUT on timeout.
             */
                    //static inline ssize_t Socket_recvExactT(Socket* this, struct iov_iter *iter, size_t len, int flags, int timeoutMS)
            static inline ssize_t Socket_recvExactT(Socket* thissock, struct iovec *iter, size_t len, int flags, int timeoutMS)
            {
                size_t numReceivedBeforeError;

                return Socket_recvExactTEx(thissock, iter, len, flags, timeoutMS, &numReceivedBeforeError); //open.hpp 中有这个函数的定义
            }

            // 函数功能 限制 iov 的数据长度不超过 length, 实现截断的方式
            void user_iov_iter_truncate(struct iovec *iov, size_t length) {
                // 确保不会超出iov_len的范围
                if (iov->iov_len > length) {
                    iov->iov_len = length;
                }
            }

            // void user_iov_iter_advance(struct iovec *iov, int iovcnt, ssize_t nread) {//移动到 open.hpp 中
            //     // 遍历iovec数组，更新iov_base和iov_len
            //     for (int i = 0; i < iovcnt && nread > 0; ++i) {
            //         // 计算当前iovec可以处理的最大字节数
            //         size_t to_read = nread;
            //         if (to_read > iov[i].iov_len) {
            //             to_read = iov[i].iov_len;
            //         }

            //         // 更新nread
            //         nread -= to_read;

            //         // 更新iov_base和iov_len
            //         iov[i].iov_len -= to_read;
            //         iov[i].iov_base = (char *)iov[i].iov_base + to_read;

            //         // 如果当前iovec已经完全处理完毕，可以跳过剩余的iovec
            //         if (iov[i].iov_len == 0) {
            //             if (i < iovcnt - 1) {
            //                 memmove(&iov[i], &iov[i + 1], (iovcnt - i - 1) * sizeof(struct iovec));
            //             }
            //             iovcnt--;
            //             i--;
            //         }
            //     }
            // }

                    // static inline ssize_t Socket_recvT(Socket* this, struct iov_iter *iter, size_t length, int flags, int timeoutMS)
        static inline ssize_t Socket_recvT(Socket* thissock, struct iovec *iter, size_t length, int flags, int timeoutMS)
        {
                // TODO: implementation function should accept length as well.
                            //    struct iov_iter copy = *iter;
                            //    iov_iter_truncate(&copy, length);

                    // struct iovec copy = *iter;
                    // user_iov_iter_truncate(&copy, length);
                    // Note : @@todo 上面的表达换成下面的这个 不要 copy
                    std::cout<<"before [Socket_recvT] iter->iov_len ="<<iter->iov_len<<"\n";

                    user_iov_iter_truncate(iter, length);
                    std::cout<<"after [Socket_recvT] iter->iov_len ="<<iter->iov_len<<"\n";

                {
                    // ssize_t nread = thissock->ops->recvT(thissock, &copy, flags, timeoutMS);
                    ssize_t nread = thissock->ops->recvT(thissock, iter, flags, timeoutMS);

                    std::cout<<"[Socket_recvT] nread= "<<nread<<"\n";
                    // for (size_t i = 0; i < nread; ++i) {
                    //     std::cout << std::hex << static_cast<int>(((char *)(iter->iov_base))[i]) << " ";
                    // }
                    std::cout<<"\n";
                    if (nread >= 0)
                    {
                                // TODO: currently some parts of the project expect that we advance
                                // the iov_iter.  But as it turns out, advancing here does not mesh
                                // well with how iov_iter is supposed to be used.  A problem can be
                                // observed when advancing an iov_iter of type ITER_PIPE. This will
                                // result in mutation of external state (struct pipe_inode_info). IOW
                                // we can't just make a copy of any iov_iter and advance that in
                                // isolation.
                                //
                                // That means, the code should be changed such that we advance only in
                                // the outermost layers of the beegfs client module.

                                //  iov_iter_advance(iter, nread);
                        user_iov_iter_advance(iter, 1,nread);//这里的 1 表示的是 iter表示的数组有几段 
                    }

                    return nread;
                }
        }



                    // static ssize_t __commkit_readfile_receive(CommKitContext* context, FileOpState* currentState,struct iov_iter *iter, size_t length, bool exact)
static ssize_t __commkit_readfile_receive(CommKitContext* context, FileOpState* currentState,struct iovec *iter, size_t length, bool exact)
{
   ssize_t recvRes;
   Socket* socket = currentState->base.socket;

                //    Config* cfg = App_getConfig(context->app);//this->cfg;
    //去掉故障注入
            //    if(BEEGFS_SHOULD_FAIL(commkit_readfile_receive_timeout, 1) )
            //       recvRes = -ETIMEDOUT;
            //    else

    // 设置超时时间
    int timeoutMS=50000;//替换 cfg->connMsgLongTimeout
   if(exact)//true
   {
                //recvRes = Socket_recvExactT(socket, iter, length, 0, cfg->connMsgLongTimeout);
    std::cout<<"[__commkit_readfile_receive] : prepare goto Socket_recvExactT \n";
      recvRes = Socket_recvExactT(socket, iter, length, 0, timeoutMS);
      std::cout<<"[__commkit_readfile_receive] : recvRes = "<<recvRes<<"\t, length = "<<length<<"\n";
    //    for (size_t i = 0; i < length; ++i) {
    //     std::cout << std::hex << static_cast<int>(((char *)(iter->iov_base))[i]) << " ";
    //     }
      //std::cout<<"\n iter->iov_len ="<<iter->iov_len<<"\n";
          //std::cout<<"[__commkit_readfile_receive] : out of Socket_recvExactT , recvRes = "<<recvRes<<"\n";


   }
   else// 进入的是这个分支
   {
                //recvRes = Socket_recvT(socket, iter, length, 0, cfg->connMsgLongTimeout);
    //std::cout<<"[__commkit_readfile_receive] : prepare goto Socket_recvExactT \n";

      recvRes = Socket_recvT(socket, iter, length, 0, timeoutMS);
      std::cout<<"recvRes = "<<recvRes<<"\t, length = "<<length<<"\n";
    //    for (size_t i = 0; i < recvRes; ++i) {
    //     std::cout << std::hex << static_cast<int>(((char *)(iter->iov_base))[i]) << " ";
    //     }
          //std::cout<<"[__commkit_readfile_receive] : out of Socket_recvExactT , recvRes = "<<recvRes<<"\n";


   }

   if(recvRes < 0) 
   {
                //   Logger_logFormatted(context->log, Log_SPAM, context->ops->logContext,
                //      "Request details: receive from %s: %lld bytes (error %zi)",
                //      Node_getNodeIDWithTypeStr(currentState->base.node), (long long)length, recvRes);
                // printf("Request details: receive from %s: %lld bytes (error %zd)\n",
                //        Node_getNodeIDWithTypeStr(currentState->base.node), (long long)length, recvRes);
        printf("Request details: receive  %lld bytes (error %zd)\n", (long long)length, recvRes);
   }

   return recvRes;
}

                    static int __commkit_readfile_recvdata_prefix(CommKitContext* context, FileOpState* currentState)
                    {       
                        std::cout<<"go into [__commkit_readfile_recvdata_prefix] \n";
                            ssize_t recvRes;
                            char dataLenBuf[sizeof(int64_t)]; // length info in fhgfs network byte order
                            size_t size = sizeof(dataLenBuf);
                            int64_t lengthInfo; // length info in fhgfs host byte order
                            DeserializeCtx ctx = { dataLenBuf, sizeof(int64_t) };

                                            // struct kvec kvec = {
                                            //     .iov_base = dataLenBuf,
                                            //     .iov_len = size,
                                            // };
                                            // struct iov_iter iter;
                                            // BEEGFS_IOV_ITER_KVEC(&iter, READ, &kvec, 1, size);
                            struct iovec iter = {
                                .iov_base = dataLenBuf,
                                .iov_len = size,
                            };


                            recvRes = __commkit_readfile_receive(context, currentState, &iter, size, true);
                            std::cout<<"after __commkit_readfile_receive ,recvRes = "<<recvRes <<"\n";
                            // for (size_t i = 0; i < sizeof(int64_t); ++i) {
                            //     std::cout << std::hex << static_cast<int>(((char *)(iter.iov_base))[i]) << " ";
                            // }
                            //std::cout<<"\n iter->iov_len ="<<iter.iov_len<<"\n";
                            
                            if(recvRes < 0)
                                return recvRes;
                            if (recvRes == 0)
                                return -ECOMM;

                            // got the length info response
                            Serialization_deserializeInt64(&ctx, &lengthInfo);

                            if(lengthInfo <= 0)// 第一次传输的8B 的字节获取给 lengthInfo 确实是0
                            { // end of file data transmission
                            //std::cout<<"end of file data transmission \n";
                                if(lengthInfo < 0)
                                { // error occurred
                                    currentState->base.nodeResult = lengthInfo;
                                }
                                else
                                { // normal end of file data transmission //输出传输结束的标志？
                                    currentState->base.nodeResult = currentState->transmitted;
                                }

                                return 0;
                            }

                            // buffer overflow check
                            if(currentState->transmitted + lengthInfo > currentState->totalSize )
                            {
                                            // Logger_logErrFormatted(context->log, context->ops->logContext,
                                            //     "Bug: Received a lengthInfo that would overflow request from %s: %lld %zu %zu",
                                            //     Node_getNodeIDWithTypeStr(currentState->base.node), (long long)lengthInfo,
                                            //     currentState->transmitted, currentState->totalSize);

                                printf("[__commkit_readfile_recvdata_prefix] Bug: Received a lengthInfo that would overflow request from %lld %zu %zu \n",
                                    (long long)lengthInfo,currentState->transmitted, currentState->totalSize);
                                return -EREMOTEIO;
                            }

                            // positive result => node is going to send some file data
                            currentState->toBeTransmitted += lengthInfo;//lengthInfo 这个变量存储的应该是数据大小
                            currentState->receiveFileData = true;
                            return 1;
                    }

            // 检查iov_iter是否还有数据
            int user_iov_iter_count(struct iovec *iter) {
                return iter->iov_len;
            }

            //这个函数是通信框架的一部分，用于处理接收文件数据的过程，并在接收过程中更新状态信息。
            static int __commkit_readfile_recvdata(CommKitContext* context, struct CommKitTargetInfo* info)
            {  
                std::cout<<"Go into __commkit_readfile_recvdata \n";
                    // 从目标信息结构体中获取当前状态结构体

                    FileOpState* currentState = container_of_v2<FileOpState,CommKitTargetInfo>(info,offsetof(FileOpState, base));

                    
                    // 获取 iov_iter 指针，它用于处理分散的内存区域
                            // struct iov_iter *iter = &currentState->data;
                    struct iovec *iter = &currentState->data;
                    if (iter ==nullptr)
                    {
                        std::cout<<"[__commkit_readfile_recvdata] iter ==nullptr \n";
                    }
                    // 计算还需要接收的数据长度
                    size_t missingLength = currentState->toBeTransmitted - currentState->transmitted;//currentState->totalSize - currentState->transmitted;//currentState->toBeTransmitted - currentState->transmitted;
                    std::cout<<"iter->iov_len = "<<iter->iov_len<<"\t,";
                    std::cout<<" currentState->totalSize = "<<currentState->totalSize<<",\t currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";
                    ssize_t recvRes;

                                 
                                //跳过 
                                        // #ifdef BEEGFS_NVFS
                                        //
                                        // If we are using the RDMA message, then the protocol is simply to wait for
                                        // a reply which is either an error code ( < 0 ), no data ( = 0 ) or length of
                                        // data already transferred ( > 0 ).
                                        //// 如果使用 RDMA 消息，则等待回复，回复可能是错误码（<0），没有数据（=0）或已传输数据的长度（>0）
                                        // std::cout<<"currentState->rdmap= "<<currentState->rdmap<<"\n";
                                        // if (currentState->rdmap)
                                        // {
                                        //     __commkit_readfile_recvdata_prefix(context, currentState);
                                        //     std::cout<<"currentState->toBeTransmitted = "<<currentState->toBeTransmitted<<"\n";
                                        //     // if (currentState->toBeTransmitted > 0)
                                        //     // {
                                        //     //     currentState->transmitted = currentState->toBeTransmitted;
                                        //     //     std::cout<<"currentState->transmitted = "<<currentState->transmitted<<"\n";
                                        //     //     currentState->base.nodeResult = currentState->transmitted;
                                        //     //     std::cout<<"currentState->base.nodeResult =  "<<currentState->base.nodeResult<<"\n";
                                        //     // }
                                        //     currentState->receiveFileData = false;
                                        //     // return 0;
                                        // }
                                        // #endif // BEEGFS_NVFS
                                

                    // 如果还没有开始接收文件数据，则调用前缀处理函数
                    // if(!currentState->receiveFileData)
                    //     {
                    //         std::cout<<"!currentState->receiveFileData \n";
                    //         return __commkit_readfile_recvdata_prefix(context, currentState);// 这个只接收 8B 
                    //         // std::cout<<"after __commkit_readfile_recvdata_prefix \n";


                    //     }

std::cout<<"iter->iov_len = "<<iter->iov_len<<"\t,";
                    std::cout<<" currentState->totalSize = "<<currentState->totalSize<<",\t currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";
                   
                    // 如果 iov_iter 没有数据，则获取下一个 iov_iter
                    // std::cout<<"如果 iov_iter 没有数据，则获取下一个 iov_iter \n";
                    if(user_iov_iter_count(iter) == 0)//确实等于0
                    {   
                        std::cout<<"user_iov_iter_count(iter) == 0 \n";
                        ((struct ReadfileIterOps*) context->private_data)->nextIter(context, currentState); //这个函数的实现还没找到哦，主要是得看 private 的初始化
                                    // BUG_ON(iov_iter_count(iter) == 0);
                        std::cout<<"user_iov_iter_count(iter) =="<<user_iov_iter_count(iter)<<"\n";
                    
                    }

                    std::cout<<"iter->iov_len = "<<iter->iov_len<<"\t,";
                    std::cout<<" currentState->totalSize = "<<currentState->totalSize<<",\t currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";
                    
                    // iter->iov_len=75; 
                    // 接收可用的数据部分
                    // receive available dataPart
                    std::cout<<"接收可用的数据部分 \n";
                    std::cout<<"missingLength = "<<missingLength<<"\n";
                    // recvRes = __commkit_readfile_receive(context, currentState, iter, missingLength, false);//这里是接收数据包的逻辑
                    recvRes = __commkit_readfile_receive(context, currentState, iter, missingLength, true);//这里是接收数据包的逻辑

                    
                    std::cout<<" 【@@__commkit_readfile_recvdata 】 recvRes = "<<recvRes<<"\n";
                    int nonzero_cnt=0;
                    for (size_t i = 0; i < recvRes; ++i) {
                        // std::cout << std::hex << static_cast<int>(((char *)(iter->iov_base))[i]) << " ";
                        if(static_cast<int>(((char *)(iter->iov_base))[i])!= 0)
                        {
                            // std::cout<<" 非 0 \n";
                            nonzero_cnt++;
                        }
                    }
                    std::cout<<"nonzero_cnt = "<<nonzero_cnt<<"\n";
                    if(recvRes < 0)
                        return recvRes;

                    // 更新已传输的数据量
                    currentState->transmitted += recvRes;

                    // 如果所有数据都已接收，则设置接收文件数据标志为 false，表示接下来应该接收下一个长度信息
                    if(currentState->toBeTransmitted == currentState->transmitted)
                    { 
                        // all of the data has been received => receive the next lengthInfo in the header stage
                        currentState->receiveFileData = false;
                    }

                    // 返回 1 表示继续接收数据
                    return 1;
            }

static void __commkit_recvdata_generic(CommKitContext* context, struct CommKitTargetInfo* info)
{
   int recvRes;
   //std::cout<<"go into __commkit_recvdata_generic \n";
    // 这个注释的 prepare_io 失败的原因是 调用 的是原生 beegfs 的socket 对应的逻辑 ，要想实现类似的功能 参考 open.hpp 中  StandardSocket_recvfromT（）这个函数中的轮询套接字
//    if(!__commkit_prepare_io(context, info, POLLIN) )
//       {
//         std::cout<<"__commkit_prepare_io error \n";//调试 发现进入的是这个分支
//         return;
//       }
    //std::cout<<"---- 接收数据 ----- \n";
   recvRes = context->ops->recvData(context, info);//接收消息的代码
    std::cout<<"[__commkit_recvdata_generic] recvRes= "<<recvRes<<"\n";
   if(recvRes < 0)
   {
      if(recvRes == -EFAULT)
      { 
                    // bad buffer address given
                    //  Logger_logFormatted(context->log, Log_DEBUG, context->ops->logContext,
                    //     "Bad buffer address");
        printf("[__commkit_recvdata_generic ] Bad buffer address \n");

        info->nodeResult = -FhgfsOpsErr_ADDRESSFAULT;
        info->state = CommKitState_SOCKETINVALIDATE;
        return;
      }
      else if(recvRes == -ETIMEDOUT)
      { 
        // timeout
                        //  Logger_logErrFormatted(context->log, context->ops->logContext,
                        //     "Communication timeout in RECVDATA stage. Node: %s",
                        //     Node_getNodeIDWithTypeStr(info->node) );

        printf("[__commkit_recvdata_generic ] Communication timeout in RECVDATA stage. \n");
      }
      else
      { // error
                        //  Logger_logErrFormatted(context->log, context->ops->logContext,
                        //     "Communication error in RECVDATA stage. Node: %s (recv result: %lld)",
                        //     Node_getNodeIDWithTypeStr(info->node), (long long)recvRes);
        
        printf("[__commkit_recvdata_generic ] Communication error in RECVDATA stage.  \n");

      }

      info->state = CommKitState_SOCKETINVALIDATE;
      return;
   }

   if(recvRes == 0)
      {
        info->state = CommKitState_CLEANUP;
        std::cout<<"recvRes = "<<recvRes<<"\n";

      }
   else
      {
        std::cout<<"recvRes = "<<recvRes<<"\n";
        if(recvRes ==1)
        {
            std::cout<<"此时要继续接收数据 \n";
        }
        __commkit_add_socket_pollstate(context, info, POLLIN);
      }
      //std::cout<<"out of __commkit_recvdata_generic \n";
}

//-----------------------------------  __commkit_cleanup_generic -------------------------------------------




        /*
            //是一个用于管理连接池中套接字的函数。它的主要功能是将一个已经使用完毕的套接字（socket）释放回连接池中，以便它可以被其他操作重用。这个函数是连接池管理的一部分，通常用于提高资源利用率和性能，特别是在需要频繁建立和断开连接的场景中。

        void NodeConnPool_releaseStreamSocket(NodeConnPool* this, Socket* sock)
        {
                PooledSocket* pooledSock = (PooledSocket*)sock;

                // test whether this socket has expired

                if(unlikely(PooledSocket_getHasExpired(pooledSock, this->fallbackExpirationSecs) ||
                            pooledSock->closeOnRelease))
                { // this socket just expired => invalidate it
                    __NodeConnPool_invalidateSpecificStreamSocket(this, sock);
                    return;
                }

                // mark the socket as available

                Mutex_lock(&this->mutex); // L O C K

                if (unlikely(PooledSocket_getPool(pooledSock) != &this->connList))
                {
                    // printk_fhgfs(KERN_ERR, "%s:%d: socket %p not in pool %p\n",
                    //    __func__, __LINE__, pooledSock, this);
                    goto exit;
                }

                this->availableConns++;
                PooledSocket_setAvailable(pooledSock, true);
                ConnectionList_moveToHead(&this->connList, pooledSock);

                if (Socket_getSockType((Socket*) pooledSock) == NICADDRTYPE_RDMA)
                {
                    NicAddressStats* st = IBVSocket_getNicStats(&((RDMASocket*) pooledSock)->ibvsock);//@@ 有多个实现
                    if (st)
                        st->available++;
                }

                Condition_signal(&this->changeCond);

                exit:
                Mutex_unlock(&this->mutex); // U N L O C K
        }
        */


    

enum
{
   CK_RETRY_BUDDY_FALLBACK = 1 << 0,
   CK_RETRY_LOOP_EAGAIN = 1 << 1,
};

// Node_put 函数，减少引用计数并销毁节点
static inline void user_Node_put(Node* node) {
    if (node != NULL) {
        atomic_fetch_sub(&node->references, 1);
        if (atomic_load(&node->references) == 0) {
            // 释放 node 相关资源
            free(node->id);
            free(node->nodeIDWithTypeStr);
            // 销毁互斥锁和条件变量
            pthread_mutex_destroy(&node->mutex);
            pthread_cond_destroy(&node->changeCond.cond);
            // 释放 node 结构体
            free(node);
        }
    }
}

static void __commkit_cleanup_generic(CommKitContext* context, struct CommKitTargetInfo* info)
{
//跳过 
                        /*
                            #ifdef BEEGFS_NVFS
                            //
                            // Clean up the RDMA mapping.
                            //
                            if (context->ops == &readfileOps)
                            {
                                FileOpState* currentState = container_of(info, FileOpState, base);
                                if (currentState->rdmap)
                                {
                                    RdmaInfo_unmapRead(currentState->rdmap);
                                    currentState->rdmap = NULL;
                                }
                            }
                            else if (context->ops == &writefileOps)
                            {
                                FileOpState* currentState = container_of(info, FileOpState, base);
                                if (currentState->rdmap)
                                {
                                    RdmaInfo_unmapWrite(currentState->rdmap);
                                    currentState->rdmap = NULL;
                                }
                            }
                            #endif // BEEGFS_NVFS
                        */

   if(info->socket)
   {
    //   NodeConnPool_releaseStreamSocket(Node_getConnPool(info->node), info->socket);
    // 释放套接字
        NodeConnPool_releaseStreamSocket(info->socket);
      context->numAcquiredConns--;
   }

   freeHeaderBuffer(info->headerBuffer);
   info->headerBuffer = nullptr;

   if(info->node) 
   {
      user_Node_put(info->node);
      info->node = nullptr;
   }

   // prepare next stage

   if(   (info->nodeResult == -FhgfsOpsErr_COMMUNICATION) ||
         (info->nodeResult == -FhgfsOpsErr_AGAIN &&
            (context->ops->retryFlags & CK_RETRY_LOOP_EAGAIN) ) )
   { 
    // comm error occurred => check whether we can do a retry
    // 在内核中，fatal_signal_pending 函数会考虑信号的紧迫性和致命性，这在用户态程序中通常不是必需的
    // 直接跳过 
                /*
                    if (fatal_signal_pending(current))
                    info->nodeResult = -FhgfsOpsErr_INTERRUPTED;

                    换成下面的
                    sigset_t pending;
                    int has_pending = sigpending(&pending); // 获取待处理信号集

                    if (has_pending == 0) {
                        // 检查信号集是否为空
                        if (sigismember(&pending, SIGINT) || sigismember(&pending, SIGTERM))
                        {
                            printf("A fatal signal is pending.\n");
                            info->nodeResult = -FhgfsOpsErr_INTERRUPTED;
                        } 
                        else 
                        {
                            printf("No fatal signals are pending.\n");
                        }
                    } 
                    else 
                    {
                        perror("sigpending failed");
                    }


                    else if (App_getConnRetriesEnabled(context->app) && 
                */
    bool connRetriesEnabled=true; //@@ 要么true 要么false

                 //   if (App_getConnRetriesEnabled(context->app) &&  //this->connRetriesEnabled;
      if (connRetriesEnabled && 
            (!context->maxNumRetries || context->currentRetryNum < context->maxNumRetries))
      { // we have retries left
         context->numRetryWaiters++;
         info->state = CommKitState_RETRYWAIT;
         return;
      }
   }

   // success or no retries left => done
   context->numDone++;
   info->state = CommKitState_DONE;
}
struct FhgfsOpsErrListEntry
{
   const char* errString; // human-readable error string
   int         sysErr;    // positive linux system error code
};


   /* -1 because last elem is NULL */

// Note: This is based on the FhgfsOpsErr entries
// Note: We use EREMOTEIO as a generic error here
struct FhgfsOpsErrListEntry const __FHGFSOPS_ERRLIST[] =
{
   {"Success", 0}, // FhgfsOpsErr_SUCCESS
   {"Internal error", EREMOTEIO}, // FhgfsOpsErr_INTERNAL
   {"Interrupted system call", EINTR}, // FhgfsOpsErr_INTERRUPTED
   {"Communication error", ECOMM}, // FhgfsOpsErr_COMMUNICATION
   {"Communication timeout", ETIMEDOUT}, // FhgfsOpsErr_COMMTIMEDOUT
   {"Unknown node", EREMOTEIO}, // FhgfsOpsErr_UNKNOWNNODE
   {"Node is not owner of entry", EREMOTEIO}, // FhgfsOpsErr_NOTOWNER
   {"Entry exists already", EEXIST}, // FhgfsOpsErr_EXISTS
   {"Path does not exist", ENOENT}, // FhgfsOpsErr_PATHNOTEXISTS
   {"Entry is in use", EBUSY}, // FhgfsOpsErr_INUSE
   {"Dynamic attributes of entry are outdated", EREMOTEIO}, // FhgfsOpsErr_INUSE
   {"Removed", 999}, // former FhgfsOpsErr_PARENTTOSUBDIR, not used
   {"Entry is not a directory", ENOTDIR}, // FhgfsOpsErr_NOTADIR
   {"Directory is not empty", ENOTEMPTY}, // FhgfsOpsErr_NOTEMPTY
   {"No space left", ENOSPC}, // FhgfsOpsErr_NOSPACE
   {"Unknown storage target", EREMOTEIO}, // FhgfsOpsErr_UNKNOWNTARGET
   {"Operation would block", EWOULDBLOCK}, // FhgfsOpsErr_WOULDBLOCK
   {"Inode not inlined", EREMOTEIO}, // FhgfsOpsErr_INODENOTINLINED
   {"Underlying file system error", EREMOTEIO}, // FhgfsOpsErr_SAVEERROR
   {"Argument too large", EFBIG},  // FhgfsOpsErr_TOOBIG
   {"Invalid argument", EINVAL},   // FhgfsOpsErr_INVAL
   {"Bad memory address", EFAULT}, // FhgfsOpsErr_ADDRESSFAULT
   {"Try again", EAGAIN},          // FhgfsOpsErr_AGAIN
   {"Potential cache loss for open file handle. (Server crash detected.)" , EREMOTEIO}, /*
                                                                  FhgfsOpsErr_STORAGE_SRV_CRASHED*/
   {"Permission denied", EPERM},   // FhgfsOpsErr_PERM
   {"Quota exceeded", EDQUOT},     // FhgfsOpsErr_DQUOT
   {"Out of memory", ENOMEM},      // FhgfsOpsErr_OUTOFMEM
   {"Numerical result out of range", ERANGE}, // FhgfsOpsErr_RANGE
   {"No data available", ENODATA}, // FhgfsOpsErr_NODATA
   {"Operation not supported", EOPNOTSUPP}, // FhgfsOpsErr_NOTSUPP
   {"Argument list too long", E2BIG}, // FhgfsOpsErr_TOOLONG
   {nullptr, 0}
};

#define __FHGFSOPS_ERRLIST_SIZE \
   ( (sizeof(__FHGFSOPS_ERRLIST) ) / (sizeof(struct FhgfsOpsErrListEntry) ) - 1)

/**
 * @return static human-readable error string
 */
const char* FhgfsOpsErr_toErrString(FhgfsOpsErr errCode)
{
   size_t unsignedErrCode = (size_t)errCode;

   if(unsignedErrCode < __FHGFSOPS_ERRLIST_SIZE) 
      return __FHGFSOPS_ERRLIST[unsignedErrCode].errString;

// #ifdef BEEGFS_DEBUG
//    printk_fhgfs(KERN_WARNING, "Unknown errCode given to FhgfsOpsErr_toErrString(): %d/%u "
//       "(dumping stack...)\n", (int)errCode, (unsigned)errCode);

//    dump_stack();
// #endif

   return "Unknown error code";
}











