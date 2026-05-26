//PS： 250220 latest version

## 250220 参数修改指南



### 更换文件读的参数修改步骤
```c
1. open.hpp中
{
    EntryInfo_init(&entryInfo);
        {   
            
            const char* parentEntryID ="0-67B6EDE2-A2";   //@@ 父目录的唯一ID //获取指令： sudo beegfs-ctl --getentryinfo /mnt/beegfs/test_250220 ; 输出提示 EntryID: xx 即为我们所需的参数
            const char* entryID = "0-67B6EE21-A2";       //@@ 文件/目录的ID //获取指令： sudo beegfs-ctl --getentryinfo image_data.csv ; 输出提示 EntryID: xx 即为我们所需的参数
            //0-672780DE-BE
            const char* dirName = StringTk_strDup("test_250220");  //@@  这个是目录名字
        }
    FileEvent_init(&event,FileEventType_READ, "image_data.csv");//@@ PS:这个参数实际上服务端并不处理，填或不填都可以，第三个参数 直接是目标文件的路径 ，相对于元数据挂载目录的相对路径； 直接传相对挂载目录/mnt/beegfs/的相对路径
    
}
2. new_read.hpp
{
    PathInfo_init{
        // 设置origParentUID或origParentEntryID，
            pathInfo->origParentUID = 29999;//@@ 父目录UID // 获取指令 stat /mnt/beegfs/test_250220 ;输出提示 Uid：(29999/     oem) 则 29999即为我们所需
            pathInfo->_origParentUID = 29999 ;//@@ 同上
            pathInfo->origParentEntryID = "0-67B6EDE2-A2";//@@ 父目录的唯一ID //获取指令： sudo beegfs-ctl --getentryinfo /mnt/beegfs/test_250220 ; 输出提示 EntryID: xx 即为我们所需的参数PS：root 是 根目录的entryid
            
            pathInfo->_origParentEntryID = "0-67B6EDE2-A2";//@@ 同上
    }
}
3. read.cpp
{
    //指定文件读取大小
    ssize_t FhgfsOpsRemoting_readfileVec()
    {
    size_t toBeRead=;//10*1024;// 1.3MB //这两个参数是VFS文件系统入口函数传递的，我自己制定的  todo
        
    }

}
```
### RDMA_read 文件 参数设置



#### 1. open 与元数据节点的通信设置

```c
//元数据节点通信的信息设置
// open 逻辑，需要设置的信息参数 -----------------------------------------------------------

1. open.hpp 
__MessagingTk_requestResponseWithRRArgsComm()
{
     printf("------------------- 3. 尝试通过IP连接-------------------\n");
      unsigned short port=8005;//@@ 元数据服务的UDP端口 //获取指令： sudo beegfs-ctl --listnodes --nodetype=meta --nicdetails ;

      initNicAddressArray_meta(nicAddr);//设置可连接的TCP 网口IP

      connectRes = sock->ops->connectByIP(sock, nicAddr[4].ipAddr, port);//@@ 通过这个参数nicAddr[4].ipAddr， 选择哪个网口
                                                                            //通过数组下标选定，nicAddr[4].ipAddr= "10.26.42.230";
                                                                            //ifconfig 查看网卡对应信息
                                                                        
}

2. open.hpp
 __MessagingTk_requestResponseWithRRArgsComm()
{
        // 第一个参数
        NumNodeID localNodeNumID;
        localNodeNumID.value=5;//@@ client NodeID //获取指令： sudo beegfs-ctl --listnodes --nodetype=client ;ex 输出
                                                //     128A1B-67B56FCE-node9 [ID: 5] //164client
                                                // 182CF0-67B57038-ubuntu-PowerEdge-R750xa [ID: 6] //162client
        
        // 第二个参数 
        EntryInfo_init(&entryInfo);
        {   
            metaOwnerInit(&rootOwner);
            {
                // 初始化为节点
                memset(rootOwner, 0, sizeof(NodeOrGroup));
                rootOwner->isGroup=false;//@@ 元数据节点特性，默认是false
                rootOwner->node.value=162;//@@ 元数据节点ID //获取指令： sudo beegfs-ctl --listnodes --nodetype=meta ;ex 输出
                                                            // ubuntu-PowerEdge-R750xa [ID: 162]
            }
            const char* parentEntryID ="0-67B6EDE2-A2";   //@@ 父目录的唯一ID //获取指令： sudo beegfs-ctl --getentryinfo /mnt/beegfs/test_250220 ; 输出提示 EntryID: xx 即为我们所需的参数
            const char* entryID = "0-67B6EE21-A2";       //@@ 文件/目录的ID //获取指令： sudo beegfs-ctl --getentryinfo image_data.csv ; 输出提示 EntryID: xx 即为我们所需的参数
            //0-672780DE-BE
            const char* dirName = StringTk_strDup("test_250220");  //@@  这个是目录名字
            DirEntryType entryType = (DirEntryType) DirEntryType_REGULARFILE;//@@表示类型是文件还是目录，DirEntryType_DIRECTORY;

        }

        FileEvent_init(&event,FileEventType_READ, "image_data.csv");//@@ PS:这个参数实际上服务端并不处理，填或不填都可以，第三个参数 直接是目标文件的路径 ，相对于元数据挂载目录的相对路径； 直接传相对挂载目录/mnt/beegfs/的相对路径
         
}
3. open.hpp 
// 这是 服务端的认证配置
__NodeConnPool_applySocketOptionsConnected(sock);
{
        uint64_t authHash = 10322703647856944136;//@@ 服务认证的哈希值，获取方式详见下文
                                            // 224 server 上这个 610185155436495494 ，可能并非最新的
                                            // 162 server: 10322703647856944136 ，可能并非最新的
        
}
```

##### 服务认证的哈希值获取方式

1. 运行脚本指令
```sh
# 进入 hash_value 计算程序目录 这是164上的例子，<162beegfs-server 164beegfs-client> 两台服务器上的 connAuthfile 配置文件是相同的，即拥有相同的配置文件方可进行通信，因为要保证从配置文件计算出的哈希值是一致的方可通信
cd /home/oem/xyp/A_Client_on_GPU/connAuthfile 
make
sudo ./hash /etc/beegfs/connauthfile
```
输出显示 ： `Authentication hash: 10322703647856944136` 即为我们所需的哈希值
2. 也可以直接运行该程序`hash.c` ,其会计算配置文件 `/etc/beegfs/connauthfile` 对应的 64bit哈希值 ，代码如下
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

// Hsieh hash function
uint32_t HashTk_HsiehHash32(const char* data, int len) {
    uint32_t hash = len, tmp;
    int rem;

    if (len <= 0 || data == NULL) {
        return 0;
    }

    rem = len & 3;
    len >>= 2;

    for (; len > 0; len--) {
        hash += (uint32_t)(*(const uint16_t*)data);
        tmp = ((uint32_t)(*(const uint16_t*)(data + 2)) << 11) ^ hash;
        hash = (hash << 16) ^ tmp;
        data += 2 * sizeof(uint16_t);
        hash += hash >> 11;
    }

    switch (rem) {
        case 3:
            hash += (uint32_t)(*(const uint16_t*)data);
            hash ^= hash << 16;
            hash ^= (uint32_t)data[sizeof(uint16_t)] << 18;
            hash += hash >> 11;
            break;
        case 2:
            hash += (uint32_t)(*(const uint16_t*)data);
            hash ^= hash << 11;
            hash += hash >> 17;
            break;
        case 1:
            hash += (uint32_t)(*data);
            hash ^= hash << 10;
            hash += hash >> 1;
    }

    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

// #define HASHTK_HSIEHHASH32 0
#define HashTkDefaultHash HASHTK_HSIEHHASH32
enum HashTkHashTypes;
typedef enum HashTkHashTypes HashTkHashTypes;
enum HashTkHashTypes
{
   HASHTK_HSIEHHASH32      = 0,
   HASHTK_HALFMD4,         // as used by ext4
};
#define CONFIG_AUTHFILE_READSIZE 1024
#define CONFIG_AUTHFILE_MINSIZE 16



// Generic hash function wrapper
uint32_t HashTk_hash32(HashTkHashTypes hashType, const char* data, int len) {
    switch (hashType) {
        default:
            printf("Unknown hash type: %d\n", hashType);
            hashType = HashTkDefaultHash;
        case HASHTK_HSIEHHASH32:
            printf("Using Hsieh hash\n");
            return HashTk_HsiehHash32(data, len);
    }
}

int __Config_initConnAuthHash(char* connAuthFile, uint64_t* outConnAuthHash) {
    int fileHandle;
    char* buf;
    ssize_t readRes;

    if (!connAuthFile || strlen(connAuthFile) == 0) {
        printf("No connAuthFile configured. Using BeeGFS without connection authentication is insecure.\n");
        return 0;
    }

    fileHandle = open(connAuthFile, O_RDONLY);
    if (fileHandle < 0) {
        perror("Failed to open auth file");
        return 0;
    }

    buf = malloc(CONFIG_AUTHFILE_READSIZE);
    if (!buf) {
        perror("Failed to alloc mem for auth file reading");
        close(fileHandle);
        return 0;
    }

    readRes = read(fileHandle, buf, CONFIG_AUTHFILE_READSIZE);
    if (readRes < 0) {
        perror("Unable to read auth file");
        free(buf);
        close(fileHandle);
        return 0;
    }

    if (!readRes || (readRes < CONFIG_AUTHFILE_MINSIZE)) {
        fprintf(stderr, "Auth file is empty or too small: '%s'\n", connAuthFile);
        free(buf);
        close(fileHandle);
        return 0;
    }

    int len1stHalf = readRes / 2;
    int len2ndHalf = readRes - len1stHalf;

    uint32_t high = HashTk_hash32(HASHTK_HSIEHHASH32, buf, len1stHalf);
    uint32_t low = HashTk_hash32(HASHTK_HSIEHHASH32, &buf[len1stHalf], len2ndHalf);

    *outConnAuthHash = ((uint64_t)high << 32) | low;

    free(buf);
    close(fileHandle);

    return 1;
}

#define CONFIG_AUTHFILE_READSIZE 1024
#define CONFIG_AUTHFILE_MINSIZE 16
#define HASHTK_HSIEHHASH32 0
#define HashTkDefaultHash HASHTK_HSIEHHASH32

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <connAuthFile>\n", argv[0]);
        return 1;
    }

    uint64_t authHash;
    if (__Config_initConnAuthHash(argv[1], &authHash)) {
        printf("Authentication hash: %lu\n", authHash);
    } else {
        fprintf(stderr, "Failed to initialize connection authentication hash.\n");
        return 1;
    }

    return 0;
}

```

PS: 编译`hash.c`的 `Makefile` 文件
```sh
# 定义编译器
CC = gcc

# 定义编译选项，这里添加了-Wall以显示所有警告
CFLAGS = -Wall

# 定义链接选项
LDFLAGS = 

# 定义源文件
SRC = hash.c

# 定义目标文件
OBJ = $(SRC:.c=.o)

# 定义可执行文件名
TARGET = hash

# 默认目标
all: $(TARGET)

# 链接目标
$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

# 编译源文件
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理编译生成的文件
clean:
	rm -f $(OBJ) $(TARGET)

# 伪目标，确保phony目标在多job并行make中不会产生错误
.PHONY: all clean
```


#### 2. read 与存储节点的通信设置
```c
// 通信流程梳理 ，重点记录一些需要的参数设置 read 

1. new_read.cpp: ioinfo 初始化 中需要 文件句柄ID 以及 pathinfo 
        initRemotingIOInfo ()
        {
                info->fileHandleID = nullptr;//@@ 不用指定，会自动通过open 函数中，向存储节点请求获取对应的句柄
                info->accessFlags = 1; //@@ 表示read模式 #define OPENFILE_ACCESS_READ           1
                PathInfo pathinfo;
                PathInfo_init(&pathinfo);
                {
                    // PS： 六个参数赋值 ，赋值的值需要和open 获取的pathinfo 信息一致
                        pathInfo->_flags = 1;   //默认为1,不用修改，PATHINFO_FEATURE_ORIG;
                        pathInfo->flags = 1;    //默认为1,不用修改，PATHINFO_FEATURE_ORIG;
                    
                    // 设置origParentUID或origParentEntryID，
                        pathInfo->origParentUID = 29999;//@@ 父目录UID // 获取指令 stat /mnt/beegfs/test_250220 ;输出提示 Uid：(29999/     oem) 则 29999即为我们所需
                        pathInfo->_origParentUID = 29999 ;//@@ 同上

                        pathInfo->origParentEntryID = "0-67B6EDE2-A2";//@@ 父目录的唯一ID //获取指令： sudo beegfs-ctl --getentryinfo /mnt/beegfs/test_250220 ; 输出提示 EntryID: xx 即为我们所需的参数PS：root 是 根目录的entryid
                        pathInfo->_origParentEntryID = "0-67B6EDE2-A2";//@@ 同上    
                }
        }
2. new_read.cpp:  
    Raid0_StripePattern_init()
    {
        int targetnum=1;//@@ 存储目标有几个，默认是 1个 //查询指令： sudo beegfs-ctl --listnodes --nodetype=storage ; 输出有几个条目就是几个
        //根据存储目标的数量，填充 targetNumID 数组 ，将存储目标的ID 一一填充到该数组
         targetNumID[0]=61;//@@ 存储 Target ID //获取指令： sudo beegfs-ctl --listtargets --longnodes ; ex 输出 :
                                        //TargetID   NodeID
                                        //========   ======
                                        //61   beegfs-storage ubuntu-PowerEdge-R750xa [ID: 6]  
        targetNumID[1]=4;//@@
    }

3. new_read.hpp 
NodeStoreEx_referenceNodeByTargetID () //这个函数需要初始化 存储节点  Node 节点的信息
{
     //连targetid 都不用传
        NumNodeID nodeID;
        nodeID.value=6;//@@ 存储Node_ID //获取指令： sudo beegfs-ctl --listnodes --nodetype=storage ; ex 输出 : ubuntu-PowerEdge-R750xa [ID: 6] 
        // 假设的节点类型、ID 和端口
            NodeType nodeType = NODETYPE_Storage;
            const char* id = "ubuntu-PowerEdge-R750xa";//@@Node_name //获取指令： sudo beegfs-ctl --listnodes --nodetype=storage ; ex 输出 : ubuntu-PowerEdge-R750xa [ID: 6] ，
            uint16_t nodenumID = 6; //@@ 存储Node_ID //获取指令： sudo beegfs-ctl --listnodes --nodetype=storage ; ex 输出 : ubuntu-PowerEdge-R750xa [ID: 6] 
            unsigned short portUDP = 8003;//@@ 存储服务的UDP端口 //获取指令： sudo beegfs-ctl --listnodes --nodetype=storage --nicdetails ;
}

//RDMA Read 修改参数步骤
4. new_read.hpp中
__commkit_prepare_generic()
{   
    printf("--------- 1.创建 socket -------------\n");
    const char* LocalRdmaIP = "192.168.5.209";       //@@本地客户端的RDMA IP,   网口名为 enp130s0np0
                                                   // dell01 的RDMA IP : 192.168.0.203 ,   网口名为 enp152s0np0
                                                   // 164 oem: ens5f0np0 192.168.5.209 mlx5_0 ; ens4f0np0 192.168.3.212 mlx5_2
    
    strncpy(nicAddr->name, "ens5f0np0", IFNAMSIZ);  //@@本地客户端 RDMA 网口名 
                                                        //164 oem : ens5f0np0/ens4f0np0  ;dell02 : enp130s0np0  ; dell 01: enp152s0np0

    ...
    printf("------------------- 2. 尝试通过IP连接-------------------\n");

    const char* RemoteRdmaIP = "192.168.5.210";   //@@存储服务端的RDMA IP   
                                                     // 162 ubuntu 的 RDMA IP : 192.168.5.210 , ens4f0np0
                                                    //存储服务器xfusion3的RDMA IP,网口名 ens2np0

    unsigned short port = 8003;    //存储服务器的port
}
5. new_read.hpp
__commkit_readfile_prepareHeader()
{
    NumNodeID localNodeNumID ;
    localNodeNumID.value=5; //@@ client Node ID//获取指令： sudo beegfs-ctl --listnodes --nodetype=client ;ex 输出
                                                //     128A1B-67B56FCE-node9 [ID: 5] //164client
                                                // 182CF0-67B57038-ubuntu-PowerEdge-R750xa [ID: 6] //162client
}
6. read.cpp

ssize_t FhgfsOpsRemoting_readfileVec()
{
    size_t toBeRead=1024;//@@ 指定文件读取大小 单位是B ,toBeRead= 1024表示 1KB
    
}


```



### Readdir TCP 的参数设置
```c
// readdir 需要修改的参数 ，和open类似，查询不同目录的时候需要修改下面的参数 -----------------------------------------------------------
    //我发现只要提供父目录ID 和 目标目录ID 即可查询成功
 __MessagingTk_requestResponseWithRRArgsComm()
{
    EntryInfo_init{
        
        const char* parentEntryID ="0-6736A39F-1";//"root";//@@ StringTk_strDup("");//它是一个简单的字符串复制函数。
        const char* entryID ="2-6736A39F-1";//@@  
        const char* dirName = StringTk_strDup("dir111");//@@ 目录或是文件名字 test_xyp.txt image_data.csv
    }

}
```


### 相关的指令，还没整理好，比较杂乱
```sh
//记录下元数据通信 ，确认通信所需数据的过程
// 服务端口 
1. cat /etc/beegfs/beegfs-meta.conf | grep -e 'connMetaPortTCP' -e 'connMetaPortUDP'
{
    connMetaPortTCP              = 8005
    connMetaPortUDP              = 8005

    //其他服务的监听端口
    connMgmtdPortTCP             = 8008
    connMgmtdPortUDP             = 8008
    //存储服务
    connStoragePortTCP           = 8003
    connStoragePortUDP           = 8003


}
//元数据的网卡配置 
2. sudo beegfs-ctl --listnodes --nodetype=meta --nicdetails
{
    
    xfusion3 [ID: 190] //IP 224
        Ports: UDP: 8005; TCP: 8005
        Interfaces: 
        + ens2np0[ip addr: 192.168.0.218; type: RDMA]
        + ens2np0[ip addr: 192.168.0.218; type: TCP]
        + ztstkzavtz[ip addr: 10.50.109.190; type: TCP]
        + ens65f1[ip addr: 10.26.42.224; type: TCP]
    dell01 [ID: 191]
        Ports: UDP: 8005; TCP: 8005
        Interfaces: 
        + enp152s0np0[ip addr: 192.168.0.203; type: RDMA]
        + br-3eb3c5932eb8[ip addr: 192.168.49.1; type: TCP]
        + enp152s0np0[ip addr: 192.168.0.203; type: TCP]
        + br-3c5d4bcb4c74[ip addr: 172.20.0.1; type: TCP]
        + br-9fc2d0c06cfc[ip addr: 172.18.0.1; type: TCP]
        + docker0[ip addr: 172.17.0.1; type: TCP]
        + br-ursa[ip addr: 10.26.42.217; type: TCP]

    Number of nodes: 2
    Root: 1
}

//存储服务器 的网卡配置
3. (base) root@xfusion3:/home/xyp/beegfs/cap# sudo beegfs-ctl --listnodes --nodetype=meta --nicdetails
{
    xfusion3 [ID: 190]
  {
        Ports: UDP: 8005; TCP: 8005
        Interfaces: 
        + ens2np0[ip addr: 192.168.0.218; type: RDMA]
        + ens2np0[ip addr: 192.168.0.218; type: TCP]
        + ztstkzavtz[ip addr: 10.50.109.190; type: TCP]
        + ens65f1[ip addr: 10.26.42.224; type: TCP] 
  }
dell01 [ID: 191]
  {
        Ports: UDP: 8005; TCP: 8005
    Interfaces: 
    + enp152s0np0[ip addr: 192.168.0.203; type: RDMA]
    + br-3eb3c5932eb8[ip addr: 192.168.49.1; type: TCP]
    + enp152s0np0[ip addr: 192.168.0.203; type: TCP]
    + br-f54ca3174bd7[ip addr: 172.20.0.1; type: TCP]
    + br-9fc2d0c06cfc[ip addr: 172.18.0.1; type: TCP]
    + br-ursa[ip addr: 10.26.42.217; type: TCP]
  }

Number of nodes: 2
Root: 1
}




//存储节点ID
sudo beegfs-ctl --listtargets --longnodes
{
    TargetID   NodeID
    ========   ======
    1911   beegfs-storage dell01 [ID: 191]
}
//client 节点 ID
root@dell01:/mnt/beegfs# sudo beegfs-ctl --listnodes --nodetype=client
{
    35E549-67248D80-dell01 [ID: 5]
    1ADB7C-67248D99-xfusion3 [ID: 6]
    //chong xin peizhi hou 
    35E549-67248D80-dell01 [ID: 5]
}
//MetaData 节点 ID
root@dell01:/mnt/beegfs# sudo beegfs-ctl --listnodes --nodetype=meta
{
    xfusion3 [ID: 190]
    dell01 [ID: 191]
}
//获取 文件 entryID
root@dell01:/mnt/beegfs# beegfs-ctl --getentryinfo test_xyp.txt 
{
    Entry type: file
    EntryID: 0-6725E8D9-BE
    Metadata buddy group: 1
    Current primary metadata node: xfusion3 [ID: 190]
    Stripe pattern details:
    + Type: RAID0
    + Chunksize: 512K
    + Number of storage targets: desired: 4; actual: 1
    + Storage targets:
    + 1911 @ dell01 [ID: 191]
}

{
    root@dell01:/mnt/beegfs# beegfs-ctl --getentryinfo dd-test/test-file
{
    Entry type: file
    EntryID: 0-67248BF0-BF
    Metadata node: dell01 [ID: 191]
    Stripe pattern details:
    + Type: RAID0
    + Chunksize: 512K
    + Number of storage targets: desired: 4; actual: 1
    + Storage targets:
    + 1911 @ dell01 [ID: 191]
}

root@dell01:/mnt/beegfs#  beegfs-ctl --getentryinfo test2_xyp.txt 
{
    Entry type: file
    EntryID: 0-672780DE-BE
    Metadata buddy group: 1
    Current primary metadata node: xfusion3 [ID: 190]
    Stripe pattern details:
    + Type: RAID0
    + Chunksize: 512K
    + Number of storage targets: desired: 4; actual: 1
    + Storage targets:
    + 1911 @ dell01 [ID: 191]
}
buddymirror/dentries/38/51/ 
0-67277CC1-BE  0-672780DE-BE

root@dell01:/mnt/beegfs# beegfs-ctl --getentryinfo dd-test/test-file2
{
    Entry type: file
    EntryID: 0-67248DF0-BF
    Metadata node: dell01 [ID: 191]
    Stripe pattern details:
    + Type: RAID0
    + Chunksize: 512K
    + Number of storage targets: desired: 4; actual: 1
    + Storage targets:
    + 1911 @ dell01 [ID: 191]
}
}


(base) root@xfusion3:/home/xyp/beegfs/connauthfile# ./hash /etc/beegfs/connauthfile 
{
    Using Hsieh hash
    Using Hsieh hash
    Authentication hash: 610185155436495494
}
//监听网卡
// 224 xfusion 

sudo tcpdump -i ens65f1 -w $(date +%Y%m%d_%H%M%S)_80.pcap port 8005
// 217 dell01 
sudo tcpdump -i br-ursa -w $(date +%Y%m%d_%H%M%S)_80.pcap port 8003

// 162 server

sudo tcpdump -i eno8403 -w $(date +%Y%m%d_%H%M%S)_80.pcap port 8003

{

}

//mirror buddy 
struct MirrorBuddyGroup
{
   uint16_t firstTargetID = 0;
   uint16_t secondTargetID = 0;

   MirrorBuddyGroup() = default;

   MirrorBuddyGroup(uint16_t firstTargetID, uint16_t secondTargetID):
      firstTargetID(firstTargetID), secondTargetID(secondTargetID)
   {
   }

   template<typename This, typename Ctx>
   static void serialize(This* obj, Ctx& ctx)
   {
      ctx
         % obj->firstTargetID
         % obj->secondTargetID;
   }
};


struct PathInfo
{
   union
   {
         const unsigned flags;          // additional flags (e.g. PATHINFO_FEATURE_INLINED)
         unsigned _flags;
   };

   union
   {
         const unsigned origParentUID; // UID who created the file, only set for FileInodes
         unsigned _origParentUID;
   };

   union
   {
         // ID of the dir in which the file was created in. Only set for FileInodes
         char const* const origParentEntryID;
         char* _origParentEntryID;
   };
};

(base) root@xfusion3:/mnt/beegfs# stat test_xyp.txt 
{
    File: test_xyp.txt
    Size: 0               Blocks: 0          IO Block: 524288 regular empty file
    Device: 3bh/59d Inode: 16071737167219738944  Links: 1
    Access: (0644/-rw-r--r--)  Uid: (    0/    root)   Gid: ( 1011/ UNKNOWN)
    Access: 2024-11-02 08:55:44.000000000 +0000
    Modify: 2024-11-02 08:55:44.000000000 +0000
    Change: 2024-11-02 08:55:45.000000000 +0000
    Birth: -
}

(base) root@xfusion3:/mnt# stat beegfs
{
    File: beegfs
    Size: 2               Blocks: 1          IO Block: 524288 directory
    Device: 3bh/59d Inode: 2           Links: 3
    Access: (0777/drwxrwxrwx)  Uid: (    0/    root)   Gid: (    0/    root)
    Access: 2024-10-22 05:18:28.000000000 +0000
    Modify: 2024-11-02 08:55:44.000000000 +0000
    Change: 2024-11-02 08:55:44.000000000 +0000
    Birth: -
}

sudo beegfs-ctl --getentryinfo test_wsy.txt 
{
    [sudo] password for xyp: 
    Entry type: file
    EntryID: 3-672833A3-BE
    Metadata node: xfusion3 [ID: 190]
    Stripe pattern details:
    + Type: RAID0
    + Chunksize: 512K
    + Number of storage targets: desired: 4; actual: 1
    + Storage targets:
    + 1911 @ dell01 [ID: 191]
}
```



