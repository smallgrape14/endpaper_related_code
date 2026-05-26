#ifndef NEW_READ_HPP
#define NEW_READ_HPP
// #ifdef __cplusplus
// 仅 C++ 编译器可见的头文件
#include <atomic>       // C++ 原子操作库
#include <cstddef>      // C++ 标准库（可选，如果不需要可删除）
// #endif

#include <pthread.h>
// #include <stdatomic.h>
#include <stddef.h> // 包含 offsetof 的头文件
#include <stdio.h>   // 用于 printf，如果需要的话
#include <sys/uio.h> // 包含iovec的定义
// #include <cstddef> // 包含 offsetof 的头文件
// #include <atomic>

//$$% ---------------------------
// #include "open.hpp"
// #include "RDMAConnection.hpp"
#include "read_pre.hpp"

//$$% ---------------------------

int test_beegfs(int input)
{
    return input + 1;
}
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
                LOG(DEBUG)<<" 如果传入的指针是nullptr，那么不能进行初始化"<<endl;
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
            pathInfo->origParentUID = 1000;//@@ root UID  stat mnt/beegfs 是 0： stat mnt/beegfs/test_1201 :1000
            pathInfo->_origParentUID = 1000;//@@
            pathInfo->origParentEntryID = "0-67C551B0-6";//@@ root 是 根目录的entryid
            
            pathInfo->_origParentEntryID = "0-67C551B0-6";//@@
            // 162 server 上：test_1201 目录的ID 是 0-674C7122-1 
            // 162 server 上 ：test_1202 是0-674D5ED6-1
            // 162 server 上 ：test_1217 是0-674D5ED6-1

            
            //Note: 以上这六个参数和 open获取的信息是一致的
        }
void PathInfo_init(PathInfo* pathInfo,unsigned int origParentUID,char* origParentEntryID) {
            if (pathInfo == nullptr) {
                // 如果传入的指针是nullptr，那么不能进行初始化
                LOG(DEBUG)<<" 如果传入的指针是nullptr，那么不能进行初始化"<<endl;
                return;
            }

            // 使用memset来将整个结构体初始化为0
            // std::memset(pathInfo, 0, sizeof(PathInfo));
                LOG(DEBUG)<<"pathinfo->origParentEntryID 2= "<<pathInfo->origParentEntryID<<"\n";

            // 由于union的特性，只需要初始化一个成员即可
            // 这里我们初始化_flags成员，其他成员将自动被初始化为0
            pathInfo->_flags = 1;//PATHINFO_FEATURE_ORIG;//PATHINFO_FEATURE_ORIG;//@@ PATHINFO_FEATURE_ORIG;//1

            // 如果需要设置特定的值，可以在这里进行设置
            // 例如，设置flags
            pathInfo->flags = 1;//PATHINFO_FEATURE_ORIG;//PATHINFO_FEATURE_ORIG;//@@ PATHINFO_FEATURE_ORIG;  PATHINFO_FEATURE_INLINED;

            // 设置origParentUID或origParentEntryID，
            pathInfo->origParentUID = origParentUID; //0;//@@ root UID  stat mnt/beegfs
            pathInfo->_origParentUID = origParentUID;//@@
            // pathInfo->_origParentEntryID = "root";//@@
            // pathInfo->origParentEntryID = "root";//@@
            LOG(DEBUG)<<"-------1-----\n";
            LOG(DEBUG)<<"origParentEntryID = "<<origParentEntryID<<"\n";
            LOG(DEBUG)<<"pathInfo->origParentEntryID = "<<pathInfo->origParentEntryID<<"\n";
            pathInfo->origParentEntryID=(char*)malloc(50*sizeof(char));
            pathInfo->_origParentEntryID=(char*)malloc(50*sizeof(char));

            strcpy(pathInfo->origParentEntryID,origParentEntryID);
            LOG(DEBUG)<<"-------2-----\n";
            LOG(DEBUG)<<"origParentEntryID = "<<origParentEntryID<<"\n";
            strcpy(pathInfo->_origParentEntryID,origParentEntryID);
            LOG(DEBUG)<<"------3------\n";

            LOG(DEBUG)<<"pathInfo->origParentEntryID = "<<pathInfo->origParentEntryID<<"\n";
            //Note: 以上这六个参数和 open获取的信息是一致的
            return ;
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
        LOG(DEBUG)<<"StripePattern_init error \n";
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
        LOG(DEBUG)<<"[Raid0_StripePattern_init] error \n";
    }
    std::memset(pattern, 0, sizeof(Raid0Pattern));
    //(1)
    StripePattern_init(&(pattern->stripePattern));
    
    // (2)init attribs
   UInt16Vec_init(&pattern->stripeTargetIDs);
   size_t len= UInt16Vec_length(&pattern->stripeTargetIDs);
   LOG(DEBUG)<<"[Raid0_StripePattern_init] len = "<<len<<"\n";
    // (3)
    int targetnum=1;//@@ todo
   pattern->defaultNumTargets=targetnum;//@@ todo默认为1
   // append 
   uint16_t targetNumID[10];//1913;//@@ todo 217 server :1913 
   // 162server : 3 4 
   targetNumID[0]=3001;//@@ 162server: 250415 更新之后就是 3001
   //224 server :重新配置 241206更新 targetID 为 1201
   targetNumID[1]=4;//@@

   for(int i=0;i<targetnum ;i++)
   {
     UInt16Vec_append(&pattern->stripeTargetIDs,targetNumID[i]);

   }
   LOG(DEBUG)<<"[Raid0_StripePattern_init] after append, len = "<<len<<"\n";


}
        // 初始化 RemotingIOInfo 结构体的函数
        void initRemotingIOInfo(RemotingIOInfo* info) {
            if (info != NULL) {
                memset(info, 0, sizeof(RemotingIOInfo)); // 将整个结构体清零

                // 初始化字段到默认值
                        //info->fileHandleID = ;//"75196CA8#0-67282B84-BE";//@@从 open 函数 中获取
                info->fileHandleID=(char*)malloc(50);
                // strcpy(info->fileHandleID,"837976248685");

                strcpy(info->fileHandleID,filehandleID_xyp);//@@直接拷贝 open 返回的信息
                //LOG(DEBUG)<<"[initRemotingIOInfo] info->fileHandleID = "<<info->fileHandleID<<"\n";
                
                // StripePattern* pattern=new StripePattern ();
                
                Raid0Pattern* pattern=new Raid0Pattern ();
                Raid0_StripePattern_init(pattern);
                info->pattern=&(pattern->stripePattern);
                //LOG(DEBUG)<<"info->pattern = "<<info->pattern<<"\n";
                if(pattern==nullptr|| info->pattern==nullptr)
                {
                    LOG(DEBUG)<<"error \n";
                }
                

                
                info->accessFlags = 1; // #define OPEN_ACCESS_READ  1          1
                PathInfo* pathinfo=new PathInfo ();
                PathInfo_init(pathinfo);//@@
                //LOG(DEBUG)<<"$$$ pathinfo->flag = "<<pathinfo->flags<<"\t, _flag = "<<pathinfo->_flags<<"\n";
                info->pathInfo = pathinfo; // OpenFileRespMsg 中 有 PathInfo pathInfo;

                
                info->needsAppendLockCleanup = nullptr;
                info->userID = 0;
                info->groupID = 0;
                // 初始化其他字段...
                //LOG(DEBUG)<<"初始化其他字段...\n";
             //LOG(DEBUG)<<"ioInfo->pattern = "<<info->pattern<<"\n";

            }
            else
            {
                LOG(DEBUG)<<"info ==NUll \n ";
            }
        }
        // 初始化 RemotingIOInfo 结构体的函数
        void initRemotingIOInfo(RemotingIOInfo* info,char * filehandleID,unsigned int origParentUID,char * origParentEntryID) {
            if (info != NULL) {
                memset(info, 0, sizeof(RemotingIOInfo)); // 将整个结构体清零

                // 初始化字段到默认值
                        //info->fileHandleID = ;//"75196CA8#0-67282B84-BE";//@@从 open 函数 中获取
                info->fileHandleID=(char*)malloc(50);
                // strcpy(info->fileHandleID,"837976248685");

                // strcpy(info->fileHandleID,filehandleID_xyp);//@@直接拷贝 open 返回的信息
                strcpy(info->fileHandleID,filehandleID);//@@直接拷贝 open 返回的信息

                //LOG(DEBUG)<<"[initRemotingIOInfo] info->fileHandleID = "<<info->fileHandleID<<"\n";
                
                // StripePattern* pattern=new StripePattern ();
                
                Raid0Pattern* pattern=new Raid0Pattern ();
                Raid0_StripePattern_init(pattern);
                info->pattern=&(pattern->stripePattern);
                LOG(DEBUG)<<"info->pattern = "<<info->pattern<<"\n";
                if(pattern==nullptr|| info->pattern==nullptr)
                {
                    LOG(DEBUG)<<"error \n";
                }
                

                
                info->accessFlags = 1; // #define OPEN_ACCESS_READ  1          1
                PathInfo* pathinfo=(PathInfo*)malloc(sizeof(PathInfo));//new PathInfo ();
                LOG(DEBUG)<<"pathinfo->origParentEntryID = "<<pathinfo->origParentEntryID<<"\n";
                PathInfo_init(pathinfo,origParentUID,origParentEntryID);//@@
                LOG(DEBUG)<<"$$$ pathinfo->flag = "<<pathinfo->flags<<"\t, _flag = "<<pathinfo->_flags<<"\n";
                info->pathInfo = pathinfo; // OpenFileRespMsg 中 有 PathInfo pathInfo;

                
                info->needsAppendLockCleanup = nullptr;
                info->userID = 0;
                info->groupID = 0;
                // 初始化其他字段...
                LOG(DEBUG)<<"初始化其他字段...\n";
             LOG(DEBUG)<<"ioInfo->pattern = "<<info->pattern<<"\n";

            }
            else
            {
                LOG(DEBUG)<<"info ==NUll \n ";
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
    //     LOG(DEBUG)<<"1-----------\n";
    //      elem->next->prev = elem->prev;
    //     LOG(DEBUG)<<"2-----------\n";

    //      elem->prev->next = elem->next;
    //     LOG(DEBUG)<<"3-----------\n";

    //      elem->next = nullptr;
    //     LOG(DEBUG)<<"4-----------\n";

    //      elem->prev = nullptr;
    //     LOG(DEBUG)<<"5-----------\n";

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
        LOG(DEBUG)<<"head==nullptr \n";
        return 0;
    }
    list_head_t *tmp = head->next; // 从头节点的下一个节点开始，因为头节点不计入长度
    if(tmp==nullptr)
    {
        LOG(DEBUG)<<"head->next==nullptr \n";
        return 0;
    }
LOG(DEBUG)<<static_cast<void*>(tmp)<<"\n";
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
            // LOG(DEBUG)<<"[commkit_initTargetInfo] info->targetID = "<<info->targetID<<"\n";
        }

        // 遍历链表并打印每个节点
        void print_list(list_head_t *head) {
            // CommKitTargetInfo *info;
            // list_for_each_entry(info, head, targetInfoList) {
            //     LOG_printf(DEBUG,"Node data: %p\n", info);
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
               
                //LOG(DEBUG) << "Node data: " << static_cast<void*>(info) << std::endl;
                //LOG(DEBUG)<<"info->state = "<<info->state<<"\n";
                 //LOG(DEBUG)<<"info->targetID = "<<info->targetID<<"\n";
                //LOG(DEBUG)<<"------------------\n";
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
         


                //  LOG(DEBUG)<<"targetIDlist[i] = "<<targetIDlist[i]<<"\n";
        // commkit_initTargetInfo(&targetInfo, targetIDlist[i]);
                 //CommKitTargetInfo * targetInfo; targetInfo->targetID 
        // list_add_tail(&targetInfo.targetInfoList, stateList);
        // LOG(DEBUG)<<"info->targetID = "<<targetInfo.targetID<<"\n";

        CommKitTargetInfo* targetInfo =new CommKitTargetInfo();
        commkit_initTargetInfo(targetInfo, targetIDlist[i]);
        list_add_tail(&(targetInfo->targetInfoList), stateList);
        // LOG(DEBUG)<<"info->targetID = "<<targetInfo->targetID<<"\n";

    }
            // 遍历链表并打印每个节点
    print_list(stateList);
}

//-------------------------------(3) readfile_nextIter

struct CommKitContext;
typedef struct CommKitContext CommKitContext;

//XYP_MODIFY
// static unsigned __commkit_readfile_prepareHeader(CommKitContext* context,struct CommKitTargetInfo* info, 
//     char *msg_buf,size_t *msg_len,
//     uint64_t vaddr,uint32_t rkey,size_t memory_len);//准备头部消息;

//XYP_0303 保留下面这个版本
static unsigned __commkit_readfile_prepareHeader(CommKitContext* context,struct CommKitTargetInfo* info);

static int __commkit_readfile_recvdata(CommKitContext* context, struct CommKitTargetInfo* info);

struct CommKitContextOps
{
//XYP_MODIFY
//  unsigned (*prepareHeader)(CommKitContext*, struct CommKitTargetInfo*,
//     char *msg_buf,size_t *msg_len,
//     uint64_t vaddr,uint32_t rkey,size_t memory_len);
    //XYP_0303
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
    RdmaInfo* rdmap;//$$% //XYP_0303
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
    // 可以添加其他成员 //XYP_0303 添加新成员
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
       int gpudRc;//$$% //XYP_0303
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
        nodeID.value=300;//@@ 162 存储节点ID=1 查询得知 // 217 dell01 : ID=191 ； 162 server :ID=1 ;224:xfusion3 241206 更新为120
        // 假设的节点类型、ID 和端口
            NodeType nodeType = NODETYPE_Storage;
            const char* id ="ubuntu-PowerEdge-R750xa" ;//@@ 162： "ubuntu-PowerEdge-R750xa"; 217： "dell01"
            uint16_t nodenumID = 300;//@@ 存储节点的NodeID   217： 191 ； 162： 1 ;224:xfusion3 241206 更新为120
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
                       LOG_printf(DEBUG,"nodeID.value ==0 \n");
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
    //LOG(DEBUG)<<"go into ReadLocalFileV2Msg_serializePayload \n";
   ReadLocalFileV2Msg* thisCast = (ReadLocalFileV2Msg*)nm;
   //LOG(DEBUG)<<"---------- 1 --------------\n";

   // offset
   //LOG(DEBUG)<<"thisCast->offset = "<<thisCast->offset<<"\n";//-1585904400
   Serialization_serializeInt64(ctx, thisCast->offset);
   //LOG(DEBUG)<<"---------- 2 --------------\n";


   // count
   Serialization_serializeInt64(ctx, thisCast->count);
   //LOG(DEBUG)<<"---------- 3 --------------\n";


   // accessFlags
   Serialization_serializeUInt(ctx, thisCast->accessFlags);
   //LOG(DEBUG)<<"---------- 4 --------------\n";



   // fileHandleID
   Serialization_serializeStrAlign4(ctx, thisCast->fileHandleIDLen, thisCast->fileHandleID);
   //LOG(DEBUG)<<"---------- 5 --------------\n";


   // clientNumID
   NumNodeID_serialize(ctx, &thisCast->clientNumID);
   //LOG(DEBUG)<<"---------- 6 --------------\n";


   // pathInfo
   PathInfo_serialize(ctx, thisCast->pathInfoPtr);
   
   //LOG(DEBUG)<<"---------- 7 --------------\n";


   // targetID
   Serialization_serializeUShort(ctx, thisCast->targetID);
   //LOG(DEBUG)<<"out of ReadLocalFileV2Msg_serializePayload \n";
}

   void ReadLocalFileV2Msg_init(ReadLocalFileV2Msg* rmsg)
   {
     //LOG(DEBUG)<<"go into ReadLocalFileV2Msg_init \n";
      NetMessage_init(&rmsg->netMessage, NETMSGTYPE_ReadLocalFileV2, &ReadLocalFileV2Msg_Ops);
   }


   /**
    * @param sessionID just a reference, so do not free it as long as you use this object!
    */
   void ReadLocalFileV2Msg_initFromSession(ReadLocalFileV2Msg* rmsg,
      NumNodeID clientNumID, const char* fileHandleID, uint16_t targetID, PathInfo* pathInfoPtr,
      unsigned accessFlags, int64_t offset, int64_t count)
   {
    //LOG(DEBUG)<<"go into ReadLocalFileV2Msg_initFromSession \n";
      ReadLocalFileV2Msg_init(rmsg);
      //LOG(DEBUG)<<"---------------------11-------\n";

      rmsg->clientNumID = clientNumID;

      rmsg->fileHandleID = fileHandleID;
      rmsg->fileHandleIDLen = strlen(fileHandleID);
      //LOG(DEBUG)<<"---------------------12-------\n";


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

// static unsigned __commkit_readfile_prepareHeader(CommKitContext* context,struct CommKitTargetInfo* info)//准备头部消息
//XYP_MODIFY
/*
static unsigned __commkit_readfile_prepareHeader(CommKitContext* context,struct CommKitTargetInfo* info, 
    char *msg_buf,size_t *msg_len,
    uint64_t vaddr,uint32_t rkey,size_t memory_len)//准备头部消息

{   
    LOG(DEBUG)<<"go into __commkit_readfile_prepareHeader \n";
                                    //    Node* localNode = App_getLocalNode(context->app);
    NumNodeID localNodeNumID ;
    localNodeNumID.value=2;         //@@ Node_getNumID(localNode);//this->numID; Client Node ID
    // 162 sever 对应的164 client ID =12 // 224 xfusion3 [ID: 6]  //217 dell01 [ID: 5]


   FileOpState* currentState = container_of_v2<FileOpState,CommKitTargetInfo>(info, offsetof(FileOpState, base));
//    currentState->offset=0;//这个参数为什么怎么改也不影响结果
    //LOG(DEBUG)<<"输出 currentState 的参数列表 ：\n";
//    {
    //  LOG(DEBUG)<<"transmitted = "<<currentState->transmitted<<"\n";
    //  LOG(DEBUG)<<"toBeTransmitted = "<<currentState->toBeTransmitted<<"\n";
    //  LOG(DEBUG)<<"totalSize = "<<currentState->totalSize<<"\n";  //@@ 321 是哪里来的
    //  LOG(DEBUG)<<"offset = "<<currentState->offset<<"\n";        //@@
    //  LOG(DEBUG)<<"firstWriteDoneForTarget = "<<currentState->firstWriteDoneForTarget<<"\n";
    //  LOG(DEBUG)<<"receiveFileData = "<<currentState->receiveFileData<<"\n";
    //  LOG(DEBUG)<<"expectedNodeResult = "<<currentState->expectedNodeResult<<"\n";//数值奇怪
    //  LOG(DEBUG)<<"data = ";
    //  for (size_t i = 0; i < sizeof(int64_t); ++i) {
    //     // LOG(DEBUG) << std::hex << static_cast<int>(((char *)(currentState->data.iov_base))[i]) << " ";
    //     LOG(DEBUG) << std::hex << static_cast<int>((((char *)currentState->data.iov_base))[i]) << " ";

    //     }
      //LOG(DEBUG)<<"\n iter->iov_len ="<<currentState->data.iov_len<<"\n";//数值奇怪
    //  LOG(DEBUG)<<"transmitted = "<<currentState->transmitted<<"\n";
    //  LOG(DEBUG)<<"transmitted = "<<currentState->transmitted<<"\n";


//    }
   //LOG(DEBUG)<<"------------------1--------------------\n";
   struct ReadfileIterOps* ops = (ReadfileIterOps*)(context->private_data); 
   //LOG(DEBUG)<<"------------------2--------------------\n";

   NetMessage *netMessage = nullptr;
   ReadLocalFileV2Msg readV2Msg;//消息类型
//    ReadLocalFileRDMAMsg readRDMAMsg;//消息类型 $$%

   //跳过 下面这个RDMA 分支
   LOG(DEBUG)<<" currentState->offset = "<< currentState->offset<<"\n";//这个参数也是正确的

   LOG(DEBUG)<<" currentState->totalSize = "<<currentState->totalSize<<"\n";//这个参数是正确的
    //$$%
    // #ifdef BEEGFS_NVFS
    LOG(DEBUG)<<"go into BEEGFS_NVFS \n";
    ReadLocalFileRDMAMsg readRDMAMsg;

    currentState->rdmap = NULL;
    LOG(DEBUG)<<"context->gpudRc = "<<context->gpudRc<<"\n";
    if (context->gpudRc > 0)
    {   
      
//    FileOpState* currentState = container_of_v2<FileOpState,CommKitTargetInfo>(info, offsetof(FileOpState, base));
        LOG(DEBUG)<<"go out 1 \n";

        struct FileOpVecState* vecState = container_of_v2<FileOpVecState,FileOpState>(currentState, offsetof(FileOpVecState, base));
        // struct FileOpVecState* vecState = container_of(currentState, struct FileOpVecState, base);
        currentState->rdmap = RdmaInfo_mapRead(&vecState->data, info->socket, vaddr, rkey,memory_len);//XYP_MODIFY
        LOG(DEBUG)<<"currentState->rdmap = "<<currentState->rdmap<<"\n";
    }
    if (context->gpudRc < 0 || IS_ERR(currentState->rdmap))
    {
        int st = currentState->rdmap != NULL? PTR_ERR(currentState->rdmap) : context->gpudRc;
        info->state = CommKitState_CLEANUP;
        info->nodeResult = (st == -ENOMEM)? -FhgfsOpsErr_OUTOFMEM : -FhgfsOpsErr_INVAL;
        currentState->rdmap = NULL;
        LOG(DEBUG)<<"go out 2 \n";
        return 0;
    }

    // prepare message
    LOG(DEBUG)<<"// prepare message \n";

    if (!currentState->rdmap)
    // #endif //  BEEGFS_NVFS
   {
   LOG(DEBUG)<<"----prepare message -2--------------------\n";

      ReadLocalFileV2Msg_initFromSession(&readV2Msg, localNodeNumID,
         context->ioInfo->fileHandleID, info->targetID, context->ioInfo->pathInfo, //
         context->ioInfo->accessFlags, currentState->offset, currentState->totalSize);

      netMessage = &readV2Msg.netMessage;
   }
   //跳过 下面这个RDMA 分支 //$$%
    // #ifdef BEEGFS_NVFS
    else
    {
   LOG(DEBUG)<<"----prepare message -4--------------------\n";

        ReadLocalFileRDMAMsg_initFromSession(&readRDMAMsg, localNodeNumID,
            context->ioInfo->fileHandleID, info->targetID, context->ioInfo->pathInfo,
            context->ioInfo->accessFlags, currentState->offset, currentState->totalSize,
            currentState->rdmap);

        netMessage = &readRDMAMsg.netMessage;
    }
    // #endif // BEEGFS_NVFS
    
   LOG(DEBUG)<<"----prepare message -5--------------------\n";

   NetMessage_setMsgHeaderTargetID(netMessage, info->selectedTargetID);

      
   //只有对要发送的header进行序列化的操作
   NetMessage_serialize(netMessage, info->headerBuffer, BEEGFS_COMMKIT_MSGBUF_SIZE);
   *msg_len = NetMessage_getMsgLength(netMessage);
   memcpy(msg_buf, info->headerBuffer, BEEGFS_COMMKIT_MSGBUF_SIZE);

   //LOG(DEBUG)<<"------------------5--------------------\n";

   currentState->transmitted = 0;
//    currentState->toBeTransmitted = 0;
   currentState->receiveFileData = false;
   beegfs_iov_iter_clear(&currentState->data);
   //LOG(DEBUG)<<"------------------6--------------------\n";


   if(ops->prepare)//null
     {
         ops->prepare(context, currentState);  // 这个prepare 的具体实现在哪里
   //LOG(DEBUG)<<"------------------7--------------------\n";

     }
   //LOG(DEBUG)<<"------------------8--------------------\n";


   return NetMessage_getMsgLength(netMessage);
}
*/
static unsigned __commkit_readfile_prepareHeader(CommKitContext* context,struct CommKitTargetInfo* info)//准备头部消息
{   
    //LOG(DEBUG)<<"go into __commkit_readfile_prepareHeader \n";
                                    //    Node* localNode = App_getLocalNode(context->app);
    NumNodeID localNodeNumID ;
    localNodeNumID.value=3;         //@@ Node_getNumID(localNode);//this->numID;
    // 162 sever 的client ID =34 // 224 xfusion3 [ID: 6]  //217 dell01 [ID: 5]


   FileOpState* currentState = container_of_v2<FileOpState,CommKitTargetInfo>(info, offsetof(FileOpState, base));
//    currentState->offset=0;//这个参数为什么怎么改也不影响结果
    //LOG(DEBUG)<<"输出 currentState 的参数列表 ：\n";
   {
    //  LOG(DEBUG)<<"transmitted = "<<currentState->transmitted<<"\n";
    //  LOG(DEBUG)<<"toBeTransmitted = "<<currentState->toBeTransmitted<<"\n";
    //  LOG(DEBUG)<<"totalSize = "<<currentState->totalSize<<"\n";  //@@ 321 是哪里来的
    //  LOG(DEBUG)<<"offset = "<<currentState->offset<<"\n";        //@@
    //  LOG(DEBUG)<<"firstWriteDoneForTarget = "<<currentState->firstWriteDoneForTarget<<"\n";
    //  LOG(DEBUG)<<"receiveFileData = "<<currentState->receiveFileData<<"\n";
    //  LOG(DEBUG)<<"expectedNodeResult = "<<currentState->expectedNodeResult<<"\n";//数值奇怪
    //  LOG(DEBUG)<<"data = ";
    //  for (size_t i = 0; i < sizeof(int64_t); ++i) {
    //     // LOG(DEBUG) << std::hex << static_cast<int>(((char *)(currentState->data.iov_base))[i]) << " ";
    //     LOG(DEBUG) << std::hex << static_cast<int>((((char *)currentState->data.iov_base))[i]) << " ";

    //     }
      //LOG(DEBUG)<<"\n iter->iov_len ="<<currentState->data.iov_len<<"\n";//数值奇怪
    //  LOG(DEBUG)<<"transmitted = "<<currentState->transmitted<<"\n";
    //  LOG(DEBUG)<<"transmitted = "<<currentState->transmitted<<"\n";


   }
   //LOG(DEBUG)<<"------------------1--------------------\n";
   struct ReadfileIterOps* ops = (ReadfileIterOps*)(context->private_data); 
   //LOG(DEBUG)<<"------------------2--------------------\n";

   NetMessage *netMessage = nullptr;
   ReadLocalFileV2Msg readV2Msg;//消息类型
   //跳过 下面这个RDMA 分支
   LOG(DEBUG)<<" currentState->offset = "<< currentState->offset<<"\n";//这个参数也是正确的

   LOG(DEBUG)<<" currentState->totalSize = "<<currentState->totalSize<<"\n";//这个参数是正确的
                                               /*
                                                    #ifdef BEEGFS_NVFS
                                                    ReadLocalFileRDMAMsg readRDMAMsg;

                                                    currentState->rdmap = NULL;
                                                    if (context->gpudRc > 0)
                                                    {
                                                        struct FileOpVecState* vecState = container_of(currentState, struct FileOpVecState, base);
                                                        currentState->rdmap = RdmaInfo_mapRead(&vecState->data, info->socket);
                                                    }
                                                    if (context->gpudRc < 0 || IS_ERR(currentState->rdmap))
                                                    {
                                                        int st = currentState->rdmap != NULL? PTR_ERR(currentState->rdmap) : context->gpudRc;
                                                        info->state = CommKitState_CLEANUP;
                                                        info->nodeResult = (st == -ENOMEM)? -FhgfsOpsErr_OUTOFMEM : -FhgfsOpsErr_INVAL;
                                                        currentState->rdmap = NULL;
                                                        return 0;
                                                    }

                                                    // prepare message
                                                    if (!currentState->rdmap)
                                                    #endif //  BEEGFS_NVFS
                                               */

   {
      ReadLocalFileV2Msg_initFromSession(&readV2Msg, localNodeNumID,
         context->ioInfo->fileHandleID, info->targetID, context->ioInfo->pathInfo, //
         context->ioInfo->accessFlags, currentState->offset, currentState->totalSize);

      netMessage = &readV2Msg.netMessage;
   }
   //LOG(DEBUG)<<"------------------3--------------------\n";

   //跳过 下面这个RDMA 分支

                                                   /*
                                                        #ifdef BEEGFS_NVFS
                                                        else
                                                        {
                                                            ReadLocalFileRDMAMsg_initFromSession(&readRDMAMsg, localNodeNumID,
                                                                context->ioInfo->fileHandleID, info->targetID, context->ioInfo->pathInfo,
                                                                context->ioInfo->accessFlags, currentState->offset, currentState->totalSize,
                                                                currentState->rdmap);

                                                            netMessage = &readRDMAMsg.netMessage;
                                                        }
                                                        #endif // BEEGFS_NVFS
                                                   */

   NetMessage_setMsgHeaderTargetID(netMessage, info->selectedTargetID);
// NetMessage_addMsgHeaderFeatureFlag(netMessage, READLOCALFILEMSG_FLAG_SESSION_CHECK);
   //LOG(DEBUG)<<"------------------4--------------------\n";
    //LOG(DEBUG)<<"currentState->firstWriteDoneForTarget = "<<currentState->firstWriteDoneForTarget<<"\n";
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

   //LOG(DEBUG)<<"------------------5--------------------\n";

   currentState->transmitted = 0;
//    currentState->toBeTransmitted = 0;
   currentState->receiveFileData = false;
   beegfs_iov_iter_clear(&currentState->data);
   //LOG(DEBUG)<<"------------------6--------------------\n";


   if(ops->prepare)//null
     {
         ops->prepare(context, currentState);  // 这个prepare 的具体实现在哪里
   //LOG(DEBUG)<<"------------------7--------------------\n";

     }
   //LOG(DEBUG)<<"------------------8--------------------\n";


   return NetMessage_getMsgLength(netMessage);
}







static bool __commkit_prepare_generic(CommKitContext* context, struct CommKitTargetInfo* info)
{   
    LOG(DEBUG)<<"go into __commkit_prepare_generic \n";
    FileOpState* currentState = container_of_v2<FileOpState,CommKitTargetInfo>(info,offsetof(FileOpState, base));


// 如果已经获取了连接，则不等待
   bool allowWaitForConn = !context->numAcquiredConns;// numAcquiredConns=0 则这个变量的值是true // don't wait if we got at least
    // one conn already (this is important to avoid a deadlock between racing commkit processes)

   info->socket = nullptr;
   info->nodeResult = -FhgfsOpsErr_COMMUNICATION;
   info->selectedTargetID = info->targetID;//statelist 中存储的信息（1）

// 为头部缓冲区分配内存
//LOG(DEBUG)<<"--为头部缓冲区分配内存 \n";
                         //info->headerBuffer = allocHeaderBuffer(allowWaitForConn ? GFP_NOFS : GFP_NOWAIT);
    size_t bufferSize = 1024; // 假设我们需要分配的缓冲区大小
    info->headerBuffer= (char *)allocHeaderBuffer(bufferSize, allowWaitForConn);
    if (info->headerBuffer == NULL) {
        LOG_printf(DEBUG,"Memory allocation failed\n");
        context->numBufferless += 1;
        return false;
    }

    // LOG(DEBUG)<<"2----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";

// 选择正确的目标ID并获取目标状态

   if(!context->ioInfo) // 进入这个分支
     {
         info->selectedTargetID = info->targetID;
         //LOG(DEBUG)<<"info->targetID = "<<info->targetID<<"\n";
     }


   info->node = NodeStoreEx_referenceNodeByTargetID();
   if(!info->node) 
   { // unable to resolve targetID
      info->nodeResult = -1;//负数表示错误 //-resolveErr;
      LOG_printf(DEBUG,"[__commkit_prepare_generic] 获取目标节点引用失败 unable to resolve targetID  \n");
      goto cleanup;
   }

    LOG(DEBUG)<<"3----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";

// 从连接池中获取 socket ，建立连接 
                                    
    /*
    {
        LOG_printf(DEBUG,"\n--------- 创建 socket -------------\n");
        info->socket=(Socket*)StandardSocket_construct(PF_INET, SOCK_STREAM, 0);
        if(info->socket==nullptr) //如果获取套接字失败，记录错误并返回通信错误。
        { 
            LOG_printf(DEBUG," 获取套接字失败 \n");
            return false;
        }
        else
        {
            LOG_printf(DEBUG," 获取套接字成功 \n");
            
        }


        LOG_printf(DEBUG,"--------- 设置 socket 接收缓冲区-------------\n");
        int tcpbufLen;
        socklen_t tcpbufLenSize = sizeof(tcpbufLen);
        if (getsockopt(((StandardSocket*) (info->socket))->sock, SOL_SOCKET, SO_RCVBUF, &tcpbufLen, &tcpbufLenSize) < 0) {
            std::cerr << "Failed to get socket receive buffer size" << std::endl;
            return false;
        }
        else
        LOG(DEBUG)<<"get socket receive buffer size = "<<tcpbufLen<<"\n";

        int bufSize =tcpbufLen;//8192*70;
                                // if (bufSize > 0)// 如果缓冲区大小大于0，则设置套接字的接收缓冲区大小。
                                //    StandardSocket_setSoRcvBuf(stdSock, bufSize);// 这里会设置ssock->sock参数，很可能和上面跳过的内核函数有关
        if(bufSize>0)
        {
            int val = bufSize;
            if (setsockopt(((StandardSocket*) (info->socket))->sock, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val)) < 0) {
                    std::cerr << "Failed to set socket receive buffer size" << std::endl;
                    return false;
            }
            else
                LOG(DEBUG)<<"Succesfully set socket receive buffer size = "<<val<<"\n";
        }

    // LOG(DEBUG)<<"4----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";

        LOG_printf(DEBUG,"-------------------  尝试通过IP连接-------------------\n");
        unsigned short port=8003; //@@ 8005 meta 8003 storage
        NicAddress nicAddr[11]; //@@
        initNicAddressArray_storage(nicAddr);//@@
        bool connectRes;
        connectRes = info->socket->ops->connectByIP((info->socket), nicAddr[2].ipAddr, port);//@@v  217: nicAddr[10];  162: nicAddr[2]
        if(connectRes)
        {
            LOG_printf(DEBUG," 尝试通过IP连接 连接成功 \n");

        }
        else
        {
            LOG_printf(DEBUG," 尝试通过IP连接 连接失败 \n");
        }

        // 启用TCP Keepalive
        LOG_printf(DEBUG,"-------------------   启用TCP Keepalive-------------------\n");
        int opt = 1;
        if (setsockopt(((StandardSocket*)info->socket)->sock, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
        }

        // 设置TCP Keepalive参数
        int keepalive_time = 30; // 30秒
        int keepalive_intvl = 5; // 每5秒发送一次Keepalive
        int keepalive_probes = 3; // 最多发送3次Keepalive

        if (setsockopt(((StandardSocket*)info->socket)->sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepalive_time, sizeof(keepalive_time)) < 0) {
            perror("setsockopt TCP_KEEPIDLE failed");
            exit(EXIT_FAILURE);
        }

        if (setsockopt(((StandardSocket*)info->socket)->sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepalive_intvl, sizeof(keepalive_intvl)) < 0) {
            perror("setsockopt TCP_KEEPINTVL failed");
            exit(EXIT_FAILURE);
        }

        if (setsockopt(((StandardSocket*)info->socket)->sock, IPPROTO_TCP, TCP_KEEPCNT, &keepalive_probes, sizeof(keepalive_probes)) < 0) {
            perror("setsockopt TCP_KEEPCNT failed");
            exit(EXIT_FAILURE);
        }
    // LOG(DEBUG)<<"5----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";

        // 现在，TCP连接将保持活跃状态
        LOG_printf(DEBUG,"Connection established and Keepalive is enabled.\n");

        // 这是 服务端的认证配置
        LOG_printf(DEBUG,"-------------------   服务端的认证配置 -------------------\n");

        bool isflag= __NodeConnPool_applySocketOptionsConnected(info->socket);
        LOG(DEBUG)<<"[read 服务端认证配置已经完成]\n";
        if(isflag==true)
        {
            LOG(DEBUG)<<"__NodeConnPool_applySocketOptionsConnected 成功 ！！\n";

        }
        else
        {
            LOG(DEBUG)<<"__NodeConnPool_applySocketOptionsConnected 失败 ！！\n";

        }
    }
    */
    // LOG(DEBUG)<<"6----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";

// 准备头部信息,
LOG(DEBUG)<<"--------- 准备头部信息 \n";
   info->headerSize = context->ops->prepareHeader(context, info);//
   if(info->headerSize == 0)
    {
        LOG(DEBUG)<<"[__commkit_prepare_generic ] info->headerSize == 0 \n";
        goto cleanup;
    }

   context->numAcquiredConns++;
   // 更新状态为发送头部
   //LOG(DEBUG)<<"---- 更新状态为发送头部----\n";
   info->state = CommKitState_SENDHEADER;
    // LOG(DEBUG)<<"7----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";

    //LOG(DEBUG)<<"ioinfo->state ="<<info->state<<"\n";
//LOG(DEBUG)<<"out of  __commkit_prepare_generic \n";

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
//RDMA 
/*
static bool __commkit_prepare_generic(CommKitContext* context, struct CommKitTargetInfo* info,
    char *msg_buf,size_t *msg_len,
    uint64_t vaddr,uint32_t rkey,size_t memory_len)
{   
    LOG(DEBUG)<<"go into __commkit_prepare_generic \n";
    FileOpState* currentState = container_of_v2<FileOpState,CommKitTargetInfo>(info,offsetof(FileOpState, base));


// 如果已经获取了连接，则不等待
   bool allowWaitForConn = !context->numAcquiredConns;// numAcquiredConns=0 则这个变量的值是true // don't wait if we got at least
    // one conn already (this is important to avoid a deadlock between racing commkit processes)

   info->socket = nullptr;
   info->nodeResult = -FhgfsOpsErr_COMMUNICATION;
   info->selectedTargetID = info->targetID;//statelist 中存储的信息（1）

// 为头部缓冲区分配内存
//LOG(DEBUG)<<"--为头部缓冲区分配内存 \n";
                         //info->headerBuffer = allocHeaderBuffer(allowWaitForConn ? GFP_NOFS : GFP_NOWAIT);
    size_t bufferSize = 1024; // 假设我们需要分配的缓冲区大小
    info->headerBuffer= (char *)allocHeaderBuffer(bufferSize, allowWaitForConn);
    if (info->headerBuffer == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        context->numBufferless += 1;
        LOG_printf(DEBUG,"分配头部缓冲区失败\n");

        return false;
    }

    // LOG(DEBUG)<<"2----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";

// 选择正确的目标ID并获取目标状态

   if(!context->ioInfo) // 进入这个分支
     {
         info->selectedTargetID = info->targetID;
         //LOG(DEBUG)<<"info->targetID = "<<info->targetID<<"\n";
      cout << "设置target ID = " << info->selectedTargetID << "\n";

     }

   info->node = NodeStoreEx_referenceNodeByTargetID();
   if(!info->node) 
   { // unable to resolve targetID
      info->nodeResult = -1;//负数表示错误 //-resolveErr;
      LOG_printf(DEBUG,"[__commkit_prepare_generic] 获取目标节点引用失败 unable to resolve targetID  \n");
      goto cleanup;
   }

// XYP_MODIFY 注释以下建立RDMA 连接 和通信认证的代码
    // {
    //     LOG_printf(DEBUG,"--------- 1.创建 socket -------------\n");
        
    //     struct in_addr srcAddr;
    //     NicAddress* nicAddr = (NicAddress*)malloc(sizeof(NicAddress));
    //     NicAddressStats* srcRdma = (NicAddressStats*)malloc(sizeof(NicAddressStats));

    //     const char* LocalRdmaIP = "192.168.3.212";       //本地dell02的RDMA IP,   网口名为 enp130s0np0
    //                                                     //@@本地 dell01 的RDMA IP : 192.168.0.203 ,   网口名为 enp152s0np0
    //                                                     // 164 Client: ens5f0np0 192.168.5.209 mlx5_0
    //                                                     //164 client ens4f0np0 192.168.3.212 mlx5_2

                                                        

    //     if (inet_pton(AF_INET, LocalRdmaIP, &srcAddr) <= 0) {    // 将点分十进制的IP地址转换为用于网络传输的数值格式。
    //         perror("Invalid IP address");
    //         return -1;
    //     }
    //     LOG_printf(DEBUG,"查看本主机IP情况 LocalIP %s and tranfer to %u\n", LocalRdmaIP, srcAddr.s_addr);
    //     nicAddr->ipAddr = srcAddr;
    //     nicAddr->nicType = NICADDRTYPE_RDMA;
    //     strncpy(nicAddr->name, "ens4f0np0", IFNAMSIZ);  //@@本地 RDMA 端口名 dell02 : enp130s0np0
    //                                                     //dell 01: enp152s0np0
    //                                                     //164 client ens5f0np0 192.168.5.209 mlx5_0
    //                                                     //164 client ens4f0np0 192.168.3.212 mlx5_2

    //  NicAddressStats_init(srcRdma, nicAddr);

    //     info->socket = (Socket*)RDMASocket_construct(srcAddr, srcRdma);  //构造一个RDMA socket
    //     if(!info->socket)
    //     { // no conn available => error or didn't want to wait
    //         LOG_printf(DEBUG,"----------------获取RDMA socket失败!----------------------\n");
    //         info->state = CommKitState_CLEANUP;
    //         return false;
    //     }
    //     else{
    //         LOG_printf(DEBUG,"----------------获取RDMA socket成功!----------------------\n");
    //     }

    //     LOG_printf(DEBUG,"--------- RDMA 没有对应的这个阶段 设置 socket 接收缓冲区-------------\n");
        

    // // LOG(DEBUG)<<"4----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";

    //     LOG_printf(DEBUG,"------------------- 2. 尝试通过IP连接-------------------\n");
    //     struct in_addr destAddr;
    //     const char* RemoteRdmaIP = "192.168.3.216";   //@@存储服务器xfusion3的RDMA IP 192.168.0.218,网口名 ens2np0
    //                                                     // 162 server 的 RDMA IP : 192.168.5.210 , ens4f0np0
    //                                                     // 162 server 的 RDMA IP : 192.168.3.216 , ens4f1np1


    //     if (inet_pton(AF_INET, RemoteRdmaIP, &destAddr) <= 0) {
    //         perror("Invalid remote IP address");
    //         return -1;
    //     }
    //     LOG_printf(DEBUG,"查看远端主机IP情况 RemoteIP %s and tranfer to %u\n", RemoteRdmaIP, destAddr.s_addr);

    //     unsigned short port = 8003;    //存储服务器的port
    //     LOG_printf(DEBUG,"----------------开始根据IP与远端建立RDMA连接-----------------------\n");
    //     bool connectRes = info->socket->ops->connectByIP(info->socket, destAddr, port);
    //     if(connectRes){
    //         LOG_printf(DEBUG,"尝试通过IP连接 连接成功 \n");
    //     }
    //     else
    //     {
    //         LOG_printf(DEBUG,"尝试通过IP连接 连接失败 \n");
    //     }



    //     // 启用TCP Keepalive
    //     LOG_printf(DEBUG,"------------------- RDMA 没有这个阶段  启用TCP Keepalive-------------------\n");
        

    //     // 这是 服务端的认证配置
    //     LOG_printf(DEBUG,"-------------------   服务端的认证配置 -------------------\n");

    //     bool isflag= __NodeConnPool_applySocketOptionsConnected(info->socket);
    //     LOG(DEBUG)<<"[read 服务端认证配置已经完成]\n";
    //     if(isflag==true)
    //     {
    //         LOG(DEBUG)<<"__NodeConnPool_applySocketOptionsConnected 成功 ！！\n";

    //     }
    //     else
    //     {
    //         LOG(DEBUG)<<"__NodeConnPool_applySocketOptionsConnected 失败 ！！\n";

    //     }
    // }


//XYP_MODIFY 以上关于RDMA连接的代码和通信认证的代码注释掉，保留如下创建socket的代码,
// 目的是 read_pre.hpp 中RdmaInfo_map
   {
        LOG_printf(DEBUG,"--------- 1.创建 socket -------------\n");
                
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
        LOG_printf(DEBUG,"查看本主机IP情况 LocalIP %s and tranfer to %u\n", LocalRdmaIP, srcAddr.s_addr);
        nicAddr->ipAddr = srcAddr;
        nicAddr->nicType = NICADDRTYPE_RDMA;
        strncpy(nicAddr->name, "ens4f0np0", IFNAMSIZ);  //@@本地 RDMA 端口名 dell02 : enp130s0np0
                                                        //dell 01: enp152s0np0
                                                        //164 client ens5f0np0 192.168.5.209 mlx5_0
                                                        //164 client ens4f0np0 192.168.3.212 mlx5_2

        NicAddressStats_init(srcRdma, nicAddr);

        info->socket = (Socket*)RDMASocket_construct(srcAddr, srcRdma);  //构造一个RDMA socket
        if(!info->socket)
        { // no conn available => error or didn't want to wait
            LOG_printf(DEBUG,"----------------获取RDMA socket失败!----------------------\n");
            info->state = CommKitState_CLEANUP;
            return false;
        }
        else{
            LOG_printf(DEBUG,"----------------获取RDMA socket成功!----------------------\n");
        }
        // LOG_printf(DEBUG,"info->socket->",info->socket->)
    }
// 准备头部信息,
LOG(DEBUG)<<"--------- 准备头部信息 \n";
   info->headerSize = context->ops->prepareHeader(context, info,msg_buf,msg_len,vaddr,rkey,memory_len);//
   if(info->headerSize == 0)
    {
        LOG(DEBUG)<<"[__commkit_prepare_generic ] info->headerSize == 0 \n";
        goto cleanup;
    }

   context->numAcquiredConns++;
   // 更新状态为发送头部
   //LOG(DEBUG)<<"---- 更新状态为发送头部----\n";
   info->state = CommKitState_SENDHEADER;
    // LOG(DEBUG)<<"7----------- currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";

    //LOG(DEBUG)<<"ioinfo->state ="<<info->state<<"\n";
//LOG(DEBUG)<<"out of  __commkit_prepare_generic \n";

   return true;

cleanup:
// 释放头部缓冲区
   info->state = CommKitState_CLEANUP;
   return false;

// error:
   freeHeaderBuffer(info->headerBuffer);
   info->headerBuffer = nullptr;
   return false;
   
}



*/

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
//         LOG_printf(DEBUG,"Processing info\n");

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
    //LOG(DEBUG)<<"go into __commkit_sendheader_generic \n";
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


    //LOG(DEBUG)<<"[__commkit_sendheader_generic] 完成 sendto \n";
   if(sendRes != info->headerSize) 
   {
    //   Logger_logFormatted(context->log, Log_WARNING, context->ops->logContext,
    //      "Failed to send message to %s: %s", Node_getNodeIDWithTypeStr(info->node),
    //      Socket_getPeername(info->socket) );
        LOG_printf(DEBUG,"[__commkit_sendheader_generic] sendRes != info->headerSize \n");
      info->state = CommKitState_SOCKETINVALIDATE;
      return;
   }

   info->state = CommKitState_SENDDATA; //state=2
   //LOG(DEBUG)<<"[] info->state ="<<  info->state<<"\n";
   info->headerSize = 0;
    //LOG(DEBUG)<<"out of  __commkit_sendheader_generic \n";
   //LOG(DEBUG)<<"[] info->state ="<<  info->state<<"\n";
      // 检查套接字是否正常
   int sockfd = ((StandardSocket*)info->socket)->sock;
   LOG(DEBUG) << "Socket file descriptor: " << sockfd << std::endl;


}

//---------------------- __commkit_add_socket_pollstate -------------------------------

static void __commkit_add_socket_pollstate(CommKitContext* context,struct CommKitTargetInfo* info, short pollEvents)
{   
    //LOG(DEBUG)<<"go into __commkit_add_socket_pollstate \n";
    //LOG(DEBUG)<<"[] info->state = "<<info->state<<"\n";
   PollState_addSocket(&context->pollState, info->socket, pollEvents);
   context->numPollSocks++;
    //LOG(DEBUG)<<"[] info->state = "<<info->state<<"\n";
    //LOG(DEBUG)<<"out of  __commkit_add_socket_pollstate \n";


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
                //LOG(DEBUG)<<"go into __commkit_prepare_io \n";
                             // if (fatal_signal_pending(current) || !Node_getIsActive(info->node))
                if (!Node_getIsActive(info->node))
                {
                    info->state = CommKitState_SOCKETINVALIDATE;
                    LOG(DEBUG)<<"[__commkit_prepare_io] 1.Node_getIsActive(info->node)==0 \n";
                    return false;
                }

                // check for a poll() timeout or error (all states have to be cancelled in that case)
                        // if(context->pollTimedOut || BEEGFS_SHOULD_FAIL(commkit_polltimeout, 1) )
                // 跳过 上面的
                if( context->pollTimedOut )
                {
                    info->nodeResult = -FhgfsOpsErr_COMMUNICATION;
                    info->state = CommKitState_SOCKETINVALIDATE;
                    LOG(DEBUG)<<"[__commkit_prepare_io] 2.context->pollTimedOut \n";
                    return false;
                }

            // // 检查 socket 是否已经关闭
            //     if (info->socket->is_closed()) { // 假设您有一个方法来检查 socket 是否关闭
            //         std::cerr << "[__commkit_prepare_io]4. Error: socket is closed" << std::endl;
            //         return false;
            //     }

                //LOG(DEBUG)<<"info->socket->poll.revents = "<<info->socket->poll.revents<<"\n"; //0 实际发生的事件是 
                //LOG(DEBUG)<<"info->socket->poll._events = "<<info->socket->poll._events<<"\n"; //1 要监听的事件是 POLLIN：有数据可读。这是最常见的事件，用于检查 socket 是否有数据可以被读取。
                //LOG(DEBUG)<<"预期的事件是 events = "<<events <<"\n"; //POLLIN =1 
                if(!(info->socket->poll.revents & events) )
                {
                    LOG(DEBUG)<<"[__commkit_prepare_io] 3.prepare go into __commkit_add_socket_pollstate \n";
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
                    LOG(DEBUG)<<"before [Socket_recvT] iter->iov_len ="<<iter->iov_len<<"\n";

                    user_iov_iter_truncate(iter, length);
                    LOG(DEBUG)<<"after [Socket_recvT] iter->iov_len ="<<iter->iov_len<<"\n";

                {
                    // ssize_t nread = thissock->ops->recvT(thissock, &copy, flags, timeoutMS);
                    ssize_t nread = thissock->ops->recvT(thissock, iter, flags, timeoutMS);

                    LOG(DEBUG)<<"[Socket_recvT] nread= "<<nread<<"\n";
                    // for (size_t i = 0; i < nread; ++i) {
                    //     LOG(DEBUG) << std::hex << static_cast<int>(((char *)(iter->iov_base))[i]) << " ";
                    // }
                    LOG(DEBUG)<<"\n";
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
    LOG(DEBUG)<<"[__commkit_readfile_receive] : prepare goto Socket_recvExactT \n";
      recvRes = Socket_recvExactT(socket, iter, length, 0, timeoutMS);
      LOG(DEBUG)<<"[__commkit_readfile_receive] : recvRes = "<<recvRes<<"\t, length = "<<length<<"\n";
    //    for (size_t i = 0; i < length; ++i) {
    //     LOG(DEBUG) << std::hex << static_cast<int>(((char *)(iter->iov_base))[i]) << " ";
    //     }
      //LOG(DEBUG)<<"\n iter->iov_len ="<<iter->iov_len<<"\n";
          //LOG(DEBUG)<<"[__commkit_readfile_receive] : out of Socket_recvExactT , recvRes = "<<recvRes<<"\n";


   }
   else// 进入的是这个分支
   {
                //recvRes = Socket_recvT(socket, iter, length, 0, cfg->connMsgLongTimeout);
    //LOG(DEBUG)<<"[__commkit_readfile_receive] : prepare goto Socket_recvExactT \n";

      recvRes = Socket_recvT(socket, iter, length, 0, timeoutMS);
      LOG(DEBUG)<<"recvRes = "<<recvRes<<"\t, length = "<<length<<"\n";
    //    for (size_t i = 0; i < recvRes; ++i) {
    //     LOG(DEBUG) << std::hex << static_cast<int>(((char *)(iter->iov_base))[i]) << " ";
    //     }
          //LOG(DEBUG)<<"[__commkit_readfile_receive] : out of Socket_recvExactT , recvRes = "<<recvRes<<"\n";


   }

   if(recvRes < 0) 
   {
                //   Logger_logFormatted(context->log, Log_SPAM, context->ops->logContext,
                //      "Request details: receive from %s: %lld bytes (error %zi)",
                //      Node_getNodeIDWithTypeStr(currentState->base.node), (long long)length, recvRes);
                // LOG_printf(DEBUG,"Request details: receive from %s: %lld bytes (error %zd)\n",
                //        Node_getNodeIDWithTypeStr(currentState->base.node), (long long)length, recvRes);
        LOG_printf(DEBUG,"Request details: receive  %lld bytes (error %zd)\n", (long long)length, recvRes);
   }

   return recvRes;
}

                    static int __commkit_readfile_recvdata_prefix(CommKitContext* context, FileOpState* currentState)
                    {       
                        LOG(DEBUG)<<"go into [__commkit_readfile_recvdata_prefix] \n";
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
                            LOG(DEBUG)<<"after __commkit_readfile_receive ,recvRes = "<<recvRes <<"\n";
                            // for (size_t i = 0; i < sizeof(int64_t); ++i) {
                            //     LOG(DEBUG) << std::hex << static_cast<int>(((char *)(iter.iov_base))[i]) << " ";
                            // }
                            //LOG(DEBUG)<<"\n iter->iov_len ="<<iter.iov_len<<"\n";
                            
                            if(recvRes < 0)
                                return recvRes;
                            if (recvRes == 0)
                                return -ECOMM;

                            // got the length info response
                            Serialization_deserializeInt64(&ctx, &lengthInfo);

                            if(lengthInfo <= 0)// 第一次传输的8B 的字节获取给 lengthInfo 确实是0
                            { // end of file data transmission
                            //LOG(DEBUG)<<"end of file data transmission \n";
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

                                LOG_printf(DEBUG,"[__commkit_readfile_recvdata_prefix] Bug: Received a lengthInfo that would overflow request from %lld %zu %zu \n",
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
                LOG(DEBUG)<<"Go into __commkit_readfile_recvdata \n";
                    // 从目标信息结构体中获取当前状态结构体

                    FileOpState* currentState = container_of_v2<FileOpState,CommKitTargetInfo>(info,offsetof(FileOpState, base));

                    
                    // 获取 iov_iter 指针，它用于处理分散的内存区域
                            // struct iov_iter *iter = &currentState->data;
                    struct iovec *iter = &currentState->data;
                    if (iter ==nullptr)
                    {
                        LOG(DEBUG)<<"[__commkit_readfile_recvdata] iter ==nullptr \n";
                    }
                    // 计算还需要接收的数据长度
                    size_t missingLength = currentState->toBeTransmitted - currentState->transmitted;//currentState->totalSize - currentState->transmitted;//currentState->toBeTransmitted - currentState->transmitted;
                    LOG(DEBUG)<<"iter->iov_len = "<<iter->iov_len<<"\t,";
                    LOG(DEBUG)<<" currentState->totalSize = "<<currentState->totalSize<<",\t currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";
                    ssize_t recvRes;

                                 
                                //跳过 
                                        // #ifdef BEEGFS_NVFS
                                        //
                                        // If we are using the RDMA message, then the protocol is simply to wait for
                                        // a reply which is either an error code ( < 0 ), no data ( = 0 ) or length of
                                        // data already transferred ( > 0 ).
                                        //// 如果使用 RDMA 消息，则等待回复，回复可能是错误码（<0），没有数据（=0）或已传输数据的长度（>0）
                                        // LOG(DEBUG)<<"currentState->rdmap= "<<currentState->rdmap<<"\n";
                                        // if (currentState->rdmap)
                                        // {
                                        //     __commkit_readfile_recvdata_prefix(context, currentState);
                                        //     LOG(DEBUG)<<"currentState->toBeTransmitted = "<<currentState->toBeTransmitted<<"\n";
                                        //     // if (currentState->toBeTransmitted > 0)
                                        //     // {
                                        //     //     currentState->transmitted = currentState->toBeTransmitted;
                                        //     //     LOG(DEBUG)<<"currentState->transmitted = "<<currentState->transmitted<<"\n";
                                        //     //     currentState->base.nodeResult = currentState->transmitted;
                                        //     //     LOG(DEBUG)<<"currentState->base.nodeResult =  "<<currentState->base.nodeResult<<"\n";
                                        //     // }
                                        //     currentState->receiveFileData = false;
                                        //     // return 0;
                                        // }
                                        // #endif // BEEGFS_NVFS
                                

                    // 如果还没有开始接收文件数据，则调用前缀处理函数
                    // if(!currentState->receiveFileData)
                    //     {
                    //         LOG(DEBUG)<<"!currentState->receiveFileData \n";
                    //         return __commkit_readfile_recvdata_prefix(context, currentState);// 这个只接收 8B 
                    //         // LOG(DEBUG)<<"after __commkit_readfile_recvdata_prefix \n";


                    //     }

LOG(DEBUG)<<"iter->iov_len = "<<iter->iov_len<<"\t,";
                    LOG(DEBUG)<<" currentState->totalSize = "<<currentState->totalSize<<",\t currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";
                   
                    // 如果 iov_iter 没有数据，则获取下一个 iov_iter
                    // LOG(DEBUG)<<"如果 iov_iter 没有数据，则获取下一个 iov_iter \n";
                    if(user_iov_iter_count(iter) == 0)//确实等于0
                    {   
                        LOG(DEBUG)<<"user_iov_iter_count(iter) == 0 \n";
                        ((struct ReadfileIterOps*) context->private_data)->nextIter(context, currentState); //这个函数的实现还没找到哦，主要是得看 private 的初始化
                                    // BUG_ON(iov_iter_count(iter) == 0);
                        LOG(DEBUG)<<"user_iov_iter_count(iter) =="<<user_iov_iter_count(iter)<<"\n";
                    
                    }

                    LOG(DEBUG)<<"iter->iov_len = "<<iter->iov_len<<"\t,";
                    LOG(DEBUG)<<" currentState->totalSize = "<<currentState->totalSize<<",\t currentState->toBeTransmitted ="<<currentState->toBeTransmitted<<"\t"<<"currentState->transmitted = "<<currentState->transmitted<<"\n";
                    
                    // iter->iov_len=75; 
                    // 接收可用的数据部分
                    // receive available dataPart
                    LOG(DEBUG)<<"接收可用的数据部分 \n";
                    LOG(DEBUG)<<"missingLength = "<<missingLength<<"\n";
                    // recvRes = __commkit_readfile_receive(context, currentState, iter, missingLength, false);//这里是接收数据包的逻辑
                    recvRes = __commkit_readfile_receive(context, currentState, iter, missingLength, true);//这里是接收数据包的逻辑

                    
                    LOG(DEBUG)<<" 【@@__commkit_readfile_recvdata 】 recvRes = "<<recvRes<<"\n";
                    int nonzero_cnt=0;
                    for (size_t i = 0; i < recvRes; ++i) {
                        // LOG(DEBUG) << std::hex << static_cast<int>(((char *)(iter->iov_base))[i]) << " ";
                        if(static_cast<int>(((char *)(iter->iov_base))[i])!= 0)
                        {
                            // LOG(DEBUG)<<" 非 0 \n";
                            nonzero_cnt++;
                        }
                    }
                    LOG(DEBUG)<<"nonzero_cnt = "<<nonzero_cnt<<"\n";
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
   //LOG(DEBUG)<<"go into __commkit_recvdata_generic \n";
    // 这个注释的 prepare_io 失败的原因是 调用 的是原生 beegfs 的socket 对应的逻辑 ，要想实现类似的功能 参考 open.hpp 中  StandardSocket_recvfromT（）这个函数中的轮询套接字
//    if(!__commkit_prepare_io(context, info, POLLIN) )
//       {
//         LOG(DEBUG)<<"__commkit_prepare_io error \n";//调试 发现进入的是这个分支
//         return;
//       }
    //LOG(DEBUG)<<"---- 接收数据 ----- \n";
   recvRes = context->ops->recvData(context, info);//接收消息的代码
    LOG(DEBUG)<<"[__commkit_recvdata_generic] recvRes= "<<recvRes<<"\n";
   if(recvRes < 0)
   {
      if(recvRes == -EFAULT)
      { 
                    // bad buffer address given
                    //  Logger_logFormatted(context->log, Log_DEBUG, context->ops->logContext,
                    //     "Bad buffer address");
        LOG_printf(DEBUG,"[__commkit_recvdata_generic ] Bad buffer address \n");

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

        LOG_printf(DEBUG,"[__commkit_recvdata_generic ] Communication timeout in RECVDATA stage. \n");
      }
      else
      { // error
                        //  Logger_logErrFormatted(context->log, context->ops->logContext,
                        //     "Communication error in RECVDATA stage. Node: %s (recv result: %lld)",
                        //     Node_getNodeIDWithTypeStr(info->node), (long long)recvRes);
        
        LOG_printf(DEBUG,"[__commkit_recvdata_generic ] Communication error in RECVDATA stage.  \n");

      }

      info->state = CommKitState_SOCKETINVALIDATE;
      return;
   }

   if(recvRes == 0)
      {
        info->state = CommKitState_CLEANUP;
        LOG(DEBUG)<<"recvRes = "<<recvRes<<"\n";

      }
   else
      {
        LOG(DEBUG)<<"recvRes = "<<recvRes<<"\n";
        if(recvRes ==1)
        {
            LOG(DEBUG)<<"此时要继续接收数据 \n";
        }
        __commkit_add_socket_pollstate(context, info, POLLIN);
      }
      //LOG(DEBUG)<<"out of __commkit_recvdata_generic \n";
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
                            LOG_printf(DEBUG,"A fatal signal is pending.\n");
                            info->nodeResult = -FhgfsOpsErr_INTERRUPTED;
                        } 
                        else 
                        {
                            LOG_printf(DEBUG,"No fatal signals are pending.\n");
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

//XYP_INsert_250301
static unsigned RDMA_readfile_prepareHeader(CommKitContext* context,struct CommKitTargetInfo* info,
    char *msg_buf,size_t *msg_len,
    uint64_t vaddr,uint32_t rkey,size_t memory_len)//准备头部消息
{   
    LOG(DEBUG)<<"go into __commkit_readfile_prepareHeader \n";
    NumNodeID localNodeNumID ;
    localNodeNumID.value=5;         //@@ Node_getNumID(localNode);//this->numID;
    // 162 sever 对应的164 client ID =12 // 224 xfusion3 [ID: 6]  //217 dell01 [ID: 5]


   FileOpState* currentState = container_of_v2<FileOpState,CommKitTargetInfo>(info, offsetof(FileOpState, base));
//    currentState->offset=0;//这个参数为什么怎么改也不影响结果
    //LOG(DEBUG)<<"输出 currentState 的参数列表 ：\n";
   {
    //  LOG(DEBUG)<<"transmitted = "<<currentState->transmitted<<"\n";
    //  LOG(DEBUG)<<"toBeTransmitted = "<<currentState->toBeTransmitted<<"\n";
    //  LOG(DEBUG)<<"totalSize = "<<currentState->totalSize<<"\n";  //@@ 321 是哪里来的
    //  LOG(DEBUG)<<"offset = "<<currentState->offset<<"\n";        //@@
    //  LOG(DEBUG)<<"firstWriteDoneForTarget = "<<currentState->firstWriteDoneForTarget<<"\n";
    //  LOG(DEBUG)<<"receiveFileData = "<<currentState->receiveFileData<<"\n";
    //  LOG(DEBUG)<<"expectedNodeResult = "<<currentState->expectedNodeResult<<"\n";//数值奇怪
    //  LOG(DEBUG)<<"data = ";
    //  for (size_t i = 0; i < sizeof(int64_t); ++i) {
    //     // LOG(DEBUG) << std::hex << static_cast<int>(((char *)(currentState->data.iov_base))[i]) << " ";
    //     LOG(DEBUG) << std::hex << static_cast<int>((((char *)currentState->data.iov_base))[i]) << " ";

    //     }
      //LOG(DEBUG)<<"\n iter->iov_len ="<<currentState->data.iov_len<<"\n";//数值奇怪
    //  LOG(DEBUG)<<"transmitted = "<<currentState->transmitted<<"\n";
    //  LOG(DEBUG)<<"transmitted = "<<currentState->transmitted<<"\n";


   }
   //LOG(DEBUG)<<"------------------1--------------------\n";
   struct ReadfileIterOps* ops = (ReadfileIterOps*)(context->private_data); 
   //LOG(DEBUG)<<"------------------2--------------------\n";

   NetMessage *netMessage = nullptr;
   ReadLocalFileV2Msg readV2Msg;//消息类型
//    ReadLocalFileRDMAMsg readRDMAMsg;//消息类型 $$%

   //跳过 下面这个RDMA 分支
   LOG(DEBUG)<<" currentState->offset = "<< currentState->offset<<"\n";//这个参数也是正确的

   LOG(DEBUG)<<" currentState->totalSize = "<<currentState->totalSize<<"\n";//这个参数是正确的
    //$$%
    // #ifdef BEEGFS_NVFS
    LOG(DEBUG)<<"go into BEEGFS_NVFS \n";




    ReadLocalFileRDMAMsg readRDMAMsg;

    currentState->rdmap = NULL;

   


        struct FileOpVecState* vecState = container_of_v2<FileOpVecState,FileOpState>(currentState, offsetof(FileOpVecState, base));
        // struct FileOpVecState* vecState = container_of(currentState, struct FileOpVecState, base);
        // currentState->rdmap = RdmaInfo_mapRead(&vecState->data, info->socket);
        //XYP_MODIFY
        currentState->rdmap = RdmaInfo_mapRead(&vecState->data, info->socket,vaddr,rkey,memory_len);

        LOG(DEBUG)<<"currentState->rdmap = "<<currentState->rdmap<<"\n";
    

        ReadLocalFileRDMAMsg_initFromSession(&readRDMAMsg, localNodeNumID,
            context->ioInfo->fileHandleID, info->targetID, context->ioInfo->pathInfo,
            context->ioInfo->accessFlags, currentState->offset, currentState->totalSize,
            currentState->rdmap);

        netMessage = &readRDMAMsg.netMessage;
    
    // #endif // BEEGFS_NVFS
    
   LOG(DEBUG)<<"----prepare message -5--------------------\n";

   NetMessage_setMsgHeaderTargetID(netMessage, info->selectedTargetID);

   
   //只有对要发送的header进行序列化的操作
   NetMessage_serialize(netMessage, info->headerBuffer, BEEGFS_COMMKIT_MSGBUF_SIZE);
    *msg_len = NetMessage_getMsgLength(netMessage);
   memcpy(msg_buf, info->headerBuffer, BEEGFS_COMMKIT_MSGBUF_SIZE);

   //LOG(DEBUG)<<"------------------5--------------------\n";

   currentState->transmitted = 0;
//    currentState->toBeTransmitted = 0;
   currentState->receiveFileData = false;
   beegfs_iov_iter_clear(&currentState->data);
   //LOG(DEBUG)<<"------------------6--------------------\n";


   if(ops->prepare)//null
     {
         ops->prepare(context, currentState);  // 这个prepare 的具体实现在哪里
   //LOG(DEBUG)<<"------------------7--------------------\n";

     }
   //LOG(DEBUG)<<"------------------8--------------------\n";


   return NetMessage_getMsgLength(netMessage);
}



// #include "new_read.hpp"
// #include "open.hpp"
#include <algorithm> // 用于 std::min 和 std::max
#include <iostream>
using namespace std;





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
        LOG(DEBUG)<<"error \n";
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
      LOG(DEBUG)<<"[beegfs_readsink_init] readsink->buffer = "<<static_cast<void*>(readsink->buffer) <<"\n";
      if (!readsink->buffer) {
         LOG(DEBUG)<<"Failed to allocate memory for readsink \n";
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
      LOG(DEBUG)<<"maxReadSize = "<<maxReadSize<<"\n";

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


//XYP_MODIFY 
// 将 read.cpp中的代码移动到这里




// 可以说 这里就是真正的通信代码 ，不仅read write 也得调用这个代码
            //void FhgfsOpsCommkit_communicate(App* app, RemotingIOInfo* ioInfo, struct list_head_t* targetInfos,const struct CommKitContextOps* ops, void* private_data) 
/*void FhgfsOpsCommkit_communicate(RemotingIOInfo* ioInfo, struct list_head* targetInfos,const struct CommKitContextOps* ops, void* private_data,
   char *msg_buf,size_t *msg_len,
   uint64_t vaddr,uint32_t rkey,size_t memory_len)
{
   // 获取应用程序的配置
                            // Config* cfg = App_getConfig(app);
   // 初始化状态计数器
   LOG(DEBUG)<<"\n ----- Go into FhgfsOpsCommkit_communicate -----------\n";
   int numStates = 0;

   // 1. 定义并初始化通信上下文结构体
   CommKitContext context = 
   {
      .ops = ops, 
                           // .app = app, 
                           // .log = App_getLogger(app),
      .private_data = private_data,
      .ioInfo = ioInfo,
      .targetInfoList = targetInfos,
      .numRetryWaiters = 0, // counter for states that encountered a comm error
      .numDone = 0, // counter for finished states
      .numAcquiredConns = 0, // counter for currently acquired conns (to wait only for first conn)
      .pollTimedOut = false,
      .pollTimeoutLogged = false,
      .connFailedLogged = false,
      .currentRetryNum = 0,
      .maxNumRetries = 10,//随便先设置这个值吧 Config_getConnNumCommRetries(cfg),
      // 下面这个 RDMA 先注释掉 $$$%
      // #ifdef BEEGFS_NVFS
            // .gpudRc = -1,
            .gpudRc = 1,//$$$%//XYP_0303

      // #endif
   };

   // 2. 进入循环，直到所有状态都完成
   LOG(DEBUG)<<"--- 进入循环，直到所有状态都完成 \n";
   do
   {
      struct CommKitTargetInfo* info;

      // 重置上下文中的计数器
      context.numUnconnectable = 0; // will be increased by states the didn't get a conn
      context.numPollSocks = 0; // will be increased by the states that need to poll
      context.numBufferless = 0;
      numStates = 0;

      // 初始化轮询状态
      LOG(DEBUG)<<"--- 初始化轮询状态 \n";
      PollState_init(&context.pollState);
      context.pollState.nfds = 0; //$$%
      // 3.遍历目标信息列表，对每个状态执行相应的操作

      
               // list_for_each_entry(&info, targetInfos, targetInfoList)
      // list_for_each_entry(&info, targetInfos)
      LOG(DEBUG)<<"--- 遍历目标信息列表，对每个状态执行相应的操作"<<"\n";
         

      
      //for(*pos = get_container_of(targetInfos->next);&(*pos)->targetInfoList != targetInfos;*pos = get_container_of((*pos)->targetInfoList.next))
      int xun_cnt=0;
      list_head_t *pos;
      for (pos = targetInfos->next; pos != targetInfos && pos!=nullptr; pos = pos->next) 
      {  
         xun_cnt++;
         LOG(DEBUG)<<"循环第 "<<xun_cnt<<" 次\n";
         info = container_of_v2<CommKitTargetInfo, list_head_t>(pos, offsetof(CommKitTargetInfo, targetInfoList));
         numStates++;
         LOG(DEBUG)<<"info->state = "<<info->state<<"\n";//0
         FileOpState* tmp_state=container_of_v2<FileOpState, CommKitTargetInfo>(info, offsetof(FileOpState, base));
         LOG(DEBUG)<<"tmp_state->offset = "<<tmp_state->offset<<"\n";//0
         LOG(DEBUG)<<"tmp_state->toBeTransmitted ="<<tmp_state->toBeTransmitted<<"\t"<<"tmp_state->transmitted = "<<tmp_state->transmitted<<"\n";




         switch(info->state)
         {
            case CommKitState_PREPARE: 
               // 准备通信
               LOG(DEBUG)<<"--- 1.准备通信 \n";

               if(!__commkit_prepare_generic(&context, info,msg_buf,msg_len,vaddr,rkey,memory_len) )//state =1
                  break;
               //XYP_MODIFY
               break;//NOTE: 这个break 是我自己加的,目的是只生成rdma_read_msg不实际进行RDMA_READ通信
               BEEGFS_FALLTHROUGH;
            case CommKitState_SENDHEADER:
               // 发送头部信息
               LOG(DEBUG)<<"--- 2.发送头部信息 \n";
               // LOG(DEBUG)<<"tmp_state->toBeTransmitted ="<<tmp_state->toBeTransmitted<<"\t"<<"tmp_state->transmitted = "<<tmp_state->transmitted<<"\n";
            //    requestedheader->iov_base=info->headerBuffer;
            //    requestedheader->iov_len=info->headerSize;
            //    break;//之所以要这么加，是因为下面的步骤，全部交给 GPUNetIO 做               // __commkit_sendheader_generic(&context, info);
               //XYP_MODIFY
               __commkit_sendheader_generic(&context, info);

               // LOG(DEBUG)<<"tmp_state->toBeTransmitted ="<<tmp_state->toBeTransmitted<<"\t"<<"tmp_state->transmitted = "<<tmp_state->transmitted<<"\n";
               __commkit_add_socket_pollstate(&context, info,context.ops->sendData ? POLLOUT : POLLIN);
               // LOG(DEBUG)<<"tmp_state->toBeTransmitted ="<<tmp_state->toBeTransmitted<<"\t"<<"tmp_state->transmitted = "<<tmp_state->transmitted<<"\n";
               //break;//@@ Note : 源代码并没有注释这个
               if(0)//@@ Note :
               {
                  
               }
               BEEGFS_FALLTHROUGH;//@@ Note : 这个是我自己加的 ，源代码并没有
            
            case CommKitState_SENDDATA:
               // 发送数据
               LOG(DEBUG)<<"--- 3.发送数据 \n";

               if(context.ops->sendData) //下面这个分支不能进去，readfileops 没有定义 senddata的操作，所以 context.ops->sendData设置为==0
               {
                              //  __commkit_senddata_generic(&context, info);
                  LOG(DEBUG)<<"[FhgfsOpsCommkit_communicate] read file error go into sendData stage \n";
                  break;
               }
               BEEGFS_FALLTHROUGH;//用于在 switch-case 语句中故意让一个 case 语句执行完毕后，继续执行下一个 case 语句的代码。

            case CommKitState_RECVHEADER:
               // 接收头部信息
               LOG(DEBUG)<<"--- 4.接收头部信息 \n";

               if(context.ops->recvHeader)//下面这个分支不进去，readfileops 没有定义 recvheader 的操作，所以 context.ops->recvHeader 设置为==0
               {
                                 // __commkit_recvheader_generic(&context, info);
                  LOG(DEBUG)<<"[FhgfsOpsCommkit_communicate] read file error go into recvHeader stage \n";

                  break;
               }
               BEEGFS_FALLTHROUGH;
            
            case CommKitState_RECVDATA:
               // 接收数据
               LOG(DEBUG)<<"--- 5.接收数据 \n";

               if(context.ops->recvData)
               {  
                  LOG(DEBUG)<<"prepare  into __commkit_recvdata_generic \n";
                  __commkit_recvdata_generic(&context, info);
                  break;
               }
               BEEGFS_FALLTHROUGH;
           
            case CommKitState_CLEANUP:
               // 清理操作
               LOG(DEBUG)<<"--- 6.清理操作 \n";

               __commkit_cleanup_generic(&context, info);//正常执行的话 下一步state 是 CommKitState_DONE
               break;
             
            // case CommKitState_SOCKETINVALIDATE:
            //    // 使套接字无效
            //    __commkit_socketinvalidate_generic(&context, info);
            //    __commkit_cleanup_generic(&context, info);
            //    break;

            // case CommKitState_RETRYWAIT:
            
            case CommKitState_DONE:
               // 这些状态不需要额外操作
               LOG(DEBUG)<<"--- 7.这些状态不需要额外操作 \n";

               break;

            default:
               // 未知状态，可能是错误
               // BUG();
               LOG(DEBUG)<<"--- 8.[FhgfsOpsCommkit_communicate] 未知状态，可能是错误 \n";
            
         }
      }

      // 如果有需要轮询的套接字，执行轮询
      // if(context.numPollSocks)
      //    __FhgfsOpsCommKitCommon_pollStateSocks(&context, numStates);
      // else if( 
      //    context.numRetryWaiters &&
      //    ( (context.numDone + context.numRetryWaiters) == numStates)  )// 如果所有状态都完成了，或者有重试等待者并且其他状态都完成了，开始重试
      // {
      //    __commkit_start_retry(&context, context.ops->retryFlags);
      // }
      break;//我自己加的
   } while(context.numDone != numStates);

}
*/

void FhgfsOpsCommkit_communicate(RemotingIOInfo* ioInfo, struct list_head* targetInfos,const struct CommKitContextOps* ops, void* private_data,struct iovec* requestedheader) 
{
   // 获取应用程序的配置
                            // Config* cfg = App_getConfig(app);
   // 初始化状态计数器
   LOG(DEBUG)<<"\n ----- Go into FhgfsOpsCommkit_communicate -----------\n";
   int numStates = 0;

   // 1. 定义并初始化通信上下文结构体
   CommKitContext context = 
   {
      .ops = ops, 
                           // .app = app, 
                           // .log = App_getLogger(app),
      .private_data = private_data,
      .ioInfo = ioInfo,
      .targetInfoList = targetInfos,
      .numRetryWaiters = 0, // counter for states that encountered a comm error
      .numDone = 0, // counter for finished states
      .numAcquiredConns = 0, // counter for currently acquired conns (to wait only for first conn)
      .pollTimedOut = false,
      .pollTimeoutLogged = false,
      .connFailedLogged = false,
      .currentRetryNum = 0,
      .maxNumRetries = 10,//随便先设置这个值吧 Config_getConnNumCommRetries(cfg),
      // 下面这个 RDMA 先注释掉
                           // #ifdef BEEGFS_NVFS
                           //       .gpudRc = -1,
                           // #endif
   };

   // 2. 进入循环，直到所有状态都完成
   LOG(DEBUG)<<"--- 进入循环，直到所有状态都完成 \n";
   do
   {
      struct CommKitTargetInfo* info;

      // 重置上下文中的计数器
      context.numUnconnectable = 0; // will be increased by states the didn't get a conn
      context.numPollSocks = 0; // will be increased by the states that need to poll
      context.numBufferless = 0;
      numStates = 0;

      // 初始化轮询状态
      LOG(DEBUG)<<"--- 初始化轮询状态 \n";
      PollState_init(&context.pollState);

      // 3.遍历目标信息列表，对每个状态执行相应的操作
                     /*
                        list_for_each_entry是 Linux 内核中用于遍历链表的宏,它是一个便捷的方式来迭代链表中的每个元素。
                        在每次迭代中，它通过结构体成员（targetInfoList）来获取链表中的下一个元素，并将其地址赋值给 info 变量。
                        struct CommKitTargetInfo
                        {
                           struct list_head targetInfoList;//用于将多个 CommKitTargetInfo 结构体链接在一起，形成一个链表。
                           。。。
                        }
                     */
      
               // list_for_each_entry(&info, targetInfos, targetInfoList)
      // list_for_each_entry(&info, targetInfos)
      LOG(DEBUG)<<"--- 遍历目标信息列表，对每个状态执行相应的操作"<<"\n";
         
         /*
            CommKitTargetInfo **pos=&info;
            for(*pos = get_container_of(targetInfos->next);&(*pos)->targetInfoList != targetInfos;*pos = get_container_of((*pos)->targetInfoList.next))
            for(info = get_container_of(targetInfos->next);&(info)->targetInfoList != targetInfos;info = get_container_of((info)->targetInfoList.next))
            LOG(DEBUG)<<"info->targetID = "<<get_container_of(targetInfos)->targetID<<"\n";
            
            info = get_container_of(targetInfos->next);
            LOG(DEBUG)<<"info->targetID = "<<info->targetID<<"\n";
            for(info = get_container_of(targetInfos->next);&(info)->targetInfoList != targetInfos;info = get_container_of((info)->targetInfoList.next))
            list_head_t *pos;
            for (pos = targetInfos->next; pos != targetInfos && pos!=nullptr; pos = pos->next) 
            {
               // if(pos==nullptr)
               // {
               //    break;
               // }
               info = container_of_v2<CommKitTargetInfo, list_head_t>(pos, offsetof(CommKitTargetInfo, targetInfoList));
                     LOG(DEBUG)<<"info->state = "<<info->state<<"\n";
               
               LOG(DEBUG)<<"info->targetID = "<<info->targetID<<"\n";
                     LOG(DEBUG)<<"------------------\n";

            }
            break;
         */
      
      //for(*pos = get_container_of(targetInfos->next);&(*pos)->targetInfoList != targetInfos;*pos = get_container_of((*pos)->targetInfoList.next))
      int xun_cnt=0;
      list_head_t *pos;
      for (pos = targetInfos->next; pos != targetInfos && pos!=nullptr; pos = pos->next) 
      {  
         xun_cnt++;
         LOG(DEBUG)<<"循环第 "<<xun_cnt<<" 次\n";
         info = container_of_v2<CommKitTargetInfo, list_head_t>(pos, offsetof(CommKitTargetInfo, targetInfoList));
         numStates++;
         LOG(DEBUG)<<"info->state = "<<info->state<<"\n";//0
         FileOpState* tmp_state=container_of_v2<FileOpState, CommKitTargetInfo>(info, offsetof(FileOpState, base));
         LOG(DEBUG)<<"tmp_state->offset = "<<tmp_state->offset<<"\n";//0
         LOG(DEBUG)<<"tmp_state->toBeTransmitted ="<<tmp_state->toBeTransmitted<<"\t"<<"tmp_state->transmitted = "<<tmp_state->transmitted<<"\n";




         switch(info->state)
         {
            case CommKitState_PREPARE: 
               // 准备通信
               LOG(DEBUG)<<"--- 1.准备通信 \n";
                //241223 之后，这个模块的通信建立代码被注释掉
               if(!__commkit_prepare_generic(&context, info) )//state =1
                  break;
               BEEGFS_FALLTHROUGH;
            case CommKitState_SENDHEADER:
               // 发送头部信息
               LOG(DEBUG)<<"--- 2.发送头部信息 \n";
               requestedheader->iov_base=info->headerBuffer;
               requestedheader->iov_len=info->headerSize;
               break;//之所以要这么加，是因为下面的步骤，全部交给 GPUNetIO 做
               // LOG(DEBUG)<<"tmp_state->toBeTransmitted ="<<tmp_state->toBeTransmitted<<"\t"<<"tmp_state->transmitted = "<<tmp_state->transmitted<<"\n";
               __commkit_sendheader_generic(&context, info);//state =2
               // LOG(DEBUG)<<"tmp_state->toBeTransmitted ="<<tmp_state->toBeTransmitted<<"\t"<<"tmp_state->transmitted = "<<tmp_state->transmitted<<"\n";
               __commkit_add_socket_pollstate(&context, info,context.ops->sendData ? POLLOUT : POLLIN);
               // LOG(DEBUG)<<"tmp_state->toBeTransmitted ="<<tmp_state->toBeTransmitted<<"\t"<<"tmp_state->transmitted = "<<tmp_state->transmitted<<"\n";
               //break;//@@ Note : 源代码并没有注释这个
            //    break;//之所以要这么加，是因为下面的步骤，全部交给 GPUNetIO 做
               if(0)//@@ Note :
               {
                  
               }
               BEEGFS_FALLTHROUGH;//@@ Note : 这个是我自己加的 ，源代码并没有
            
            case CommKitState_SENDDATA:
               // 发送数据
               LOG(DEBUG)<<"--- 3.发送数据 \n";

               if(context.ops->sendData) //下面这个分支不能进去，readfileops 没有定义 senddata的操作，所以 context.ops->sendData设置为==0
               {
                              //  __commkit_senddata_generic(&context, info);
                  LOG(DEBUG)<<"[FhgfsOpsCommkit_communicate] read file error go into sendData stage \n";
                  break;
               }
               BEEGFS_FALLTHROUGH;//用于在 switch-case 语句中故意让一个 case 语句执行完毕后，继续执行下一个 case 语句的代码。

            case CommKitState_RECVHEADER:
               // 接收头部信息
               LOG(DEBUG)<<"--- 4.接收头部信息 \n";

               if(context.ops->recvHeader)//下面这个分支不进去，readfileops 没有定义 recvheader 的操作，所以 context.ops->recvHeader 设置为==0
               {
                                 // __commkit_recvheader_generic(&context, info);
                  LOG(DEBUG)<<"[FhgfsOpsCommkit_communicate] read file error go into recvHeader stage \n";

                  break;
               }
               BEEGFS_FALLTHROUGH;
            
            case CommKitState_RECVDATA:
               // 接收数据
               LOG(DEBUG)<<"--- 5.接收数据 \n";

               if(context.ops->recvData)
               {  
                  LOG(DEBUG)<<"prepare  into __commkit_recvdata_generic \n";
                  __commkit_recvdata_generic(&context, info);
                  break;
               }
               BEEGFS_FALLTHROUGH;
           
            case CommKitState_CLEANUP:
               // 清理操作
               LOG(DEBUG)<<"--- 6.清理操作 \n";

               __commkit_cleanup_generic(&context, info);//正常执行的话 下一步state 是 CommKitState_DONE
               break;
             /*
            case CommKitState_SOCKETINVALIDATE:
               // 使套接字无效
               __commkit_socketinvalidate_generic(&context, info);
               __commkit_cleanup_generic(&context, info);
               break;

            case CommKitState_RETRYWAIT:
            */
            case CommKitState_DONE:
               // 这些状态不需要额外操作
               LOG(DEBUG)<<"--- 7.这些状态不需要额外操作 \n";

               break;

            default:
               // 未知状态，可能是错误
               // BUG();
               LOG(DEBUG)<<"--- 8.[FhgfsOpsCommkit_communicate] 未知状态，可能是错误 \n";
            
         }
      }

      // 如果有需要轮询的套接字，执行轮询
      // if(context.numPollSocks)
      //    __FhgfsOpsCommKitCommon_pollStateSocks(&context, numStates);
      // else if( 
      //    context.numRetryWaiters &&
      //    ( (context.numDone + context.numRetryWaiters) == numStates)  )// 如果所有状态都完成了，或者有重试等待者并且其他状态都完成了，开始重试
      // {
      //    __commkit_start_retry(&context, context.ops->retryFlags);
      // }
      break;//我自己加的
   } while(context.numDone != numStates);

}

//             // void FhgfsOpsCommKit_readfileV2bCommunicate(App* app, RemotingIOInfo* ioInfo,
/*void FhgfsOpsCommKit_readfileV2bCommunicate(RemotingIOInfo* ioInfo,
   struct list_head* states, void (*nextIter)(CommKitContext*, FileOpState*),// 下一次迭代的回调函数
   void (*prepare)(CommKitContext*, FileOpState*),
   char *msg_buf,size_t *msg_len,
   uint64_t vaddr,uint32_t rkey,size_t memory_len)// 准备操作的回调函数
{
   struct ReadfileIterOps iops = {
      .nextIter = nextIter,
      .prepare = prepare,
   };

   FhgfsOpsCommkit_communicate(ioInfo, states, &readfileOps, &iops,msg_buf,msg_len,vaddr,rkey,memory_len);
}
*/

void FhgfsOpsCommKit_readfileV2bCommunicate(RemotingIOInfo* ioInfo,
   struct list_head* states, void (*nextIter)(CommKitContext*, FileOpState*),// 下一次迭代的回调函数
   void (*prepare)(CommKitContext*, FileOpState*),struct iovec* requestedheader)// 准备操作的回调函数
{
   struct ReadfileIterOps iops = {
      .nextIter = nextIter,
      .prepare = prepare,
   };

   FhgfsOpsCommkit_communicate(ioInfo, states, &readfileOps, &iops,requestedheader);
}
// 函数：输出 struct iovec 中的数据
void print_iovec_data(struct iovec *iov) {
    if (iov == NULL || iov->iov_base == NULL || iov->iov_len == 0) {
        LOG_printf(DEBUG,"No data to print.\n");
        return;
    }

    // 为了简化，这里假设数据是字符串（char数组）
    // 如果数据不是字符串，这里的代码可能需要调整
    const char *data = (const char *)iov->iov_base;
    size_t len = iov->iov_len;

    // 检查字符串是否以空字符结尾，如果不是，限制输出长度
    if (memchr(data, '\0', len) == NULL) {
        // 非空终止字符串，仅输出前iov_len个字符
        LOG_printf(DEBUG,"Data (%zu bytes): ");
        for (size_t i = 0; i < len; ++i) {
            fputc(data[i], stdout);
        }
    } else {
        // 空终止字符串，输出直到第一个空字符
        LOG_printf(DEBUG,"String: %s\n", data);
    }
}




// 替换成后面这个 ,
//XYP_MODIFY
/*ssize_t FhgfsOpsRemoting_readfileVec(char *msg_buf,size_t *msg_len,
   uint64_t vaddr,uint32_t rkey,size_t memory_len,uint32_t toberead)
{
    RemotingIOInfo* ioInfo=new RemotingIOInfo();
      initRemotingIOInfo(ioInfo);//@@
   // XYP_MODIFY
    size_t toBeRead=toberead;//1*1024;//10*1024;// 1.3MB //这两个参数是VFS文件系统入口函数传递的，我自己制定的  todo
    int64_t offset=0;//loff_t offset;  //   todo

    struct iovec* iter=new iovec();//todo ?这个参数里面需要存储值吗，我是文件读
      size_t capacity=512*1024;//512KByte
      char * iter_buffer=(char *)malloc(capacity);//@@ todo
      iter->iov_base=iter_buffer;
      iter->iov_len=capacity;


    ssize_t errnum = 0;
    ssize_t retVal = 0; 

   //pattern

    StripePattern* pattern = ioInfo->pattern;//获取条带模式



    int64_t currentOffset = offset; //当前偏移量loff_t  todo
    unsigned chunkSize = StripePattern_getChunkSize(pattern);//块大小 512KB 默认大小是如此，也是可以改的，beegfs-ctl 

   LOG(DEBUG)<<"chunkSize = "<<chunkSize<<"\n";
    UInt16Vec* targetIDs = Raid0Pattern_getStripeTargetIDs(pattern); // pattern->getStripeTargetIDs(pattern);//目标ID列表 todo

    unsigned numStripeNodes = UInt16Vec_length(targetIDs);//存储节点数量

   LOG(DEBUG)<<"numStripeNodes = "<<numStripeNodes<<"\n";
    size_t stripeSetSize = (size_t) chunkSize * numStripeNodes;//并计算一个条带集的大小
   //  size_t stripeSetSize = (size_t) 3000;//并计算一个条带集的大小  @@todo
 
  
  
 
   
  // App* app = ioInfo->app;   

   
   

   // 定义读取缓冲区和当前偏移量
   LOG(DEBUG)<<"定义读取缓冲区和当前偏移量 \n";
   BeeGFS_ReadSink* readsink = new BeeGFS_ReadSink();// todo free  //BeeGFS_ReadSink readsink = {0};
   // size_t capacity=1024;// todo 自定义 readsink 的缓冲区大小
   // beegfs_readsink_init(readsink,capacity);
   LOG(DEBUG)<<"-------------0  ------------ \n";

   //ioinfo   
   const char* fileHandleID = ioInfo->fileHandleID;//文件句柄ID
   //int maxUsedTargetIndex = AtomicInt_read(ioInfo->maxUsedTargetIndex);//最大使用的存储节点索引，todo 这个参数我传参的时候有赋值吗
            //__FhgfsOpsRemoting_logDebugIOCall(__func__, iov_iter_count(iter), offset, ioInfo, NULL);//好像是日志，可以注释跳过
   LOG_printf(DEBUG,"fileHandleID: %s",ioInfo->fileHandleID);

   int _cnt=0;
     while(toBeRead && !errnum)//开始一个循环，只要还有数据要读取并且没有错误发生，就继续循环。
   {  
      _cnt++;
      LOG(DEBUG)<<"\n @@ 循环次数 _cnt = "<< _cnt<<"----------------------------------------------------------\n";
         LOG(DEBUG)<<"1. toBeRead = "<< toBeRead<<"\n";

      //1. 根据文件的偏移量 ，可以确定文件对应的target storage index,即这个函数的逻辑，我们可以得知文件的条带规则或者设计
      unsigned currentTargetIndex = Raid0Pattern_getStripeTargetIndex(pattern, currentOffset); //pattern->getStripeTargetIndex(pattern, currentOffset); //todo
     
      //创建一个用于读取条带集的iov_iter结构体，并计算本轮最大读取大小。
      size_t maxReadSize = min_t(size_t, stripeSetSize, toBeRead);//计算本轮最大读取大小
      LOG(DEBUG)<<"2. 计算本轮最大读取大小 maxReadSize = "<<maxReadSize<<"\n";//512KB

               // LIST_HEAD(stateList);
      list_head_t stateList;
      INIT_LIST_HEAD(&stateList);

      struct FileOpVecState* state;
      size_t bytesReadThisRound = 0;


       struct iovec stripeSetIter;//struct iov_iter stripeSetIter;
      // iter --> readsink 
      beegfs_readsink_reserve(readsink, iter, maxReadSize);//为读取操作预留空间，并获取用于读取的iov_iter。

      stripeSetIter = readsink->sanitized_iter;
      // LOG(DEBUG)<<"stripeSetIter.iov_len = "<<stripeSetIter.iov_len<<"\n";

      // stripeset-loop: loop over one stripe set (using dynamically determined stripe node indices).
      
      //开始一个循环，遍历所有的存储节点，只要还有数据要读取并且iov_iter中还有数据。
      for(unsigned numWorks = 0;
            (numWorks < numStripeNodes
           && toBeRead && user_iov_iter_count(&stripeSetIter));
          ++ numWorks)
      {  // 计算当前块的大小和本地偏移量

         size_t currentChunkSize =
            StripePattern_getChunkEnd(pattern, currentOffset) - currentOffset + 1;//在当前存储target

         size_t currentReadSize = MIN(currentChunkSize, toBeRead);
         // LOG(DEBUG)<<"currentChunkSize = "<<currentChunkSize<<"\t, currentReadSize ="<<currentReadSize<<"\n";
         //下面这个函数很关键，如何根据文件的偏移量得到在对应的存储节点上的偏移量
         int64_t currentNodeLocalOffset = __FhgfsOpsRemoting_getChunkOffset(
            currentOffset, chunkSize, numStripeNodes, currentTargetIndex);//loff_t

            // LOG(DEBUG)<<"currentNodeLocalOffset =  "<<currentNodeLocalOffset<<"\n";//0 80000
         struct iovec chunkIter;

         // 分配状态结构体
                           // state = kmalloc(sizeof(*state), list_empty(&stateList) ? GFP_NOFS : GFP_NOWAIT);
         state = (FileOpVecState *)malloc(sizeof(*state));
            if (state==nullptr)
            {  
               LOG(DEBUG)<<"state ==0 \n";
               break;
            }

        
        
        // maxUsedTargetIndex = MAX(maxUsedTargetIndex, (int)currentTargetIndex);
         // 初始化块迭代器
         chunkIter = stripeSetIter;
         user_iov_iter_truncate(&chunkIter, currentChunkSize);

         // prepare the state information
         LOG(DEBUG)<<"3. prepare the state information ,currentNodeLocalOffset = "<<currentNodeLocalOffset<<"\t,user_iov_iter_count(&chunkIter) ="
         <<user_iov_iter_count(&chunkIter)<<"\t,targetID = "<<UInt16Vec_at(targetIDs, currentTargetIndex)<<"\n";
         FhgfsOpsCommKit_initFileOpState(
            &state->base,
            currentNodeLocalOffset,//存储节点上的偏移量
            user_iov_iter_count(&chunkIter),
            UInt16Vec_at(targetIDs, currentTargetIndex) );

         // state->base.firstWriteDoneForTarget =
         //    BitStore_getBit(ioInfo->firstWriteDone, currentTargetIndex);//todo 

         state->data = chunkIter;
            // LOG(DEBUG)<<"state->base.base.targetID  = "<<state->base.base.targetID<<"\n";

         // 将状态信息添加到列表
         list_add_tail(&state->base.base.targetInfoList, &stateList);


         // prepare for next loop
         currentOffset += currentReadSize;
         toBeRead -= currentReadSize;

         currentTargetIndex = (currentTargetIndex + 1) % numStripeNodes;
         int stripeSetIter_iocnt=1;
         user_iov_iter_advance(&stripeSetIter,stripeSetIter_iocnt, user_iov_iter_count(&chunkIter));
      }
      // 如果状态列表为空，表示内存分配失败
      if(list_empty(&stateList) )
      {
         errnum = -FhgfsOpsErr_OUTOFMEM;
         break;
      }

      // communicate with the nodes
      //与节点通信，发送读取请求。
                  //FhgfsOpsCommKit_readfileV2bCommunicate(app, ioInfo, &stateList, readfile_nextIter, NULL);
      //NOTE通信先注释掉
      
         FileOpVecState* tmp4 = container_of_v2<FileOpVecState, list_head_t>(stateList.next, offsetof(FileOpVecState, base.base.targetInfoList));
            // LOG(DEBUG)<<"tmp4->base.base.targetID  = "<<tmp4->base.base.targetID<<"\n";

         FhgfsOpsCommKit_readfileV2bCommunicate(ioInfo, &stateList, readfile_nextIter, nullptr,msg_buf,msg_len,vaddr,rkey,memory_len);//XYP_MODIFY
      

      // verify results // 验证结果
      
    // 释放读取缓冲区
      beegfs_readsink_release(readsink);//NOTE: 这个函数里面是空的，全给注释了，为什么？


      retVal += bytesReadThisRound;
      int iter_iocnt=1;//todo 
      user_iov_iter_advance(iter, iter_iocnt,bytesReadThisRound);

      // break;

   } // end of while(toBeRead)
   LOG(DEBUG)<<"end of while(toBeRead) \n";
  beegfs_readsink_release(readsink); // Make sure it's released even if we broke early from the loop


   // 返回读取的总字节数，如果没有读取任何数据，则返回错误码
   return retVal ? retVal : errnum;

}
*/

ssize_t FhgfsOpsRemoting_readfileVec(char * filehandleID,unsigned int origParentUID,char * origParentEntryID,size_t readlen,struct iovec* requestedheader)
{
    RemotingIOInfo* ioInfo=new RemotingIOInfo();
      initRemotingIOInfo(ioInfo,filehandleID,origParentUID,origParentEntryID);

    size_t toBeRead=readlen;//1*1024;// 1.3MB //这两个参数是VFS文件系统入口函数传递的，我自己制定的  todo
    int64_t offset=0;//loff_t offset;  //   todo

    struct iovec* iter=new iovec();//todo ?这个参数里面需要存储值吗，我是文件读
      size_t capacity=512*1024;//512KByte
      char * iter_buffer=(char *)malloc(capacity);//@@ todo
      iter->iov_base=iter_buffer;
      iter->iov_len=capacity;


    ssize_t errnum = 0;
    ssize_t retVal = 0; 

   //pattern

    StripePattern* pattern = ioInfo->pattern;//获取条带模式



    int64_t currentOffset = offset; //当前偏移量loff_t  todo
    unsigned chunkSize = StripePattern_getChunkSize(pattern);//块大小 512KB 默认大小是如此，也是可以改的，beegfs-ctl 

   LOG(DEBUG)<<"chunkSize = "<<chunkSize<<"\n";
    UInt16Vec* targetIDs = Raid0Pattern_getStripeTargetIDs(pattern); // pattern->getStripeTargetIDs(pattern);//目标ID列表 todo

    unsigned numStripeNodes = UInt16Vec_length(targetIDs);//存储节点数量

   LOG(DEBUG)<<"numStripeNodes = "<<numStripeNodes<<"\n";
    size_t stripeSetSize = (size_t) chunkSize * numStripeNodes;//并计算一个条带集的大小
   //  size_t stripeSetSize = (size_t) 3000;//并计算一个条带集的大小  @@todo
 
  
  
 
   
  // App* app = ioInfo->app;   

   
   

   // 定义读取缓冲区和当前偏移量
   LOG(DEBUG)<<"定义读取缓冲区和当前偏移量 \n";
   BeeGFS_ReadSink* readsink = new BeeGFS_ReadSink();// todo free  //BeeGFS_ReadSink readsink = {0};
   // size_t capacity=1024;// todo 自定义 readsink 的缓冲区大小
   // beegfs_readsink_init(readsink,capacity);
   LOG(DEBUG)<<"-------------0  ------------ \n";

   //ioinfo   
   const char* fileHandleID = ioInfo->fileHandleID;//文件句柄ID
   //int maxUsedTargetIndex = AtomicInt_read(ioInfo->maxUsedTargetIndex);//最大使用的存储节点索引，todo 这个参数我传参的时候有赋值吗
            //__FhgfsOpsRemoting_logDebugIOCall(__func__, iov_iter_count(iter), offset, ioInfo, NULL);//好像是日志，可以注释跳过
   // LOG_printf(DEBUG,"size: %lld; offset: %lld; fileHandleID: %s",(long long)size, (long long)offset, ioInfo->fileHandleID);

   int _cnt=0;
     while(toBeRead && !errnum)//开始一个循环，只要还有数据要读取并且没有错误发生，就继续循环。
   {  
      _cnt++;
      LOG(DEBUG)<<"\n @@ 循环次数 _cnt = "<< _cnt<<"----------------------------------------------------------\n";
         LOG(DEBUG)<<"1. toBeRead = "<< toBeRead<<"\n";

      //1. 根据文件的偏移量 ，可以确定文件对应的target storage index,即这个函数的逻辑，我们可以得知文件的条带规则或者设计
      unsigned currentTargetIndex = Raid0Pattern_getStripeTargetIndex(pattern, currentOffset); //pattern->getStripeTargetIndex(pattern, currentOffset); //todo
     
      //创建一个用于读取条带集的iov_iter结构体，并计算本轮最大读取大小。
      size_t maxReadSize = min_t(size_t, stripeSetSize, toBeRead);//计算本轮最大读取大小
      LOG(DEBUG)<<"2. 计算本轮最大读取大小 maxReadSize = "<<maxReadSize<<"\n";//512KB

               // LIST_HEAD(stateList);
      list_head_t stateList;
      INIT_LIST_HEAD(&stateList);

      struct FileOpVecState* state;
      size_t bytesReadThisRound = 0;


       struct iovec stripeSetIter;//struct iov_iter stripeSetIter;
      // iter --> readsink 
      beegfs_readsink_reserve(readsink, iter, maxReadSize);//为读取操作预留空间，并获取用于读取的iov_iter。

      stripeSetIter = readsink->sanitized_iter;
      // LOG(DEBUG)<<"stripeSetIter.iov_len = "<<stripeSetIter.iov_len<<"\n";

      // stripeset-loop: loop over one stripe set (using dynamically determined stripe node indices).
      
      //开始一个循环，遍历所有的存储节点，只要还有数据要读取并且iov_iter中还有数据。
      for(unsigned numWorks = 0;
            (numWorks < numStripeNodes
           && toBeRead && user_iov_iter_count(&stripeSetIter));
          ++ numWorks)
      {  // 计算当前块的大小和本地偏移量

         size_t currentChunkSize =
            StripePattern_getChunkEnd(pattern, currentOffset) - currentOffset + 1;//在当前存储target

         size_t currentReadSize = MIN(currentChunkSize, toBeRead);
         // LOG(DEBUG)<<"currentChunkSize = "<<currentChunkSize<<"\t, currentReadSize ="<<currentReadSize<<"\n";
         //下面这个函数很关键，如何根据文件的偏移量得到在对应的存储节点上的偏移量
         int64_t currentNodeLocalOffset = __FhgfsOpsRemoting_getChunkOffset(
            currentOffset, chunkSize, numStripeNodes, currentTargetIndex);//loff_t

            // LOG(DEBUG)<<"currentNodeLocalOffset =  "<<currentNodeLocalOffset<<"\n";//0 80000
         struct iovec chunkIter;

         // 分配状态结构体
                           // state = kmalloc(sizeof(*state), list_empty(&stateList) ? GFP_NOFS : GFP_NOWAIT);
         state = (FileOpVecState *)malloc(sizeof(*state));
            if (state==nullptr)
            {  
               LOG(DEBUG)<<"state ==0 \n";
               break;
            }

        
        
        // maxUsedTargetIndex = MAX(maxUsedTargetIndex, (int)currentTargetIndex);
         // 初始化块迭代器
         chunkIter = stripeSetIter;
         user_iov_iter_truncate(&chunkIter, currentChunkSize);

         // prepare the state information
         LOG(DEBUG)<<"3. prepare the state information ,currentNodeLocalOffset = "<<currentNodeLocalOffset<<"\t,user_iov_iter_count(&chunkIter) ="
         <<user_iov_iter_count(&chunkIter)<<"\t,targetID = "<<UInt16Vec_at(targetIDs, currentTargetIndex)<<"\n";
        //  FhgfsOpsCommKit_initFileOpState(
        //     &state->base,
        //     currentNodeLocalOffset,//存储节点上的偏移量
        //     user_iov_iter_count(&chunkIter),
        //     UInt16Vec_at(targetIDs, currentTargetIndex) );
         FhgfsOpsCommKit_initFileOpState(
            &state->base,
            0,//存储节点上的偏移量
            readlen,
            UInt16Vec_at(targetIDs, currentTargetIndex) );
         // state->base.firstWriteDoneForTarget =
         //    BitStore_getBit(ioInfo->firstWriteDone, currentTargetIndex);//todo 

         state->data = chunkIter;
            // LOG(DEBUG)<<"state->base.base.targetID  = "<<state->base.base.targetID<<"\n";

         // 将状态信息添加到列表
         list_add_tail(&state->base.base.targetInfoList, &stateList);

                        /*
                           //根据文件模式决定是否使用辅助镜像
                           // use secondary buddy mirror for odd inode numbers
                           if( (StripePattern_getPatternType(pattern) == STRIPEPATTERN_BuddyMirror) )
                              state->base.base.useBuddyMirrorSecond =
                                 fhgfsInode ? (fhgfsInode->vfs_inode.i_ino & 1) : false;
                        */

                        //App_incNumRemoteReads(app);//app todo 看是否要保留,先不保留因为app我都不保留了 ，保留与否取决于 这个参数是否用于和对端通信

         // prepare for next loop
         currentOffset += currentReadSize;
         toBeRead -= currentReadSize;

         currentTargetIndex = (currentTargetIndex + 1) % numStripeNodes;
         // LOG(DEBUG)<<"currentReadSize = "<<currentReadSize <<"\t, currentTargetIndex = "<<currentTargetIndex<<"\n";
         int stripeSetIter_iocnt=1;
         user_iov_iter_advance(&stripeSetIter,stripeSetIter_iocnt, user_iov_iter_count(&chunkIter));
      }
      // 如果状态列表为空，表示内存分配失败
      if(list_empty(&stateList) )
      {
         errnum = -FhgfsOpsErr_OUTOFMEM;
         break;
      }

      // communicate with the nodes
      //与节点通信，发送读取请求。
                  //FhgfsOpsCommKit_readfileV2bCommunicate(app, ioInfo, &stateList, readfile_nextIter, NULL);
      //NOTE通信先注释掉
      
         FileOpVecState* tmp4 = container_of_v2<FileOpVecState, list_head_t>(stateList.next, offsetof(FileOpVecState, base.base.targetInfoList));
            // LOG(DEBUG)<<"tmp4->base.base.targetID  = "<<tmp4->base.base.targetID<<"\n";

         FhgfsOpsCommKit_readfileV2bCommunicate(ioInfo, &stateList, readfile_nextIter, nullptr,requestedheader);
      


      // 释放读取缓冲区
      beegfs_readsink_release(readsink);//NOTE: 这个函数里面是空的，全给注释了，为什么？


      retVal += bytesReadThisRound;
      int iter_iocnt=1;//todo 
      user_iov_iter_advance(iter, iter_iocnt,bytesReadThisRound);

      break; //@@ GPUNetIO insert 为的是只发一次 request 请求，就能读取超过 chunkdsize 的文件内容，而不是分几次 发送read请求，每次读至多 512KB chunk

   } // end of while(toBeRead)
   LOG(DEBUG)<<"end of while(toBeRead) \n";
  beegfs_readsink_release(readsink); // Make sure it's released even if we broke early from the loop


   // 返回读取的总字节数，如果没有读取任何数据，则返回错误码
   return retVal ? retVal : errnum;

}
//XYP——MODIFY
void generate_read_request(char *fileHandleID,unsigned int origParentUID,char *origParentEntryID,char * msg_buffer,unsigned int  *msg_len,uint64_t vaddr,uint32_t rkey,size_t vaddr_len,int64_t toberead,int64_t begin_offset)
{

    ReadLocalFileRDMAMsg readRDMAMsg;
    NetMessage *netMessage = NULL;

    NumNodeID clientNodeNumID;
    clientNodeNumID.value=2;

    uint16_t targetID=3001;//@@TODO 其实和还有一个地方的TargetID 也得修改 0415更新
                            //new_read.hpp:   Raid0_StripePattern_init() ==》 targetNumID[0]=
    

    PathInfo* pathInfo=(PathInfo*)malloc(sizeof(PathInfo));
    {    
         if(pathInfo==NULL)
         {
            perror("pathInfo==NULL");
            return ;
         }


        std::memset(pathInfo, 0, sizeof(PathInfo));

        pathInfo->_flags = 1;//PATHINFO_FEATURE_ORIG
        pathInfo->flags = 1;
        pathInfo->origParentUID = origParentUID;
        pathInfo->_origParentUID = origParentUID;
      pathInfo->origParentEntryID=(char*)malloc(50);
      pathInfo->_origParentEntryID=(char*)malloc(50);
      if(pathInfo->origParentEntryID==NULL|| pathInfo->_origParentEntryID==NULL)
      {
         perror("pathInfo->origParentEntryID==NULL|| pathInfo->_origParentEntryID==NULL");
         return ;
      }
        strcpy(pathInfo->origParentEntryID,origParentEntryID);

        strcpy(pathInfo->_origParentEntryID,origParentEntryID);
    }
    
    unsigned int accessFlags = 1; // #define OPEN_ACCESS_READ  1
    int64_t offset=begin_offset;


    RdmaInfo *rdmap0 = (RdmaInfo*)malloc(sizeof(RdmaInfo));
    {   
       if(rdmap0==NULL)
         {
            perror("rdmap0==NULL");
            return ;
         }
        memset(rdmap0, 0, sizeof(RdmaInfo));
        rdmap0->sg_count = 1;
        rdmap0->dma_count = 1;  // 这里假设只有一块连续内存
        rdmap0->tag = 0;
        rdmap0->key = rkey;
        LOG_printf(DEBUG,"rdmap0->key = %d\n", rdmap0->key);
        LOG_printf(DEBUG,"-------------检测RDMA设备-----------------\n");
            struct ibv_device **dev_list = (ibv_device**)NULL;
            int num_devices = 0;

            dev_list = ibv_get_device_list(&num_devices);    //获取RDMA列表
            if (!dev_list) {
                perror("Failed to get IB devices list");
                return ;
            }
            LOG_printf(DEBUG,"Found %d RDMA device(s):\n", num_devices);
            for (int i = 0; i < num_devices; ++i) {
                LOG_printf(DEBUG,"  Device %d: %s\n", i, ibv_get_device_name(dev_list[i]));
            }
            
            struct ibv_device *ib_dev = (ibv_device*)NULL;
            for (int i = 0; i < num_devices; ++i) {
            if (strcmp(ibv_get_device_name(dev_list[i]), "mlx5_2") == 0) {       //RDMA设备名来寻找设备对象， 用 ibv_devices 或 ls /sys/class/infiniband 查看
            // if (strcmp(ibv_get_device_name(dev_list[i]), "mlx5_0") == 0) {  //xyp_0305     //RDMA设备名来寻找设备对象， 用 ibv_devices 或 ls /sys/class/infiniband 查看

                ib_dev = dev_list[i];
                break;
                } 
            }
        rdmap0->device = ib_dev;
            if(!rdmap0->device){
                perror("rdmap0->device = NULL");
                return ;
            }
        rdmap0->pages = NULL;
        rdmap0->sglist = (struct scatterlist*)malloc(rdmap0->sg_count * sizeof(struct scatterlist));
        rdmap0->dmalist = (struct scatterlist*)malloc(rdmap0->dma_count * sizeof(struct scatterlist));

            if(!rdmap0->sglist || !rdmap0->dmalist)
            {
                fprintf(stderr, "failed to allocate sglist or dmalist\n");
                free(rdmap0->sglist);
                free(rdmap0->dmalist);
                free(rdmap0);
                return ;
            }

        rdmap0->dmalist[0].dma_address =vaddr;
        rdmap0->dmalist[0].length = vaddr_len;
        rdmap0->dmalist[0].offset = 0;
        rdmap0->dmalist[0].page_link = 0;

        rdmap0->sglist[0].dma_address = vaddr;
        rdmap0->sglist[0].length = vaddr_len;
        rdmap0->sglist[0].offset = 0;
        rdmap0->sglist[0].page_link = 0;
    }

    ReadLocalFileRDMAMsg_initFromSession(&readRDMAMsg, clientNodeNumID,
            fileHandleID, targetID, pathInfo,
            accessFlags, offset, toberead,
            rdmap0);

    netMessage = &(readRDMAMsg.netMessage);
    
    uint16_t selectedTargetID=81;//表示当前选择的TargetID
    netMessage->msgHeader.msgTargetID =selectedTargetID;
    
    NetMessage_serialize(netMessage, msg_buffer, BEEGFS_COMMKIT_MSGBUF_SIZE);//BEEGFS_COMMKIT_MSGBUF_SIZE 4096
    
    *msg_len = NetMessage_getMsgLength(netMessage);

    return ;
}

//XYP——MODIFY
void rdma_read_msg_generate(char * send_buf,unsigned int * msg_len,
   uint64_t vaddr,uint32_t rkey,size_t vaddr_len,int64_t toberead,int64_t begin_offset)
{
   // open code
   Socket* sock=nullptr;
   LOG_printf(DEBUG,"start open file in TCP mode \n");
   // 把 open 逻辑加进来 ，获取 ioinfo 的 fileHandleID 和 pathInfo 这个参数
    __MessagingTk_requestResponseWithRRArgsComm(sock);//TCP 连接

    // 方式一，通过如下的方式，出现段错误，但却定位不到问题的位置
   //  LOG(DEBUG)<<"prepare into readfile \n";
   //  FhgfsOpsRemoting_readfileVec(send_buf,msg_len,vaddr,rkey,memory_len,toberead);
   
   // 方法二：简化版本，只生成对应的请求消息到给定的缓冲区中
   unsigned int origParentUID=1000;
   char origParentEntryID[]="0-67C551B0-6";
   if(filehandleID_xyp==NULL)
      LOG_printf(DEBUG,"error \n");
   else  
      LOG_printf(DEBUG,"file_handle %s \n",filehandleID_xyp);
   generate_read_request(filehandleID_xyp,origParentUID,origParentEntryID,
         send_buf,msg_len,vaddr, rkey,vaddr_len,toberead,begin_offset);

    return ;
   
}

void open_RDMA(Socket* sock,char* parentEntryID,char* entryID,char* fileName,DirEntryType entryType,char* &OutFileHandleID)
{

   

   unsigned sendBufLen = sendBufLen = 1024; //@@ 这个自己指定，需要比发送的消息长度要长，也是接收缓冲区的长度
   unsigned int recvBufLen=1024;            //@@ 接收缓冲区长度，这个长度决定了 bufLen 的长度
 
   ssize_t respRes = 0;
   ssize_t sendRes;
   bool isConsturctSocket=false;
   
      if (sock == NULL) 
      {
         isConsturctSocket=true;

          printf("----- 1. 创建套接字 -----\n");
         struct in_addr srcAddr;
         NicAddress* nicAddr = (NicAddress*)malloc(sizeof(NicAddress));
         NicAddressStats* srcRdma = (NicAddressStats*)malloc(sizeof(NicAddressStats));

         const char* LocalRdmaIP = "192.168.3.212";       //本地dell02的RDMA IP,   网口名为 enp130s0np0
                                                         //@@本地 dell01 的RDMA IP : 192.168.0.203 ,   网口名为 enp152s0np0
                                                         // 164Client ens4f0np0  192.168.3.212

         if (inet_pton(AF_INET, LocalRdmaIP, &srcAddr) <= 0) {    // 将点分十进制的IP地址转换为用于网络传输的数值格式。
            perror("Invalid IP address");
            return ;
         }
         printf("查看本主机IP情况 LocalIP %s and tranfer to %u\n", LocalRdmaIP, srcAddr.s_addr);
         nicAddr->ipAddr = srcAddr;
         nicAddr->nicType = NICADDRTYPE_RDMA;
         strncpy(nicAddr->name, "ens4f0np0", IFNAMSIZ);  //@@本地 RDMA 端口名 dell02 : enp130s0np0
                                                         //@@本地 dell01 的RDMA IP : 192.168.0.203 ,   网口名为 enp152s0np0
                                                          // 164Client ens4f0np0  192.168.3.212

         NicAddressStats_init(srcRdma, nicAddr);

            sock = (Socket*)RDMASocket_construct(srcAddr, srcRdma);  //构造一个RDMA socket
           if(sock==NULL)
                LOG_printf(DEBUG,"@@ Error ----------- 3 ---------------\n"); 
            if(!sock)
            { // no conn available => error or didn't want to wait
               printf("----------------获取RDMA socket失败!----------------------\n");
               {
                  
                //   if(sock)
                //   {
                //      NodeConnPool_releaseStreamSocket(sock);   //这里的释放类型要注意
                //   }
               }
               return ;
            }
            else{
               printf("----------------获取RDMA socket成功!----------------------\n");
            } 

         //3. 尝试通过IP连接
             if(sock==NULL)
    LOG_printf(DEBUG,"@@ Error ----------- 4 ---------------\n");
               printf("------------------- 3. 尝试通过IP连接-------------------\n");
               
               struct in_addr destAddr;
               const char* RemoteRdmaIP = "192.168.3.216";   //@@存储服务器xfusion3的RDMA IP 192.168.0.218 ,网口名 ens2np0
                                                         // 162Server ens4f1np1 192.168.3.216 mlx5_0


               if (inet_pton(AF_INET, RemoteRdmaIP, &destAddr) <= 0) {
               perror("Invalid remote IP address");
                  return ;
               }
               printf("查看远端主机IP情况 RemoteIP %s and tranfer to %u\n", RemoteRdmaIP, destAddr.s_addr);

               unsigned short port = 8005;    //存储服务器的port 8003 元数据服务器的端口是 8005
               printf("----------------开始根据IP与远端建立RDMA连接-----------------------\n");
               // bool connectRes = info->socket->ops->connectByIP(info->socket, destAddr, port);
               bool connectRes = sock->ops->connectByIP(sock, destAddr, port);

               if(connectRes){
                  printf("尝试通过IP连接RDMA 连接成功 \n");
               }
               else
               {
                  printf("尝试通过IP连接RDMA 连接失败 \n");
               }

if(sock==NULL)
    LOG_printf(DEBUG,"@@ Error ----------- 5 ---------------\n");
         // 这是 服务端的认证配置
            bool isflag= __NodeConnPool_applySocketOptionsConnected(sock);
               if(isflag==true)
               {
                  LOG(DEBUG)<<"__NodeConnPool_applySocketOptionsConnected 成功 ！！\n";
            
               }
               else
               {
                  LOG(DEBUG)<<"__NodeConnPool_applySocketOptionsConnected 失败 ！！\n";

               }
        if(sock==NULL)
    LOG_printf(DEBUG,"@@ Error ----------- 6 ---------------\n");
      }
   
   //4. 准备发送缓冲区 我怎么感觉是准备接收缓冲区
   LOG_printf(DEBUG,"--------------------4.准备发送缓冲区-------------------\n");

    
    int recvBufSize;
    socklen_t optLen = sizeof(recvBufSize);

   
   NoAllocBufferStore* bufStore=(NoAllocBufferStore*)malloc(sizeof(NoAllocBufferStore));
   bufStore->numBufs = 10;
   bufStore->bufSize = recvBufLen; //@@  
   bufStore->numAvailable = 0;
   
   bufStore->bufArray = (char**)malloc(bufStore->numBufs * sizeof(char*));

   
   Condition_init(&bufStore->newBufCond);
   
   for (size_t i = 0; i < bufStore->numBufs; ++i) {

         bufStore->bufArray[i] =(char*) malloc(bufStore->bufSize);
   }

   bufStore->numAvailable = bufStore->numBufs;

   Condition_signal(&bufStore->newBufCond);


   // 准备 消息接收缓冲区，outRespBuf
      unsigned bufLen;  
      MessagingTkBufType respBufType=MessagingTkBufType_BufStore;
      char* outRespBuf=nullptr;
      if(respBufType == MessagingTkBufType_BufStore)//根据响应缓冲区类型，从缓冲区存储中获取或分配缓冲区，回溯前面的代码 确实响应缓冲区的类型是这个
      { 
         
         

         bufLen = bufStore->bufSize;// 获取缓冲区长度

         if(bufLen < sendBufLen )// 如果缓冲区长度小于发送缓冲区长度
         { 
            LOG_printf(DEBUG,"buflen<sendBuflen\n");
         }

         outRespBuf = NoAllocBufferStore_waitForBuf(bufStore);// 这个函数其实就是从 bufStore->bufArray[] 取上一个，我定义的时候是有10个256Byte的缓冲区

         LOG_printf(DEBUG,"Buffer received: %p\n", (void*)outRespBuf);  // 打印获取的缓冲区地址

      }
   LOG_printf(DEBUG,"--------------------5.准备请求信息-------------------\n");
//    RDMASocket* rs = (RDMASocket*)sock;
     // requestMsg 赋值
     OpenFileMsg requestMsg;
    
          // 第一个参数
         NumNodeID localNodeNumID;
         localNodeNumID.value=10;//@@ dell01 :1201 ; 164 client :10 ;// 假设本地节点 NodeID 为这个值
         bool isGroup=false; //@@ 
          uint32_t metaNodeNumID=1;//@@
          // 第二个参数 
          EntryInfo entryInfo;
            //  void EntryInfo_init(EntryInfo* outEntryInfo,char* parentEntryID,char* entryID,char* fileName,DirEntryType entryType,bool isGroup,uint32_t NodeNumID);
          
          EntryInfo_init(&entryInfo, parentEntryID, entryID, fileName, entryType, isGroup,metaNodeNumID);

          // 第三个参数
         unsigned accessFlags=accessFlagsInit();


          // requestMsg 第四个参数 
          FileEvent event;
         // FileEvent_init(&event,FileEventType_READ, "test_xyp.txt");//第三个参数是 直接传相对挂载目录/mnt/beegfs/的相对路径 ，Note:好奇怪：前面的 entryID 和这个 路径不匹配 open 逻辑也是成功获取的,原因是服务端处理逻辑根本不识别这个参数
         // FileEvent_init(&event,FileEventType_READ, fileName);//第三个参数是 直接传相对挂载目录/mnt/beegfs/的相对路径 ，Note:好奇怪：前面的 entryID 和这个 路径不匹配 open 逻辑也是成功获取的,原因是服务端处理逻辑根本不识别这个参数
         FileEvent_init(&event,FileEventType_READ, NULL);//第三个参数是 直接传相对挂载目录/mnt/beegfs/的相对路径 ，Note:好奇怪：前面的 entryID 和这个 路径不匹配 open 逻辑也是成功获取的,原因是服务端处理逻辑根本不识别这个参数
         
         if (event.path) {
            LOG_printf(DEBUG,"Relative path: %s\n", event.path);
         } else {
            LOG_printf(DEBUG,"Failed to get relative path\n");
         }
               
     OpenFileMsg_initFromSession(&requestMsg, localNodeNumID, &entryInfo, accessFlags,&event);
   
     
        
  

   LOG_printf(DEBUG,"--------------------6.请求消息序列化-------------------\n");

      //5. 序列化
      //在网络通信中，序列化用于将消息转换为字节流，以便在网络上发送。将rrArgs->requestMsg 序列化后存放在rrArgs->outRespBuf
          
      NetMessage_serialize((NetMessage*)&requestMsg, outRespBuf, sendBufLen); //往缓冲区中放请求的信息
   LOG(DEBUG)<<"PS: length = "<<NetMessage_getMsgLength((NetMessage*)&requestMsg)<<"\n";
     
            if(sendBufLen<=0)
            {
                        LOG_printf(DEBUG,"6. send request error \n");
            }
            else
             LOG_printf(DEBUG,"send request len is %d\n",sendBufLen); // 输出 是 1024

      struct iovec  *iter = STACK_ALLOC_BEEGFS_ITER_KVEC(outRespBuf, NetMessage_getMsgLength((NetMessage*)&requestMsg));//感觉这个操作是彻底解除了消息过长的问题




    
         

      int iovcnt=1;
      fhgfs_sockaddr_in *to=nullptr;


      sendRes=sock->ops->sendto(sock, iter,  iovcnt,0, to);//$$有好多实现形式
            LOG_printf(DEBUG,"-------6 -4 -----------------\n");
    LOG_printf(DEBUG,"@@ 后续处理 Error sock %p \n",sock); 

    if(sock==NULL)
    LOG_printf(DEBUG,"@@ Error ----------- 7 ---------------\n");
      if(sendRes != (ssize_t)sendBufLen)
         {
            LOG_printf(DEBUG,"sendRes != sendBufLen \n");
         }
   
   LOG_printf(DEBUG,"--------------------7. 接收消息-------------------\n");
    LOG_printf(DEBUG,"@@ 后续处理 Error sock %p \n",sock); 

      //7. receive response
      respRes = MessagingTk_recvMsgBuf(sock, outRespBuf, bufLen);

      if(respRes <= 0) 
      { 
         LOG_printf(INFO,"respRes<=0 \n");
      }
   if(sock==NULL)
    LOG_printf(DEBUG,"@@ Error ----------- 8 ---------------\n");

   LOG_printf(DEBUG,"--------------------8. 反序列化解析-------------------\n");
    LOG_printf(DEBUG,"@@ 后续处理 Error sock %p \n",sock); 
      
      //8. 反序列化解析
      NetMessage* outRespMsg = NetMessageFactory_createFromBuf(outRespBuf, respRes);
      
      if (outRespMsg->msgHeader.msgType == NETMSGTYPE_AckNotifyResp)
      {
         LOG_printf(DEBUG,"错误处理1 \n");
      }
      if(NetMessage_getMsgType(outRespMsg) == NETMSGTYPE_GenericResponse)
      {  

         LOG_printf(DEBUG,"错误处理2 \n");
      }
      LOG_printf(DEBUG,"Correct response  \n");
      //LOG(DEBUG)<<"outRespMsg->msgHeader.msgType = "<<outRespMsg->msgHeader.msgType<<"\n";
      if(outRespMsg->msgHeader.msgType!=NETMSGTYPE_OpenFileResp)
      {
         LOG(DEBUG)<<"Received invalid response type:"<<outRespMsg->msgHeader.msgType<<"\n";
      }

   LOG_printf(DEBUG,"--------------------9. 获取文件句柄-------------------\n");

   OpenFileRespMsg* openResp;
    openResp = (OpenFileRespMsg*)outRespMsg;

    FhgfsOpsErr retVal;
    retVal = (FhgfsOpsErr)(openResp->result);
   if(retVal == FhgfsOpsErr_SUCCESS) 
   { 
      // success => store file details
      LOG(DEBUG)<<"retVal == FhgfsOpsErr_SUCCESS \n";
   }
   else
   {
      LOG(DEBUG)<<"retVal == "<<retVal<<"\n";//FhgfsOpsErr_PATHNOTEXISTS  = 8,
   }
    LOG_printf(DEBUG,"@@ 后续处理 Error sock %p \n",sock); 

   const PathInfo* msgPathInfoPtr;

      const char*  fileHandleID = StringTk_strDup(openResp->fileHandleID ); //StringTk_strDup(OpenFileRespMsg_getFileHandleID(openResp) );

         filehandleID_xyp=(char*)malloc(strlen(fileHandleID) + 5); // 分配足够的内存来存储字符串及其终止符'\0'
         if (filehandleID_xyp != NULL) {
               strcpy(filehandleID_xyp, fileHandleID); // 复制字符串
               // 现在 filehandleID_xyp 指向一个可以修改的字符串副本
               LOG(DEBUG)<<"file_handleID_xyp = "<<filehandleID_xyp<<"\n";
         }
         OutFileHandleID=StringTk_strDup(fileHandleID);
         // strcpy(OutFileHandleID,fileHandleID);
         LOG(DEBUG)<<"OutFileHandleID = "<<OutFileHandleID<<"\n";
        LOG(DEBUG)<<"openResp->fileHandleID = "<<openResp->fileHandleID<<"\n";
        LOG(DEBUG)<<"openResp->pathInfo (1) flags = "<<openResp->pathInfo.flags<<"\n";
        LOG(DEBUG)<<"openResp->pathInfo (2) _flags = "<<openResp->pathInfo._flags<<"\n"; 
        LOG(DEBUG)<<"openResp->pathInfo (3) origParentUID = "<<openResp->pathInfo.origParentUID<<"\n";
        LOG(DEBUG)<<"openResp->pathInfo (4) _origParentUID = "<<openResp->pathInfo._origParentUID<<"\n";
        LOG(DEBUG)<<"openResp->pathInfo (5) origParentEntryID = "<<openResp->pathInfo.origParentEntryID<<"\n";
        LOG(DEBUG)<<"openResp->pathInfo (6) _origParentEntryID ="<<openResp->pathInfo._origParentEntryID<<"\n";

    LOG_printf(DEBUG,"@@ 后续处理 Error sock %p \n",sock); 


     delete iter; 
//    if(isConsturctSocket)
//    {
//       // 释放套接字
//       LOG(DEBUG)<<"释放套接字 \n";
//       NodeConnPool_releaseStreamSocket(sock);
//    }
   
   // 释放内存
    if (event.path) {
        free(event.path);
    }
   // 释放资源
    for (size_t i = 0; i < bufStore->numBufs; ++i) {
        free(bufStore->bufArray[i]);  // 释放每个缓冲区的内存
    }
    free(bufStore->bufArray);  // 释放缓冲区数组
    Condition_destroy(&bufStore->newBufCond);  // 销毁条件变量
    
    LOG_printf(DEBUG,"@@ 后续处理 Error sock %p \n",sock); 
   // 7. 后续处理

}




#endif





