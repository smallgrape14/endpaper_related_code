//  AuthenticateChannelMsg生成

bool __NodeConnPool_applySocketOptionsConnected(Socket* sock)
{
    uint64_t authHash = 10322703647856944136;
    //xfusion3: 574355913317798958
    // 162 server: 10322703647856944136

    if(authHash)
    { // authenticate channel
      char* sendBuf;
      size_t sendBufLen;
      AuthenticateChannelMsg authMsg;
      size_t sendRes;

      //1.填充信息
      AuthenticateChannelMsg_initFromValue(&authMsg, authHash); //
      SimpleInt64Msg_initFromValue( (SimpleInt64Msg*)acmsg, NETMSGTYPE_AuthenticateChannel, authHash);




      sendBufLen = NetMessage_getMsgLength( (NetMessage*)&authMsg);
      // sendBuf = (char*)os_kmalloc(sendBufLen);
      sendBuf = (char*)malloc(sendBufLen);
      if (sendBuf == NULL) {
         // 处理内存分配失败的情况
         perror("Memory allocation failed");
         // 可能的操作包括退出程序或执行其他清理工作
         return false;
      }

      //2.信息序列化
      NetMessage_serialize( (NetMessage*)&authMsg, sendBuf, sendBufLen);
      std::cout<<"send buffer = \n";
      for(int i=0;i<sendBufLen;i++)
      {
         std::cout<<sendBuf[i];
      }
      std::cout<<"\n";
    }
    else
    {
        return false;
    }
    return true;
}

