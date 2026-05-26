## 250306 对ReadLocalRDMAMSG 解析
- 1. 从 wireshark 选中 payload 右键 copy ==> as Escaped String 选这个
- 2. 按照如下顺序依次解析
```sh
"\xa6\x00\x00\x00"                  \unsigned       msgLength;
"\x00\x00"                          \uint16_t       msgFeatureFlags;
"\x00"                              \uint8_t        msgCompatFeatureFlags;
"\x00"                              \uint8_t        msgFlags;
"\x00\x00\x00\x00\x53\x46\x47\x42"  \char*          msgPrefix; 
"\xdf\x0b"                          \unsigned short msgType  //17-18B 序号从1开始是消息类型
"\x51\x00"                          \uint16_t       msgTargetID; 
"\x00\x00\x00\x00"                  \unsigned       msgUserID;
"\x00\x00\x00\x00\x00\x00\x00\x00"  \uint64_t       msgSequence;
"\x00\x00\x00\x00\x00\x00\x00\x00"  \uint64_t       msgSequenceDone;// 前 40 B 
"\x00\x00\x00\x00\x00\x00\x00\x00"  \int64_t         offset; =0
"\x14\x00\x00\x00\x00\x00\x00\x00"  \int64_t count; =20
"\x01\x00\x00\x00"                  \unsigned accessFlags; =1
"\x15\x00\x00\x00"                  \NumNodeID clientNumID;=21 // 前 64 B 
"\x34\x46\x38\x33\x44\x33\x44\x30\x23\x31\x2d\x36\x37\x43\x37\x31\x35\x37\x42\x2d\x36\x00\x00\x00" \const char* fileHandleID
"\x02\x00\x00\x00"        \unsigned fileHandleIDLen;//4
"\x01\x00\x00\x00"   \unsigned flags; =1
"\xe8\x03\x00\x00"   \unsigned origParentUID; =1000
"\x0c\x00\x00\x00\x30\x2d\x36\x37\x43\x35\x35\x31\x42\x30\x2d\x36\x00\x00\x00\x00
"\x51\x00"              \ uint16_t targetID ; = 81
"\x01\x00\x00\x00"   \size_t              sg_count;
"\x00\x00\x00\x00"   \size_t              dma_count
"\x00\x00\x00\x00"   \ [131-134] 是 tag 4B
"\x42\x3e\x20\x00\x00\x00\x00\x00"   \ [134-141] 是 key 8B
"\x00\x00\xb6\x03\xd9\x7f\x00\x00"   \ [142-149]是 vaddr 8B
"\x64\x00
"\x00\x00\x00\x00\x00\x00\x00\x00" 
"\x00\x00\x00\x00\x00\x00\x00\x00"
```


### 源码中 ReadLocalRDMAMSG 相关数据结构定义
源码中相关数据结构的定义如下，可做参考哦
```c


struct ReadLocalFileRDMAMsg
{
   NetMessage netMessage;

   int64_t offset;//8
   int64_t count;//8
   unsigned accessFlags;//4
   NumNodeID clientNumID;//4
   const char* fileHandleID;//
   unsigned fileHandleIDLen;//4
   PathInfo* pathInfoPtr;
   uint16_t targetID;
   RdmaInfo *rdmap;
};
      struct NetMessage
   {
      struct NetMessageHeader msgHeader;

      const struct NetMessageOps* ops;
   };

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
   struct NetMessageOps
   {
      void (*serializePayload) (NetMessage* netM, SerializeCtx* ctx);// 这个函数有好多种实现方式
      unsigned (*getSupportedHeaderFeatureFlagsMask) (NetMessage* thismsg);
      bool (*deserializePayload) (NetMessage* thismsg, DeserializeCtx* ctx);
   }
            struct NumNodeID
      {
               uint32_t value;
      };

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
struct scatterlist {
	unsigned long	page_link;  // 指示物理页
	unsigned int	offset;   // 页中偏移
	unsigned int	length;    // 数据长度
	dma_addr_t	dma_address;  // dma地址
#ifdef CONFIG_NEED_SG_DMA_LENGTH
	unsigned int	dma_length;
#endif
};
```


### BeeGFS_Storage_RDMA_read请求消息的处理流程


- 处理的大致流程见如下：
```c
//入口： common/source/common/components/streamlistenerv2/IncomingPreprocessedMsgWork.cpp
processRes = msg->processIncoming(rctx); 
if(processRes)
	LogContext(logContextStr).log(Log_WARNING, "Message processed successfully." );

//上面是处理一个消息的宏观流程 
//下面是具体对于 ReadLocalFileRDMAMsg 的处理，流程
1. processIncoming:
//bool ReadLocalFileMsgExBase<Msg, ReadState>::processIncoming(NetMessage::ResponseContext& ctx)

2. incrementalReadStatefulAndSendV2
//template <class Msg, typename ReadState>
//int64_t ReadLocalFileMsgExBase<Msg, ReadState>::incrementalReadStatefulAndSendV2(NetMessage::ResponseContext& ctx,SessionLocalFile* sessionLocalFile)

3. readStateSendData
//class ReadLocalFileRDMAMsgSender : public ReadLocalFileRDMAMsg：：
//inline ssize_t readStateSendData(Socket* sock, ReadState& rs, char* buf, bool isFinal)
{
	ssize_t writeRes = sock->write(buf, rs.readRes, 0, rs.rBuf + rs.rOff, rs.rdma->key);
}

4. RDMASocketImpl::write
ssize_t RDMASocketImpl::write(const void *buf, size_t len, unsigned lkey, const uint64_t rbuf, unsigned rkey)
{
	size_t status = IBVSocket_write(this->ibvsock, (char *)buf, len, lkey, rbuf, rkey);
	return (status == 0) ? len : -1;
}

5. IBVSocket_write
   ...
```

- 发送 Malformed Packet 的过程：
- 推测是在这个函数中 `ssize_t RDMASocketImpl::send(const void *buf, size_t len, int flags)` 遇到了 `SocketException`
- 要看malformed packet 的详细包内容 见：
   - 抓包文件位于 /home/greench/Desktop/162cap/0306_malformed_pkt/20250106_103824_read_CPU_beegfs_1KB_RDMA_164_Send.pcap
- 禁用 Malformed packet 发送 ，重新编译修改的源码版本是这个
   - /home/ubuntu/xyp/wsy_beegfs_250305/beegfs_no_privateData
```c


inline void sendLengthInfo(Socket* sock, int64_t lengthInfo)
{  
    lengthInfo = HOST_TO_LE_64(lengthInfo);
    sock->send(&lengthInfo, sizeof(int64_t), 0);
}
DECLARE_NAMEDEXCEPTION(SocketException, "SocketException")
/**
 * Note: This is a synchronous (blocking) version
 *
 * @param flags ignored
 * @throw SocketException
 */
ssize_t RDMASocketImpl::send(const void *buf, size_t len, int flags)
{
   ssize_t sendRes = IBVSocket_send(ibvsock, (const char*)buf, len, flags | MSG_NOSIGNAL);
   if(sendRes == (ssize_t)len)
   {
      stats->incVals.netSendBytes += len;
      return sendRes;
   }
   else
   if(sendRes > 0)
   {
      throw SocketException(
         std::string("send(): Sent only ") + StringTk::int64ToStr(sendRes) +
         std::string(" bytes of the requested ") + StringTk::int64ToStr(len) +
         std::string(" bytes of data") );
   }

   throw SocketDisconnectException(
      "Disconnect during send() to: " + peername);
}



ssize_t IBVSocket_send(IBVSocket* _this, const char* buf, size_t bufLen, int flags)
{
   IBVCommContext* commContext = _this->commContext;
   int flowControlRes;
   size_t currentBufIndex;
   int postRes;
   size_t postedLen = 0;
   int currentPostLen;
   int waitRes;

   if(unlikely(_this->errState) )
      return -1;

   do
   {
      flowControlRes = __IBVSocket_flowControlOnSendWait(_this,
         _this->timeoutCfg.flowSendMS);
      if(unlikely(flowControlRes <= 0) )
         goto err_invalidateSock;

      // note: we only poll for completed sends after we used up all (!) available bufs

      if(commContext->incompleteSend.numAvailable == commContext->commCfg.bufNum)
      { // wait for all (!) incomplete sends
         waitRes = __IBVSocket_waitForTotalSendCompletion(
            _this, commContext->incompleteSend.numAvailable, 0, 0);
         if(waitRes < 0)
            goto err_invalidateSock;

         commContext->incompleteSend.numAvailable = 0;
      }

      currentPostLen = BEEGFS_MIN(bufLen-postedLen, commContext->commCfg.bufSize);
      currentBufIndex = commContext->incompleteSend.numAvailable;

      memcpy( (commContext->sendBufs)[currentBufIndex], &buf[postedLen], currentPostLen);

      commContext->incompleteSend.numAvailable++; /* inc'ed before postSend() for conn checks */

      postRes = __IBVSocket_postSend(_this, currentBufIndex, currentPostLen);
      if(unlikely(postRes) )
      {
         commContext->incompleteSend.numAvailable--;
         goto err_invalidateSock;
      }


      postedLen += currentPostLen;

   } while(postedLen < bufLen);

   return (ssize_t)bufLen;


err_invalidateSock:
   _this->errState = -1;

   return -ECOMM;
}

/**
 * Note: Contains flow control.
 *
 * @return 0 on success, -1 on error
 */
int __IBVSocket_postSend(IBVSocket* _this, size_t bufIndex, int bufLen)
{
   IBVCommContext* commContext = _this->commContext;
   struct ibv_sge list;
   struct ibv_send_wr wr;
   struct ibv_send_wr *bad_wr;
   int postRes;

   list.addr   = (uint64_t)commContext->sendBufs[bufIndex];
   list.length = bufLen;
   list.lkey   = commContext->sendMR->lkey;

   wr.wr_id      = bufIndex + IBVSOCKET_SEND_WORK_ID_OFFSET;
   wr.next       = NULL;
   wr.sg_list    = &list;
   wr.num_sge    = 1;
   wr.opcode     = IBV_WR_SEND;
   wr.send_flags = IBV_SEND_SIGNALED;

   postRes = ibv_post_send(commContext->qp, &wr, &bad_wr);
   if(unlikely(postRes) )
   {
      LOG(SOCKLIB, WARNING, "ibv_post_send() failed.", sysErr(postRes));
      return -1;
   }

   // flow control
   __IBVSocket_flowControlOnSendUpdateCounters(_this);

   return 0;
}
```




### 暂时没有整理的， 乱七八糟的过程数据

```sh
"\xa4\x00\x00\x00
\x00\x00
\x00
\x00
\x00\x00\x00\x00\x53\x46\x47\x42" \
"\x0b\x08 \NETMSGTYPE_LookupIntent 2059 消息类型
\x00\x00
\x00\x00\x00\x00
\x00\x00\x00\x00\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00\x00\x00 
\x11\x00\x00\x00\x01\x00\x00\x00" \int64_t         offset; =0
"\x00\x00\x00\x00\x00\x00\x00\x00 \int64_t count; =20
\x00\x00\x00\x00\x04\x00\x00\x00" \
"\x72\x6f\x6f\x74\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
"\x01\x00\x00\x00\x00\x00\x09\x00\x00\x00\x74\x65\x73\x74\x5f\x31" \
"\x32\x31\x37\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x04\x00" \
"\x00\x00\x72\x6f\x6f\x74\x00\x00\x00\x00\x0c\x00\x00\x00\x30\x2d" \
"\x36\x37\x36\x31\x33\x36\x31\x31\x2d\x31\x00\x00\x00\x00\x09\x00" \
"\x00\x00\x74\x65\x73\x74\x5f\x31\x32\x31\x37\x00\x00\x00\x01\x00" \
"\x00\x00\x00\x00"

```

```c
struct LookupIntentMsg
{
   NetMessage netMessage;

   int intentFlags; // combination of LOOKUPINTENTMSG_FLAG_...

   const char* entryName; // (lookup data), set for all but revalidate
   unsigned entryNameLen; // (lookup data), set for all but revalidate

   const EntryInfo* entryInfoPtr; // (revalidation data)

   unsigned userID; // (file creation data)
   unsigned groupID; // (file creation data)
   int mode; // file creation mode permission bits  (file creation data)
   int umask; // umask of the context (file creation data)
   const struct FileEvent* fileEvent;

   NumNodeID clientNumID; // (file open data)
   unsigned accessFlags; // OPENFILE_ACCESS_... flags (file open data)

   // for serialization
   const EntryInfo* parentInfoPtr; // not owned by this object (lookup/open/creation data)
   UInt16List* preferredTargets; // not owned by this object! (file creation data)
};
```



```c


struct ReadLocalFileRDMAMsg
{
   NetMessage netMessage;

   int64_t offset;//8
   int64_t count;//8
   unsigned accessFlags;//4
   NumNodeID clientNumID;//4
   const char* fileHandleID;//
   unsigned fileHandleIDLen;//4
   PathInfo* pathInfoPtr;
   uint16_t targetID;
   RdmaInfo *rdmap;
};
      struct NetMessage
   {
      struct NetMessageHeader msgHeader;

      const struct NetMessageOps* ops;
   };

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
   struct NetMessageOps
   {
      void (*serializePayload) (NetMessage* netM, SerializeCtx* ctx);// 这个函数有好多种实现方式
      unsigned (*getSupportedHeaderFeatureFlagsMask) (NetMessage* thismsg);
      bool (*deserializePayload) (NetMessage* thismsg, DeserializeCtx* ctx);
   }
            struct NumNodeID
      {
               uint32_t value;
      };

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
struct scatterlist {
	unsigned long	page_link;  // 指示物理页
	unsigned int	offset;   // 页中偏移
	unsigned int	length;    // 数据长度
	dma_addr_t	dma_address;  // dma地址
#ifdef CONFIG_NEED_SG_DMA_LENGTH
	unsigned int	dma_length;
#endif
};
```