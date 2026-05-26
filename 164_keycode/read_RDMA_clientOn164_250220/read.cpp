// #include "read.h"
// #include "read_communicate.cpp"
// #include "new_read.hpp"
#include "readfilevec.hpp" 
// #include "read_pre.hpp"
#include <iostream>
using namespace std;


// 可以说 这里就是真正的通信代码 ，不仅read write 也得调用这个代码
            //void FhgfsOpsCommkit_communicate(App* app, RemotingIOInfo* ioInfo, struct list_head_t* targetInfos,const struct CommKitContextOps* ops, void* private_data) 
void FhgfsOpsCommkit_communicate(RemotingIOInfo* ioInfo, struct list_head* targetInfos,const struct CommKitContextOps* ops, void* private_data) 
{
   // 获取应用程序的配置
                            // Config* cfg = App_getConfig(app);
   // 初始化状态计数器
   std::cout<<"\n ----- Go into FhgfsOpsCommkit_communicate -----------\n";
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
            .gpudRc = 1,//$$$%

      // #endif
   };

   // 2. 进入循环，直到所有状态都完成
   std::cout<<"--- 进入循环，直到所有状态都完成 \n";
   do
   {
      struct CommKitTargetInfo* info;

      // 重置上下文中的计数器
      context.numUnconnectable = 0; // will be increased by states the didn't get a conn
      context.numPollSocks = 0; // will be increased by the states that need to poll
      context.numBufferless = 0;
      numStates = 0;

      // 初始化轮询状态
      std::cout<<"--- 初始化轮询状态 \n";
      PollState_init(&context.pollState);
      context.pollState.nfds = 0; //$$%
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
      std::cout<<"--- 遍历目标信息列表，对每个状态执行相应的操作"<<"\n";
         
         /*
            CommKitTargetInfo **pos=&info;
            for(*pos = get_container_of(targetInfos->next);&(*pos)->targetInfoList != targetInfos;*pos = get_container_of((*pos)->targetInfoList.next))
            for(info = get_container_of(targetInfos->next);&(info)->targetInfoList != targetInfos;info = get_container_of((info)->targetInfoList.next))
            std::cout<<"info->targetID = "<<get_container_of(targetInfos)->targetID<<"\n";
            
            info = get_container_of(targetInfos->next);
            std::cout<<"info->targetID = "<<info->targetID<<"\n";
            for(info = get_container_of(targetInfos->next);&(info)->targetInfoList != targetInfos;info = get_container_of((info)->targetInfoList.next))
            list_head_t *pos;
            for (pos = targetInfos->next; pos != targetInfos && pos!=nullptr; pos = pos->next) 
            {
               // if(pos==nullptr)
               // {
               //    break;
               // }
               info = container_of_v2<CommKitTargetInfo, list_head_t>(pos, offsetof(CommKitTargetInfo, targetInfoList));
                     std::cout<<"info->state = "<<info->state<<"\n";
               
               std::cout<<"info->targetID = "<<info->targetID<<"\n";
                     std::cout<<"------------------\n";

            }
            break;
         */
      
      //for(*pos = get_container_of(targetInfos->next);&(*pos)->targetInfoList != targetInfos;*pos = get_container_of((*pos)->targetInfoList.next))
      int xun_cnt=0;
      list_head_t *pos;
      for (pos = targetInfos->next; pos != targetInfos && pos!=nullptr; pos = pos->next) 
      {  
         xun_cnt++;
         std::cout<<"循环第 "<<xun_cnt<<" 次\n";
         info = container_of_v2<CommKitTargetInfo, list_head_t>(pos, offsetof(CommKitTargetInfo, targetInfoList));
         numStates++;
         std::cout<<"info->state = "<<info->state<<"\n";//0
         FileOpState* tmp_state=container_of_v2<FileOpState, CommKitTargetInfo>(info, offsetof(FileOpState, base));
         std::cout<<"tmp_state->offset = "<<tmp_state->offset<<"\n";//0
         std::cout<<"tmp_state->toBeTransmitted ="<<tmp_state->toBeTransmitted<<"\t"<<"tmp_state->transmitted = "<<tmp_state->transmitted<<"\n";




         switch(info->state)
         {
            case CommKitState_PREPARE: 
               // 准备通信
               std::cout<<"--- 1.准备通信 \n";

               if(!__commkit_prepare_generic(&context, info) )//state =1
                  break;
               BEEGFS_FALLTHROUGH;
            case CommKitState_SENDHEADER:
               // 发送头部信息
               std::cout<<"--- 2.发送头部信息 \n";
               // std::cout<<"tmp_state->toBeTransmitted ="<<tmp_state->toBeTransmitted<<"\t"<<"tmp_state->transmitted = "<<tmp_state->transmitted<<"\n";
               __commkit_sendheader_generic(&context, info);//state =2
               // std::cout<<"tmp_state->toBeTransmitted ="<<tmp_state->toBeTransmitted<<"\t"<<"tmp_state->transmitted = "<<tmp_state->transmitted<<"\n";
               __commkit_add_socket_pollstate(&context, info,context.ops->sendData ? POLLOUT : POLLIN);
               // std::cout<<"tmp_state->toBeTransmitted ="<<tmp_state->toBeTransmitted<<"\t"<<"tmp_state->transmitted = "<<tmp_state->transmitted<<"\n";
               //break;//@@ Note : 源代码并没有注释这个
               if(0)//@@ Note :
               {
                  
               }
               BEEGFS_FALLTHROUGH;//@@ Note : 这个是我自己加的 ，源代码并没有
            
            case CommKitState_SENDDATA:
               // 发送数据
               std::cout<<"--- 3.发送数据 \n";

               if(context.ops->sendData) //下面这个分支不能进去，readfileops 没有定义 senddata的操作，所以 context.ops->sendData设置为==0
               {
                              //  __commkit_senddata_generic(&context, info);
                  std::cout<<"[FhgfsOpsCommkit_communicate] read file error go into sendData stage \n";
                  break;
               }
               BEEGFS_FALLTHROUGH;//用于在 switch-case 语句中故意让一个 case 语句执行完毕后，继续执行下一个 case 语句的代码。

            case CommKitState_RECVHEADER:
               // 接收头部信息
               std::cout<<"--- 4.接收头部信息 \n";

               if(context.ops->recvHeader)//下面这个分支不进去，readfileops 没有定义 recvheader 的操作，所以 context.ops->recvHeader 设置为==0
               {
                                 // __commkit_recvheader_generic(&context, info);
                  std::cout<<"[FhgfsOpsCommkit_communicate] read file error go into recvHeader stage \n";

                  break;
               }
               BEEGFS_FALLTHROUGH;
            
            case CommKitState_RECVDATA:
               // 接收数据
               std::cout<<"--- 5.接收数据 \n";

               if(context.ops->recvData)
               {  
                  std::cout<<"prepare  into __commkit_recvdata_generic \n";
                  __commkit_recvdata_generic(&context, info);
                  break;
               }
               BEEGFS_FALLTHROUGH;
           
            case CommKitState_CLEANUP:
               // 清理操作
               std::cout<<"--- 6.清理操作 \n";

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
               std::cout<<"--- 7.这些状态不需要额外操作 \n";

               break;

            default:
               // 未知状态，可能是错误
               // BUG();
               std::cout<<"--- 8.[FhgfsOpsCommkit_communicate] 未知状态，可能是错误 \n";
            
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
void FhgfsOpsCommKit_readfileV2bCommunicate(RemotingIOInfo* ioInfo,
   struct list_head* states, void (*nextIter)(CommKitContext*, FileOpState*),// 下一次迭代的回调函数
   void (*prepare)(CommKitContext*, FileOpState*))// 准备操作的回调函数
{
   struct ReadfileIterOps iops = {
      .nextIter = nextIter,
      .prepare = prepare,
   };

   FhgfsOpsCommkit_communicate(ioInfo, states, &readfileOps, &iops);
}


// 函数：输出 struct iovec 中的数据
void print_iovec_data(struct iovec *iov) {
    if (iov == NULL || iov->iov_base == NULL || iov->iov_len == 0) {
        LOG_PRINTF("No data to print.\n");
        return;
    }

    // 为了简化，这里假设数据是字符串（char数组）
    // 如果数据不是字符串，这里的代码可能需要调整
    const char *data = (const char *)iov->iov_base;
    size_t len = iov->iov_len;

    // 检查字符串是否以空字符结尾，如果不是，限制输出长度
    if (memchr(data, '\0', len) == NULL) {
        // 非空终止字符串，仅输出前iov_len个字符
        LOG_PRINTF("Data (%zu bytes): ");
        for (size_t i = 0; i < len; ++i) {
            fputc(data[i], stdout);
        }
    } else {
        // 空终止字符串，输出直到第一个空字符
        LOG_PRINTF("String: %s\n", data);
    }
}


//简洁版本的这个函数

// ssize_t FhgfsOpsRemoting_readfileVec(struct iov_iter* iter, size_t toBeRead, loff_t offset,RemotingIOInfo* ioInfo, FhgfsInode* fhgfsInode)
/*
   void  FhgfsOpsRemoting_readfileVec()
   {  
      
      RemotingIOInfo ioInfo;
      initRemotingIOInfo(&ioInfo);
      list_head_t stateList;
      statelist_init(&stateList);
      //真正通信的代码
      FhgfsOpsCommKit_readfileV2bCommunicate(&ioInfo, &stateList, readfile_nextIter, nullptr);

      //通信结束后的代码
      

   }

*/

// 替换成后面这个 ,

ssize_t FhgfsOpsRemoting_readfileVec()
{
    RemotingIOInfo* ioInfo=new RemotingIOInfo();
      initRemotingIOInfo(ioInfo);//@@

    size_t toBeRead=1*1024;//10*1024;// 1.3MB //这两个参数是VFS文件系统入口函数传递的，我自己制定的  todo
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

   std::cout<<"chunkSize = "<<chunkSize<<"\n";
    UInt16Vec* targetIDs = Raid0Pattern_getStripeTargetIDs(pattern); // pattern->getStripeTargetIDs(pattern);//目标ID列表 todo

    unsigned numStripeNodes = UInt16Vec_length(targetIDs);//存储节点数量

   std::cout<<"numStripeNodes = "<<numStripeNodes<<"\n";
    size_t stripeSetSize = (size_t) chunkSize * numStripeNodes;//并计算一个条带集的大小
   //  size_t stripeSetSize = (size_t) 3000;//并计算一个条带集的大小  @@todo
 
  
  
 
   
  // App* app = ioInfo->app;   

   
   

   // 定义读取缓冲区和当前偏移量
   std::cout<<"定义读取缓冲区和当前偏移量 \n";
   BeeGFS_ReadSink* readsink = new BeeGFS_ReadSink();// todo free  //BeeGFS_ReadSink readsink = {0};
   // size_t capacity=1024;// todo 自定义 readsink 的缓冲区大小
   // beegfs_readsink_init(readsink,capacity);
   std::cout<<"-------------0  ------------ \n";

   //ioinfo   
   const char* fileHandleID = ioInfo->fileHandleID;//文件句柄ID
   //int maxUsedTargetIndex = AtomicInt_read(ioInfo->maxUsedTargetIndex);//最大使用的存储节点索引，todo 这个参数我传参的时候有赋值吗
            //__FhgfsOpsRemoting_logDebugIOCall(__func__, iov_iter_count(iter), offset, ioInfo, NULL);//好像是日志，可以注释跳过
   LOG_PRINTF("fileHandleID: %s",ioInfo->fileHandleID);

   int _cnt=0;
     while(toBeRead && !errnum)//开始一个循环，只要还有数据要读取并且没有错误发生，就继续循环。
   {  
      _cnt++;
      std::cout<<"\n @@ 循环次数 _cnt = "<< _cnt<<"----------------------------------------------------------\n";
         std::cout<<"1. toBeRead = "<< toBeRead<<"\n";

      //1. 根据文件的偏移量 ，可以确定文件对应的target storage index,即这个函数的逻辑，我们可以得知文件的条带规则或者设计
      unsigned currentTargetIndex = Raid0Pattern_getStripeTargetIndex(pattern, currentOffset); //pattern->getStripeTargetIndex(pattern, currentOffset); //todo
     
      //创建一个用于读取条带集的iov_iter结构体，并计算本轮最大读取大小。
      size_t maxReadSize = min_t(size_t, stripeSetSize, toBeRead);//计算本轮最大读取大小
      std::cout<<"2. 计算本轮最大读取大小 maxReadSize = "<<maxReadSize<<"\n";//512KB

               // LIST_HEAD(stateList);
      list_head_t stateList;
      INIT_LIST_HEAD(&stateList);

      struct FileOpVecState* state;
      size_t bytesReadThisRound = 0;


       struct iovec stripeSetIter;//struct iov_iter stripeSetIter;
      // iter --> readsink 
      beegfs_readsink_reserve(readsink, iter, maxReadSize);//为读取操作预留空间，并获取用于读取的iov_iter。

      stripeSetIter = readsink->sanitized_iter;
      // std::cout<<"stripeSetIter.iov_len = "<<stripeSetIter.iov_len<<"\n";

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
         // std::cout<<"currentChunkSize = "<<currentChunkSize<<"\t, currentReadSize ="<<currentReadSize<<"\n";
         //下面这个函数很关键，如何根据文件的偏移量得到在对应的存储节点上的偏移量
         int64_t currentNodeLocalOffset = __FhgfsOpsRemoting_getChunkOffset(
            currentOffset, chunkSize, numStripeNodes, currentTargetIndex);//loff_t

            // std::cout<<"currentNodeLocalOffset =  "<<currentNodeLocalOffset<<"\n";//0 80000
         struct iovec chunkIter;

         // 分配状态结构体
                           // state = kmalloc(sizeof(*state), list_empty(&stateList) ? GFP_NOFS : GFP_NOWAIT);
         state = (FileOpVecState *)malloc(sizeof(*state));
            if (state==nullptr)
            {  
               std::cout<<"state ==0 \n";
               break;
            }

        
        
        // maxUsedTargetIndex = MAX(maxUsedTargetIndex, (int)currentTargetIndex);
         // 初始化块迭代器
         chunkIter = stripeSetIter;
         user_iov_iter_truncate(&chunkIter, currentChunkSize);

         // prepare the state information
         std::cout<<"3. prepare the state information ,currentNodeLocalOffset = "<<currentNodeLocalOffset<<"\t,user_iov_iter_count(&chunkIter) ="
         <<user_iov_iter_count(&chunkIter)<<"\t,targetID = "<<UInt16Vec_at(targetIDs, currentTargetIndex)<<"\n";
         FhgfsOpsCommKit_initFileOpState(
            &state->base,
            currentNodeLocalOffset,//存储节点上的偏移量
            user_iov_iter_count(&chunkIter),
            UInt16Vec_at(targetIDs, currentTargetIndex) );

         // state->base.firstWriteDoneForTarget =
         //    BitStore_getBit(ioInfo->firstWriteDone, currentTargetIndex);//todo 

         state->data = chunkIter;
            // std::cout<<"state->base.base.targetID  = "<<state->base.base.targetID<<"\n";

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
         // std::cout<<"currentReadSize = "<<currentReadSize <<"\t, currentTargetIndex = "<<currentTargetIndex<<"\n";
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
            // std::cout<<"tmp4->base.base.targetID  = "<<tmp4->base.base.targetID<<"\n";

         FhgfsOpsCommKit_readfileV2bCommunicate(ioInfo, &stateList, readfile_nextIter, nullptr);
      

      // verify results // 验证结果
      
                              // list_for_each_entry(state, &stateList, base.base.targetInfoList)
       /*  
         state = container_of_v2<FileOpVecState, list_head_t>(stateList.next, offsetof(FileOpVecState, base.base.targetInfoList));
            std::cout<<"state->base.base.targetID  = "<<state->base.base.targetID<<"\n";
         print_iovec_data(&(state->data));   // 输出 state->base.base.nodeResult
         std::cout<<"-------- 2 --------------\n";
         print_iovec_data(&(state->base.data));// 输出 state->base.base.nodeResult
         std::cout<<"-------- 3 --------------\n";
         std::cout<<"state->base.base.nodeResult="<<state->base.base.nodeResult<<"\n";


         // CommKitTargetInfo* tmp1=container_of_v2<CommKitTargetInfo, list_head_t>(stateList.next, offsetof(CommKitTargetInfo, targetInfoList));
         // FileOpState* tmp2=container_of_v2<FileOpState, CommKitTargetInfo>(tmp1, offsetof(FileOpState, base));
         // FileOpVecState* tmp3= container_of_v2<FileOpVecState, FileOpState>(tmp2, offsetof(FileOpVecState, base));
         // std::cout<<"tmp3->base.base.targetID  = "<<tmp3->base.base.targetID<<"\n";
         list_head_t *pos;
         for (pos = stateList.next; pos != &stateList && pos!=nullptr; pos = pos->next)
         {
            
            state = container_of_v2<FileOpVecState, list_head_t>(pos, offsetof(FileOpVecState, base.base.targetInfoList));

            std::cout<<"state->base.base.targetID  = "<<state->base.base.targetID<<"\n";
            if(state->base.base.nodeResult == state->base.expectedNodeResult)// 如果节点返回的结果与预期相符，则累加读取的字节数
            {
               bytesReadThisRound += state->base.base.nodeResult;
               std::cout<<"如果节点返回的结果与预期相符，则累加读取的字节数 \n";
               continue;
            }

            // result not as expected => check cause: end-of-file or error condition
            // 结果不符合预期，检查原因：文件结束或错误条件
            if(state->base.base.nodeResult >= 0)
            { // we have an end of file here (but some data might have been read)
            // 这里表示文件结束（但可能已经读取了一些数据）
            std::cout<<"这里表示文件结束（但可能已经读取了一些数据） \n";
               bytesReadThisRound += state->base.base.nodeResult;
            }
            else
            { // error occurred
               std::cout<<"error occurred \n";
               std::cout<<"-(state->base.base.nodeResult) = "<<-(state->base.base.nodeResult)<<"\n";
               FhgfsOpsErr nodeError = static_cast<FhgfsOpsErr>(-(state->base.base.nodeResult));//强制类型转换

               // Logger* log = App_getLogger(app);
               const char* logContext = "Remoting (read file)";

               if(nodeError == FhgfsOpsErr_INTERRUPTED) { // normal on ctrl+c (=> no logErr() )
                  printf("%s: Storage targetID: %hu; Msg: %s; FileHandle: %s\n",
                        logContext,
                        state->base.base.targetID,
                        FhgfsOpsErr_toErrString(nodeError),
                        fileHandleID);
               } else {
                  printf("ERROR: %s: Error storage targetID: %hu; Msg: %s; FileHandle: %s\n",
                        logContext,
                        state->base.base.targetID,
                        FhgfsOpsErr_toErrString(nodeError),
                        fileHandleID);
               }
               errnum = state->base.base.nodeResult;
               std::cout<<"errnum = "<<errnum<<"\n";
            }

            toBeRead = 0; // abort the read here due to incomplete result/error 
            break;
         } // end of results verification for-loop
      
      std::cout<<" end of results verification for-loop \n";
      
      // int listcnt=list_count(&stateList);
      // std::cout<<"listcnt = "<<listcnt<<"\n";
         while (!list_empty(&stateList) )// 清理状态列表
         {
            int listcnt=list_count(&stateList);
            std::cout<<"listcnt = "<<listcnt<<"\n";
            struct FileOpVecState* state = container_of_v2<FileOpVecState,list_head>(&stateList,offsetof(FileOpVecState, base.base.targetInfoList));
            // list_first_entry<FileOpVecState,list_head>(&stateList,FileOpVecState, base.base.targetInfoList);
               // base.base.targetInfoList);
               // (&stateList, struct FileOpVecState,base.base.targetInfoList);
            //struct FileOpVecState* state =  container_of_v2<FileOpVecState,FileOpState>(&stateList,offsetof(FileOpVecState, base.base.targetInfoList));
            std::cout<<static_cast<void*>(state)<<"\t"<<static_cast<void*>(&(state->base.base.targetInfoList))<<"\n";
            if(state==nullptr)
            {
               std::cout<<"null \n";
            }

            list_del_tail(&state->base.base.targetInfoList);
            // delete state;
            // free(state);//kfree(state);
            state=nullptr;
         }
      
      */
      // 释放读取缓冲区
      beegfs_readsink_release(readsink);//NOTE: 这个函数里面是空的，全给注释了，为什么？


      retVal += bytesReadThisRound;
      int iter_iocnt=1;//todo 
      user_iov_iter_advance(iter, iter_iocnt,bytesReadThisRound);

      // break;

   } // end of while(toBeRead)
   std::cout<<"end of while(toBeRead) \n";
  beegfs_readsink_release(readsink); // Make sure it's released even if we broke early from the loop

   /*
      AtomicInt_max(ioInfo->maxUsedTargetIndex, maxUsedTargetIndex);


      #ifdef BEEGFS_NVFS// 如果使用了非易失性文件系统，释放相关资源
         if (ioInfo->nvfs)
         {
            RdmaInfo_releaseNVFS();
            ioInfo->nvfs = false;
         }
      #endif
   */
   // 返回读取的总字节数，如果没有读取任何数据，则返回错误码
   return retVal ? retVal : errnum;

}


// 记得要 把 open 逻辑加进来 ，获取 ioinfo 的 fileHandleID 和 pathInfo 这个参数

int main()
{  
   
   // open code
   //XYP_MODIFY
   Socket* sock=nullptr;
   printf("start open file in TCP mode \n");
    __MessagingTk_requestResponseWithRRArgsComm(sock);//TCP 连接
   
   std::cout<<"prepare into readfile \n";
   FhgfsOpsRemoting_readfileVec();
   return 0;
}


