// #include"readfilevec.hpp"
#include"new_read.hpp"

#include<iostream>
using namespace std;
int main()
{  
   
   // open code
   // Socket* sock=nullptr;
   // printf("start open file in TCP mode \n");
   //  __MessagingTk_requestResponseWithRRArgsComm(sock);//TCP 连接

   // std::cout<<"prepare into readfile \n";
   //XYP_MODIFY 注释如下
   // char *send_buf=(char*)malloc(1024);
   // size_t msg_len=0;
   // vaddr=0x12345678;
   // FhgfsOpsRemoting_readfileVec(send_buf,&msg_len,vaddr,rkey,memory_len);

   char* read_msg=(char*)malloc(1024);
		unsigned int read_msg_len=0;
		size_t memory_len=256;//vaddr对应的内存长度,A BUFF 只有 256B
		uint32_t toberead=128;//请求文件长度 128B A BUFF 只有 256B
      uint64_t vaddr=0x00007f33b5b60000;
      uint32_t rkey=0x00203e42;
		rdma_read_msg_generate( read_msg,&read_msg_len,vaddr,rkey,memory_len,toberead);
   return 0;
}


