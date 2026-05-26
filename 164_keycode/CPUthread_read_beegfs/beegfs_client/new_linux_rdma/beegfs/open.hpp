#ifndef OPEN_HPP
#define OPEN_HPP
// #ifdef __cplusplus
// 仅 C++ 编译器可见的头文件
#include <atomic>       // C++ 原子操作库
#include <cstddef>      // C++ 标准库（可选，如果不需要可删除）
#include <cstdlib>     // For malloc and free
#include <cstring> // C++ 标准库中的内存操作函数
#include <atomic>
#include <iostream>
#include <memory> // for unique_ptr 
#include <sstream>
#include <iostream>

using namespace std;
// #endif


// #include <cstdlib>     // For malloc and free
#include <string.h>     // For memset, memcpy, memcmp, and memmove
#include <stddef.h>
// #include <iostream>
// #include <cstring> // C++ 标准库中的内存操作函数
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>      // for open 系统调用 osflag=O_RDONLY|O_WRONLY
#include <sys/socket.h> // For socket operations,struct msghdr 
#include <sys/types.h> // for typeof
#include <sys/uio.h> //struct iovec 的定义，它通常与 readv、writev、sendmsg 和 recvmsg
#include <unistd.h>           // for getuid
#include <poll.h> // 或者 #include <sys/poll.h>  //要在你的程序中使用 struct pollfd
#include <errno.h>



#include <pthread.h>
// #include <memory> // for unique_ptr 
#include <linux/sockios.h>  // 包含 SIOCOUTQ 命令
#include <sys/ioctl.h>      // 包含 ioctl 函数声明
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
// using namespace std; 
#define LOG_PRINTF(fmt, ...) printf(DEBUG,"[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__) 
// #define LOG CoutStream日志( __FILE__, __LINE__ )
// #define CoutStream日志(file, line) LOG << "[" << file << ":" << line << "] "
// class Logger {
// public:
//     Logger(const std::string& file, int line) : file_(file), line_(line) {}
//     std::ostream& stream() {
//         return std::cout << "[" << file_ << ":" << line_ << "] ";
//     }
// private:
//     std::string file_;
//     int line_;
// };

// #define LOG Logger(__FILE__, __LINE__).stream()

//-------------------- 20250114----------------增加日志输出-------------begin
//----说明 
// std::cout 替换成  LOG(DEBUG)
// printf(DEBUG, 替换成  LOG_printf(DEBUG,

enum LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL,
    NONE  // 不输出任何日志
};

std::string logLevelToString(LogLevel level) {
    switch (level) {
        case DEBUG: return "DEBUG";
        case INFO: return "INFO";
        case WARNING: return "WARNING";
        case ERROR: return "ERROR";
        case FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

// 全局日志等级
LogLevel globalLogLevel = INFO;//DEBUG or INFO

class Logger {
public:
    Logger(const std::string& file, int line, LogLevel level) 
        : file_(file), line_(line), level_(level) {}

    std::ostream& stream() {
        if (level_ >= globalLogLevel) {
            std::string levelStr = logLevelToString(level_);
            return std::cout << "[" << levelStr << "] [" << file_ << ":" << line_ << "] ";
        } else {
            // return *static_cast<std::ostream*>(nullptr);
             // 返回一个不会输出任何内容的流
            static std::stringstream nullStream;
            return nullStream;
        }
    }

private:
    std::string file_;
    int line_;
    LogLevel level_;
};

#define LOG(level) Logger(__FILE__, __LINE__, level).stream()

// #define LOG_printf(DEBUG,level, ...) \
//     do { \
//         if (level >= globalLogLevel) { \
//             const char* levelStr[] = {"DEBUG", "INFO", "WARNING", "ERROR", "FATAL", "NONE"}; \
//             fprintf(DEBUG,stderr, "[%s] [%s:%d] ", levelStr[level], __FILE__, __LINE__); \
//             fprintf(DEBUG,stderr, __VA_ARGS__); \
//             fprintf(DEBUG,stderr, "\n"); \
//         } \
//     } while (0)

#define LOG_printf(level, ...) \
    do { \
        if (level >= globalLogLevel) { \
            const char* levelStr[] = {"DEBUG", "INFO", "WARNING", "ERROR", "FATAL", "NONE"}; \
            fprintf(stderr, "[%s] [%s:%d] ", levelStr[level], __FILE__, __LINE__); \
            fprintf(stderr, __VA_ARGS__); \
            fprintf(stderr, "\n"); \
        } \
    } while (0)


char* rdma_recv_buffer=nullptr;
int rdma_recv_len=0;
//用来替换iov_iter作为iovec的迭代器
typedef struct iovec_iter{
    struct iovec* iov;     // 指向 iovec 数组的指针
    int iovcnt;            // iovec 数组的长度
    int index;             // 当前处理的 iovec 索引
    size_t offset;         // 当前 iovec 已处理的偏移量
    size_t total_len;      // 总的剩余数据长度
} iovec_iter;  

inline void iovec_iter_init(iovec_iter* iter, struct iovec* iov, int iovcnt) {
    iter->iov = iov;
    iter->iovcnt = iovcnt;
    iter->index = 0;
    iter->offset = 0;
    iter->total_len = 0;
    for (int i = 0; i < iovcnt; ++i) {
        iter->total_len += iov[i].iov_len;
    }
}

inline size_t iovec_iter_count(iovec_iter* iter) {
    return iter->total_len;
}

inline void iovec_iter_advance(iovec_iter* iter, size_t count) {
    iter->total_len -= count;
    while (count > 0 && iter->index < iter->iovcnt) {
        size_t left = iter->iov[iter->index].iov_len - iter->offset;
        if (count >= left) {
            count -= left;
            iter->index++;
            iter->offset = 0;
        } else {
            iter->offset += count;
            count = 0;
        }
    }
}

inline void iovec_iter_truncate(iovec_iter* iter, size_t max_len) {
    if (iter->total_len > max_len) {
        iter->total_len = max_len;
        // 需要根据 max_len 调整 iovcnt
        size_t len = 0;
        for (int i = iter->index; i < iter->iovcnt; ++i) {
            size_t iov_len = iter->iov[i].iov_len - (i == iter->index ? iter->offset : 0);
            if (len + iov_len > max_len) {
                iter->iovcnt = i + 1;
                iter->iov[i].iov_len = max_len - len + (i == iter->index ? iter->offset : 0);
                break;
            }
            len += iov_len;
        }
    }
}
//从 iter copy to dest
inline ssize_t copy_from_iovec(char* dest, size_t max_len, iovec_iter* iter) {
    size_t copied = 0;
    while (max_len > 0 && iter->index < iter->iovcnt) {
        struct iovec* iov = &iter->iov[iter->index];
        size_t iov_len = iov->iov_len - iter->offset;
        size_t to_copy = (max_len < iov_len) ? max_len : iov_len;
        memcpy(dest + copied, (char*)iov->iov_base + iter->offset, to_copy);
      //    LOG(DEBUG)<<"dest = \n";
      //   for(int i=0;i<to_copy;i++)
      //   {
      //    LOG(DEBUG)<<dest[i];
      //   }
      //   LOG(DEBUG)<<"%%##ss$$\n";
        iter->offset += to_copy;
        if (iter->offset == iov->iov_len) {
            iter->index++;
            iter->offset = 0;
        }
        copied += to_copy;
        max_len -= to_copy;
    }
    iter->total_len -= copied;
    return copied;
}


inline ssize_t copy_to_iovec(const char* src, size_t len, iovec_iter* iter) {
    size_t copied = 0;

    while (len > 0 && iter->index < iter->iovcnt) {
        struct iovec* iov = &iter->iov[iter->index];
        size_t iov_len = iov->iov_len - iter->offset;
        size_t to_copy = (len < iov_len) ? len : iov_len;

        memcpy((char*)iov->iov_base + iter->offset, src + copied, to_copy);

        iter->offset += to_copy;
        if (iter->offset == iov->iov_len) {
            iter->index++;
            iter->offset = 0;
        }

        copied += to_copy;
        len -= to_copy;
    }

    iter->total_len -= copied;
    return copied;
}

inline void beegfs_iovec_iter_clear(struct iovec_iter *iter)
{
   iter->iovcnt = 0;
}


char* filehandleID_xyp=nullptr;
enum NodeType
   {NODETYPE_Invalid = 0, NODETYPE_Meta = 1, NODETYPE_Storage = 2, NODETYPE_Client = 3,NODETYPE_Mgmt = 4, NODETYPE_Helperd = 5};
//---------------------------------1. 套接字创建 和 套接字连接----------------------------------------------------------------------------------------      
      enum NicAddrType        {NICADDRTYPE_STANDARD=0, NICADDRTYPE_SDP=1, NICADDRTYPE_RDMA=2};//有三种类型
      typedef enum NicAddrType NicAddrType_t;
      
      #define SOCKET_PEERNAME_LEN   24

      // typedef unsigned int __u32;

      // // typedef __u32 __bitwise __be32;
      // typedef __u32 __be32;


      // /* Internet address. */
      // struct in_addr {
      //    __be32	s_addr;
      // };
      
      typedef struct list_head {
         struct list_head *next;
         struct list_head *prev;
      } list_head_t;

      struct Socket
      {
         NicAddrType_t sockType;
         char peername[SOCKET_PEERNAME_LEN];
         struct in_addr peerIP;
         int boundPort;

         const struct SocketOps* ops;

         struct {
             struct list_head _list;//$$ 头文件
            //改成下面的
           

            short _events;// 要监听的事件
            short revents;// 实际发生的事件
         } poll;
      };
      
      struct PointerListElem
      {
         void* valuePointer;//一个指向数据的指针，可以指向任何类型的数据

         struct PointerListElem* prev;
         struct PointerListElem* next;
      };

      struct PointerList//双向链表
      {
         struct PointerListElem* head;
         struct PointerListElem* tail;

         size_t length; //$$ size_t 的头文件
      };
      struct ConnectionList
      {
         PointerList pointerList;
         bool owner;
      };
      typedef long		__kernel_long_t;

      typedef __kernel_long_t	__kernel_old_time_t;

      struct Time {
         __kernel_old_time_t	tv_sec;		/* seconds */
         long			tv_nsec;	/* nanoseconds */
      };



      struct PooledSocket
      {
         Socket socket;
         ConnectionList* pool;
         PointerListElem* poolElem;
         bool available; // == !acquired
         bool hasActivity; // true if channel was not idle (part of channel class in fhgfs_common)
         bool closeOnRelease; /* release must close socket. used for signal handling */
         Time expireTimeStart; // 0 means "doesn't expire", otherwise time when conn was established
         NicAddrType_t nicType; // same as the interface for which this conn was established
      };

      struct StandardSocket
      {
            PooledSocket pooledSocket;
         //struct pollfd pollfd;
            // struct socket* sock; //$$ 首字母小写的这个socket好像是内核定义的 我要把这个换掉 换成用户程序空间的 POSIX Socket API
         int sock;
         unsigned short sockDomain;
      };

      #define PF_INET		2	/* IP protocol family.  */
      // enum __socket_type
      // {
      //    SOCK_STREAM = 1
      // };
      

      



      //typedef __u16 __bitwise __be16;
      struct fhgfs_sockaddr_in
      {
         struct in_addr addr;
            // __be16 port;
         uint16_t port;         /* 端口号，使用主机字节序 */
      };

      
      /*
      // 注释掉 是因为 这里调用了内核的socket创建函数，sock_create_kern，不知道在用户程序空间同种的初始化应该怎么写
      bool _StandardSocket_initSock(StandardSocket* ssock, int domain, int type, int protocol)
      {
         int createRes;

         // prepare/create socket
      // #ifndef KERNEL_HAS_SOCK_CREATE_KERN_NS
         // createRes = sock_create_kern(domain, type, protocol, &ssock->sock);//$$ 头文件
         createRes=socket(domain, type, protocol);
      // #else
      //    createRes = sock_create_kern(&init_net, domain, type, protocol, &this->sock);
      // #endif
         if(createRes < 0)
         {

            return false;
         }

         // ssock->sock->sk->sk_allocation = GFP_NOFS;// $$
         // ssock->sock->sk->sk_data_ready = sock_readable;
         // ssock->sock->sk->sk_write_space = sock_write_space;
         // ssock->sock->sk->sk_state_change = sock_wakeup;
         // ssock->sock->sk->sk_error_report = sock_error_report;

         return true;
      }*/
   //   bool _StandardSocket_connectByIP(Socket* thissock, struct in_addr ipaddress, unsigned short port);
         struct SocketOps
      {
         // void (*uninit)(Socket* this);

         bool (*connectByIP)(Socket* thissock, struct in_addr ipaddress, unsigned short port);
         ssize_t (*sendto)(Socket* sock, struct iovec* iter,int iovcnt, int flags, fhgfs_sockaddr_in *to);
                  // ssize_t (*recvT)(Socket* this, struct iov_iter* iter, int flags, int timeoutMS);
         ssize_t (*recvT)(Socket* thissock, struct iovec* iter, int flags, int timeoutMS);

         // bool (*bindToAddr)(Socket* this, struct in_addr ipaddress, unsigned short port);
         // bool (*listen)(Socket* this);
         // bool (*shutdown)(Socket* this);
         // bool (*shutdownAndRecvDisconnect)(Socket* this, int timeoutMS);

        
      };
      

      bool _StandardSocket_connectByIP(Socket* thissock, struct in_addr ipaddress, unsigned short port);
      ssize_t _StandardSocket_sendto(Socket* sock, struct iovec* iter, int iovcnt,int flags,fhgfs_sockaddr_in *to);
      ssize_t _StandardSocket_recvT(Socket* thissock, struct iovec* iter, int flags, int timeoutMS);
      static const struct SocketOps standardOps = {
         // .uninit = _StandardSocket_uninit,

         .connectByIP = _StandardSocket_connectByIP,
         .sendto = _StandardSocket_sendto,
         .recvT = _StandardSocket_recvT,
         // .bindToAddr = _StandardSocket_bindToAddr,
         // .listen = _StandardSocket_listen,
         // .shutdown = _StandardSocket_shutdown,
         // .shutdownAndRecvDisconnect = _StandardSocket_shutdownAndRecvDisconnect,

         
      };







    // #define IPPROTO_TCP  6

    #include <arpa/inet.h>
    #define IFNAMSIZ	IF_NAMESIZE
    #define IF_NAMESIZE	16
    struct NicAddress
    {
    struct in_addr    ipAddr;//它通常用于存储IPv4地址。ipAddr 成员包含了NIC的IP地址。
    NicAddrType_t     nicType;
    char              name[IFNAMSIZ];//存储NIC的名称，例如 "eth0" 或 "bond0"
    #ifdef BEEGFS_RDMA
    struct ibv_device *ibdev;//ib_device 通常用于表示InfiniBand设备，这是一种高性能的计算机网络通信标准
    #endif
    };


   #define BEEGFS_DATA_VERSION (uint32_t(0))
   #define NETMSG_PREFIX            ((0x42474653ULL << 32) + BEEGFS_DATA_VERSION)

   struct NetMessageHeader
   {
      unsigned       msgLength; // in bytes
      uint16_t       msgFeatureFlags; // feature flags for derived messages (depend on msgType)
      uint8_t        msgCompatFeatureFlags; // for derived messages, similar to msgFeatureFlags, but
                                             // "compat" because there is no check whether receiver
                                             //understands these flags, so they might be ignored. 
      uint8_t        msgFlags;
      char*          msgPrefix; // NETMSG_PREFIX_STR (8 bytes) //$$ 这个为什么源代码注释了 ，可是__NetMessage_serializeHeader 中还有对prefix 的处理逻辑，不过确实不是对这个成员变量，就继续注释着吧
      unsigned short msgType; // the type of payload, defined as NETMSGTYPE_x
      uint16_t       msgTargetID; // targetID (not groupID) for per-target workers on storage server
      unsigned       msgUserID; // system user ID for per-user msg queues, stats etc.
      uint64_t       msgSequence; // for retries, 0 if not present
      uint64_t       msgSequenceDone; // a sequence number that has been fully processed, or 0
   };
   struct NetMessageOps;

      struct NetMessage
   {
      struct NetMessageHeader msgHeader;

      const struct NetMessageOps* ops;
   };
         struct NumNodeID
      {
               uint32_t value;
      };
   typedef struct NodeOrGroup
   {
      union
      {
         NumNodeID node;
         uint32_t group;
      };
      bool isGroup;
   } NodeOrGroup;


               //open file 相关的两个实现,选第二个 
               /**
                * Dummy function for serialize pointers
                * _NetMessage_serializeDummy 函数通常被用作一个默认的序列化函数，当某个需要序列化的对象没有提供具体的序列化实现时，就会调用这个函数。它的目的不是进行实际的序列化操作，而是作为一个错误处理机制，提醒开发者检查代码，确保正确的序列化函数被调用
                */
               /*
               void _NetMessage_serializeDummy(NetMessage* netM, SerializeCtx* ctx)
               {
                  printk_fhgfs(KERN_INFO, "Bug: Serialize function called, although it should not\n");
                  dump_stack();
               }

               const struct NetMessageOps OpenFileRespMsg_Ops = {//openfileRespMsg.c 
                  .serializePayload    = _NetMessage_serializeDummy,
                  // .deserializePayload  = OpenFileRespMsg_deserializePayload,
                  // .processIncoming = NetMessage_processIncoming,
                  // .getSupportedHeaderFeatureFlagsMask = NetMessage_getSupportedHeaderFeatureFlagsMask,
               };
               */




   //和序列化有关的逻辑先注释
   


 


   #define MIN_ENTRY_ID_LEN 1 // usually A-B-C, but also "root" and "disposal"
   


            // enum class FileEventType : uint32_t
            // {
            //    FLUSH       =  0,
            //    TRUNCATE    =  1,
            //    SETATTR     =  2,
            //    CLOSE_WRITE =  3,
            //    CREATE      =  4,
            //    MKDIR       =  5,
            //    MKNOD       =  6,
            //    SYMLINK     =  7,
            //    RMDIR       =  8,
            //    UNLINK      =  9,
            //    HARDLINK    = 10,
            //    RENAME      = 11,
            //    READ        = 12,
            // };
   enum FileEventType
   {
      FileEventType_FLUSH,
      FileEventType_TRUNCATE,
      FileEventType_SETATTR,
      FileEventType_CLOSE_WRITE,
      FileEventType_CREATE,
      FileEventType_MKDIR,
      FileEventType_MKNOD,
      FileEventType_SYMLINK,
      FileEventType_RMDIR,
      FileEventType_UNLINK,
      FileEventType_HARDLINK,
      FileEventType_RENAME,
      FileEventType_READ,
   };

   struct FileEvent
   {
      uint32_t eventType; // enum FileEventType 
         //const char* path; // NULL if invalid/could not be determined (empty is also allowed)  相对于挂载目录的相对路径
      char* path; // NULL if invalid/could not be determined (empty is also allowed)  相对于挂载目录的相对路径
      
      const char* target; //  link target for link, new name for rename 

      unsigned long pathPagePFN; //缓冲区 存 相对于挂载目录的相对路径
      unsigned long targetPagePFN;
   };


   //-------------------------requestMsg 初始化 -------------------------------
         // requestMsg 第二个参数初始化

         #define META_ROOTDIR_ID_STR            "root"

        
         #define ENTRYINFO_FEATURE_BUDDYMIRRORED 2 // entry is buddy mirrored


      // requestMsg 第三个参数 初始化
            enum FileCacheType
            { FILECACHETYPE_None = 0, FILECACHETYPE_Buffered = 1, FILECACHETYPE_Paged = 2,
            FILECACHETYPE_Native = 3};
         // open rw flags
            #define OPENFILE_ACCESS_READ           1
            #define OPENFILE_ACCESS_WRITE          2
            #define OPENFILE_ACCESS_READWRITE      4
            // open extra flags
            #define OPENFILE_ACCESS_APPEND         8
            #define OPENFILE_ACCESS_TRUNC         16
            #define OPENFILE_ACCESS_DIRECT        32 /* for direct IO */
            #define OPENFILE_ACCESS_SYNC          64 /* for sync'ed IO */



   #define NETMSGTYPE_OpenFile                        3001
   
   #define OPENFILEMSG_FLAG_HAS_EVENT          2 // contains file event logging information 

      typedef struct SerializeCtx {
      char* const data;
      unsigned length;
   } SerializeCtx;

   typedef struct DeserializeCtx {
      const char* data;
      size_t length;
   } DeserializeCtx;

   struct NetMessageOps
   {
      void (*serializePayload) (NetMessage* netM, SerializeCtx* ctx);// 这个函数有好多种实现方式
      unsigned (*getSupportedHeaderFeatureFlagsMask) (NetMessage* thismsg);
      bool (*deserializePayload) (NetMessage* thismsg, DeserializeCtx* ctx);

      // bool (*processIncoming) (NetMessage* this, struct App* app, fhgfs_sockaddr_in* fromAddr,
      //    struct Socket* sock, char* respBuf, size_t bufLen);
      

      // void (*release)(NetMessage* this);


      // // not strictly operations, but these are common to all messages and do not warrant their own functions
      // bool supportsSequenceNumbers;
   };

      void OpenFileMsg_serializePayload(NetMessage* netM, SerializeCtx* ctx);
      unsigned NetMessage_getSupportedHeaderFeatureFlagsMask(NetMessage* thismsg);
      // bool _NetMessage_deserializeDummy(NetMessage* thismsg, DeserializeCtx* ctx);
      bool OpenFileRespMsg_deserializePayload(NetMessage* thismsg, DeserializeCtx* ctx);


   // const struct NetMessageOps OpenFileMsg_Ops = {//openfileMsg.c
   NetMessageOps OpenFileMsg_Ops = {//openfileMsg.c 

      .serializePayload = OpenFileMsg_serializePayload,
      // .getSupportedHeaderFeatureFlagsMask = NetMessage_getSupportedHeaderFeatureFlagsMask,
      // .deserializePayload = _NetMessage_deserializeDummy,
      // .processIncoming = NetMessage_processIncoming,
      
      // .supportsSequenceNumbers = true,
   };
   // const struct NetMessageOps OpenFileRespMsg_Ops = {//openfileRespMsg.c 
    NetMessageOps OpenFileRespMsg_Ops = {//openfileRespMsg.c 

                  // .serializePayload    = _NetMessage_serializeDummy,
                  .getSupportedHeaderFeatureFlagsMask = NetMessage_getSupportedHeaderFeatureFlagsMask,
                  .deserializePayload  = OpenFileRespMsg_deserializePayload,
                  // .processIncoming = NetMessage_processIncoming,
                  
               };



   enum DirEntryType
   { // don't use these directly, use the DirEntryType_IS...() macros below to check entry types
      DirEntryType_INVALID = 0, DirEntryType_DIRECTORY = 1, DirEntryType_REGULARFILE = 2,
      DirEntryType_SYMLINK = 3, DirEntryType_BLOCKDEV = 4, DirEntryType_CHARDEV = 5,
      DirEntryType_FIFO = 6, DirEntryType_SOCKET = 7
   };

      struct EntryInfo// 看能不能删除一些信息
   {
      NodeOrGroup owner; //条目所有者的用户或组标识符
      const char* parentEntryID;// 推测是父目录的唯一标识，
      const char* entryID;//这是文件系统内部用于唯一标识文件或目录的标识符。
      const char* fileName;//这是用户在文件系统中看到的名称。
      DirEntryType entryType;// 表明是文件还是目录
      int featureFlags;//一般都是初始化为0，
            
   };
   struct OpenFileMsg
      {
         NetMessage netMessage;

         NumNodeID clientNumID;
         unsigned sessionIDLen;
         const EntryInfo* entryInfoPtr; // not owned by this object
         unsigned accessFlags;
         const struct FileEvent* fileEvent;
      };


//-----------------------------------2. 配置响应缓冲区 outrespbuffer-----------------------------------------------------------------------------

      
      //根据响应缓冲区类型，从缓冲区存储中获取或分配缓冲区。
      enum MessagingTkBufType
      {
         MessagingTkBufType_BufStore = 0,
         MessagingTkBufType_kmalloc  = 1, // only for small response messages (<4KiB)
      };

      struct Condition
      {
            // wait_queue_head_t queue;
         pthread_cond_t cond;
         pthread_mutex_t mutex;
      };

      struct NoAllocBufferStore
      {
         char** bufArray;

         size_t numBufs;
         size_t bufSize;

         size_t numAvailable; // number of currently available buffers in the store

         // Mutex mutex;
            //pthread_mutex_t mutex; 这个结构体没有互斥锁 锁反而放在了 Condition 结构中

         Condition newBufCond;

      // #ifdef BEEGFS_DEBUG
      //    RBTree pidDebugTree; // store tasks that have taken a buffer
      // #endif
      };

      


 //7. receive response ---------------------------------------------------------
// 接收消息 

#define NETMSGTYPE_AckNotifyResp                   3032
#define NETMSGTYPE_GenericResponse                 4009

#define NETMSG_HEADER_LENGTH     40 /* length of the header (see struct NetMessageHeader) */
#define NETMSG_MIN_LENGTH        NETMSG_HEADER_LENGTH





//8. 反序列化解析-------------------------------------------------------------------------



// invalid messages
#define NETMSGTYPE_Invalid                            0

#define MSGHDRFLAG_BUDDYMIRROR_SECOND  (0x01) //这个标志表示消息是发送给镜像组中的第二个（辅助）节点
#define MSGHDRFLAG_IS_SELECTIVE_ACK    (0x02) //标志用于指示接收方应该发送一个选择性确认（selective ACK）
#define MSGHDRFLAG_HAS_SEQUENCE_NO     (0x04) //标志表示消息包含一个序列号。序列号用于确保消息的顺序和完整性


// extern const struct NetMessageOps OpenFileRespMsg_Ops;

#define NETMSGTYPE_OpenFileResp                    3002

struct PathInfo
{
   union
   {
               // const unsigned flags;          // additional flags (e.g. PATHINFO_FEATURE_INLINED)
         unsigned flags;          // additional flags (e.g. PATHINFO_FEATURE_INLINED)
         unsigned _flags;
   };

   union
   {
                  // const unsigned origParentUID; // UID who created the file, only set for FileInodes
         unsigned origParentUID;
         unsigned _origParentUID;
   };

   union
   {
         // ID of the dir in which the file was created in. Only set for FileInodes
                  // char const* const origParentEntryID;
         char* origParentEntryID;
         char* _origParentEntryID;
   };
};
struct UInt16List
{
   struct PointerList pointerList;
};
/**
 * Note: Derived from the corresponding list. Use the list iterator for read-only access
 */
struct UInt16Vec
{
   struct UInt16List UInt16List;

   uint16_t* vecArray;
   size_t vecArrayLen;
};

#define STRIPEPATTERN_Invalid      0 
#define STRIPEPATTERN_Raid0        1
#define STRIPEPATTERN_Raid10       2
#define STRIPEPATTERN_BuddyMirror  3
struct StripePattern
{
   unsigned patternType; // STRIPEPATTERN_...
   unsigned chunkSize; // must be a power of two (optimizations rely on it)
   unsigned serialPatternLength; // for (de)serialization 表示序列化后的数据长度。

   // 我把它的变量全注释了


   // unsigned patternType; // STRIPEPATTERN_...
   // unsigned chunkSize; // must be a power of two (optimizations rely on it)

   // unsigned serialPatternLength; // for (de)serialization 表示序列化后的数据长度。

   // // virtual functions
   // void (*uninit) (StripePattern* this);

   // // (de)serialization
   // bool (*deserializePattern) (StripePattern* this, DeserializeCtx* ctx);//用于从给定的反序列化上下文DeserializeCtx中反序列化条带模式。

   // size_t (*getStripeTargetIndex) (StripePattern* this, int64_t pos);//用于根据数据的位置pos计算应该存储在哪个条带目标上。
   // uint16_t (*getStripeTargetID) (StripePattern* this, int64_t pos);//用于根据数据的位置pos获取对应的条带目标ID。
   // void (*getStripeTargetIDsCopy) (StripePattern* this, UInt16Vec* outTargetIDs);//用于将条带目标ID列表复制到提供的UInt16Vec向量中。
   // UInt16Vec* (*getStripeTargetIDs) (StripePattern* this);//用于获取一个指向条带目标ID列表的指针。
   // UInt16Vec* (*getMirrorTargetIDs) (StripePattern* this);//用于获取镜像目标ID列表的指针，这通常用于支持数据冗余的条带模式。
   // unsigned (*getMinNumTargets) (StripePattern* this);//用于获取条带模式所需的最小目标（存储节点）数量。
   // unsigned (*getDefaultNumTargets) (StripePattern* this);//用于获取条带模式的默认目标（存储节点）数量。
};

struct OpenFileRespMsg
{
    NetMessage netMessage;

    int result;
    unsigned fileHandleIDLen;
    const char* fileHandleID;

    PathInfo pathInfo;

    uint64_t fileVersion;

    // for serialization
    StripePattern* pattern; // not owned by this object!

    // for deserialization
    const char* patternStart;
    uint32_t patternLength;
};

#define PATHINFO_FEATURE_ORIG         1 /* inidicate chunks are stored with origParentUID and and origParentEntryID */
#define PATHINFO_FEATURE_ORIG_UNKNOWN 2 /* indicates FEATURE_ORIG is unknown and needs to be requested from the meta-inode */
                                          


//--------------------------------------channel认证 -----------

struct SimpleInt64Msg
{
   NetMessage netMessage;

   int64_t value;
};

struct AuthenticateChannelMsg
{
   SimpleInt64Msg simpleInt64Msg;
};

struct SimpleIntMsg
{
   NetMessage netMessage;

   int value;
};

struct SetChannelDirectMsg
{
   SimpleIntMsg simpleIntMsg;
};




#define NETMSGTYPE_AuthenticateChannel             4007
#define NETMSGTYPE_PeerInfo                        4011



void SimpleInt64Msg_serializePayload(NetMessage* nmsg, SerializeCtx* ctx);
const struct NetMessageOps SimpleInt64Msg_Ops = {
   .serializePayload = SimpleInt64Msg_serializePayload,
//    .deserializePayload = SimpleInt64Msg_deserializePayload,
//    .processIncoming = NetMessage_processIncoming,
//    .getSupportedHeaderFeatureFlagsMask = NetMessage_getSupportedHeaderFeatureFlagsMask,
};




/**
 * Note: Remember to keep this in sync with FHGFSOPS_ERRLIST
 *
 * Note: We need the negative dummy (-1) because some return values (like CommKit) cast this enum to
 * negative int64_t and this leads to bad (positive) values when the enum isn't signed. So the dummy
 * forces the compiler to make the enum a signed variable.
 */
enum FhgfsOpsErr
{
   FhgfsOpsErr_DUMMY_DONTUSEME         =    -1, /* see comment above */
   FhgfsOpsErr_SUCCESS                 =     0,
   FhgfsOpsErr_INTERNAL                =     1,
   FhgfsOpsErr_INTERRUPTED             =     2,
   FhgfsOpsErr_COMMUNICATION           =     3,
   FhgfsOpsErr_COMMTIMEDOUT            =     4,
   FhgfsOpsErr_UNKNOWNNODE             =     5,
   FhgfsOpsErr_NOTOWNER                =     6,
   FhgfsOpsErr_EXISTS                  =     7,
   FhgfsOpsErr_PATHNOTEXISTS           =     8, //路径并不存在
   FhgfsOpsErr_INUSE                   =     9,
   FhgfsOpsErr_DYNAMICATTRIBSOUTDATED  =    10,
   FhgfsOpsErr_PARENTTOSUBDIR          =    11,
   FhgfsOpsErr_NOTADIR                 =    12,
   FhgfsOpsErr_NOTEMPTY                =    13,
   FhgfsOpsErr_NOSPACE                 =    14,
   FhgfsOpsErr_UNKNOWNTARGET           =    15,
   FhgfsOpsErr_WOULDBLOCK              =    16,
   FhgfsOpsErr_INODENOTINLINED         =    17, // inode is not inlined into the dentry
   FhgfsOpsErr_SAVEERROR               =    18, // saving to the underlying file system failed
   FhgfsOpsErr_TOOBIG                  =    19, // corresponds to EFBIG
   FhgfsOpsErr_INVAL                   =    20, // corresponds to EINVAL
   FhgfsOpsErr_ADDRESSFAULT            =    21, // corresponds to EFAULT
   FhgfsOpsErr_AGAIN                   =    22, // corresponds to EAGAIN
   FhgfsOpsErr_STORAGE_SRV_CRASHED     =    23, /* Potential cache loss for open file handle.
                                                   (Server crash detected.)*/
   FhgfsOpsErr_PERM                    =    24, // corresponds to EPERM
   FhgfsOpsErr_DQUOT                   =    25, // corresponds to EDQUOT (quota exceeded)
   FhgfsOpsErr_OUTOFMEM                =    26, // corresponds to ENOMEM (mem allocation failed)
   FhgfsOpsErr_RANGE                   =    27, // corresponds to ERANGE (needed for xattrs)
   FhgfsOpsErr_NODATA                  =    28, // corresponds to ENODATA==ENOATTR (xattr not found)
   FhgfsOpsErr_NOTSUPP                 =    29, // corresponds to EOPNOTSUPP
   FhgfsOpsErr_UNKNOWNPOOL             =    30, // unknown storage pool
};
typedef enum FhgfsOpsErr FhgfsOpsErr;

//----------------------open_communicate1.cpp 的函数实现--------------------------------------------------


//----------------------1------------------------------------


#include <cstdlib>     // For malloc and free
#include <string.h>     // For memset, memcpy, memcmp, and memmove
#include <stddef.h>
#include <iostream>
#include <cstring> // C++ 标准库中的内存操作函数
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>      // for open 系统调用 osflag=O_RDONLY|O_WRONLY
#include <sys/socket.h> // For socket operations,struct msghdr 
#include <sys/types.h> // for typeof
#include <sys/uio.h> //struct iovec 的定义，它通常与 readv、writev、sendmsg 和 recvmsg
#include <unistd.h>           // for getuid
#include <pthread.h>
// #include "communicate.h"
#include <memory> // for unique_ptr 
#include <linux/sockios.h>  // 包含 SIOCOUTQ 命令
#include <sys/ioctl.h>      // 包含 ioctl 函数声明
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
// #include <netinet/in.h> // For struct in_addr and other IPv4 related definitions
// #include <err是no.h>      // For error codes
// #include <time.h>       // For struct timespec and other time related functions
// #include <endian.h>     // For byte order conversions, or <arpa/inet.h>
using namespace std; 

      void _PooledSocket_init(PooledSocket* psock, NicAddrType_t nicType)
      {  

         Socket* sock=(Socket*)psock;
         memset(sock, 0, sizeof(*sock) );//$$ 头文件

         sock->sockType = NICADDRTYPE_STANDARD;
         sock->boundPort = -1;

         psock->available = false;
         psock->hasActivity = true; // initially active to avoid immediate disconnection
         psock->closeOnRelease = false;
         psock->expireTimeStart.tv_sec = 0;
         psock->expireTimeStart.tv_nsec = 0;
         psock->nicType = nicType;
         psock->pool = nullptr;
         psock->poolElem = nullptr;
      }


      bool _StandardSocket_connectByIP(Socket* thissock, struct in_addr ipaddress, unsigned short port)
      {  
         LOG(DEBUG)<<"go into _StandardSocket_connectByIP \n";
         // const int timeoutMS = STANDARDSOCKET_CONNECT_TIMEOUT_MS;
         
         // 将通用套接字类型转换为标准套接字类型。
         StandardSocket* thisCast = (StandardSocket*)thissock;

         // 设置服务器地址 $$这个数据结构得引用某个头文件才行
         // struct sockaddr_in serveraddr = 
         // {
         //    .sin_family = AF_INET,
         //    .sin_addr = ipaddress,
         //    .sin_port = htons(port),// $$ 这个函数的具体实现 得看
         // };
         struct sockaddr_in serveraddr;
         memset(&serveraddr, 0, sizeof(serveraddr));
         serveraddr.sin_family = AF_INET;
         serveraddr.sin_port = htons(port); // 端口号需要转换为网络字节序
         serveraddr.sin_addr.s_addr = ipaddress.s_addr;

      

         // int connRes = kernel_connect(thisCast->sock,  // 调用内核连接函数
         //    (struct sockaddr*) &serveraddr, // $$ 这个数据结构得引用某个头文件才行
         //    sizeof(serveraddr),
         //    O_NONBLOCK);

         // 尝试连接到服务器
         int connRes = connect(thisCast->sock, (struct sockaddr*) &serveraddr, sizeof(serveraddr));
         // 尝试连接
            if (connRes< 0) {
                  LOG_printf(DEBUG,"尝试连接失败\n");
                  if (errno != EINPROGRESS) {
                     LOG_printf(DEBUG,"连接失败，错误码：%d\n", errno);
                     switch (errno) {
                              case ECONNREFUSED:
                                 LOG_printf(DEBUG,"地址已在使用\n");
                                 break;
                              case ENETUNREACH:
                                 LOG_printf(DEBUG,"网络不可达\n");
                                 break;
                              case ETIMEDOUT:
                                 LOG_printf(DEBUG,"连接超时\n");
                                 break;
                              default:
                                 LOG_printf(DEBUG,"未知错误\n");
                                 break;
                           }
                     // 连接失败，处理错误
                     return false;
                  }

                  LOG_printf(DEBUG,"连接正在进行中\n");

                  // 连接正在进行中，使用 select/poll检查连接状态
                  struct pollfd pfd = { .fd = thisCast->sock, .events = POLLOUT };
                  int ret = poll(&pfd, 1, -1); // 无限期等待

                  if (ret <0) {
                     LOG_printf(DEBUG,"poll 错误\n");
                     // 错误处理
                     return false;
                  } else if (ret ==0) {
                     LOG_printf(DEBUG,"poll 超时\n");
                     // 超时处理
                     return false;
                  }

               // 检查连接是否成功
               int error;
               socklen_t len = sizeof(error);
               if (getsockopt(thisCast->sock, SOL_SOCKET, SO_ERROR, &error, &len) <0) {
                  LOG_printf(DEBUG,"getsockopt 错误\n");
                  // 错误处理
                  return false;
               }

               if (error != 0) {
                  LOG_printf(DEBUG,"连接失败，错误码：%d\n", error);
                  // 连接失败，处理错误
                  return false;
               }

               LOG_printf(DEBUG,"连接成功\n");
               // 连接成功
               return true;
            }
         else if(connRes>=0)
         {
               LOG_printf(DEBUG,"连接成功\n");
         }
        
      }
            /*
                  // connected immediately 如果连接立即成功
                  if(!connRes)
                  {
                     // if(!this->peername[0])
                     // {
                     //    SocketTk_endpointAddrToStrNoAlloc(this->peername, SOCKET_PEERNAME_LEN, ipaddress, port);
                     //    this->peerIP = ipaddress;// 设置对等IP
                     // }
                     LOG_printf(DEBUG,"连接成功 \n");
                     return true;
                  }
                  else
                  {
                     if(connRes == -EINPROGRESS)// 如果连接正在进行中
                     {
                        LOG_printf(DEBUG,"连接正在进行中 \n");
                     }
                  }
                  LOG_printf(DEBUG,"连接失败 \n");

                  return false;
         */
         /* 下面这段轮询代码 可以暂时忽略 不影响连接
         if(connRes)
         {
            if(connRes == -EINPROGRESS)// 如果连接正在进行中
            { // wait for "ready to send data"
            // 等待“准备好发送数据”
               PollState state;// 轮询状态
               int pollRes;// 轮询结果

               PollState_init(&state);// 初始化轮询状态
               PollState_addSocket(&state, this, POLLOUT);// 添加套接字到轮询状态

               pollRes = SocketTk_poll(&state, timeoutMS);// 执行轮询

               if(pollRes > 0)// 如果我们得到了某些结果（也可能是错误）
               { // we got something (could also be an error)

                  // note: it's important to test ERR/HUP/NVAL here instead of POLLOUT only, because
                     POLLOUT and POLLERR can be returned together. 
                  // 测试ERR/HUP/NVAL而不是仅仅POLLOUT，因为POLLOUT和POLLERR可以一起返回
                  if(this->poll.revents & (POLLERR | POLLHUP | POLLNVAL) )
                     return false;

                  // connection successfully established
                  // 连接成功建立
                  // 如果还没有设置对等名称
                  if(!this->peername[0])
                  {
                     SocketTk_endpointAddrToStrNoAlloc(this->peername, SOCKET_PEERNAME_LEN, ipaddress, port);
                     this->peerIP = ipaddress;
                  }

                  return true;
               }
               else
               if(!pollRes)
                  return false; // timeout
               else
                  return false; // connection error

            } // end of "EINPROGRESS"
         }
         else
         { // connected immediately // 如果连接立即成功
            // 如果还没有设置对等名称（例如，通过connect(hostname)）
            // set peername if   not done so already (e.g. by connect(hostname) )
            if(!this->peername[0])
            {
               SocketTk_endpointAddrToStrNoAlloc(this->peername, SOCKET_PEERNAME_LEN, ipaddress, port);
               this->peerIP = ipaddress;// 设置对等IP
            }

            return true;
         }
         */
         
         
     
      

      // static int beegfs_sendmsg(struct socket *sock, struct msghdr *msg, size_t len)
      static ssize_t beegfs_sendmsg(int sockfd, const struct msghdr *msg, int flags) 
      {
                     // #ifdef KERNEL_HAS_RECVMSG_SIZE
                     //    return sock_sendmsg(sock, msg, len);// 这两个函数是linux kernel 函数实现 在 linux/net/socket.c
                     // #else
                     //    return sock_sendmsg(sock, msg);
                     // #endif
               // return sendmsg(sockfd, msg, flags);
         ssize_t sentBytes = sendmsg(sockfd, msg, flags);
            
            // 检查发送结果
            if (sentBytes < 0) {
               // 发送失败，可以进一步检查 errno 来确定错误原因
               perror("sendmsg failed");
               return -1; // 或者可以选择返回 errno 值
            } else if (sentBytes == 0) {
               // 发送成功，但没有发送任何数据
               LOG_printf(DEBUG,"sendmsg completed with zero bytes sent\n");
               return 0;
            } else {
               // 发送成功，并发送了 sentBytes 指定的字节数
               LOG_printf(DEBUG,"sendmsg sent %zd bytes\n", sentBytes);
               return sentBytes;
            }

      }     
            //iovec 和 msghdr 是 头文件里已经定义好的
            /*
            struct iovec
            {
               void *iov_base;	// Pointer to data.  
               size_t iov_len;	// Length of data.  
            };

            struct msghdr
            {
               void *msg_name;		// Address to send to/receive from.  
               socklen_t msg_namelen;	// Length of address data.  

               struct iovec *msg_iov;	// Vector of data to send/receive into.  
               size_t msg_iovlen;		// Number of elements in the vector.  

               void *msg_control;		// Ancillary data (eg BSD filedesc passing). 
               size_t msg_controllen;	// Ancillary data buffer length.
                           //!! The type should be socklen_t but the
                           //definition of the kernel is incompatible
                          // with this.  

               int msg_flags;		// Flags on received message.  
            };
            */
      /*
      用于发送数据到网络套接字的函数。这个函数是 StandardSocket 类的一个成员函数，它封装了 Linux 内核的 sendmsg 系统调用。
         this：指向 Socket 结构体的指针，代表一个网络套接字。
         iter：指向 iov_iter 结构体的指针，描述了要发送的数据缓冲区。
         flags：发送操作的标志。
         to：指向 fhgfs_sockaddr_in 结构体的指针，包含了目标地址信息。
      */
      ssize_t _StandardSocket_sendto(Socket* sock, struct iovec* iter, int iovcnt,int flags,
         fhgfs_sockaddr_in *to)
      {  
         LOG(DEBUG)<<"@@ 进入 _StandardSocket_sendto\n";
      
         StandardSocket* thisCast = (StandardSocket*)sock;
               //从 StandardSocket 结构体中获取 Linux 内核的 socket 结构体指针。
                  //struct socket *sock = thisCast->sock; //


         size_t len;
         struct sockaddr_in toSockAddr;
         memset(&toSockAddr, 0, sizeof(toSockAddr));
                  // len = iov_iter_count(iter); //这个是内核函数 不能用
         //初始化 msghdr 结构体，它用于 sendmsg 系统调用。msghdr 结构体包含了发送操作的所有信息，如控制信息、标志、目标地址和数据缓冲区。
         struct msghdr msg =
         {
            .msg_name         = (struct sockaddr*)(to ? &toSockAddr : nullptr), // to是个nullptr
            .msg_namelen      = sizeof(toSockAddr),
                  //.msg_iter         = *iter,
            .msg_iov          = iter,
            .msg_iovlen       = (size_t)iovcnt,
            .msg_control      = nullptr,
            .msg_controllen   = 0,
            .msg_flags        = flags   //flags | MSG_NOSIGNAL,
         };

         
         //如果提供了目标地址，初始化 sockaddr_in 结构体，并设置目标地址和端口。 感觉这个toSockAddr 赋值了也没有用上
         if (to)
         {
            toSockAddr.sin_family = thisCast->sockDomain;
            toSockAddr.sin_addr = to->addr;
            toSockAddr.sin_port = to->port;
         }

         // Check if any iov_len is greater than 0
         LOG(DEBUG)<<"Check if any iov_len is greater than 0 \n";
         LOG(DEBUG)<<"iovcnt = "<<iovcnt<<std::endl;
            int hasData = 0;
            for (int i = 0; i < iovcnt; ++i) {
                     LOG_printf(DEBUG,"%d ===[_StandardSocket_sendto] iter[i].iov_len = %ld \n",i,iter[i].iov_len );

               if (iter[i].iov_len > 0) {
                     hasData = 1;
                     break;
               }
            }

            if (!hasData) {
               LOG_printf(DEBUG,"No data in the iov buffer.\n");
               return 0; // No data to send
            }

               //beegfs_sendmsg 调用了linux 内核的函数 sock_sendmsg(sock, msg, len)or  sock_sendmsg(sock, msg);// 这两个函数是linux kernel 函数实现 在 linux/net/socket.c
               // sendRes = beegfs_sendmsg(sock, &msg, len);
         // ssize_t sendRes = beegfs_sendmsg(thisCast->sock, &msg, flags);
         ssize_t sentBytes = sendmsg(thisCast->sock,&msg, flags);
               
         // 检查发送结果
         if (sentBytes < 0) {
            // 发送失败，可以进一步检查 errno 来确定错误原因
            perror("sendmsg failed");
            return -1; // 或者可以选择返回 errno 值
         } else if (sentBytes == 0) {
            // 发送成功，但没有发送任何数据
            LOG_printf(DEBUG,"sendmsg completed with zero bytes sent\n");

            // Check if the send buffer is empty
         int bytesQueued = 0;
         socklen_t intSize = sizeof(bytesQueued);

         // Use ioctl to get the amount of data queued in the send buffer
         if (ioctl(thisCast->sock, SIOCOUTQ, &bytesQueued) < 0) {
               perror("ioctl SIOCOUTQ failed");
         } else if (bytesQueued == 0) {
               LOG_printf(DEBUG,"The send buffer is empty.\n");
         }

         // Check if the connection is half-closed
         int sockOptVal;
         socklen_t optLen = sizeof(sockOptVal);
         if (getsockopt(thisCast->sock, SOL_SOCKET, SO_LINGER, &sockOptVal, &optLen) == 0) {
               if (sockOptVal) {
                  LOG_printf(DEBUG,"The connection is half-closed.\n");
               }
         }

            int sockErr;
                  socklen_t optlen = sizeof(sockErr);
                  // Check if the connection has been closed
                  if (getsockopt(thisCast->sock, SOL_SOCKET, SO_ERROR, &sockErr, &optlen) == 0 && sockErr != 0) {
                        LOG_printf(DEBUG,"Socket error: %s\n", strerror(sockErr));
                  } else {
                        LOG_printf(DEBUG,"sendmsg completed with zero bytes sent. Check if the send buffer is empty or the connection is half-closed.\n");
                  }


            return 0;
         } else {
            // 发送成功，并发送了 sentBytes 指定的字节数
            LOG_printf(DEBUG,"sendmsg sent %zd bytes\n", sentBytes);
            return sentBytes;
         }

               //如果发送成功，更新 iov_iter 结构体，以反映已经发送的数据量。
               // if(sentBytes >= 0)
               //    iov_iter_advance(iter, sentBytes);

         return sentBytes;
      }









      bool StandardSocket_init(StandardSocket* ssock, int domain, int type, int protocol)
      {
         Socket* thisBase = (Socket*)ssock;

         NicAddrType_t nicType = NICADDRTYPE_STANDARD;

         _PooledSocket_init( (PooledSocket*)ssock, nicType);

         thisBase->ops = &standardOps;

         // normal init part

         ssock->sock = 0;

         ssock->sockDomain = domain;

         // 用户空间程序中 对 StandardSocket 中 的sock变量初始化 

         // 创建套接字
         int sock = socket(AF_INET, SOCK_STREAM, 0);
         if (sock == -1) {
            LOG_printf(DEBUG,"Error creating socket \n");
            return false;
         }
         ssock->sock =sock;

         // 设置套接字为非阻塞模式
         int flags = fcntl(ssock->sock, F_GETFL, 0);
         if (flags == -1) {
            LOG_printf(DEBUG,"Error getting socket flags\n");
            close(ssock->sock);
            return false;
         }
         if (fcntl(ssock->sock, F_SETFL, flags | O_NONBLOCK) == -1) {
            LOG_printf(DEBUG,"Error setting socket non-blocking\n");
            close(ssock->sock);
            return false;
         }

         return true;
         // return _StandardSocket_initSock(ssock, domain, type, protocol);// 这个函数的功能应该是给内核定义的结构 socket 初始化，但是我这里是用户空间程序，不能直接访问和操作该结构体
      }
      
      StandardSocket* StandardSocket_construct(int domain, int type, int protocol)
      {
         // StandardSocket* ssock = kmalloc(sizeof(*this), GFP_NOFS);//需要引用这个函数的头文件
         StandardSocket* ssock = (StandardSocket*)malloc(sizeof(StandardSocket));
         if(!ssock ||
            !StandardSocket_init(ssock, domain, type, protocol) )
         {
            free(ssock);
            return ssock;
         }

         return ssock;
      }







    void initNicAddressArray_meta(NicAddress nicAddr[]) { //initNicAddressArray 原名是这个

      struct in_addr ipAddr;
      int interface_num=10;//@@
      const char* ipStr[interface_num] ;
      //0~4 是 162 server 元数据服务器的网卡接口
      //5~9 是 224 server 元数据服务器的网卡接口

      ipStr[0]= "192.168.100.1";
      ipStr[1]= "192.168.3.211";
      ipStr[2]= "192.168.0.211";
      ipStr[3]= "192.168.0.210";
      ipStr[4]= "10.26.42.230";
      strcpy(nicAddr[0].name,"tmfifo_net0");
      strcpy(nicAddr[1].name,"ens4f1np1");
      strcpy(nicAddr[2].name,"ens4f1np1");
      strcpy(nicAddr[3].name,"ens4f0np0");
      strcpy(nicAddr[4].name,"eno8403");

      // 224 server
      ipStr[5]= "192.168.0.218";//RDMA
      ipStr[6]= "192.168.0.218";
      ipStr[7]= "10.50.109.190";
      ipStr[8]= "10.26.42.224";
      strcpy(nicAddr[5].name,"ens2np0");
      strcpy(nicAddr[6].name,"ens2np0");
      strcpy(nicAddr[7].name,"ztstkzavtz");
      strcpy(nicAddr[8].name,"ens65f1");
      //217 server
      ipStr[9]= "10.26.42.217";//RDMA
      // ipStr[6]= "192.168.0.218";
      // ipStr[7]= "10.50.109.190";
      // ipStr[8]= "10.26.42.224";
      strcpy(nicAddr[9].name,"br-ursa");
      // strcpy(nicAddr[6].name,"ens2np0");
      // strcpy(nicAddr[7].name,"ztstkzavtz");
      // strcpy(nicAddr[8].name,"ens65f1");


      for(int i=0;i<interface_num;i++)
      { 
         // 使用inet_pton函数将IP字符串转换为网络字节序的二进制形式
         nicAddr[i].ipAddr.s_addr=inet_addr(ipStr[i]);
         // if (inet_pton(AF_INET, ipStr[i], &(nicAddr[i].ipAddr.s_addr)) != 1) {
         //    LOG_printf(DEBUG,"%s occurs error\n",ipStr[i]); 
         // }
         // 打印转换后的IP地址，用于验证
         // LOG_printf(DEBUG,"The IP address is: %s\n", inet_ntoa(nicAddr[i].ipAddr));

      } 
    }
    void initNicAddressArray_storage(NicAddress nicAddr[]) {

      struct in_addr ipAddr;
      int interface_num=12;//@@

      const char* ipStr[interface_num] ;
      //0~4 是 162 server 元数据服务器的网卡接口
      //5~9 是 217 server 存储服务器的网卡接口

      ipStr[0]= "192.168.100.1";
      ipStr[1]= "192.168.3.211";
      ipStr[2]= "192.168.0.211";
      // ipStr[3]= "192.168.0.210";//
      ipStr[3]= "192.168.5.210";//

      ipStr[4]= "10.26.42.230";
      strcpy(nicAddr[0].name,"tmfifo_net0");
      strcpy(nicAddr[1].name,"ens4f1np1");
      strcpy(nicAddr[2].name,"ens4f1np1");
      strcpy(nicAddr[3].name,"ens4f0np0");//ens4f0np0
      strcpy(nicAddr[4].name,"eno8403");
      // 217 server
      ipStr[5]= "192.168.0.203";//RDMA
      ipStr[6]= "192.168.49.1";
      ipStr[7]= " 192.168.0.203";
      ipStr[8]= "172.20.0.1";
      ipStr[9]= "172.18.0.1";
      ipStr[10]= "10.26.42.217";
      ipStr[11] = "10.26.42.224";     //xfusion3 TCP


      strcpy(nicAddr[5].name,"enp152s0np0");
      strcpy(nicAddr[6].name,"br-3eb3c5932eb8");
      strcpy(nicAddr[7].name,"enp152s0np0");
      strcpy(nicAddr[8].name,"br-f54ca3174bd7");
      strcpy(nicAddr[9].name,"br-9fc2d0c06cfc");
      strcpy(nicAddr[10].name,"br-ursa");
      strcpy(nicAddr[11].name, "ens65f1");   // xfusion3 TCP



      for(int i=0;i<interface_num;i++)
      { 
         // 使用inet_pton函数将IP字符串转换为网络字节序的二进制形式
         nicAddr[i].ipAddr.s_addr=inet_addr(ipStr[i]);
         // if (inet_pton(AF_INET, ipStr[i], &(nicAddr[i].ipAddr.s_addr)) != 1) {
         //    LOG_printf(DEBUG,"%s occurs error\n",ipStr[i]); 
         // }
         // 打印转换后的IP地址，用于验证
         LOG_printf(DEBUG,"The IP address is: %s\n", inet_ntoa(nicAddr[i].ipAddr));

      } 
    }






//-----------------------------------------------3. 请求信息的序列化 -----------------------------------------------
 


   static inline void Serialization_serializeUInt(SerializeCtx* ctx, unsigned value)
   {
      if(ctx->data)
         {
            // put_unaligned_le32(value, ctx->data + ctx->length); // 这个是内核程序接口 替换成以下的手动实现
            // 将ctx->data转换为uint8_t*类型，以便按字节处理数据
            // LOG_printf(DEBUG,"[Serialization_serializeUInt] ctx->length = %d \n ",ctx->length);
         uint8_t *p = (uint8_t *)ctx->data + ctx->length;
         // 按小端序写入value的每个字节
         p[0] = (value & 0xFF);
         p[1] = (value >> 8) & 0xFF;
         p[2] = (value >> 16) & 0xFF;
         p[3] = (value >> 24) & 0xFF;
         }
      // else
      // {
      //    LOG_printf(DEBUG,"[Serialization_serializeUInt] error 1 \n");
      // }
      ctx->length += sizeof(value);
   }


   static inline void Serialization_serializeUShort(SerializeCtx* ctx, unsigned short value)
   {
      if(ctx->data)
      {
         // put_unaligned_le16(value, ctx->data + ctx->length);// 这个是内核程序接口 替换成以下的手动实现

         // 将ctx->data转换为uint8_t*类型，以便按字节处理数据
         uint8_t *p = (uint8_t *)ctx->data + ctx->length;
         
         // 按小端序写入value的每个字节
         p[0] = (value & 0xFF);            // 写入低字节
         p[1] = (value >> 8) & 0xFF;       // 写入高字节

      }

      ctx->length += sizeof(value);
   }

   static inline void Serialization_serializeChar(SerializeCtx* ctx, char value)
   {
      if(ctx->data)
         ctx->data[ctx->length] = value;

      ctx->length += sizeof(value);
   }


   static inline void Serialization_serializeUInt64(SerializeCtx* ctx, uint64_t value)
   {
      if(ctx->data)
      {
         // put_unaligned_le64(value, ctx->data + ctx->length);// 这个是内核程序接口 替换成以下的手动实现
         // 将ctx->data转换为uint8_t*类型，以便按字节处理数据
         uint8_t *p = (uint8_t *)ctx->data + ctx->length;
         
         // 按小端序写入value的每个字节
         p[0] = (value & 0xFF);               // 写入低字节
         p[1] = (value >> 8) & 0xFF;          // 写入次低字节
         p[2] = (value >> 16) & 0xFF;         // 写入次高字节
         p[3] = (value >> 24) & 0xFF;         // 写入高字节
         p[4] = (value >> 32) & 0xFF;         // 写入第四高字节
         p[5] = (value >> 40) & 0xFF;         // 写入第五高字节
         p[6] = (value >> 48) & 0xFF;         // 写入第六高字节
         p[7] = (value >> 56) & 0xFF;         // 写入最高字节
      }
      ctx->length += sizeof(value);
   }

   void Serialization_serializeBlock(SerializeCtx* ctx, const void* value, unsigned length)
   {
      if(ctx->data)
         memcpy(ctx->data + ctx->length, value, length);

      ctx->length += length;
   }

   static inline void Serialization_serializeInt(SerializeCtx* ctx, int value)
   {
      Serialization_serializeUInt(ctx, value);
   }

   void Serialization_serializeStrAlign4(SerializeCtx* ctx, unsigned strLen, const char* strStart)
   {
      // write length field
      Serialization_serializeUInt(ctx, strLen);

      // write raw string
      Serialization_serializeBlock(ctx, strStart, strLen);

      // write termination char
      Serialization_serializeChar(ctx, 0);

      // align to 4b length
      if( (strLen + 1) % 4)
         Serialization_serializeBlock(ctx, "\0\0\0", 4 - (strLen + 1) % 4);
   }








   void NumNodeID_serialize(SerializeCtx* ctx, const NumNodeID* NNI)
   {
      Serialization_serializeUInt(ctx, NNI->value);
   }

/**
 * Serialize into outBuf, 4-byte aligned
 */
void PathInfo_serialize(SerializeCtx* ctx, const PathInfo* pathinfo)
{  
   LOG(DEBUG)<<"go into PathInfo_serialize \n";
   // flags
   Serialization_serializeUInt(ctx, pathinfo->flags);
   LOG(DEBUG)<<"---------- 71 --------------\n";
   // pathinfo->flags=PATHINFO_FEATURE_ORIG;
   LOG(DEBUG)<<"pathinfo->flags = "<<pathinfo->flags<<"\n";//2106179648 这么奇怪？

   if (pathinfo->flags & PATHINFO_FEATURE_ORIG)//pathinfo->flags  被我设置为 PATHINFO_FEATURE_ORIG
   {  
      LOG(DEBUG)<<"---------- 72 --------------\n";

      // origParentUID
      Serialization_serializeUInt(ctx, pathinfo->origParentUID);
   LOG(DEBUG)<<"---------- 73 --------------\n";

      // origParentEntryID
      Serialization_serializeStrAlign4(ctx, strlen(pathinfo->origParentEntryID),
         pathinfo->origParentEntryID);
   }
   LOG(DEBUG)<<"---------- 74 --------------\n";

}






            // 用户空间的字符串复制函数
         char* StringTk_strDup(const char* s) {
            if (s == NULL) {
               return nullptr;
            }
            char* duplicate = (char *)malloc(strlen(s) + 1); // 分配足够的内存
            if (duplicate == NULL) {
               return nullptr; // 内存分配失败
            }
            strcpy(duplicate, s); // 复制字符串
            return duplicate;
         }

         void metaOwnerInit(NodeOrGroup *rootOwner,bool isGounp,uint32_t nodeNumID)
         {  
            // 初始化为节点
            memset(rootOwner, 0, sizeof(NodeOrGroup));
            rootOwner->isGroup=isGounp;//@@
            rootOwner->node.value=nodeNumID;//@@ 162 元数据节点 ID 确实是 1  // 224 server(xfusion3) ID=190
            
         }

         void metaOwnerInit(NodeOrGroup *rootOwner)
         {  
            // 初始化为节点
            memset(rootOwner, 0, sizeof(NodeOrGroup));
            rootOwner->isGroup=false;//@@
            rootOwner->node.value=200;//@@0415 更新 162 元数据节点 ID 确实是 200  // 224 server(xfusion3) ID=190
            
         }
               // bool MetadataTk_getRootEntryInfoCopy(App* app, EntryInfo* outEntryInfo)
         void EntryInfo_init(EntryInfo* outEntryInfo)
      {
               // NodeStoreEx* nodes = app->metaNodes;//App_getMetaNodes(app);
               // NodeOrGroup rootOwner = nodes->_rootOwner;//NodeStoreEx_getRootOwner(nodes);
         //猜测值 直接赋值;
         NodeOrGroup rootOwner;  
         metaOwnerInit(&rootOwner);
         // const char* parentEntryID ="0-674D5ED6-1 ";// StringTk_strDup("");//它是一个简单的字符串复制函数。
         // const char* entryID = "0-674D66B4-1"; //"4-6743E5DB-BE";

         // const char* parentEntryID ="0-674C7122-1";// StringTk_strDup("");//它是一个简单的字符串复制函数。如果是根目录下的文件或是子目录：这里填 root
         // const char* entryID ="1-674C7122-1";//"1-672B3BCE-BE";

         const char* parentEntryID ="0-67C551B0-6";
         const char* entryID ="1-67C551B0-6";


         const char* dirName = StringTk_strDup("test_0303");//test_write.txt //test_xyp.txt
         DirEntryType entryType = (DirEntryType) DirEntryType_REGULARFILE;//DirEntryType_DIRECTORY;

         /* Even if rootOwner is invalid, we still init outEntryInfo and malloc as FhGFS
         * policy says that kfree(NULL) is not allowed (the kernel allows it). */

                  //EntryInfo_init(outEntryInfo, rootOwner, parentEntryID, entryID, dirName, entryType, 0);
         
         outEntryInfo->owner = rootOwner; //存储根节点的元数据节点 app->metaNodes->_rootOwner ; NodeOrGroup 类型 主要是节点ID
         outEntryInfo->parentEntryID = parentEntryID;
         outEntryInfo->entryID = entryID;
         outEntryInfo->fileName = dirName;
         outEntryInfo->entryType = entryType;
         outEntryInfo->featureFlags = 0 | (rootOwner.isGroup ? ENTRYINFO_FEATURE_BUDDYMIRRORED : 0);

         LOG(DEBUG)<<"@@@----outEntryInfo->featureFlags = "<<outEntryInfo->featureFlags<<"\n";

               // return NodeOrGroup_valid(rootOwner);
         return ;
      }
      void EntryInfo_init(EntryInfo* outEntryInfo,char* parentEntryID,char* entryID,char* fileName,DirEntryType entryType,bool isGroup,uint32_t NodeNumID)
      {
               
         //猜测值 直接赋值;
         
         // const char* parentEntryID =StringTk_strDup(parentEntryID);//"root";//@@ StringTk_strDup("");//它是一个简单的字符串复制函数。
         // const char* entryID =StringTk_strDup(entryID);//@@  
         // 162 server :  0-6732140E-1
            // image_data.csv:0-67317DE7-BE
            // test_xyp.txt : 1-6734020D-1
            //dir :  test_xyp :0-673494EA-1 
            //dir : test_dir : 1-6736A360-1
            //测试非根目录： test_dir/dir1 : 2-6736A360-1
            //测试更深路径的非根目录： test_dir/dir1/dir11/dir111 : 2-6736A39F-1 父目录:0-6736A39F-1
            // small_image : 0-675A5708-1 (目录ID) 其父目录ID
         //以下是 224 xfusion
            //image_data.csv:0-67317DE7-BE
            //test_xyp.txt : 0-67282B84-BE(vim 修改文件内容后 1-672B3BCE-BE)  //StringTk_strDup(META_ROOTDIR_ID_STR);//"4-67151359-1";//"1-66BB827F-1";//"0-671513D6-1";//"0-66DEDB51-1";//StringTk_strDup(META_ROOTDIR_ID_STR);
            //test_wsy.txt : 3-672833A3-BE
         // const char* dirName = StringTk_strDup(dirName);//@@ 目录或是文件名字 test_xyp.txt image_data.csv
         // DirEntryType entryType = (DirEntryType) DirEntryType_DIRECTORY;//@@  我发现这个参数，之前是open文件的时候也是这个类型，感觉是设置错误了，应该是文件类型

         

                 
         NodeOrGroup rootOwner;  
         metaOwnerInit(&rootOwner,isGroup,NodeNumID);//@@
         outEntryInfo->owner = rootOwner; 
         outEntryInfo->parentEntryID = StringTk_strDup(parentEntryID);
         outEntryInfo->entryID = StringTk_strDup(entryID);
         outEntryInfo->fileName = StringTk_strDup(fileName);
         outEntryInfo->entryType = entryType;
         // LOG(DEBUG)<<"rootOwner.isGroup  = "<<rootOwner.isGroup <<"\n";
         outEntryInfo->featureFlags = 0 | (rootOwner.isGroup ? ENTRYINFO_FEATURE_BUDDYMIRRORED : 0);
         // LOG(DEBUG)<<"outEntryInfo->featureFlags = "<<outEntryInfo->featureFlags<<"\n";
     
         return ;
      }
               int OsTypeConv_openFlagsOsToFhgfs(int osFlags, bool isPagedMode)
            {
               int fhgfsFlags = 0; // 初始化为0

               if(osFlags & O_RDWR)
                  fhgfsFlags |= OPENFILE_ACCESS_READWRITE;
               else
               if(osFlags & O_WRONLY)
               {
                  if (!isPagedMode)
                     fhgfsFlags |= OPENFILE_ACCESS_WRITE;
                  else
                  {  /* in order to update read-modify-write pages with the storage content we a
                     * read-write handle */
                     fhgfsFlags |= OPENFILE_ACCESS_READWRITE;
                  }
               }
               else
                  fhgfsFlags |= OPENFILE_ACCESS_READ;


               if(osFlags & O_APPEND)
                  fhgfsFlags |= OPENFILE_ACCESS_APPEND;

               if(osFlags & O_TRUNC)
                  fhgfsFlags |= OPENFILE_ACCESS_TRUNC;

               if(osFlags & O_DIRECT)
                  fhgfsFlags |= OPENFILE_ACCESS_DIRECT;

               if(osFlags & O_SYNC)
                  fhgfsFlags |= OPENFILE_ACCESS_SYNC;


               return fhgfsFlags;
            }

         unsigned accessFlagsInit(){
         
         /*
         需要 #include <fcntl.h>
         O_RDWR // 以读写方式打开文件
         O_WRONLY // 以只写方式打开文件
         O_APPEND // 如果设置，写操作会追加到文件末尾
         O_TRUNC // 如果文件已存在，将其大小截断为零
         O_DIRECT // 尝试直接将 I/O 操作绕过文件系统缓存
         O_SYNC // 写操作将根据文件完整性完成的要求进行
         // 前五种是OsTypeConv_openFlagsOsToFhgfs 这个函数考虑的一些分支
         O_RDONLY // 以只读方式打开文件
         O_CREAT // 如果文件不存在，则创建新文件
         O_EXCL // 与 O_CREAT 一起使用，确保文件不存在时才创建
         O_NOCTTY // 如果文件是终端设备，不将其设置为控制终端
         O_NONBLOCK // 以非阻塞模式打开文件
         O_NDELAY // 与 O_NONBLOCK 相同，用于兼容旧系统
         O_DSYNC // 写操作将根据数据完整性完成的要求进行
         O_ASYNC // 启用信号驱动的 I/O
         O_LARGEFILE // 允许文件大小超过 2GB
         O_NOFOLLOW // 不跟随符号链接
         O_CLOEXEC // 设置新文件描述符的 close-on-exec 标志
         */

            int openFlags =O_RDWR ;//file->f_flags;

            FileCacheType type=FILECACHETYPE_None ; // 随便初始化一下 有四种值
            bool isPagedMode;
            if(type==FILECACHETYPE_Paged || type==FILECACHETYPE_Native)
               isPagedMode=true;
            else
               isPagedMode=false;
            
            int fhgfsOpenFlags= OsTypeConv_openFlagsOsToFhgfs(openFlags, isPagedMode );
            unsigned accessFlags=fhgfsOpenFlags;
            return accessFlags;
         }
      // 初始化requestMsg 的第四个参数 FileEvent

      // 获取dentry相对于挂载点的相对路径
            // char *dentry_path_raw(struct dentry *dentry, char *buf, int buflen)
      char *get_relative_path(const char *absolute_path, char *buf, int buflen) {
         char *relative_path = nullptr;
         char *temp_path = nullptr;

         // 使用strdup来复制绝对路径，因为realpath可能修改输入字符串
         temp_path = strdup(absolute_path);
         if (!temp_path) {
            LOG_printf(DEBUG,"------------error1-----\n");
            return nullptr;
         }

         // 使用realpath获取绝对路径
         relative_path = realpath(temp_path, buf);
         if (!relative_path) {
            LOG_printf(DEBUG,"------------error2-----\n");
            LOG_printf(DEBUG,"realpath failed with error: %s\n", strerror(errno));

            free(temp_path);
            return nullptr;
         }

         // 计算相对路径
         char *pos = strrchr(relative_path, '/');
         if (pos) {
            *pos = '\0'; // 截断路径，只保留目录部分
            relative_path = pos + 1; // 指向文件名
         }

         // 将相对路径复制到buf中
         if (relative_path) {
            strncpy(buf, relative_path, buflen);
            buf[buflen - 1] = '\0'; // 确保字符串终止
         }

         free(temp_path);
            LOG_printf(DEBUG,"------------error3-----\n");

         LOG(DEBUG)<<"get_relative_path ="<<buf<<"\n";
         return buf;
      }
      //void FileEvent_init(struct FileEvent* event, enum FileEventType eventType, struct dentry* dentry) 
      void FileEvent_init(FileEvent *event, FileEventType eventType, const char *absolute_path) { //和原来的函数表达不一样,因为dentry结构 用户态程序不让用，如果一定要用这个数据结构，我能想象到的 自己定义一个差不多的，重写一些逻辑
         
         memset(event, 0, sizeof(*event));

         event->eventType = eventType;

         if (!absolute_path) {

            return;
         }
         

          // 分配内存来存储路径
            event->path = (char *)malloc(4096); // 用户空间使用malloc
            if (!event->path) {
               return;
            }
            strcpy((char *)event->path,absolute_path);

         if (strlen(event->path) == 0) {
            free(event->path);
            event->path = nullptr;
            LOG_printf(DEBUG,"[FileEvent_init]: strlen(event->path) == 0 \n");
         }

      }

   void OpenFileMsg_initFromSession(OpenFileMsg* openmsg,
   NumNodeID clientNumID, const EntryInfo* entryInfo, unsigned accessFlags,
      const struct FileEvent* fileEvent)
   {
            //OpenFileMsg_init(openmsg);
      NetMessage nmsg=openmsg->netMessage;
      memset(&(openmsg->netMessage), 0, sizeof(openmsg->netMessage) ); // clear function pointers etc.  一个bug 原来在这里，已解决
      //  memset(&nmsg, 0, sizeof(nmsg) );//XYP_0304
            // nmsg.msgHeader.msgType =  NETMSGTYPE_OpenFile;
            // nmsg.msgHeader.msgUserID = geteuid();  //FhgfsCommon_getCurrentUserID();
            // nmsg.ops = &OpenFileMsg_Ops;

       openmsg->netMessage.msgHeader.msgLength=0;
       openmsg->netMessage.msgHeader.msgType =  NETMSGTYPE_OpenFile;
      //  LOG(DEBUG)<<"@@@ "
      openmsg->netMessage.msgHeader.msgUserID = geteuid();  //FhgfsCommon_getCurrentUserID();
      // openmsg->netMessage.msgHeader.msgUserID = 0;  //FhgfsCommon_getCurrentUserID();

      LOG(DEBUG)<<"openmsg->netMessage.msgHeader.msgUserID  = "<<openmsg->netMessage.msgHeader.msgUserID <<"\n";
      openmsg->netMessage.ops = &OpenFileMsg_Ops;
   //    LOG << "--------nmsg.ops = " << &OpenFileMsg_Ops << ".\t..serialize ...=" << (void*)&OpenFileMsg_Ops.serializePayload << "\n";
   //  LOG(DEBUG)<<"--------nmsg.ops = "<<openmsg->netMessage.ops<<".\t..serialize ...="<<(void*)openmsg->netMessage.ops->serializePayload<<"\n";
      openmsg->clientNumID = clientNumID;

      openmsg->entryInfoPtr = entryInfo;

      openmsg->accessFlags = accessFlags;
      openmsg->fileEvent = fileEvent;

      if (fileEvent)
         openmsg->netMessage.msgHeader.msgFeatureFlags |= OPENFILEMSG_FLAG_HAS_EVENT;
   }

/**
 * Serialize into outBuf, 4-byte aligned
 */
 // 跟序列化有关的逻辑先注释

   void EntryInfo_serialize(SerializeCtx* ctx, const EntryInfo* eInfo)
   {
      // DirEntryType
      // LOG(DEBUG)<<"[EntryInfo_serialize]ctx->length = "<<ctx->length<<"\n";//

      Serialization_serializeUInt(ctx, eInfo->entryType); //48+

      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//

      // featureFlags
      Serialization_serializeInt(ctx, eInfo->featureFlags);//4

      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//

      // parentEntryID
      Serialization_serializeStrAlign4(ctx, strlen(eInfo->parentEntryID), eInfo->parentEntryID);

      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//

      if (strlen(eInfo->entryID) < MIN_ENTRY_ID_LEN) // 这个代码替换下面的包含内核函数的代码
      {
         LOG_printf(DEBUG,"Warning: EntryID is NULL or too small!\n");
         return ;
      }
            // if (unlikely(strlen(eInfo->entryID) < MIN_ENTRY_ID_LEN) )
            // {
            //    printk_fhgfs(KERN_WARNING, "EntryID too small!\n"); // server side deserialization will fail
            //    dump_stack();
            // }

      // entryID
      Serialization_serializeStrAlign4(ctx, strlen(eInfo->entryID), eInfo->entryID);
      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//

      // fileName
      Serialization_serializeStrAlign4(ctx, strlen(eInfo->fileName), eInfo->fileName);
      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//

               // ownerNodeID
               // also serializes owner.group, if buddymirrored. both MUST have the same size and underlying
               // type!
               // BUILD_BUG_ON(
               //       sizeof(eInfo->owner.node) != sizeof(eInfo->owner.group)
               //          || !__builtin_types_compatible_p(
               //             __typeof(eInfo->owner.node.value), __typeof(eInfo->owner.group)));
      // 检查大小 替换上面的检查代码
      if (sizeof(eInfo->owner.node) != sizeof(eInfo->owner.group)) {
         LOG_printf(DEBUG,"Error: owner.node and owner.group must have the same size\n");
         return ;
      }

      // 检查类型 替换上面的检查代码
      // if (typeof(eInfo->owner.node.value) != typeof(eInfo->owner.group)) {
      if(!(std::is_same<decltype(eInfo->owner.node.value), decltype(eInfo->owner.group)>::value)){
         LOG_printf(DEBUG,"Error: owner.node.value and owner.group must have the same type\n");
         return ;
      }

      NumNodeID_serialize(ctx, &eInfo->owner.node);
      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//

      // padding for 4-byte alignment
      Serialization_serializeUShort(ctx, 0);
      // LOG(DEBUG)<<"[EntryInfo_serialize]ctx->length = "<<ctx->length<<"\n";//

   }

   #define OPENFILEMSG_FLAG_HAS_EVENT          2 // contains file event logging information 

   void Serialization_serializeStr(SerializeCtx* ctx, unsigned strLen, const char* strStart)
   {
      // write length field
      Serialization_serializeUInt(ctx, strLen);

      // write raw string
      Serialization_serializeBlock(ctx, strStart, strLen);

      // write termination char
      Serialization_serializeChar(ctx, 0);
   }

   static inline void Serialization_serializeBool(SerializeCtx* ctx, bool value)
   {
      Serialization_serializeChar(ctx, value ? 1 : 0);
   }


   void FileEvent_serialize(SerializeCtx* ctx, const struct FileEvent* event)
   {
      // LOG(DEBUG)<<"[FileEvent_serialize]ctx->length = "<<ctx->length<<"\n";//

      Serialization_serializeUInt(ctx, event->eventType);
      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//


      if (event->path)
         Serialization_serializeStr(ctx, strlen(event->path), event->path);
      else
         Serialization_serializeStr(ctx, 0, "");

      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//

      Serialization_serializeBool(ctx, event->target != NULL);
      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//

      if (event->target != NULL)
         Serialization_serializeStr(ctx, strlen(event->target), event->target);
      LOG(DEBUG)<<"[FileEvent_serialize]ctx->length = "<<ctx->length<<"\n";//
      
   }

   void OpenFileMsg_serializePayload(NetMessage* netM, SerializeCtx* ctx)
   {  
      LOG(DEBUG)<<"go into OpenFileMsg_serializePayload \n";
      OpenFileMsg* thisCast = (OpenFileMsg*)netM;

      // clientNumID
      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//40

      NumNodeID_serialize(ctx, &thisCast->clientNumID);
      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//40+4=44

      // accessFlags
      Serialization_serializeUInt(ctx, thisCast->accessFlags);
      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//44+4=48

      // entryInfo
      EntryInfo_serialize(ctx, thisCast->entryInfoPtr);
      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//48+42=90


      if (netM->msgHeader.msgFeatureFlags & OPENFILEMSG_FLAG_HAS_EVENT)
         {
            FileEvent_serialize(ctx, thisCast->fileEvent);
      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//

         }
      

   }

  unsigned NetMessage_getMsgLength(NetMessage* netM);
  void __NetMessage_serializeHeader(NetMessage* netM, SerializeCtx* ctx, bool zeroLengthField)
   {  
      // if(ctx->data==NULL)
      // {
      //    LOG_printf(DEBUG,"[__NetMessage_serializeHeader] ctx->data==NULL \n");
      // }
      // else
      //    LOG_printf(DEBUG,"[__NetMessage_serializeHeader] ctx->data!=NULL \n");

      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n"; //0 

      // message length
      Serialization_serializeUInt(ctx, zeroLengthField ? 0 : NetMessage_getMsgLength(netM) );
      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//4

      // if(ctx->data==NULL)
      // {
      //    LOG_printf(DEBUG,"[__NetMessage_serializeHeader] ctx->data==NULL \n");
      // }
      // else
      //    LOG_printf(DEBUG,"[__NetMessage_serializeHeader] ctx->data!=NULL \n");

      // if(strlen(ctx->data))
      // {
      //    LOG_printf(DEBUG,"----1---[__NetMessage_serializeHeader] %ld size %s \n",strlen(ctx->data),ctx->data);
      // }
      // else
      // {
      //    LOG_printf(DEBUG,"----1---[__NetMessage_serializeHeader] error \n");
      // }
      // feature flags
      Serialization_serializeUShort(ctx, netM->msgHeader.msgFeatureFlags);
      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//4+2=6

      Serialization_serializeChar(ctx, netM->msgHeader.msgCompatFeatureFlags);
      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//6+1=7

      Serialization_serializeChar(ctx, netM->msgHeader.msgFlags);
      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//7+1=8


      // message prefix
      Serialization_serializeUInt64(ctx, NETMSG_PREFIX);
      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//8+8=16


      // message type
      Serialization_serializeUShort(ctx, netM->msgHeader.msgType);
      LOG(DEBUG)<<"netM->msgHeader.msgType = "<<netM->msgHeader.msgType<<"\n";
      LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//16+2=18


      // targetID
      Serialization_serializeUShort(ctx, netM->msgHeader.msgTargetID);
      LOG(DEBUG)<<"netM->msgHeader.msgTargetID = "<<netM->msgHeader.msgTargetID<<"\n";

      LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//18+2=20


      // userID
      Serialization_serializeUInt(ctx, netM->msgHeader.msgUserID);
      LOG(DEBUG)<<"netM->msgHeader.msgUserID) = "<<netM->msgHeader.msgUserID<<"\n";
      LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//20+4=24


      Serialization_serializeUInt64(ctx, netM->msgHeader.msgSequence);
      LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//24+8=32


      Serialization_serializeUInt64(ctx, netM->msgHeader.msgSequenceDone);
      // LOG(DEBUG)<<"ctx->length = "<<ctx->length<<"\n";//32+8=40

      //  if(strlen(ctx->data))
      // {
      //    LOG_printf(DEBUG,"----end---[__NetMessage_serializeHeader] %d size %s \n",strlen(ctx->data),ctx->data);
      // }
      // else
      // {
      //    LOG_printf(DEBUG,"----end---[__NetMessage_serializeHeader] error \n");
      // }
   }





   unsigned NetMessage_getMsgLength(NetMessage* netM)
   {
      if(!netM->msgHeader.msgLength)//消息头信息的长度为0
      {
         SerializeCtx ctx = { nullptr, 0 };

         __NetMessage_serializeHeader(netM, &ctx, true); //序列化信息头部 
         // LOG(DEBUG)<<"[NetMessage_getMsgLength] ctx->length = "<<ctx.length<<"\n";
         if(netM->ops->serializePayload==NULL)
         {
               LOG(DEBUG)<<"---error 1-------\n";
         }
         LOG(DEBUG)<<"netM->ops->serializePayload = "<<(void *)netM->ops->serializePayload<<"\n";
         netM->ops->serializePayload(netM, &ctx);
         LOG(DEBUG)<<"---------nmsh 1-------------\n";

         netM->msgHeader.msgLength = ctx.length;
         LOG(DEBUG)<<"[NetMessage_getMsgLength] netM->msgHeader.msgLength ="<<netM->msgHeader.msgLength<<"\n"; 
      }

      return netM->msgHeader.msgLength;
   }

 
   //序列化一个网络消息。序列化是将数据结构或对象状态转换为可以存储或传输的格式的过程。
   bool NetMessage_serialize(NetMessage* netM, char* buf, size_t bufLen)
   {  
      SerializeCtx ctx = {
         .data = buf,
         .length = 0,//xyp_insert_250301 XYP_0303
      };
      if(buf==NULL)
      {

            LOG_printf(DEBUG,"[NetMessage_serialize]  buf ==null \n");
      }
      else 
      {
         LOG_printf(DEBUG,"[NetMessage_serialize] buf !=null \n");
      }
      if(ctx.data==NULL)
      {
            LOG_printf(DEBUG,"[NetMessage_serialize]  ctx.data==NULL \n");

      }
      else
         LOG_printf(DEBUG,"[NetMessage_serialize] ctx.data !=null \n");

      //检查提供的缓冲区是否足够大，以容纳序列化后的消息。如果缓冲区太小，函数返回 false。
      LOG(DEBUG)<<"[NetMessage_serialize ]bufLen = "<<bufLen<<"\n";
      if(bufLen < NetMessage_getMsgLength(netM) )
         {
            LOG(DEBUG)<<"[NetMessage_serialize] bufLen < NetMessage_getMsgLength(netM) \n";
            return false;
         }
      //调用 __NetMessage_serializeHeader 函数序列化消息头。消息头通常包含了消息的一些元数据，如类型、长度等。
      __NetMessage_serializeHeader(netM, &ctx, false);

      LOG_printf(DEBUG,"[RDMA_Read_request] ctx.length = %d\n",ctx.length);
      //序列化消息的有效负载（payload）。ops 结构体通常包含了一组操作函数，这些函数用于处理特定类型消息的序列化和反序列化。
      // std:cout<<"netM->ops = "<<netM->ops<<"\t ,serialize..="<<(void *)netM->ops->serializePayload<<"\n";


      netM->ops->serializePayload(netM, &ctx);//这个函数好像有多种实现

      LOG_printf(DEBUG,"[RDMA_Read_request] ctx.length = %d\n",ctx.length);

      return true;
   }


//-----------------------------------2. 配置响应缓冲区 outrespbuffer-----------------------------------------------------------------------------





      // void Condition_wait(Condition* this, Mutex* mutex)
      void Condition_wait(Condition* var_cond)
      {
         /*wait_queue_t wait;

         init_waitqueue_entry(&wait, current);
         add_wait_queue(&var_cond->queue, &wait); 

         set_current_state(TASK_UNINTERRUPTIBLE);

         Mutex_unlock(mutex);

         schedule();

         Mutex_lock(mutex);

         remove_wait_queue(&var_cond->queue, &wait);

         __set_current_state(TASK_RUNNING);
         */
         pthread_cond_wait(&var_cond->cond, &var_cond->mutex);
      }
      // 初始化条件变量
      void Condition_init(Condition* var_cond) {
         pthread_cond_init(&var_cond->cond, nullptr);
         pthread_mutex_init(&var_cond->mutex, nullptr);
      }

      // 销毁条件变量
      void Condition_destroy(Condition* var_cond) {
         pthread_cond_destroy(&var_cond->cond);
         pthread_mutex_destroy(&var_cond->mutex);
      }

      // 通知条件变量（单线程）
      void Condition_signal(Condition* var_cond) {
         pthread_cond_signal(&var_cond->cond);
      }

      // 通知所有等待条件变量的线程
      void Condition_broadcast(Condition* var_cond) {
         pthread_cond_broadcast(&var_cond->cond);
      }


      char* NoAllocBufferStore_waitForBuf(NoAllocBufferStore* bstore)
      {
         /*
         char* buf;

         // Mutex_lock(&bstore->mutex);
         pthread_mutex_lock(&bstore->mutex);
         // $$这个函数只在调试构建中使用，用于帮助诊断和预防潜在的死锁问题。，我直接先注释它
         // __NoAllocBufferStore_debugCheckTask(bstore);

         while(!bstore->numAvailable)
            Condition_wait(&bstore->newBufCond, &bstore->mutex);

         buf = (bstore->bufArray)[bstore->numAvailable-1];
         (bstore->numAvailable)--;
         //$$目的是在调试构建中将当前线程的缓冲区分配器信息添加到一个名为 pidDebugTree 的红黑树（RBTree）中，同时包括调用栈跟踪。这样做可以在后续的调试中帮助识别是否有来自同一线程的两次（禁止的）调用，从而预防潜在的死锁或资源冲突问题。
         // __NoAllocBufferStore_debugAddTask(bstore);

         Mutex_unlock(&bstore->mutex);
         */

         //换成下面的这个表达
         char* buf = nullptr;
         pthread_mutex_lock(&bstore->newBufCond.mutex);

         while (bstore->numAvailable == 0) {
            Condition_wait(&bstore->newBufCond);
         }

         buf = bstore->bufArray[bstore->numAvailable - 1];
         bstore->numAvailable--;
         pthread_mutex_unlock(&bstore->newBufCond.mutex);

         return buf;
      }


/*
   // 我认为这个调用 内核函数iov_iter_kvec 函数 就是把 kev的参数信息初始化给iter结构
   static inline void BEEGFS_IOV_ITER_KVEC(struct iov_iter *iter, int direction,
   const struct kvec* kvec, unsigned long nr_segs, size_t count)
   {
   // #ifndef KERNEL_HAS_IOV_ITER_KVEC_NO_TYPE_FLAG_IN_DIRECTION
      direction |= ITER_KVEC;
   // #endif
      iov_iter_kvec(iter, direction, kvec, nr_segs, count);
   }
   //这个内联函数用于初始化 kvec 结构体，它包含了指向数据缓冲区的指针和数据长度。
   static inline struct iov_iter *___BEEGFS_IOV_ITER_KVEC(
         struct iov_iter *iter, struct kvec* kvec,
         const char *ptr, size_t size, int direction)
   {
      unsigned nr_segs = 1;//nr_segs 表示缓冲区段的数量，这里设置为 1，表示只有一个缓冲区段。
      *kvec = (struct kvec) {
         .iov_base = (char *) ptr,
         .iov_len = size,
      };
      //来进一步初始化 iov_iter 结构体。
      BEEGFS_IOV_ITER_KVEC(iter, direction, kvec, nr_segs, size);
      return iter;
   }
   // 定义一个宏，用于在栈上分配并初始化 iov_iter 和 kvec 结构体
   #define STACK_ALLOC_BEEGFS_ITER_KVEC(ptr, size, direction) \
   ___BEEGFS_IOV_ITER_KVEC(&(struct iov_iter){0}, &(struct kvec){0}, (ptr), (size), (direction))
*/

 // 上面的内核表达式改成如下表达
// 用于初始化 iovec 结构体的内联函数
static inline struct iovec *__beegfs_iov_iter_kvec(struct iovec *kvec,
    const char *ptr, size_t size) {
    // 初始化 kvec 结构体
    kvec= new iovec;
   if (kvec == NULL) {
        // 如果 kvec 是 NULL，打印错误信息并返回 NULL
   
        fprintf(stderr, "Error: kvec cannot be NULL\n");
        return nullptr;
    }
    
    *kvec = (struct iovec) {
        .iov_base = (void *)ptr,  // 用户空间中，我们使用(void *)来转换指针类型
        .iov_len = size,
    };
                  //  if(kvec->iov_base==NULL)
                  //  {

                  //    LOG_printf(DEBUG,"[__beegfs_iov_iter_kvec] error kvec->iov_base==NULL \n");
                  //  }
                  //  if(kvec->iov_len<=0)
                  //  {
                  //    LOG_printf(DEBUG,"[__beegfs_iov_iter_kvec] error kvec->iov_len<=0 \n");

                  //  }
                  //  else
                  //    LOG_printf(DEBUG,"[__beegfs_iov_iter_kvec] kvec->iov_len = %ld \n",kvec->iov_len);

    return kvec;
}

// 使用宏来简化 iov_iovec 的初始化过程 和 iov_iter 相比 少了一个direction 
#define STACK_ALLOC_BEEGFS_ITER_KVEC(ptr, size) \
    __beegfs_iov_iter_kvec((struct iovec *){0}, (ptr), (size))






//7. receive response---------------------------------------------------------------------
void user_iov_iter_advance(struct iovec *iov, int iovcnt, ssize_t nread) {
                // 遍历iovec数组，更新iov_base和iov_len
                for (int i = 0; i < iovcnt && nread > 0; ++i) {
                    // 计算当前iovec可以处理的最大字节数
                    size_t to_read = nread;
                    if (to_read > iov[i].iov_len) {
                        to_read = iov[i].iov_len;
                    }

                    // 更新nread
                    nread -= to_read;

                    // 更新iov_base和iov_len
                    iov[i].iov_len -= to_read;
                    iov[i].iov_base = (char *)iov[i].iov_base + to_read;

                    // 如果当前iovec已经完全处理完毕，可以跳过剩余的iovec
                    if (iov[i].iov_len == 0) {
                        if (i < iovcnt - 1) {
                            memmove(&iov[i], &iov[i + 1], (iovcnt - i - 1) * sizeof(struct iovec));
                        }
                        iovcnt--;
                        i--;
                    }
                }
            }
            // ssize_t StandardSocket_recvfrom(StandardSocket* ssock, struct iov_iter* iter, int flags,fhgfs_sockaddr_in *from)
ssize_t StandardSocket_recvfrom(StandardSocket* ssock, struct iovec* iov, int flags,fhgfs_sockaddr_in *from)
{
      LOG(DEBUG)<<"go into   StandardSocket_recvfrom \n";
      if(!ssock)
      {
         LOG_printf(DEBUG,"[StandardSocket_recvfrom] error : ssock==null \n");
         return -1; // 检查空指针
      }
      int sockfd=ssock->sock;

      struct sockaddr_in fromSockAddr;
      socklen_t fromAddrLen = sizeof(fromSockAddr);
      ssize_t recvRes = recvfrom(sockfd, iov->iov_base, iov->iov_len, flags, (struct sockaddr*)&fromSockAddr, &fromAddrLen);


      if (recvRes < 0) {
        // 处理错误
        std::cerr << "[StandardSocket_recvfrom] recvfrom failed: " << strerror(errno) << "\n";
        return -1; // 返回错误
      }else if (recvRes == 0) {
        // 对端关闭了连接
        std::cerr << "[StandardSocket_recvfrom] Connection closed by peer" << std::endl;
        return 0; // 返回 0 表示连接已关闭
    }

      if (recvRes > 0 && from) { // 将网络字节序转换为主机字节序
         from->addr = fromSockAddr.sin_addr;
         from->port = ntohs(fromSockAddr.sin_port);
      }
     
      return recvRes;

                /*
                int recvRes;
                size_t len;
                struct sockaddr_in fromSockAddr;
                struct socket *sock = ssock->sock;

                struct msghdr msg =
                {
                    .msg_control      = NULL,
                    .msg_controllen   = 0,
                    .msg_flags        = flags,
                    .msg_name         = (struct sockaddr*)&fromSockAddr,
                    .msg_namelen      = sizeof(fromSockAddr),
                    .msg_iter         = *iter,
                };

                len = iov_iter_count(iter);

                recvRes = beegfs_recvmsg(sock, &msg, len, flags);

                if(recvRes > 0)
                    iov_iter_advance(iter, recvRes);

                if(from)
                {
                    from->addr = fromSockAddr.sin_addr;
                    from->port = fromSockAddr.sin_port;
                }

                return recvRes;
                */
}
/**
 * @return -ETIMEDOUT on timeout
 */
        // ssize_t StandardSocket_recvfromT(StandardSocket* thissock, struct iov_iter* iter, int flags,fhgfs_sockaddr_in *from, int timeoutMS)
ssize_t StandardSocket_recvfromT(StandardSocket* thissock, struct iovec* iov, int flags,fhgfs_sockaddr_in *from, int timeoutMS)
{

   LOG(DEBUG)<<"go into StandardSocket_recvfromT \n";
    struct pollfd pollfd;
    pollfd.fd = thissock->sock;
    pollfd.events = POLLIN;
    

   // 设置 socket 超时时间
    struct timeval tv;
    tv.tv_sec = timeoutMS / 1000;  // 秒
    tv.tv_usec = (timeoutMS % 1000) * 1000;  // 微秒
    if (setsockopt(thissock->sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt failed");
        return -1;
    }

    int pollRes = ::poll(&pollfd, 1, timeoutMS);            //pollRes 是 poll 系统调用的返回值，它表示有多少个文件描述符（在本例中是 socket）已经准备好了 I/O 操作。
    if (pollRes > 0 && (pollfd.revents & POLLIN)){          //检查 pollfd.revents 是否包含 POLLIN 事件，这表示 socket 上有数据可读。

         LOG_printf(DEBUG,"[StandardSocket_recvfromT]  pollRes > 0 && (pollfd.revents & POLLIN) \n");
        return StandardSocket_recvfrom(thissock,iov, flags, from);   //如果这两个条件都满足，说明 socket 已经准备好接收数据，函数将返回 recvfrom 函数的调用结果，该结果可能是接收到的数据字节数，或者是错误码（如果是负数）。
    } 
    else if (pollRes == 0) {                                //这意味着 poll 系统调用超时了，因为在指定的超时时间内没有文件描述符准备好 I/O 操作。
        errno = ETIMEDOUT;
        LOG_printf(DEBUG,"[StandardSocket_recvfromT] error: pollRes == 0 超时错误\n");
        return -1;
    } 
    else {
        // Error handling
                // std::cerr << "recvfromT: poll() failed with error: " << strerror(errno) << std::endl;
        LOG_printf(DEBUG,"[StandardSocket_recvfromT] error: poll() failed with error: %s\n", strerror(errno));
    
        return -1;
    }
                        /*
                        Socket* thisBase = (Socket*)thissock;

                        int pollRes;
                        PollState state;

                        if(timeoutMS < 0)
                            return StandardSocket_recvfrom(thissock, iter, flags, from);//

                        PollState_init(&state);
                        PollState_addSocket(&state, thisBase, POLLIN);

                        pollRes = SocketTk_poll(&state, timeoutMS);

                        if( (pollRes > 0) && (thisBase->poll.revents & POLLIN) )
                            return StandardSocket_recvfrom(thissock, iter, flags, from);// 这两个用到iter参数

                        if(!pollRes)
                            return -ETIMEDOUT;

                        if(thisBase->poll.revents & POLLERR)
                            printk_fhgfs_debug(KERN_DEBUG, "StandardSocket_recvfromT: poll(): %s: Error condition\n",
                                thisBase->peername);
                        else
                        if(thisBase->poll.revents & POLLHUP)
                            printk_fhgfs_debug(KERN_DEBUG, "StandardSocket_recvfromT: poll(): %s: Hung up\n",
                                thisBase->peername);
                        else
                        if(thisBase->poll.revents & POLLNVAL)
                            printk_fhgfs(KERN_DEBUG, "StandardSocket_recvfromT: poll(): %s: Invalid request\n",
                                thisBase->peername);
                        else
                            printk_fhgfs(KERN_DEBUG, "StandardSocket_recvfromT: poll(): %s: ErrCode: %d\n",
                                thisBase->peername, pollRes);

                        return -ECOMM;
                        */
}

/**
 * @return -ETIMEDOUT on timeout
 */
            //    ssize_t _StandardSocket_recvT(Socket* this, struct iov_iter* iter, int flags, int timeoutMS)
ssize_t _StandardSocket_recvT(Socket* thissock, struct iovec* iter, int flags, int timeoutMS)

{  
   LOG(DEBUG)<<"go into _StandardSocket_recvT \n";
   StandardSocket* thisCast = (StandardSocket*)thissock;

   return StandardSocket_recvfromT(thisCast, iter, flags, nullptr, timeoutMS);
}



        // static inline ssize_t Socket_recvExactTEx(Socket* this, struct iov_iter *iter, size_t len, int flags, int timeoutMS, size_t* outNumReceivedBeforeError)
static inline ssize_t Socket_recvExactTEx(Socket* thissock, struct iovec *iter, size_t len, int flags, int timeoutMS, size_t* outNumReceivedBeforeError)
{
   ssize_t missingLen = len;
   LOG(DEBUG)<<"missingLen = "<<missingLen<<"\n";
   ssize_t total_recv=0;
   int cnt=0;
   do
   {
      cnt++;
      LOG(DEBUG)<<"[Socket_recvExactTEx] cnt = "<<cnt<<"\n";
      LOG(DEBUG)<<"iter->len = "<<iter->iov_len<<"\n";

      ssize_t recvRes = thissock->ops->recvT(thissock, iter, flags, timeoutMS);//$$ 这个函数有两个实现 标准的TCP 套接字和RDMA套接字 
      if(recvRes)
      iter->iov_base+=recvRes;
      // LOG(DEBUG)<<"---------------输出buffer -------------\n";
      LOG(DEBUG)<<"recvRes ="<<recvRes<<"\n";
      if(recvRes)
      total_recv+=recvRes;
      for(int i=0;i<rdma_recv_len;i++)
      {
         std::cout<<rdma_recv_buffer[i]<<" ";
      }
      LOG(DEBUG)<<"-------";



      // LOG(DEBUG)<<"\n---------------输出buffer end-------------\n";

      // LOG(DEBUG)<<"[Socket_recvExactTEx] recvRes = "<<recvRes<<"\n";
      // for(int i=0;i<iter->iov_len;i++)
      // {
      //    LOG(DEBUG)<<static_cast<char*>(iter->iov_base)[i];
      // }
      // LOG(DEBUG)<<"end \n";
      
      if(recvRes <= 0) 
         return recvRes;

      missingLen -= recvRes;
      *outNumReceivedBeforeError += recvRes;
      LOG(DEBUG)<<"missinglen ="<<missingLen<<"\n";

   } while(missingLen);

   // all received if we got here
   LOG(DEBUG)<<"all received if we got here \n";
   LOG(DEBUG)<<"total_revcv =" <<total_recv<<"\n";
   return len;
}
static inline ssize_t Socket_recvExactTEx_kernel(Socket* thissock, char *buf, size_t len, int flags, int timeoutMS,
   size_t* outNumReceivedBeforeError)
{
                //   struct iov_iter *iter = STACK_ALLOC_BEEGFS_ITER_KVEC(buf, len, READ);
      struct iovec *iter = STACK_ALLOC_BEEGFS_ITER_KVEC(buf, len);

     
      return Socket_recvExactTEx(thissock, iter, len, flags, timeoutMS, outNumReceivedBeforeError);
}


inline uint32_t get_unaligned_le32(const char* ptr) {
    uint32_t value;
    memcpy(&value, ptr, sizeof(value));
    return value;
}


static inline bool Serialization_deserializeUInt(DeserializeCtx* ctx, unsigned* outValue)
{
                //return __Serialization_deserializeRaw(ctx, get_unaligned_le32, outValue);
    if (ctx->length < sizeof(*outValue)) {
        return false;
    }
   //    for(int i=0;i<4;i++)
   //  LOG_printf(DEBUG,"ctx->data[%d] %02x \n",i,ctx->data[i]);// 47 00 6e 30
    *outValue = get_unaligned_le32(ctx->data);
    ctx->data += sizeof(*outValue);
    ctx->length -= sizeof(*outValue);
    return true;


}

    // recvBuf must be at least NETMSG_MIN_LENGTH long
unsigned NetMessage_extractMsgLengthFromBuf(const char* recvBuf)
{
   unsigned msgLength;
   DeserializeCtx ctx = { recvBuf, sizeof(msgLength) };

   Serialization_deserializeUInt(&ctx, &msgLength);

   return msgLength;
}
        // ssize_t MessagingTk_recvMsgBuf(App* app, Socket* sock, char* bufIn, size_t bufInLen)
ssize_t MessagingTk_recvMsgBuf(Socket* sock, char* bufIn, size_t bufInLen)

{
   
   size_t numReceived = 0;
   ssize_t recvRes;
   size_t msgLength;
            //Config* cfg = App_getConfig(app);
    int  connMsgLongTimeout=500;// 可以暂时自己定义这个超时时间 500ms 50s 180000 
                           //500 5000 10000 都不行

   // receive at least the message header
                //recvRes = Socket_recvExactTEx_kernel(sock, bufIn, NETMSG_MIN_LENGTH, 0, cfg->connMsgLongTimeout,&numReceived);
   recvRes = Socket_recvExactTEx_kernel(sock, bufIn, NETMSG_MIN_LENGTH, 0, connMsgLongTimeout,&numReceived);
   LOG_printf(DEBUG,"[MessagingTk_recvMsgBuf] out of 1th Socket_recvExactTEx_kernel , recvRes: %lu \n",recvRes);
   if(recvRes <= 0 )
   { 
                    // socket error
                    // Logger* log = App_getLogger(app);
                    // Logger_logFormatted(log, Log_DEBUG, logContext, "Failed to receive message header from: %s",
                    //    Socket_getPeername(sock) );
                    //   goto socket_exception;
        LOG_printf(DEBUG,"[MessagingTk_recvMsgBuf] error :recvRes <= 0 \n");
        return recvRes;
   }

   msgLength = NetMessage_extractMsgLengthFromBuf(bufIn);

   if(msgLength <= numReceived)
      return msgLength; // success (msg had no additional payload)


   // receive the message payload part
   LOG_printf(DEBUG,"[MessagingTk_recvMsgBuf] msgLength %lu ,bufinlen %lu \n",msgLength,bufInLen);
   if(msgLength > bufInLen) 
   { 
                    // message too big to be accepted
                    // Logger* log = App_getLogger(app);
                    // Logger_logFormatted(log, Log_WARNING, logContext,
                    //    "Received a message that is too large from: %s (bufLen: %lld, msgLen: %lld)",
                    //    Socket_getPeername(sock), (long long)bufInLen, (long long)msgLength);
                    //   return -EMSGSIZE;
        LOG_printf(DEBUG,"[MessagingTk_recvMsgBuf] error : msgLength > bufInLen \n");
        return 0;
   }
   // 到这里了 
                //recvRes = Socket_recvExactTEx_kernel(sock, &bufIn[numReceived], msgLength-numReceived, 0,cfg->connMsgLongTimeout, &numReceived);
   recvRes = Socket_recvExactTEx_kernel(sock, &bufIn[numReceived], msgLength-numReceived, 0,connMsgLongTimeout, &numReceived);

   if(recvRes <= 0) 
    {
                //goto socket_exception;
        LOG_printf(DEBUG,"[MessagingTk_recvMsgBuf] error : recvRes <= 0\n");
        return recvRes;
    }

   // success
   return msgLength;


                //    socket_exception://错误处理
                //    {
                //       // Logger* log = App_getLogger(app);

                //       // if (fatal_signal_pending(current))
                //       //    Logger_logFormatted(log, Log_DEBUG, logContext,
                //       //       "Receive interrupted by signal");
                //       // else
                //       //    Logger_logFormatted(log, Log_DEBUG, logContext,
                //       //       "Receive failed from: %s (ErrCode: %lld)",
                //       //       Socket_getPeername(sock), (long long)recvRes);
                //    }

                //    return recvRes;
}



//8. 反序列化解析-------------------------------------------------------------------------




         inline uint16_t get_unaligned_le16(const char* ptr) {
                  uint16_t value;
                  memcpy(&value, ptr, sizeof(value));
                  return value;
            }

      static inline bool Serialization_deserializeUShort(DeserializeCtx* ctx, unsigned short* outValue)
      {
                     //return __Serialization_deserializeRaw(ctx, get_unaligned_le16, outValue);
         if (ctx->length < sizeof(*outValue)) {
            return false;
         }
         *outValue = get_unaligned_le16(ctx->data);
         ctx->data += sizeof(*outValue);
         ctx->length -= sizeof(*outValue);
         return true;
      }
#define __Serialization_deserializeRaw(ctx, convert, outValue) \
   ({ \
      DeserializeCtx* ctx__ = (ctx); \
      bool result = false; \
      (void) (outValue == (__typeof__(convert(ctx__->data) )*) 0); \
      if(ctx__->length >= sizeof(*(outValue) )  ) \
      { \
         *(outValue) = convert(ctx__->data); \
         ctx__->data += sizeof(*(outValue) ); \
         ctx__->length -= sizeof(*(outValue) ); \
         result = true; \
      } \
      result; \
   })

      static inline bool Serialization_deserializeChar(DeserializeCtx* ctx, char* outValue)
      {
                     // return __Serialization_deserializeRaw(ctx, *, outValue);
         if (ctx->length < sizeof(char)) {
            return false; // 不够数据来反序列化一个 char
         }
         *outValue = *ctx->data; // 使用解引用来读取数据
         ctx->data += sizeof(char); // 更新数据指针
         ctx->length -= sizeof(char); // 更新剩余长度
         return true;

         
      }


      static inline bool Serialization_deserializeUInt8(DeserializeCtx* ctx,uint8_t* outValue)
      {
         return Serialization_deserializeChar(ctx, (char*)outValue);
      }

            inline uint64_t get_unaligned_le64(const char* ptr) {
                  uint64_t value;
                  memcpy(&value, ptr, sizeof(value));
                  return value;
            }


      static inline bool Serialization_deserializeUInt64(DeserializeCtx* ctx,uint64_t* outValue)
      {
                  //return __Serialization_deserializeRaw(ctx, get_unaligned_le64, outValue);
         if (ctx->length < sizeof(*outValue)) {
            LOG(DEBUG)<<"[] ctx->length <sizeof(*outValue) \n";
            return false;
         }
         // for(int i=0;i<8;i++)
         //    LOG_printf(DEBUG,"@@ ctx->data[%d]= %02x \n",ctx->data[i]);
         *outValue = get_unaligned_le64(ctx->data);//函数通常用于从内存中读取一个 64 位的小端（little-endian）整数，而不考虑其对齐情况。小端表示法是指低位字节存储在低地址端，高位字节存储在高地址端。
         LOG(DEBUG)<<"*outValue = "<<*outValue<<"\n";
         ctx->data += sizeof(*outValue);
         ctx->length -= sizeof(*outValue);
         return true;
      }

       
        /**
         * Reads the (common) header part of a message from a buffer.
         *
         * Note: Message type will be set to NETMSGTYPE_Invalid if deserialization fails.
         */
        void __NetMessage_deserializeHeader(DeserializeCtx* ctx, NetMessageHeader* outHeader)// 把 ctx 的值 反序列化 给 outHeader 的各个字段赋值
        {
            size_t totalLength = ctx->length;
            uint64_t prefix = 0;
            // check min buffer length

            if(ctx->length < NETMSG_HEADER_LENGTH) 
            {
                outHeader->msgType = NETMSGTYPE_Invalid;
                return;
            }

            // message length
            Serialization_deserializeUInt(ctx, &outHeader->msgLength);//已经定义过了

            // verify contained msg length
            if(outHeader->msgLength != totalLength) 
            {
                outHeader->msgType = NETMSGTYPE_Invalid;
                return;
            }

            // feature flags
            Serialization_deserializeUShort(ctx, &outHeader->msgFeatureFlags);
            Serialization_deserializeUInt8(ctx, &outHeader->msgCompatFeatureFlags);
            Serialization_deserializeUInt8(ctx, &outHeader->msgFlags);

            // check message prefix
            Serialization_deserializeUInt64(ctx, &prefix);
            if (prefix != NETMSG_PREFIX)
            {
                outHeader->msgType = NETMSGTYPE_Invalid;
                return;
            }

            if (outHeader->msgFlags & ~(MSGHDRFLAG_BUDDYMIRROR_SECOND | MSGHDRFLAG_IS_SELECTIVE_ACK | MSGHDRFLAG_HAS_SEQUENCE_NO))
            {
                outHeader->msgType = NETMSGTYPE_Invalid;
                return;
            }

            // message type
            Serialization_deserializeUShort(ctx, &outHeader->msgType);

            // targetID
            Serialization_deserializeUShort(ctx, &outHeader->msgTargetID);

            // userID
            Serialization_deserializeUInt(ctx, &outHeader->msgUserID);

            Serialization_deserializeUInt64(ctx, &outHeader->msgSequence);
            Serialization_deserializeUInt64(ctx, &outHeader->msgSequenceDone);
        }


            void NetMessage_init(NetMessage* thismsg, unsigned short msgType, const struct NetMessageOps* ops)
            {
               LOG(DEBUG)<<"go into NetMessage_init \n";
                memset(thismsg, 0, sizeof(*thismsg) ); // clear function pointers etc.

                // thismsg->msgLength = 0; // zero'ed by memset
                // thismsg->msgFeatureFlags = 0; // zero'ed by memset

                thismsg->msgHeader.msgType = msgType;
                LOG_printf(DEBUG,"msgType = %u \n",(unsigned)msgType);

                // needs to be set to actual ID by some async flushers etc
                thismsg->msgHeader.msgUserID = geteuid();//FhgfsCommon_getCurrentUserID();

                // thismsg->msgTargetID = 0; // zero'ed by memset

                thismsg->ops = ops;
            }


            void OpenFileRespMsg_init(OpenFileRespMsg* openmsg)
            {
                NetMessage_init(&openmsg->netMessage, NETMSGTYPE_OpenFileResp, &OpenFileRespMsg_Ops);
            }
            // 插入补充一些  NETMSGTYPE_ListDirFromOffsetResp 的相关定义
#define NETMSGTYPE_ListDirFromOffsetResp           2030

typedef struct RawList {
   const char* data;
   unsigned length;
   unsigned elemCount;
} RawList;

/**
 * Note: Only deserialization supported, no serialization.
 */
struct ListDirFromOffsetRespMsg
{
   NetMessage netMessage;

   int result;

   int64_t newServerOffset;

   // for deserialization
   RawList namesList;
   RawList entryTypesList;
   RawList entryIDsList;
   RawList serverOffsetsList;
};

bool ListDirFromOffsetRespMsg_deserializePayload(NetMessage* msg, DeserializeCtx* ctx);


const struct NetMessageOps ListDirFromOffsetRespMsg_Ops = {
//    .serializePayload = _NetMessage_serializeDummy,
//    .processIncoming = NetMessage_processIncoming,
   .getSupportedHeaderFeatureFlagsMask = NetMessage_getSupportedHeaderFeatureFlagsMask,
   .deserializePayload = ListDirFromOffsetRespMsg_deserializePayload,// deserializePayload
   
};

void ListDirFromOffsetRespMsg_init(ListDirFromOffsetRespMsg* msg)
{
   NetMessage_init(&msg->netMessage, NETMSGTYPE_ListDirFromOffsetResp,
      &ListDirFromOffsetRespMsg_Ops);
}

//补充readdir 的饭序列化的相关函数 ----------------------------------


static inline unsigned Serialization_serialLenUInt64(void)
{
   return sizeof(uint64_t);
}

static inline unsigned Serialization_serialLenInt64(void)
{
   return Serialization_serialLenUInt64();
}

static inline unsigned Serialization_serialLenUInt(void)
{
   return sizeof(unsigned);
}

bool __Serialization_deserializeNestedField(DeserializeCtx* ctx, DeserializeCtx* outCtx)
{
   unsigned length;
   // for(int i=0;i<8;i++)
   //        LOG_printf(DEBUG,"[NestedField] @@ ctx->data[%d] = %02x \n",ctx->data[i]);
   if(!Serialization_deserializeUInt(ctx, &length) )
      {
         LOG_printf(DEBUG,"!Serialization_deserializeUInt(ctx, &length) \n");
         return false;
      }
      
   LOG_printf(DEBUG,"1. length = %u",length);
   length -= Serialization_serialLenUInt();
   LOG_printf(DEBUG,"2. length = %u, ctx->length = %lu",length,ctx->length);

   if (ctx->length < length)
      return false;

   outCtx->data = ctx->data;
   outCtx->length = length;

   ctx->data += length;
   ctx->length -= length;
   return true;
}

/**
 * Pre-processes a serialized Int64CpyList().
 *
 * @return false on error or inconsistency
 */
bool Serialization_deserializeInt64CpyListPreprocess(DeserializeCtx* ctx, RawList* outList)
{
   DeserializeCtx outCtx;

   if(!__Serialization_deserializeNestedField(ctx, &outCtx) )
      {  
         LOG_printf(DEBUG,"1. __Serialization_deserializeNestedField \n");
         return false;
      }

   // elem count field
   if(!Serialization_deserializeUInt(&outCtx, &outList->elemCount) ) 
         {  
         LOG_printf(DEBUG,"2. Serialization_deserializeUInt \n");
         return false;
      }

   if(outCtx.length != (outList->elemCount * Serialization_serialLenInt64() ) )
      {  
         LOG_printf(DEBUG,"3. outCtx.length != (outList->elemCount * Serialization_serialLenInt64() ) \n");
         return false;
      }

   outList->data = outCtx.data;
   outList->length = outCtx.length;

   return true;
}

/**
 * Pre-processes a serialized Int64CpyVec().
 *
 * @return false on error or inconsistency
 */
bool Serialization_deserializeInt64CpyVecPreprocess(DeserializeCtx* ctx, RawList* outList)
{
   // serialization format is 100% compatible to the list version, so we can just use the list
      // version code here...

   return Serialization_deserializeInt64CpyListPreprocess(ctx, outList);
}





// 2
// char (de)serialization
static inline unsigned Serialization_serialLenChar(void)
{
   return sizeof(char);
}


static inline unsigned Serialization_serialLenUInt8(void)
{
   return Serialization_serialLenChar();
}

/**
 * Pre-processes a serialized UInt8List().
 *
 * @return false on error or inconsistency
 */
bool Serialization_deserializeUInt8ListPreprocess(DeserializeCtx* ctx, RawList* outList)
{
   DeserializeCtx outCtx;

   if(!__Serialization_deserializeNestedField(ctx, &outCtx) )
      return false;

   // elem count field
   if(!Serialization_deserializeUInt(&outCtx, &outList->elemCount) ) 
      return false;

   if(outCtx.length != outList->elemCount * Serialization_serialLenUInt8() )
      return false;

   outList->data = outCtx.data;
   outList->length = outCtx.length;

   return true;
}



/**
 * Pre-processes a serialized UInt8Vec().
 *
 * @return false on error or inconsistency
 */
bool Serialization_deserializeUInt8VecPreprocess(DeserializeCtx* ctx, RawList* outList)
{
   // serialization format is 100% compatible to the list version, so we can just use the list
      // version code here...

   return Serialization_deserializeUInt8ListPreprocess(ctx, outList);
}


/**
 * Pre-processes a serialized StrCpyList.
 *
 * @return false on error or inconsistency
 */
bool Serialization_deserializeStrCpyListPreprocess(DeserializeCtx* ctx, RawList* outList)
{
   DeserializeCtx outCtx;

   if(!__Serialization_deserializeNestedField(ctx, &outCtx) )
      return false;

   // elem count field
   if(!Serialization_deserializeUInt(&outCtx, &outList->elemCount) ) 
      return false;

   if(outList->elemCount && ( outCtx.data[outCtx.length - 1] != 0) ) 
      return false;

   outList->data = outCtx.data;
   outList->length = outCtx.length;

   return true;
}


/**
 * Pre-processes a serialized StrCpyVec.
 *
 * Note: We keep the serialization format compatible to the string list format.
 *
 * @return false on error or inconsistency
 */
bool Serialization_deserializeStrCpyVecPreprocess(DeserializeCtx* ctx, RawList* outList)
{
   // serialization format is 100% compatible to the list version, so we can just use the list
      // version code here...

   return Serialization_deserializeStrCpyListPreprocess(ctx, outList);
}












        /**
         * @return NetMessage that must be deleted by the caller
         * (msg->msgType is NETMSGTYPE_Invalid on error)
         * 功能是根据传入的消息类型 msgType 创建并返回一个特定类型的网络消息对象。这个函数通常用于网络通信协议中，以便根据不同的消息类型生成相应的消息结构体。
         */
        NetMessage* NetMessageFactory_createFromMsgType(unsigned short msgType)
        {
                    //#define HANDLE(ID, TYPE) case NETMSGTYPE_##ID: return NETMESSAGE_CONSTRUCT(TYPE)

            switch(msgType)
            {
                        //HANDLE(OpenFileResp, OpenFileRespMsg); //消息类型应该就是 OpenFile
                case NETMSGTYPE_OpenFileResp: {
                    OpenFileRespMsg* msg = (OpenFileRespMsg*)malloc(sizeof(OpenFileRespMsg));
                    OpenFileRespMsg_init(msg);
                    return (NetMessage*)msg;
                }
               case NETMSGTYPE_ListDirFromOffsetResp: {
                   LOG(DEBUG)<<"消息类型是 NETMSGTYPE_ListDirFromOffsetRespMsg  \n";
                    ListDirFromOffsetRespMsg* msg = (ListDirFromOffsetRespMsg*)malloc(sizeof(ListDirFromOffsetRespMsg));
                    ListDirFromOffsetRespMsg_init(msg);
                    return (NetMessage*)msg;
                }

                default:
                {   
                    LOG_printf(INFO," 未知的消息类型, 消息类型不是 OpenFileRespMsg \n");
                    //干脆就不处理
                            // SimpleMsg* msg = os_kmalloc(sizeof(*msg));
                            // SimpleMsg_init(msg, NETMSGTYPE_Invalid);
                            // return (NetMessage*)msg;
                    // NetMessage* msg = (NetMessage*)malloc(sizeof(NetMessage));
                    // SimpleMsg_init(msg, NETMSGTYPE_Invalid);
                    // return msg;
                    return nullptr;
                }
            }
            /*
            {
                // control messages
                HANDLE(Ack, AckMsgEx);
                HANDLE(GenericResponse, GenericResponseMsg);
                // helperd messages
                HANDLE(GetHostByNameResp, GetHostByNameRespMsg);
                HANDLE(LogResp, LogRespMsg);
                // nodes messages
                HANDLE(GetNodesResp, GetNodesRespMsg);
                HANDLE(GetStatesAndBuddyGroupsResp, GetStatesAndBuddyGroupsRespMsg);
                HANDLE(GetTargetMappingsResp, GetTargetMappingsRespMsg);
                HANDLE(HeartbeatRequest, HeartbeatRequestMsgEx);
                HANDLE(Heartbeat, HeartbeatMsgEx);
                HANDLE(MapTargets, MapTargetsMsgEx);
                HANDLE(RefreshTargetStates, RefreshTargetStatesMsgEx);
                HANDLE(RegisterNodeResp, RegisterNodeRespMsg);
                HANDLE(RemoveNode, RemoveNodeMsgEx);
                HANDLE(RemoveNodeResp, RemoveNodeRespMsg);
                HANDLE(SetMirrorBuddyGroup, SetMirrorBuddyGroupMsgEx);
                // storage messages
                HANDLE(LookupIntentResp, LookupIntentRespMsg);
                HANDLE(MkDirResp, MkDirRespMsg);
                HANDLE(RmDirResp, RmDirRespMsg);
                HANDLE(MkFileResp, MkFileRespMsg);
                HANDLE(RefreshEntryInfoResp, RefreshEntryInfoRespMsg);
                HANDLE(RenameResp, RenameRespMsg);
                HANDLE(HardlinkResp, HardlinkRespMsg);;
                HANDLE(UnlinkFileResp, UnlinkFileRespMsg);
                HANDLE(ListDirFromOffsetResp, ListDirFromOffsetRespMsg);
                HANDLE(SetAttrResp, SetAttrRespMsg);
                HANDLE(StatResp, StatRespMsg);
                HANDLE(StatStoragePathResp, StatStoragePathRespMsg);
                HANDLE(TruncFileResp, TruncFileRespMsg);
                HANDLE(ListXAttrResp, ListXAttrRespMsg);
                HANDLE(GetXAttrResp, GetXAttrRespMsg);
                HANDLE(RemoveXAttrResp, RemoveXAttrRespMsg);
                HANDLE(SetXAttrResp, SetXAttrRespMsg);
                // session messages
                HANDLE(OpenFileResp, OpenFileRespMsg);  //-----------------
                HANDLE(CloseFileResp, CloseFileRespMsg);
                HANDLE(WriteLocalFileResp, WriteLocalFileRespMsg);
            #ifdef BEEGFS_NVFS
                HANDLE(WriteLocalFileRDMAResp, WriteLocalFileRDMARespMsg);
            #endif
                HANDLE(FSyncLocalFileResp, FSyncLocalFileRespMsg);
                HANDLE(FLockAppendResp, FLockAppendRespMsg);
                HANDLE(FLockEntryResp, FLockEntryRespMsg);
                HANDLE(FLockRangeResp, FLockRangeRespMsg);
                HANDLE(LockGranted, LockGrantedMsgEx);
                HANDLE(BumpFileVersionResp, BumpFileVersionRespMsg);
                HANDLE(GetFileVersionResp, GetFileVersionRespMsg);

                case NETMSGTYPE_AckNotifyResp: {
                    SimpleMsg* msg = os_kmalloc(sizeof(*msg));
                    SimpleMsg_init(msg, msgType);
                    return &msg->netMessage;
                }

                default:
                {
                    SimpleMsg* msg = os_kmalloc(sizeof(*msg));
                    SimpleMsg_init(msg, NETMSGTYPE_Invalid);
                    return (NetMessage*)msg;
                }
            }
            */
        }


        unsigned short NetMessage_getMsgType(NetMessage* thismsg)
        {
            return thismsg->msgHeader.msgType;
        }


        /**
         * Returns all feature flags that are supported by this message. Defaults to "none", so this
         * method needs to be overridden by derived messages that actually support header feature
         * flags.
         *
         * @return combination of all supported feature flags
         */
        unsigned NetMessage_getSupportedHeaderFeatureFlagsMask(NetMessage* thismsg)
        {
            return 0;
        }

        /**
         * Check if the msg sender has set an incompatible feature flag.
         *
         * @return false if an incompatible feature flag was set
         */
        bool NetMessage_checkHeaderFeatureFlagsCompat(NetMessage* thismsg)
        {
            unsigned unsupportedFlags = ~(thismsg->ops->getSupportedHeaderFeatureFlagsMask(thismsg) );
            if(thismsg->msgHeader.msgFeatureFlags & unsupportedFlags) 
                return false; // an unsupported flag was set

            return true;
        }

            void _NetMessage_setMsgType(NetMessage* thismsg, unsigned short msgType)
            {
                thismsg->msgHeader.msgType = msgType;
            }


         /**
          * Dummy function for deserialize pointers
          * 总的来说，_NetMessage_deserializeDummy 函数似乎用于调试目的，当程序逻辑中出现不应该发生的情况时，提供错误日志和调试信息。在生产环境中，这样的函数通常用于诊断问题，而不是作为正常逻辑的一部分。
          */
         /*bool _NetMessage_deserializeDummy(NetMessage* thismsg, DeserializeCtx* ctx)
         {
            printk_fhgfs(KERN_INFO, "Bug: Deserialize function called, although it should not\n");
            dump_stack();

            return true;
         }*/

        static bool __NetMessageFactory_deserializeRaw(DeserializeCtx* ctx,NetMessageHeader* header, NetMessage* outMsg)
        {
            bool checkCompatRes;
            bool deserPayloadRes;
            LOG_printf(DEBUG,"Go into __NetMessageFactory_deserializeRaw \n");

            outMsg->msgHeader = *header;

            checkCompatRes = NetMessage_checkHeaderFeatureFlagsCompat(outMsg);
            if(!checkCompatRes) 
            { // incompatible feature flag was set => log error with msg type
                // printk_fhgfs(KERN_NOTICE,
                //    "Received a message with incompatible feature flags. Message type: %hu; Flags (hex): %X",
                //    header->msgType, NetMessage_getMsgHeaderFeatureFlags(outMsg) );
                
               LOG_printf(DEBUG,"[__NetMessageFactory_deserializeRaw] checkCompatRes = False \n");
                _NetMessage_setMsgType(outMsg, NETMSGTYPE_Invalid);
                return false;
            }

            // deserialize message payload

            deserPayloadRes = outMsg->ops->deserializePayload(outMsg, ctx);
            if(!deserPayloadRes) 
            {
                // printk_fhgfs_debug(KERN_NOTICE, "Failed to decode message. Message type: %hu",
                //    header->msgType);
               LOG_printf(DEBUG,"[__NetMessageFactory_deserializeRaw] deserPayloadRes = False \n");

                _NetMessage_setMsgType(outMsg, NETMSGTYPE_Invalid);
                return false;
            }

            return true;
        }


/**
 * The standard way to create message objects from serialized message buffers.
 *
 * @return NetMessage which must be deleted by the caller
 * (msg->msgType is NETMSGTYPE_Invalid on error)
 */
NetMessage* NetMessageFactory_createFromBuf(char* recvBuf, size_t bufLen)// recvBuf 中放着接收到的信息
{
   LOG_printf(DEBUG,"GO into NetMessageFactory_createFromBuf\n");
   NetMessageHeader header;

   NetMessage* msg;

   DeserializeCtx ctx = {
      .data = recvBuf,
      .length = bufLen,
   };
   LOG_printf(DEBUG,"before ctx->length = %lu \n",ctx.length);

   // decode the message header
   __NetMessage_deserializeHeader(&ctx, &header);// 给 header 赋值
   LOG_printf(DEBUG,"差值 : %d ",ctx.data-recvBuf);
   // create the message object for the given message type
   LOG_printf(DEBUG,"header.msgType = %u \n",(unsigned)header.msgType);
   LOG_printf(DEBUG,"after ctx->length = %lu \n",ctx.length);

   msg = NetMessageFactory_createFromMsgType(header.msgType);//此时消息类型是正确的

   if(NetMessage_getMsgType(msg) == NETMSGTYPE_Invalid) 
   {
      // printk_fhgfs_debug(KERN_NOTICE,
      //    "Received an invalid or unhandled message. "
      //    "Message type (from raw header): %hu", header.msgType);
      LOG_printf(DEBUG,"NetMessage_getMsgType(msg) == NETMSGTYPE_Invalid \n");
      return msg;
   }

   __NetMessageFactory_deserializeRaw(&ctx, &header, msg);
   return msg;
}

static inline bool Serialization_deserializeInt(DeserializeCtx* ctx, int* outValue)
{
   return Serialization_deserializeUInt(ctx, (unsigned*)outValue);
}

/**
 * @return false on error (e.g. strLen is greater than bufLen)
 */
bool Serialization_deserializeStrAlign4(DeserializeCtx* ctx,unsigned* outStrLen, const char** outStrStart)
{
   const char* const bufAtStart = ctx->data;
   unsigned padding;

   // length field
   if(!Serialization_deserializeUInt(ctx, outStrLen) ) 
      return false;

   // check length and terminating zero
   if( (ctx->length < *outStrLen + 1) || ( (ctx->data)[*outStrLen] != 0) ) 
      return false;

   // string start
   *outStrStart = ctx->data;
   ctx->data += *outStrLen + 1;
   ctx->length -= *outStrLen + 1;

   padding = (ctx->data - bufAtStart) % 4;
   if(padding != 0)
   {
      if(ctx->length < padding) 
         return false;

      ctx->data += 4 - padding;
      ctx->length -= 4 - padding;
   }

   return true;
}




/**
 * Main initialization function for PathInfo, should typically be used
 *
 * @param origParentEntryID will be free'd on uninit
 */
void PathInfo_init(PathInfo* pathinfo, unsigned origParentUID, const char* origParentEntryID, unsigned flags)
{
        //    PathInfo_setOrigUID(pathinfo, origParentUID);
        //    PathInfo_setOrigParentEntryID(pathinfo, origParentEntryID);
        //    PathInfo_setFlags(pathinfo, flags);
    pathinfo->_origParentUID = origParentUID;
    pathinfo->_origParentEntryID = (char *) origParentEntryID;
    pathinfo->_flags = flags;

}


/**
 * deserialize the given buffer
 */
bool PathInfo_deserialize(DeserializeCtx* ctx, PathInfo* outThis)
{
   unsigned flags;

   unsigned origParentUID;
   unsigned origParentEntryIDLen;
   const char* origParentEntryID;

   // flags
   if(!Serialization_deserializeUInt(ctx, &flags) )
      return false;

   if (flags & PATHINFO_FEATURE_ORIG)
   {  // file has origParentUID and origParentEntryID
      // origParentUID
      if(!Serialization_deserializeUInt(ctx, &origParentUID) )
         return false;

      // origParentEntryID
      if(!Serialization_deserializeStrAlign4(ctx, &origParentEntryIDLen, &origParentEntryID) )
         return false;
   }
   else
   {  // either a directory or a file stored in old format
      origParentUID = 0;
      origParentEntryID = nullptr;
   }

   PathInfo_init(outThis, origParentUID, origParentEntryID, flags);

   return true;
}


bool StripePattern_deserializePatternPreprocess(DeserializeCtx* ctx,const char** outPatternStart, uint32_t* outPatternLength)
{
   DeserializeCtx temp = *ctx;

   if(!Serialization_deserializeUInt(&temp, outPatternLength))
      return false;

   *outPatternStart = ctx->data;

   if (*outPatternLength > ctx->length)
      return false;

   ctx->data += *outPatternLength;
   ctx->length -= *outPatternLength;

   return true;
}


bool OpenFileRespMsg_deserializePayload(NetMessage* thismsg, DeserializeCtx* ctx)
{
   OpenFileRespMsg* thisCast = (OpenFileRespMsg*)thismsg;

   // result
   if(!Serialization_deserializeInt(ctx, &thisCast->result) )
      return false;

   // fileHandleID
   if(!Serialization_deserializeStrAlign4(ctx, &thisCast->fileHandleIDLen,
         &thisCast->fileHandleID) )
      return false;

   // pathInfo
   if (!PathInfo_deserialize(ctx, &thisCast->pathInfo) )
      return false;

   // stripePattern
   if(!StripePattern_deserializePatternPreprocess(ctx,
      &thisCast->patternStart, &thisCast->patternLength) )
      return false;

   if (!Serialization_deserializeUInt64(ctx, &thisCast->fileVersion))
      return false;

   return true;
}
//--------------------------channel------------------------------------------------
// int64 (de)serialization
static inline void Serialization_serializeInt64(SerializeCtx* ctx, int64_t value)
{
   Serialization_serializeUInt64(ctx, value);
}

static inline bool Serialization_deserializeInt64(DeserializeCtx* ctx,
   int64_t* outValue)
{
   return Serialization_deserializeUInt64(ctx, (uint64_t*)outValue);
}



void SimpleInt64Msg_serializePayload(NetMessage* nmsg, SerializeCtx* ctx)
{
   SimpleInt64Msg* thisCast = (SimpleInt64Msg*)nmsg;

   Serialization_serializeInt64(ctx, thisCast->value);
}

bool SimpleInt64Msg_deserializePayload(NetMessage* nmsg, DeserializeCtx* ctx)
{
   SimpleInt64Msg* thisCast = (SimpleInt64Msg*)nmsg;

   return Serialization_deserializeInt64(ctx, &thisCast->value);
}
void SimpleInt64Msg_init(SimpleInt64Msg* smsg, unsigned short msgType)
{
   NetMessage_init(&smsg->netMessage, msgType, &SimpleInt64Msg_Ops);
}

void SimpleInt64Msg_initFromValue(SimpleInt64Msg* smsg, unsigned short msgType, int64_t value)
{
   SimpleInt64Msg_init(smsg, msgType);

   smsg->value = value;
}


void AuthenticateChannelMsg_initFromValue(AuthenticateChannelMsg* acmsg, uint64_t authHash)
{
   SimpleInt64Msg_initFromValue( (SimpleInt64Msg*)acmsg, NETMSGTYPE_AuthenticateChannel, authHash);
}



// bool __NodeConnPool_applySocketOptionsConnected(NodeConnPool* this, Socket* sock)
bool __NodeConnPool_applySocketOptionsConnected(Socket* sock)
{
   // const char* logContext = "NodeConn (apply socket options)";
   // Logger* log = App_getLogger(this->app);

   // Config* cfg = App_getConfig(this->app);
   uint64_t authHash = 10322703647856944136;//610185155436495494;//10322703647856944136; //我从配置文件中 哈希计算出来的   //Config_getConnAuthHash(cfg); //574355913317798958是xfusion3上的
   //xfusion3: 574355913317798958
   // 162 server: 10322703647856944136
   bool allSuccessful = true;

   LOG(DEBUG)<<"go into __NodeConnPool_applySocketOptionsConnected \n";
   // apply general socket options
   /*
      NicAddrType_t sockType = Socket_getSockType(sock);
      if( (sockType == NICADDRTYPE_STANDARD) ||
         (sockType == NICADDRTYPE_SDP) )
      {
         StandardSocket* standardSock = (StandardSocket*)sock;
         bool corkRes;
         bool noDelayRes;
         bool keepAliveRes;

         corkRes = StandardSocket_setTcpCork(standardSock, false);
         if(!corkRes)
         {
            // Logger_log(log, Log_NOTICE, logContext, "Failed to disable TcpCork");
            allSuccessful = false;
         }

         noDelayRes = StandardSocket_setTcpNoDelay(standardSock, true);
         if(!noDelayRes)
         {
            // Logger_log(log, Log_NOTICE, logContext, "Failed to enable TcpNoDelay");
            allSuccessful = false;
         }

         keepAliveRes = StandardSocket_setSoKeepAlive(standardSock, true);
         if(!keepAliveRes)
         {
            // Logger_log(log, Log_NOTICE, logContext, "Failed to enable SoKeepAlive");
            allSuccessful = false;
         }
      }
   */

   // send control messages

   if(authHash) //这个分支是肯定要进来的
   { // authenticate channel
      char* sendBuf;
      size_t sendBufLen;
      AuthenticateChannelMsg authMsg;
      size_t sendRes;

      AuthenticateChannelMsg_initFromValue(&authMsg, authHash); //
      sendBufLen = NetMessage_getMsgLength( (NetMessage*)&authMsg);
      // sendBuf = (char*)os_kmalloc(sendBufLen);
      sendBuf = (char*)malloc(sendBufLen);
      if (sendBuf == NULL) {
         // 处理内存分配失败的情况
         perror("Memory allocation failed");
         // 可能的操作包括退出程序或执行其他清理工作
         return false;
      }

      NetMessage_serialize( (NetMessage*)&authMsg, sendBuf, sendBufLen);
      // LOG(DEBUG)<<"send buffer = \n";
      // for(int i=0;i<sendBufLen;i++)
      // {
      //    LOG(DEBUG)<<sendBuf[i];
      // }
      // LOG(DEBUG)<<"\n";
            // sendRes = Socket_send_kernel(sock, sendBuf, sendBufLen, 0);
      struct iovec  *iter = STACK_ALLOC_BEEGFS_ITER_KVEC(sendBuf, sendBufLen);
      int iovcnt=1;
      fhgfs_sockaddr_in *to=nullptr;
      sendRes=sock->ops->sendto(sock, iter,  iovcnt,0, to);
      LOG(DEBUG)<<" sendRes = "<<sendRes<<"\n";
      if(sendRes <= 0)
      {
         // Logger_log(log, Log_WARNING, logContext, "Failed to send authentication");
         LOG_printf(DEBUG,"Failed to send authentication \n");
         allSuccessful = false;
      }

      free(sendBuf);
   }
/*

   { // make channel indirect
      char* sendBuf;
      size_t sendBufLen;
      SetChannelDirectMsg directMsg;
      size_t sendRes;

      SetChannelDirectMsg_initFromValue(&directMsg, 0);
      sendBufLen = NetMessage_getMsgLength( (NetMessage*)&directMsg);
      sendBuf = (char*)os_kmalloc(sendBufLen);

      NetMessage_serialize( (NetMessage*)&directMsg, sendBuf, sendBufLen);
      sendRes = Socket_send_kernel(sock, sendBuf, sendBufLen, 0);
      if(sendRes <= 0)
      {
         // Logger_log(log, Log_WARNING, logContext, "Failed to set channel to indirect mode");
         allSuccessful = false;
      }

      kfree(sendBuf);
   }

   {
      char* sendBuf;
      size_t sendBufLen;
      PeerInfoMsg peerInfo;
      size_t sendRes = -1;

      PeerInfoMsg_init(&peerInfo, NODETYPE_Client, this->app->localNode->numID);
      sendBufLen = NetMessage_getMsgLength(&peerInfo.netMessage);
      sendBuf = kmalloc(sendBufLen, GFP_KERNEL);

      if (sendBuf)
      {
         NetMessage_serialize(&peerInfo.netMessage, sendBuf, sendBufLen);
         sendRes = Socket_send_kernel(sock, sendBuf, sendBufLen, 0);
      }

      if(sendRes <= 0)
      {
         // Logger_log(log, Log_WARNING, logContext, "Failed to transmit local node info");
         allSuccessful = false;
      }

      kfree(sendBuf);
   }
   */

   return allSuccessful;
}

inline bool NodeConnPool_releaseStreamSocket(Socket* sock) {

        if (sock == NULL || ((StandardSocket*)sock)->sock == 0) {
            // 套接字已经关闭或未初始化
            return true;
        }

        // 关闭套接字
        if (close(((StandardSocket*)sock)->sock) == -1) {
            perror("Error closing socket");
            return false;
        }

        // 清理 StandardSocket 结构体
        free(sock); // 假设 sock 是通过 malloc 分配的

        return true;
    }

// 函数声明：输出当前时间，精确到微秒
void printCurrentTime() {
    struct timeval tv;
    struct tm* tm_info;

    // 获取当前时间
    if (gettimeofday(&tv, NULL) != 0) {
        perror("gettimeofday");
        return;
    }

    // 将时间转换为本地时间
    tm_info = localtime(&tv.tv_sec);

    // 打印格式化的时间字符串，包括微秒
    LOG_printf(DEBUG,"Arrival Time: %d-%02d-%02d %02d:%02d:%02d.%06ld\n",
        tm_info->tm_year + 1900, // 年份从1900开始计数
        tm_info->tm_mon + 1,     // 月份从0开始计数
        tm_info->tm_mday,        // 一个月中的第几天
        tm_info->tm_hour,        // 小时
        tm_info->tm_min,         // 分钟
        tm_info->tm_sec,         // 秒
        tv.tv_usec               // 微秒
    );
}
void open(Socket* sock,char* parentEntryID,char* entryID,char* fileName,DirEntryType entryType,char* &OutFileHandleID)
{

   

   unsigned sendBufLen = sendBufLen = 1024; //@@ 这个自己指定，需要比发送的消息长度要长，也是接收缓冲区的长度
   unsigned int recvBufLen=1024;            //@@ 接收缓冲区长度，这个长度决定了 bufLen 的长度
 
   ssize_t respRes = 0;
   ssize_t sendRes;
   bool isConsturctSocket=false;
   
      if (sock == NULL) 
      {
         isConsturctSocket=true;
         sock = (Socket*)StandardSocket_construct(PF_INET, SOCK_STREAM, 0);
         if(!sock) //如果获取套接字失败，记录错误并返回通信错误。
         { 
            LOG_printf(DEBUG," 获取套接字失败 \n");
            return ;
         }
         else
         {
            LOG_printf(DEBUG," 获取套接字成功 \n");
            
         }
            //2. 尝试连接
         LOG_printf(DEBUG,"---------2. 尝试连接-------------\n");
   
         int tcpbufLen;
         socklen_t tcpbufLenSize = sizeof(tcpbufLen);
         if (getsockopt(((StandardSocket*) sock)->sock, SOL_SOCKET, SO_RCVBUF, &tcpbufLen, &tcpbufLenSize) < 0) {
               std::cerr << "Failed to get socket receive buffer size" << std::endl;
               return ;
         }
         else
         LOG(DEBUG)<<"get socket receive buffer size = "<<tcpbufLen<<"\n";

         int bufSize =tcpbufLen;//8192*70;
         if(bufSize>0)
         {
            int val = bufSize;
               if (setsockopt(((StandardSocket*) sock)->sock, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val)) < 0) {
                     std::cerr << "Failed to set socket receive buffer size" << std::endl;
                     return ;
               }
               else
                  LOG(DEBUG)<<"Succesfully set socket receive buffer size = "<<val<<"\n";
         }
      
         //3. 尝试通过IP连接
         LOG_printf(DEBUG,"------------------- 3. 尝试通过IP连接-------------------\n");
            // StandardSocket* stdSock = (StandardSocket*)sock;
      
         unsigned short port=8005;//@@
         NicAddress nicAddr[10]; 
         initNicAddressArray_meta(nicAddr);//@@
         bool connectRes;
         connectRes = sock->ops->connectByIP(sock, nicAddr[2].ipAddr, port);//@@ 224: nicAddr[8] ;162 :  nicAddr[4]

         if(connectRes)
         {
            LOG_printf(DEBUG,"3. 尝试通过IP连接 连接成功 \n");

         }
         else
         {
            LOG_printf(DEBUG,"3. 尝试通过IP连接 连接失败 \n");
         }

               // 启用TCP Keepalive
               int opt = 1;
            if (setsockopt(((StandardSocket*)sock)->sock, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) < 0) {
               perror("setsockopt failed");
               exit(EXIT_FAILURE);
            }

            // 设置TCP Keepalive参数
            int keepalive_time = 30; // 30秒
            int keepalive_intvl = 5; // 每5秒发送一次Keepalive
            int keepalive_probes = 3; // 最多发送3次Keepalive

            if (setsockopt(((StandardSocket*)sock)->sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepalive_time, sizeof(keepalive_time)) < 0) {
               perror("setsockopt TCP_KEEPIDLE failed");
               exit(EXIT_FAILURE);
            }

            if (setsockopt(((StandardSocket*)sock)->sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepalive_intvl, sizeof(keepalive_intvl)) < 0) {
               perror("setsockopt TCP_KEEPINTVL failed");
               exit(EXIT_FAILURE);
            }

            if (setsockopt(((StandardSocket*)sock)->sock, IPPROTO_TCP, TCP_KEEPCNT, &keepalive_probes, sizeof(keepalive_probes)) < 0) {
               perror("setsockopt TCP_KEEPCNT failed");
               exit(EXIT_FAILURE);
            }

            // 现在，TCP连接将保持活跃状态
            LOG_printf(DEBUG,"Connection established and Keepalive is enabled.\n");


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

      }
   
   //4. 准备发送缓冲区 我怎么感觉是准备接收缓冲区
   LOG_printf(DEBUG,"--------------------4.准备发送缓冲区-------------------\n");

    
    int recvBufSize;
    socklen_t optLen = sizeof(recvBufSize);
   //  // 获取当前的接收缓冲区大小
   //  if (getsockopt(((StandardSocket*)sock)->sock,SOL_SOCKET, SO_RCVBUF, &recvBufSize, &optLen) < 0) {
   //      perror("getsockopt failed");
   //      close(((StandardSocket*)sock)->sock);
   //      return ;
   //  }

   //  LOG_printf(DEBUG,"Current receive buffer size: %d\n", recvBufSize);

   
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
         FileEvent_init(&event,FileEventType_READ, fileName);//第三个参数是 直接传相对挂载目录/mnt/beegfs/的相对路径 ，Note:好奇怪：前面的 entryID 和这个 路径不匹配 open 逻辑也是成功获取的,原因是服务端处理逻辑根本不识别这个参数
         
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

      if(sendRes != (ssize_t)sendBufLen)
         {
            LOG_printf(DEBUG,"sendRes != sendBufLen \n");
         }
   
   LOG_printf(DEBUG,"--------------------7. 接收消息-------------------\n");
      //7. receive response
      respRes = MessagingTk_recvMsgBuf(sock, outRespBuf, bufLen);

      if(respRes <= 0) 
      { 
         LOG_printf(DEBUG,"respRes<=0 \n");
      }
   
   LOG_printf(DEBUG,"--------------------8. 反序列化解析-------------------\n");
      
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



     delete iter; 
   if(isConsturctSocket)
   {
      // 释放套接字
      LOG(DEBUG)<<"释放套接字 \n";
      NodeConnPool_releaseStreamSocket(sock);
   }
   
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
    
   // 7. 后续处理

}


void __MessagingTk_requestResponseWithRRArgsComm(Socket* sock)
// (App* app,RequestResponseArgs* rrArgs, MirrorBuddyGroup* group, bool* wasIndirectCommErr,Socket* sock) 
{

   
   ssize_t respRes = 0;
   ssize_t sendRes;
   // rrArgs->outRespBuf = nullptr;
   // rrArgs->outRespMsg = nullptr;

      //  printCurrentTime();
      if (sock == NULL) //传参的时候确实看到sock==NULL
         // sock = NodeConnPool_acquireStreamSocket(connPool);
         // NodeConnPool_acquireStreamSocketEx(connPool, true, NULL);
         sock = (Socket*)StandardSocket_construct(PF_INET, SOCK_STREAM, 0);
         // sock = (Socket*)StandardSocket_construct(PF_INET, SOCK_STREAM, IPPROTO_TCP);
      
      //  printCurrentTime();
      if(!sock) //如果获取套接字失败，记录错误并返回通信错误。
      { 
         LOG_printf(DEBUG," 获取套接字失败 \n");
         return ;
      }
      else
      {
         LOG_printf(DEBUG," 获取套接字成功 \n");
         
      }
      
   
   
   //2. 尝试连接
   LOG_printf(DEBUG,"---------2. 尝试连接-------------\n");
   // bool __NodeConnPool_applySocketOptionsPreConnect(NodeConnPool* this, Socket* sock)
   // Config* cfg = App_getConfig(this->app);
      // Config* cfg;

      // NicAddrType_t sockType = NICADDRTYPE_STANDARD;//Socket_getSockType(sock);

      // StandardSocket* stdSock = (StandardSocket*)sock;
      // 先跳过这个
      // // 获取配置中的TCP接收缓冲区大小。
      // int bufSize = cfg->connTCPRcvBufSize;

      // // 我先默认 TCP 接收缓冲区 比 接收缓冲区 小 ，不需要增加 套接字的接收缓冲区 
      // if (bufSize > 0)// 如果缓冲区大小大于0，则设置套接字的接收缓冲区大小。
      //    StandardSocket_setSoRcvBuf(stdSock, bufSize);// 这里会设置ssock->sock参数，很可能和上面跳过的内核函数有关
      
      int tcpbufLen;
        socklen_t tcpbufLenSize = sizeof(tcpbufLen);
        if (getsockopt(((StandardSocket*) sock)->sock, SOL_SOCKET, SO_RCVBUF, &tcpbufLen, &tcpbufLenSize) < 0) {
            std::cerr << "Failed to get socket receive buffer size" << std::endl;
            return ;
        }
        else
        LOG(DEBUG)<<"get socket receive buffer size = "<<tcpbufLen<<"\n";

      int bufSize =tcpbufLen;//8192*70;
      // if (bufSize > 0)// 如果缓冲区大小大于0，则设置套接字的接收缓冲区大小。
      //    StandardSocket_setSoRcvBuf(stdSock, bufSize);// 这里会设置ssock->sock参数，很可能和上面跳过的内核函数有关
      if(bufSize>0)
      {
         int val = bufSize;
            if (setsockopt(((StandardSocket*) sock)->sock, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val)) < 0) {
                  std::cerr << "Failed to set socket receive buffer size" << std::endl;
                  return ;
            }
            else
               LOG(DEBUG)<<"Succesfully set socket receive buffer size = "<<val<<"\n";
      }
   
   //3. 尝试通过IP连接
   LOG_printf(DEBUG,"------------------- 3. 尝试通过IP连接-------------------\n");
      // StandardSocket* stdSock = (StandardSocket*)sock;
     
      unsigned short port=8005;//@@
      NicAddress nicAddr[10]; 
      initNicAddressArray_meta(nicAddr);//@@
      bool connectRes;
       printCurrentTime();
      connectRes = sock->ops->connectByIP(sock, nicAddr[4].ipAddr, port);//@@ 224server : 8 ; 162 server 选 4
       printCurrentTime();

      // const char *message = "Hello, server!";
      // send(((StandardSocket*)sock)->sock, message, strlen(message), 0);
      //   LOG_printf(DEBUG,"Hello, server 发送信息\n");

      if(connectRes)
      {
         LOG_printf(DEBUG,"3. 尝试通过IP连接 连接成功 \n");

      }
      else
      {
         LOG_printf(DEBUG,"3. 尝试通过IP连接 连接失败 \n");
      }
      // 这里我感觉缺乏显示的close(sock)操作 记得要补充一下


               //通过IP 和端口建立连接之后，还有些获取对等IP ，如果是非主接口，则设置过期计数器，这些操作在 NodeConnPool_acquireStreamSocketEx
               //Note:__NodeConnPool_applySocketOptionsConnected 这个函数用于应用套接字选项调整并发送通道控制消息。先不管他等一会信息获取不到再回来看
      // 应用连接后套接字选项
         // __NodeConnPool_applySocketOptionsConnected(this, (Socket*)sock);
      // sock->
      // 启用TCP Keepalive
      int opt = 1;
    if (setsockopt(((StandardSocket*)sock)->sock, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // 设置TCP Keepalive参数
    int keepalive_time = 30; // 30秒
    int keepalive_intvl = 5; // 每5秒发送一次Keepalive
    int keepalive_probes = 3; // 最多发送3次Keepalive

    if (setsockopt(((StandardSocket*)sock)->sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepalive_time, sizeof(keepalive_time)) < 0) {
        perror("setsockopt TCP_KEEPIDLE failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(((StandardSocket*)sock)->sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepalive_intvl, sizeof(keepalive_intvl)) < 0) {
        perror("setsockopt TCP_KEEPINTVL failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(((StandardSocket*)sock)->sock, IPPROTO_TCP, TCP_KEEPCNT, &keepalive_probes, sizeof(keepalive_probes)) < 0) {
        perror("setsockopt TCP_KEEPCNT failed");
        exit(EXIT_FAILURE);
    }

    // 现在，TCP连接将保持活跃状态
    LOG_printf(DEBUG,"Connection established and Keepalive is enabled.\n");


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
   

   //4. 准备发送缓冲区 我怎么感觉是准备接收缓冲区
   LOG_printf(DEBUG,"--------------------4.准备发送缓冲区-------------------\n");
    printCurrentTime();

    
    int recvBufSize;
    socklen_t optLen = sizeof(recvBufSize);
    // 获取当前的接收缓冲区大小
    if (getsockopt(((StandardSocket*)sock)->sock,SOL_SOCKET, SO_RCVBUF, &recvBufSize, &optLen) < 0) {
        perror("getsockopt failed");
        close(((StandardSocket*)sock)->sock);
        return ;
    }

    LOG_printf(DEBUG,"Current receive buffer size: %d\n", recvBufSize);

            // rrArgs->requestMsg 里面的值是在哪里初始化的 rrArgs->requestMsg->msgHeader 每个字段中的值都做了序列化
   unsigned sendBufLen;
               //sendBufLen = NetMessage_getMsgLength(rrArgs->requestMsg); // 这个也可以改成直接赋值
   sendBufLen = 150; //@@ 设置必须大于 111 
   // 这个也可以改成直接赋值 Ps: 获取sendbuflen 可以直接赋值，而且我看函数逻辑其实这里返回的是消息头部的长度 但是 这个getmessagelength有另外的功能是对头部信息序列化
                //NoAllocBufferStore* bufStore = app->msgBufStore;//初始化直接改成app->msgBuffStore的具体赋值
   NoAllocBufferStore* bufStore=(NoAllocBufferStore*)malloc(sizeof(NoAllocBufferStore));
   bufStore->numBufs = 10;
   bufStore->bufSize = 150; //@@ 40 就会消息太大（已经解决了） 设置必须大于 111 
   bufStore->numAvailable = 0;
   
   bufStore->bufArray = (char**)malloc(bufStore->numBufs * sizeof(char*));

   
   Condition_init(&bufStore->newBufCond);
   
   // pthread_mutex_lock(&bufStore->newBufCond.mutex);// 为什么这里要锁定 这里不能锁定要不然  NoAllocBufferStore_waitForBuf 这个函数就被阻塞了
                                                 // 根据 POSIX 标准，条件变量必须总是与互斥锁一起使用。在调用 pthread_cond_wait 或相关函数等待条件变量之前，线程必须已经持有互斥锁。当条件变量被信号唤醒时，互斥锁会被自动释放，允许等待的线程继续执行。

   for (size_t i = 0; i < bufStore->numBufs; ++i) {

         bufStore->bufArray[i] =(char*) malloc(bufStore->bufSize);

   }

   bufStore->numAvailable = bufStore->numBufs;

   Condition_signal(&bufStore->newBufCond);


   // 准备 消息接收缓冲区，outRespBuf
      unsigned bufLen;  
      MessagingTkBufType respBufType=MessagingTkBufType_BufStore;
      char* outRespBuf=nullptr;
         //if(rrArgs->respBufType == MessagingTkBufType_BufStore)//根据响应缓冲区类型，从缓冲区存储中获取或分配缓冲区，回溯前面的代码 确实响应缓冲区的类型是这个
      if(respBufType == MessagingTkBufType_BufStore)//根据响应缓冲区类型，从缓冲区存储中获取或分配缓冲区，回溯前面的代码 确实响应缓冲区的类型是这个
      { 
         // pre-alloc'ed buffer from store
            //NoAllocBufferStore* bufStore = app->msgBufStore;//初始化直接改成app->msgBuffStore的具体赋值
         

         bufLen = bufStore->bufSize;// 获取缓冲区长度

         if(bufLen < sendBufLen )// 如果缓冲区长度小于发送缓冲区长度
         { 
            LOG_printf(DEBUG,"buflen<sendBuflen\n");
         }
            // rrArgs->outRespBuf = NoAllocBufferStore_waitForBuf(bufStore);

         outRespBuf = NoAllocBufferStore_waitForBuf(bufStore);// 这个函数其实就是从 bufStore->bufArray[] 取上一个，我定义的时候是有10个256Byte的缓冲区

            //printf("Buffer received: %p\n", (void*)rrArgs->outRespBuf);  // 打印获取的缓冲区地址
         LOG_printf(DEBUG,"Buffer received: %p\n", (void*)outRespBuf);  // 打印获取的缓冲区地址

      }
   LOG_printf(DEBUG,"--------------------5.准备请求信息-------------------\n");
    printCurrentTime();
     // requestMsg 赋值
     OpenFileMsg requestMsg;
    
               //const NumNodeID localNodeNumID = Node_getNumID(App_getLocalNode(app) );
          // 第一个参数
         NumNodeID localNodeNumID;
         localNodeNumID.value=3;//假设本地client 节点 NodeID 为这个值 0415更新
                                             // 164 client NodeID=3  162 client NodeID=1 和 21
         
          // 第二个参数 
          EntryInfo entryInfo;
          EntryInfo_init(&entryInfo);
          //FhgfsOpsRemoting_openfile(const EntryInfo* entryInfo)

          // 第三个参数
          //... RemotingIOInfo* ioInfo) ioInfo->accessFlags
         unsigned accessFlags=accessFlagsInit();


          // requestMsg 第四个参数 
          FileEvent event;
               // FileEvent_init(&event, FileEventType_TRUNCATE, dentry);
         printCurrentTime();
         // 在这里设置要读取的文件名
         FileEvent_init(&event,FileEventType_READ, "image_data.csv");//第三个参数是 直接传相对挂载目录/mnt/beegfs/的相对路径 ，Note:好奇怪：前面的 entryID 和这个 路径不匹配 open 逻辑也是成功获取的
          if (event.path) {
            LOG_printf(DEBUG,"Relative path: %s\n", event.path);
         } else {
            LOG_printf(DEBUG,"Failed to get relative path\n");
         }
       printCurrentTime();
               //void OpenFileMsg_initFromSession(OpenFileMsg* openmsg,NumNodeID clientNumID, const EntryInfo* entryInfo, unsigned accessFlags,const struct FileEvent* fileEvent)
               //OpenFileMsg_initFromSession(&requestMsg, localNodeNumID, &entryInfo, ioInfo->accessFlags,&event);
     OpenFileMsg_initFromSession(&requestMsg, localNodeNumID, &entryInfo, accessFlags,&event);
   
      LOG(DEBUG)<<"msgHeader.msgLength = "<<((NetMessage*)&requestMsg)->msgHeader.msgLength<<"\n";//1729519185

            //   if(Config_getQuotaEnabled(cfg) )
            //    NetMessage_addMsgHeaderFeatureFlag((NetMessage*)&requestMsg, OPENFILEMSG_FLAG_USE_QUOTA);
        
  

   LOG_printf(DEBUG,"--------------------6.请求消息序列化-------------------\n");

      //5. 序列化
      //在网络通信中，序列化用于将消息转换为字节流，以便在网络上发送。将rrArgs->requestMsg 序列化后存放在rrArgs->outRespBuf
            // NetMessage_serialize(rrArgs->requestMsg, rrArgs->outRespBuf, sendBufLen); //往缓冲区中放请求的信息
      printCurrentTime();
      NetMessage_serialize((NetMessage*)&requestMsg, outRespBuf, sendBufLen); //往缓冲区中放请求的信息
      LOG(DEBUG)<<"NetMessage_getMsgLength(netM)="<<NetMessage_getMsgLength((NetMessage*)&requestMsg)<<"\n";
      // sendBufLen=NetMessage_getMsgLength((NetMessage*)&requestMsg);
      LOG(DEBUG)<<"msgHeader.msgLength = "<<((NetMessage*)&requestMsg)->msgHeader.msgLength<<"\n";//1729519185
      // if (outRespBuf == NULL) {
      //   LOG_printf(DEBUG,"缓冲区为空或未初始化\n");
      // } 
      // if (strlen(outRespBuf) == 0) {
      //       LOG_printf(DEBUG,"outRespBuf  缓冲区为空，没有数据 \n");
      // }
      // else
      // {
      //    LOG_printf(DEBUG,"outRespBuf  缓冲区不为空，有数据，%d 数据长度 %ld   %s\n",strlen(outRespBuf),sizeof(outRespBuf),outRespBuf);
      // }

      //6. send request
            // sendRes = Socket_send_kernel(sock, rrArgs->outRespBuf, sendBufLen, 0);
            //struct iov_iter *iter = STACK_ALLOC_BEEGFS_ITER_KVEC(rrArgs->outRespBuf, sendBufLen, WRITE);
            if(sendBufLen<=0)
            {
                        LOG_printf(DEBUG,"6. send request error \n");
            }
            else
             LOG_printf(DEBUG,"send request len is %d\n",sendBufLen); // 输出 是 1024

      struct iovec  *iter = STACK_ALLOC_BEEGFS_ITER_KVEC(outRespBuf, sendBufLen);




      // ssize_t sentBytes=send(((StandardSocket*)sock)->sock, message, strlen(message), 0);
      //   if (sentBytes < 0) {
      //   // 发送失败，可以打印错误信息
      //   perror("Send failed");
      //   close(((StandardSocket*)sock)->sock);
      //       return ;
      //    } else if (static_cast<size_t>(sentBytes) != strlen(message)) {
      //       // 数据没有完全发送
      //       LOG_printf(DEBUG,"Only %zd bytes of %zu bytes were sent\n", sentBytes, strlen(message));
      //    } else {
      //       // 数据发送成功
      //       LOG_printf(DEBUG,"-----Hello 2, server 发送信息\n");
      //    }
         

      int iovcnt=1;
      fhgfs_sockaddr_in *to=nullptr;
      printCurrentTime();


      sendRes=sock->ops->sendto(sock, iter,  iovcnt,0, to);//$$有好多实现形式
            LOG_printf(DEBUG,"-------6 -4 -----------------\n");



      //   sentBytes=send(((StandardSocket*)sock)->sock, message, strlen(message), 0);
      //   if (sentBytes < 0) {
      //   // 发送失败，可以打印错误信息
      //    perror("Send failed");
      //    close(((StandardSocket*)sock)->sock);
      //          return ;
      //       } else if (static_cast<size_t>(sentBytes) != strlen(message)) {
      //          // 数据没有完全发送
      //          LOG_printf(DEBUG,"Only %zd bytes of %zu bytes were sent\n", sentBytes, strlen(message));
      //       } else {
      //          // 数据发送成功
      //          LOG_printf(DEBUG,"-----Hello 3, server 发送信息\n");
      //       }


      
      if(sendRes != (ssize_t)sendBufLen)
         {
            LOG_printf(DEBUG,"sendRes != sendBufLen \n");
         }
   
   LOG_printf(DEBUG,"--------------------7. 接收消息-------------------\n");
       printCurrentTime();
      //7. receive response
                // respRes = MessagingTk_recvMsgBuf(app, sock, rrArgs->outRespBuf, bufLen);
      respRes = MessagingTk_recvMsgBuf(sock, outRespBuf, bufLen);

      if(respRes <= 0) 
      { 
         LOG_printf(DEBUG,"respRes<=0 \n");
      }
   
   LOG_printf(DEBUG,"--------------------8. 反序列化解析-------------------\n");
      
      //8. 反序列化解析
            // rrArgs->outRespMsg = NetMessageFactory_createFromBuf(app, rrArgs->outRespBuf, respRes);
       // std::unique_ptr<NetMessage> outRespMsg;
         //NetMessage* outRespMsg=new NetMessage();
      NetMessage* outRespMsg = NetMessageFactory_createFromBuf(outRespBuf, respRes);
      
      if (outRespMsg->msgHeader.msgType == NETMSGTYPE_AckNotifyResp)
      {
         LOG_printf(DEBUG,"错误处理1 \n");
      }
      if(NetMessage_getMsgType(outRespMsg) == NETMSGTYPE_GenericResponse)
      {  
               // special control msg received
               // retVal = __MessagingTk_handleGenericResponse(app, rrArgs, group, wasIndirectCommErr);
               // if(retVal != FhgfsOpsErr_INTERNAL)
               // { // we can re-use the connection
               //    if (releaseSock)
               //       NodeConnPool_releaseStreamSocket(connPool, sock);
               //    goto cleanup_no_socket;
               // }
               // goto socket_invalidate;

         LOG_printf(DEBUG,"错误处理2 \n");
      }
      LOG_printf(DEBUG,"Correct response  \n");
      LOG(DEBUG)<<"outRespMsg->msgHeader.msgType = "<<outRespMsg->msgHeader.msgType<<"\n";
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
   const PathInfo* msgPathInfoPtr;

      const char*  fileHandleID = StringTk_strDup(openResp->fileHandleID ); //StringTk_strDup(OpenFileRespMsg_getFileHandleID(openResp) );

         filehandleID_xyp=(char*)malloc(strlen(fileHandleID) + 5); // 分配足够的内存来存储字符串及其终止符'\0'
         if (filehandleID_xyp != NULL) {
               strcpy(filehandleID_xyp, fileHandleID); // 复制字符串
               // 现在 filehandleID_xyp 指向一个可以修改的字符串副本
               LOG(DEBUG)<<"file_handleID_xyp = "<<filehandleID_xyp<<"\n";
         }
        LOG(DEBUG)<<"openResp->fileHandleID = "<<openResp->fileHandleID<<"\n";
        LOG(DEBUG)<<"openResp->pathInfo (1) flags = "<<openResp->pathInfo.flags<<"\n";
        LOG(DEBUG)<<"openResp->pathInfo (2) _flags = "<<openResp->pathInfo._flags<<"\n"; 
        LOG(DEBUG)<<"openResp->pathInfo (3) origParentUID = "<<openResp->pathInfo.origParentUID<<"\n";
        LOG(DEBUG)<<"openResp->pathInfo (4) _origParentUID = "<<openResp->pathInfo._origParentUID<<"\n";
        LOG(DEBUG)<<"openResp->pathInfo (5) origParentEntryID = "<<openResp->pathInfo.origParentEntryID<<"\n";
        LOG(DEBUG)<<"openResp->pathInfo (6) _origParentEntryID ="<<openResp->pathInfo._origParentEntryID<<"\n";



     delete iter; 
   // 释放套接字
   NodeConnPool_releaseStreamSocket(sock);
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
    
   // 7. 后续处理

   /*
      bool releaseSock = sock == NULL;
      if(unlikely(rrArgs->outRespMsg->msgHeader.msgType== NETMSGTYPE_GenericResponse) )
      { // special control msg received
         retVal = __MessagingTk_handleGenericResponse(app, rrArgs, group, wasIndirectCommErr);
         if(retVal != FhgfsOpsErr_INTERNAL)
         { // we can re-use the connection
            if (releaseSock)
               NodeConnPool_releaseStreamSocket(connPool, sock);
            goto cleanup_no_socket;
         }

         goto socket_invalidate;
      }

      if(unlikely(NetMessage_getMsgType(rrArgs->outRespMsg) != rrArgs->respMsgType) )
      { 
         
         goto socket_invalidate;
      }

      // correct response => return it (through rrArgs)

      if (releaseSock)
         NodeConnPool_releaseStreamSocket(connPool, sock);

      LOG_printf(DEBUG,"Success!! \n");
      //    return FhgfsOpsErr_SUCCESS;

      // 下面是错误处理
      // error handling (something went wrong)...

      socket_exception:
      {
         if(!(rrArgs->logFlags & REQUESTRESPONSEARGS_LOGFLAG_COMMERR) )
         {
            
            rrArgs->logFlags |= REQUESTRESPONSEARGS_LOGFLAG_COMMERR;
         }
      }
      
      socket_invalidate:
         if (releaseSock)
         {
            NodeConnPool_invalidateStreamSocket(connPool, sock);
         }

      // clean up
      //××
      cleanup_no_socket:

         if(rrArgs->outRespMsg)
         {
            NETMESSAGE_FREE(rrArgs->outRespMsg);
            rrArgs->outRespMsg = NULL;
         }

         if(rrArgs->outRespBuf)
         {
            if(rrArgs->respBufType == MessagingTkBufType_BufStore)
            {
               NoAllocBufferStore* bufStore = App_getMsgBufStore(app);
               NoAllocBufferStore_addBuf(bufStore, rrArgs->outRespBuf);
            }
            else
               kfree(rrArgs->outRespBuf);

            rrArgs->outRespBuf = NULL;
         }

      return retVal;
   */
}


// XYP_INsert_250301
void AuthenticateChannelMsg_generate(char* sendBuf,size_t* sendBufLen)
{

   uint64_t authHash = 10322703647856944136;
   //xfusion3: 574355913317798958
   // 162 server: 10322703647856944136

   if(authHash) 
   { // authenticate channel
      // char* sendBuf;
      
      AuthenticateChannelMsg authMsg;
      size_t sendRes;

      AuthenticateChannelMsg_initFromValue(&authMsg, authHash); //
      *sendBufLen = NetMessage_getMsgLength( (NetMessage*)&authMsg);
   
     

      NetMessage_serialize( (NetMessage*)&authMsg, sendBuf, *sendBufLen);
      LOG_printf(DEBUG,"[AuthenticateChannelMsg_generate] send buffer = %s \t Total msg len is %lu\n",sendBuf,*sendBufLen);

   }
   else
   {
     LOG_printf(DEBUG,"[AuthenticateChannelMsg_generate] authHash is 0 \n");
   }

   return ;
}


#endif