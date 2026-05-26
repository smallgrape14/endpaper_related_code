### 树立 RDMA OPEN 的接收逻辑代码

```sh
open_RDMA
    #open.hpp
    MessagingTk_recvMsgBuf
        Socket_recvExactTEx_kernel(40B的头部)
            Socket_recvExactTEx
                thissock->ops->recvT
                    # RDMAConnection.hpp
                    _RDMASocket_recvT
        Socket_recvExactTEx_kernel（剩余负载信息）


# RDMAConnection.hpp
_RDMASocket_recvT
    IBVSocket_recvT
        __IBVSocket_receiveCheck
            __IBVSocket_flowControlOnSendWait # 没有触发
            __IBVSocket_recvWC
                __IBVSocket_waitForRecvCompletionEvent
                    IBVSocket_checkConnection
                __IBVSocket_recvContinueIncomplete


# 超时参数设置在 open.hpp 自实现的代码
    MessagingTk_recvMsgBuf
        {int  connMsgLongTimeout=50000;}
```

DEBUG 过程做的更改
```c
总结正确的更改/会造成影响的更改是：
    Step1 和 Step4 和 Step 7
// RDMAConnection.hpp

1. __IBVSocket_waitForRecvCompletionEvent 中 注释下面的检查连接
     checkRes = IBVSocket_checkConnection(_this);//$$$% 因为这里发送包 ，我把这个先注释了
2. _RDMASocket_recvT 中将 
    timeoutMS=0;
    目的是：__IBVSocket_waitForRecvCompletionEvent 函数中直接触发队列轮询
        if(!timeoutMS)
        return ibv_poll_cq(commContext->recvCQ, 1, outWC);
    结果是： 还是超时了

3. 直接注释 IBVSocket_recvT
      __IBVSocket_receiveCheck 的相关逻辑
      {
           do {
                checkRes = __IBVSocket_receiveCheck(_this, wait);//卡在这里
                LOG(DEBUG)<<"checkRes = "<<checkRes<<"\n";
            } while (checkRes == 0 && timeoutMS < 0);

            // if(checkRes < 0)
            //    return -ECOMM;

            // if(checkRes == 0)
            //    return -ETIMEDOUT;
      }
    //PS： 上面这个对于DEBUG不起作用，又取消了相关的注释
4. _RDMASocket_recvT()
    {
        timeoutMS=5000;//这样做的目的是只修改RDMA通信的timeout 

    }
PS: TCP 超时时间是在这里设置，也算是总的超时时间既设置了TCP 也设置了RDMA 接收时候的超时时间
    open.hpp : MessagingTk_recvMsgBuf()
    {
        int  connMsgLongTimeout=500;// 可以暂时自己定义这个超时时间 单位是 ms
    }
5. RDMAConnection.hpp 中的
    #define IBVSOCKET_POLL_TIMEOUT_MS                 10000 //修改成 200
    成功： 200 20 1
    主要是影响// IBVSOCKET_POLL_TIMEOUT_MS 的设置影响了轮询的次数，轮询的间隔，这就是为什么timeoutMS设置太小，RDMA 通信的 open 接收不到消息 //500 5000 10000 ms 都不行,因为轮询次数太少
    __IBVSocket_waitForRecvCompletionEvent()
    {
        int pollTimeoutMS = MIN(_this->timeoutCfg.pollMS, timeoutMS);
        。。。
        std::this_thread::sleep_for(std::chrono::milliseconds(pollTimeoutMS));

    }
    //从根本修改，应该是修改这个 IBVSocket_init()
    {
        _this->timeoutCfg.pollMS = IBVSOCKET_POLL_TIMEOUT_MS;
    }


6. 注释了和添加了如下代码,因为storage注释检查连接的代码逻辑，从而client 收到的 8B 是全0 ，从而某些字段没有根据checkconnection 而相应设置比如 buffer->bufferSize，buffer->bufferSize 但是其实代码逻辑是没有进来这里的，
    __IBVSocket_recvContinueIncomplete() 
    {
        // size_t bufIndex = recv->bufIndex;//xyp_0317 注释
        size_t bufIndex = 0; //xyp_0317
        buffer->bufferSize=4096;//xyp_0317
    }
    PS:0318_2136 又恢复回去了 这才解决了 BUG: 读 image_32KB 时候 ，open 第 4个文件的时候 轮询到超时都没有收到完成事件，好奇怪
7. 我不想以ms 为轮询间隔 ，调整代码为 us 轮询间隔 ,在如下函数修改，具体修改见下文 //XYP_0317 标注的就是注释和修改的地方
    __IBVSocket_waitForRecvCompletionEvent()//目前设置轮询间隔是200us
    {

    }
```
us 轮询时间间隔的修改后代码如下__IBVSocket_waitForRecvCompletionEvent()：
```c
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
   int timeoutCfg_pollUS=200;//XYP_0317
   // while(timeoutMS != 0)//XYP_0317
   while(timeoutUS != 0)//XYP_0317
   {
      /* note: we use pollTimeoutMS to check the conn every few secs (otherwise we might
         wait for a very long time in case the other side disconnected silently) */

      // int pollTimeoutMS = MIN(_this->timeoutCfg.pollMS, timeoutMS);//XYP_0317
      
      int pollTimeoutUS =MIN(timeoutCfg_pollUS, timeoutMS*1000);//XYP_0317
      cnt++;
      LOG_printf(DEBUG,"------------%d--------------\n",cnt);
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
```

PS: 注意
```c
1. beegfs/client_module/source/common/toolkit/MessagingTk.h 
    MessagingTk_recvMsgBuf()
    {
        // 设置的超时参数：
        cfg->connMsgLongTimeout
        //recvRes = Socket_recvExactTEx_kernel(sock, bufIn, NETMSG_MIN_LENGTH, 0, cfg->connMsgLongTimeout,&numReceived);
    }
2. 搜索匹配 connMsgLongTimeout

beegfs/client_module/build/dist/etc/beegfs-client.conf 中 配置文件中
/*
    # [connMessagingTimeouts]
    # These constants are used to set some of the connection timeouts for sending
    # and receiving data between services in the cluster. They used to be hard-coded
    # (CONN_LONG_TIMEOUT, CONN_MEDIUM_TIMEOUT and CONN_SHORT_TIMEOUT) but are now
    # made configurable for experimentation purposes.
    # This option takes three integer values of milliseconds, separated by a comma
    # in the order long, medium, short.
    # WARNING: This is an EXPERIMENTAL configuration option that should not be
    #     changed in production environments unless properly tested and validated.
    #     Some configurations can lead to service lockups and other subtle issues.
    #     Please make sure that you know exactly what you are doing and properly
    #     test any changes you make.
    # Default: 600000,90000,30000
*/
/home/ubuntu/xyp/beegfs/client_module/source/app/config/Config.c 中
/home/ubuntu/xyp/beegfs/client_module/source/common/Common.h
/**
 * NOTE: These timeouts can now be overridden by the connMessagingTimeouts
 * option in the configuration file. If that option is unset or set to <=0, we
 * still default to these constants.
 */
#define CONN_LONG_TIMEOUT     600000
#define CONN_MEDIUM_TIMEOUT    90000
#define CONN_SHORT_TIMEOUT     30000
```





### 解析下初始化 RDMA SOCKET 的流程

```sh
# new_read.hpp
RDMASocket_construct
    # RDMAConnection.hpp
    RDMASocket_init
        _PooledSocket_init

.connectByIP = _RDMASocket_connectByIP
    IBVSocket_connectByIP
        __IBVSocket_routeResolvedHandler
            __IBVSocket_createCommContext
                IBVBuffer_init
                IBVBuffer_initRegistration
            IBVBuffer_initRegistration
            __IBVSocket_initCommDest
            rdma_connect 

```

#### DEBUG : open从TCP 切换到 RDMA 之后，为什么还是比原生beegfs 要慢许多
GPUNETIO:5~6s Beegfs : 1~1.5S

对比抓包发现二者

gpunetio:
```sh
"\x9a\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x53\x46\x47\x42" \
"\xb9\x0b\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00\x00\x00" # 前 40B 头部
"x0a\x00\x00\x00" # clientNumID
"\x04\x00\x00\x00" # accessFlags
"\x02\x00\x00\x00" # eInfo->entryType
"\x00\x00\x00\x00" # eInfo->featureFlags
"\x0c\x00\x00\x00" # strlen( eInfo->parentEntryID) 12
"\x30\x2d\x36\x37\x44\x32\x33\x41\x43\x44\x2d\x36\x00\x00\x00\x00" # eInfo->parentEntryID
"\x0f\x00\x00\x00" # strlen( eInfo->entryID) 15
"\x31\x30\x30\x36\x2d\x36\x37\x44\x32\x33\x42\x30\x43\x2d\x36\x00"  # eInfo->entryID
"\x12\x00\x00\x00" # strlen( eInfo->fileName) 18
"\x6e\x30\x32\x31\x30\x30\x38\x37\x37\x5f\x33\x37\x30\x2e\x4a\x50\x45\x47\x00\x00" #eInfo->fileName
"\x01\x00\x00\x00" # unsigned accessFlags;
"\x00\x00\x0c\x00" # eventType
"\x00\x00\x12\x00\x00\x00\x6e\x30\x32\x31\x30\x30\x38\x37\x37\x5f" \
"\x33\x37\x30\x2e\x4a\x50\x45\x47\x00\x00\x00\x00"



```

BEEGFS:
```sh
"\x82\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x53\x46\x47\x42" \
"\xb9\x0b\x00\x00\xeb\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00\x00\x00" # 前 40B 头部
"\x02\x00\x00\x00" # clientNumID
"\x01\x00\x00\x00" # accessFlags
"\x02\x00\x00\x00"  # eInfo->entryType
"\x01\x00\x00\x00"  # eInfo->featureFlags
\x0c\x00\x00\x00 # strlen( eInfo->parentEntryID) 12
"\x30\x2d\x36\x37\x44\x38\x30\x41\x38\x41\x2d\x41" # eInfo->parentEntryID
"\x00\x00\x00\x00"
"\x0e\x00\x00\x00" # strlen( eInfo->entryID) 14
"\x43\x39\x44\x2d\x36\x37\x44\x38\x30\x41\x38\x41\x2d\x41" # eInfo->entryID
"\x00\x00" \
"\x14\x00\x00\x00" # strlen( eInfo->fileName) 20
"\x6e\x30\x32\x31\x31\x33\x30\x32\x33\x5f\x31\x35\x35\x37\x36\x2e\x4a\x50\x45\x47" #eInfo->fileName
"\x00\x00\x00\x00" # unsigned accessFlags;
"\x0a\x00\x00\x00\x00\x00\x00\x00" # struct FileEvent* fileEvent;


```




