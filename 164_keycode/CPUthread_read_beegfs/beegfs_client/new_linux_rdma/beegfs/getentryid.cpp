
#include "getentryid.hpp"
// #include "new_read.hpp"

/**
 * Incremental search for a metadata owner node.
 *
 * @param nodes metadata nodes
 * @param outOwner is only valid if return is FhgfsOpsErr_SUCCESS (and needs to be kfreed by the
 * caller)
 */
// FhgfsOpsErr MetadataTk::findOwner(Path* searchPath, unsigned searchDepth, NodeStoreServers* nodes,EntryInfo* outEntryInfo, const RootInfo& metaRoot, MirrorBuddyGroupMapper* metaBuddyGroupMapper)

// FhgfsOpsErr findOwner()
/*
FhgfsOpsErr Path_resolution(char* inputPathStr,DirEntryType inputEntryType,NumNodeID& outOwnerNodeID, std::string& outParentEntryID,std::string& outEntryID, std::string& outFileName,DirEntryType& outEntryType, int& outFeatureFlags)
{
   // EntryInfo* outEntryInfo=nullptr;

   //参数1  传入参数 PS： 参数3 初始化的时候有 searchPath->back(),这个操作
   
   //为什么这些查询结果的 entry type =0 表示无效DirEntryType_INVALID  问题出在 direntrytype 和 unsigned int 的强制类型转换，本质上是我对 FindOwnerRespMsg 的反序列化解析的逻辑出了问题 
      // std::string cfgPathStr="/test/dir1/1.txt";//@@ 我给它初始化吧
      // std::string cfgPathStr="test_dir/dir1/test1.txt";//@@ 我给它初始化吧
      //路径可以获取正确的ID
      // std::string cfgPathStr="test_dir";//@@ 我给它初始化吧
      // std::string cfgPathStr="test_dir/dir1";//@@ 我给它初始化吧
      // std::string cfgPathStr="test_dir/1.txt";//@@ 我给它初始化吧
      // std::string cfgPathStr="test_dir/dir1";//@@ 我给它初始化吧
      // std::string cfgPathStr="test_dir/dir1/1.txt";//@@ 我给它初始化吧
      // std::string cfgPathStr="test_dir/dir1/dir11";//@@ 我给它初始化吧
      // std::string cfgPathStr="test_dir/dir1/dir11/test11.txt";//@@ 我给它初始化吧
      // std::string cfgPathStr="test_dir/dir1/dir11/test11.txt";//@@ 我给它初始化吧 ,为什么这个的Feature Flags: 1 

      std::string cfgPathStr=inputPathStr;//@@ 我给它初始化吧 ,为什么这个的Feature Flags: 1 

      //为 1 的含义是 STATFLAG_HINT_INLINE宏定义了一个标志， 用于提供关于 inode 是否内联到 dentry 中的提示，内联 inode 可以优化文件系统操作

      //错误的路径
      // std::string cfgPathStr="test_dir/dir1/test1.txt";//@@ 我给它初始化吧

      Path searchPath(cfgPathStr);//Path* searchPath 
      
      LOG(DEBUG)<<"---------begin\n"; 
      

   //参数2 传入参数

      size_t searchDepth = searchPath.size(); // 0-based incl. root 
      LOG(DEBUG)<<" searchDepth = "<<searchDepth<<"\n";
      if(!searchDepth)//路径深度
      {
         LOG(DEBUG)<<"// looking for the root node (ignore referenceParent)\n";//@@ MetadataTk::referenceOwner 这个函数实现里有对应的处理逻辑，我先省略了
      }
     
   // 参数 currentNode

      // reference root node
                  //    auto currentNode = referenceRoot(*nodes, *metaRoot, *metaBuddyGroupMapper);//
      auto currentNode = referenceRoot();//返回的是元数据节点

      if (!currentNode)
      {
         //   LogContext(logContextStr).logErr("Unable to proceed without a working root metadata server");
         LOG(DEBUG)<<"currentNode==null , Unable to proceed without a working root metadata server \n";
         return FhgfsOpsErr_UNKNOWNNODE;
      }

   //参数3
   LOG(DEBUG)<<"currentNode->getNumID() = "<<currentNode->getNumID().value<<"\n";
      EntryInfo_class lastEntryInfo(currentNode->getNumID(), "", META_ROOTDIR_ID_STR,
      // searchPath.back(), DirEntryType_DIRECTORY, 0);//DirEntryType_REGULARFILE
      searchPath.back(), inputEntryType, 0);//DirEntryType_REGULARFILE

      LOG(DEBUG)<<"searchPath.back() ="<<searchPath.back()<<"\n";
      //  EntryInfo_class(const NumNodeID ownerNodeID, const std::string& parentEntryID,
      //    const std::string& entryID, const std::string& fileName, const DirEntryType entryType,
      //    const int featureFlags) :


   // 参数4
      unsigned lastEntryDepth = 0;

    FindOwnerMsg requestMsg(&searchPath, searchDepth, &lastEntryInfo, lastEntryDepth);

    EntryInfoWithDepth entryInfoWDepth;

    FhgfsOpsErr findRes;


    findRes = findOwnerStep(*currentNode, (NetMessage_class*)&requestMsg, &entryInfoWDepth);
    if(entryInfoWDepth.getEntryDepth() == searchDepth)
      { // successful end of search
         // outEntryInfo->set(&entryInfoWDepth);
        LOG(DEBUG)<<"successful end of search \n";
      }
      else
         {
            LOG(DEBUG)<<"failed entryInfoWDepth.getEntryDepth() != searchDepth \n";
         }
     // 设置 entryInfo 的成员变量

    NumNodeID ownerNodeID;
    std::string parentEntryID, entryID, fileName;
    DirEntryType entryType;
    int featureFlags;
   LOG(DEBUG)<<"待查询的路径信息 = "<<cfgPathStr<<"\n";
   // NumNodeID& outOwnerNodeID, std::string& outParentEntryID,std::string& outEntryID, std::string& outFileName,DirEntryType& outEntryType, int& outFeatureFlags)
    entryInfoWDepth.getEntryInfo(outOwnerNodeID, outParentEntryID, outEntryID, outFileName, outEntryType, outFeatureFlags);

     return FhgfsOpsErr_SUCCESS;
}

void PrintEntryInfo(NumNodeID& outOwnerNodeID, std::string& outParentEntryID,
                      std::string& outEntryID, std::string& outFileName,
                      DirEntryType& outEntryType, int& outFeatureFlags)
{
   LOG(DEBUG)<<"--------- print out Entryinfo ----------- \n";
        LOG(DEBUG) << "Owner Node ID: " << outOwnerNodeID.value << std::endl;
        LOG(DEBUG) << "Parent Entry ID: " << outParentEntryID << std::endl;
        LOG(DEBUG) << "Entry ID: " << outEntryID << std::endl;
        LOG(DEBUG) << "File Name: " << outFileName << std::endl;
      //   LOG(DEBUG) << "Entry Type: " << static_cast<int>(outEntryType) << std::endl;
        LOG(DEBUG) << "Entry Type: " << (outEntryType) << std::endl;

        LOG(DEBUG) << "Feature Flags: " << outFeatureFlags << std::endl;
        LOG(DEBUG)<<"--------- ----------- ----------- \n";
}
*/
// void test(char** &list,int cnt)
// {
//    list=(char**)malloc(cnt*sizeof(char*));
//    if(list==nullptr)
//    {
//       LOG(DEBUG)<<"分配内存失败 \n";
//       return ;
//    }
//    for(int i=0;i<cnt;i++)
//    {
//       list[i]=(char*)malloc(sizeof(char)*25);
//       if(list[i]==nullptr)
//       {
//          LOG(DEBUG)<<"分配内存失败2 \n";
//          for (int j = 0; j < i; j++) {
//                   free(list[j]);
//                }
//          free(list);
//          return ;
//       }

//    }
//    LOG(DEBUG)<<"分配成功 \n";
//    LOG(DEBUG)<<" list = "<<list<<"\n";
//    return ;

// }
// void PrintResultOfReaddir(int elemcnt,char** namelist,char** idlist,int* typelist)
// {  
//    LOG(DEBUG)<<"--- start PrintResultOfReaddir ------\n";

//    LOG(DEBUG)<<"----------- print namelist -----------\n";
//    for(int i=0;i<elemcnt;i++)
//    {
//       LOG(DEBUG)<<"NO."<<i<<"  :  "<<namelist[i]<<"\n";
//    }
//    LOG(DEBUG)<<"----------- print idlist -----------\n";
//    for(int i=0;i<elemcnt;i++)
//    {
//       LOG(DEBUG)<<"NO."<<i<<"  :  "<<idlist[i]<<"\n";
//    }
//    LOG(DEBUG)<<"----------- print namelist -----------\n";
//    for(int i=0;i<elemcnt;i++)
//    {
//       LOG(DEBUG)<<"NO."<<i<<"  :  "<<typelist[i]<<"\n";
//    }
//    LOG(DEBUG)<<"--- end of PrintResultOfReaddir ------\n";
// }
// void read_dir(char * inputsearchPath)
void read_dir(char * msg_buf,unsigned int * msg_len,
   uint64_t vaddr,uint32_t rkey,size_t vaddr_len,int64_t toberead,int64_t begin_offset,int file_seq)
{
   //1.路径解析，获取目标目录、文件的 EntryInfo
   LOG_printf(DEBUG,"----------------- 1.路径解析，获取目标目录、文件的 EntryInfo --------------------------\n");
   char inputsearchPath[50]="test_0303";
   // DirEntryType searchEntryType=DirEntryType_REGULARFILE;// or DirEntryType_DIRECTORY
   // char searchPath[50]="small_image";
   char searchPath[50];
   strcpy(searchPath,inputsearchPath);
   LOG(DEBUG)<<"searchPath = "<<searchPath<<"\n";
   DirEntryType searchEntryType=DirEntryType_DIRECTORY;// DirEntryType_REGULARFILE or DirEntryType_DIRECTORY

   NumNodeID outOwnerNodeID;
   char* ParentEntryID=(char*)malloc(50*sizeof(char));
   char* currentDirEntryID=(char*)malloc(50*sizeof(char));
   char* currentDirName=(char*)malloc(50*sizeof(char));
   DirEntryType currentDirType;
   int outFeatureFlags;

   FhgfsOpsErr res = Path_resolution(searchPath,searchEntryType,outOwnerNodeID, ParentEntryID, currentDirEntryID, currentDirName, currentDirType, outFeatureFlags);
   PrintEntryInfo_char(outOwnerNodeID, ParentEntryID, currentDirEntryID, currentDirName, currentDirType, outFeatureFlags);

   LOG_printf(DEBUG,"----------------- 2. readdir  --------------------------\n");

   Socket* sock=nullptr;

    char** namelist=nullptr;
    char** idlist=nullptr;
    int* typelist=nullptr;
    int DirElemCnt=0;
   //  test(namelist,DirElemCnt);

    
    readdir(sock,ParentEntryID,currentDirEntryID,currentDirName,currentDirType,namelist,idlist,typelist,DirElemCnt);

      PrintResultOfReaddir(DirElemCnt,namelist,idlist,typelist);
      
   LOG_printf(DEBUG,"------------------ 3. 依次open /read 对应的文件内容 -------------------\n");
// void open(Socket* sock,char* parentEntryID,char* entryID,char* fileName,DirEntryType entryType,char* OutFileHandleID);
   
LOG(DEBUG)<<"sock = "<<sock<<"\n";
   // for(int i=0;i<DirElemCnt;i++)
    int file_cnt=0;
   for(int i=0;i<DirElemCnt;i++)
   {
      if(typelist[i]==DirEntryType_REGULARFILE)
      {  
         char* FileHandleID=(char*)malloc(50*sizeof(char));
         open(sock,currentDirEntryID,idlist[i],namelist[i], DirEntryType_REGULARFILE,FileHandleID);
         LOG(DEBUG)<<"FileName: "<<namelist[i]<<"\t, FileHandleID: "<<FileHandleID<<"\n";
         unsigned int origParentUID=29999;
         char * origParentEntryID=(char*)malloc(50*sizeof(char));
         strcpy(origParentEntryID,currentDirEntryID);
         LOG(DEBUG)<<"origParentEntryID = "<<origParentEntryID<<"\n";
         LOG(DEBUG)<<"--------------- open 成功 ---------------------\n";
         LOG(DEBUG)<<"--------------- 开始 read  ---------------------\n";
         // size_t toberead=8481;
         // struct iovec* requestedheader=(struct iovec*)malloc(sizeof(struct iovec));
         // FhgfsOpsRemoting_readfileVec(FileHandleID,origParentUID,origParentEntryID,toberead,requestedheader);
         
         size_t toberead=10;
         file_cnt++;
         if(file_cnt == file_seq)
            {
               generate_read_request(FileHandleID,origParentUID,origParentEntryID,
                  msg_buf,msg_len,vaddr, rkey,vaddr_len,toberead,begin_offset);
                  break;
            }
         
         
         // 打开文件，准备写入
         // FILE* file = fopen("output.txt", "wb"); // "w" 模式表示写入，并在文件不存在时创建文件
         // if (file == NULL) {
         //    perror("Error opening file");
         //    return ;
         // }

         // // 写入数据到文件
         // fwrite(requestedheader->iov_base, sizeof(char), requestedheader->iov_len, file); 

         // // 关闭文件
         // fclose(file);

         // ssize_t FhgfsOpsRemoting_readfileVec(char * filehandleID,unsigned int origParentUID,char * origParentEntryID)
         
         LOG(DEBUG)<<"出来了 \n";
         free(FileHandleID);
         free(origParentEntryID);
         // break;
      }
      else if(typelist[i]==DirEntryType_DIRECTORY)//还需要继续 readdir
      {

      }
   }
   // *file_num=file_cnt;







   //1.释放内存 路径解析阶段的
      free(ParentEntryID);
      free(currentDirEntryID);
      free(currentDirName);

   //2. 释放内存 namelist idlist typelist readdir阶段的
   
      //释放内存 namelist
      if(namelist!=nullptr)
         {  
               for (int i = 0; i < DirElemCnt; i++) {
                  free(namelist[i]); // 释放每个字符串的内存
               }
               free(namelist); // 释放指针数组
         }
   

      //释放内存 idlist
      if(idlist!=nullptr)
        {
            for (int i = 0; i < DirElemCnt; i++) {
                     free(idlist[i]); // 释放每个字符串的内存
               }
            free(idlist); // 释放指针数组
        }

      //释放内存 typelist

         free(typelist); // 释放指针数组
   //3.释放套接字
   NodeConnPool_releaseStreamSocket(sock);
        return ;
}


int main()
{  
   //1.路径解析，获取目标目录、文件的 EntryInfo
   // printf("-------\n");
   LOG_printf(DEBUG,"----------------- 1.路径解析，获取目标目录、文件的 EntryInfo --------------------------\n");
   // char searchPath[50]="test_dir/dir1/dir11/test11.txt";
   // DirEntryType searchEntryType=DirEntryType_REGULARFILE;// or DirEntryType_DIRECTORY
   char searchPath[50]="FashionMNIST";
   DirEntryType searchEntryType=DirEntryType_DIRECTORY;// DirEntryType_REGULARFILE or DirEntryType_DIRECTORY

   NumNodeID outOwnerNodeID;
   char* ParentEntryID=(char*)malloc(50*sizeof(char));
   char* currentDirEntryID=(char*)malloc(50*sizeof(char));
   char* currentDirName=(char*)malloc(50*sizeof(char));
   DirEntryType currentDirType;
   int outFeatureFlags;

   FhgfsOpsErr res = Path_resolution(searchPath,searchEntryType,outOwnerNodeID, ParentEntryID, currentDirEntryID, currentDirName, currentDirType, outFeatureFlags);
   PrintEntryInfo_char(outOwnerNodeID, ParentEntryID, currentDirEntryID, currentDirName, currentDirType, outFeatureFlags);

   LOG_printf(DEBUG,"----------------- 2. readdir  --------------------------\n");

   Socket* sock=nullptr;

    char** namelist=nullptr;
    char** idlist=nullptr;
    int* typelist=nullptr;
    int DirElemCnt=0;
   //  test(namelist,DirElemCnt);

    
    readdir(sock,ParentEntryID,currentDirEntryID,currentDirName,currentDirType,namelist,idlist,typelist,DirElemCnt);

      PrintResultOfReaddir(DirElemCnt,namelist,idlist,typelist);
      
   LOG_printf(DEBUG,"------------------ 3. 依次open /read 对应的文件内容 -------------------\n");
// void open(Socket* sock,char* parentEntryID,char* entryID,char* fileName,DirEntryType entryType,char* OutFileHandleID);
   
LOG(DEBUG)<<"sock = "<<sock<<"\n";
   // for(int i=0;i<DirElemCnt;i++)
   int filecnt=1;
   for(int i=0;i<DirElemCnt;i++)
   {
      if(typelist[i]==DirEntryType_REGULARFILE)
      {  
         
         char* FileHandleID=(char*)malloc(50*sizeof(char));
         open(sock,currentDirEntryID,idlist[i],namelist[i], DirEntryType_REGULARFILE,FileHandleID);
         LOG(DEBUG)<<"FileName: "<<namelist[i]<<"\t, FileHandleID: "<<FileHandleID<<"\n";
         unsigned int origParentUID=0;
         char * origParentEntryID=(char*)malloc(50*sizeof(char));
         strcpy(origParentEntryID,currentDirEntryID);
         LOG(DEBUG)<<"origParentEntryID = "<<origParentEntryID<<"\n";
         LOG(INFO)<<"--------------- open 成功 ---------------------第"<<filecnt<<"个文件\n";
         LOG(INFO)<<"--------------- 开始 read  ---------------------第"<<filecnt<<"个文件\n";
         
         /* //这里是生成 TCP READ File Request
            size_t toberead=8481;
            struct iovec* requestedheader=(struct iovec*)malloc(sizeof(struct iovec));
            FhgfsOpsRemoting_readfileVec(FileHandleID,origParentUID,origParentEntryID,toberead,requestedheader);
         */
         // generate_read_request(FileHandleID,origParentUID,origParentEntryID,
         //       send_buf,msg_len,vaddr, rkey,vaddr_len,toberead);
         
         
         /* // 打开文件，准备写入
            FILE* file = fopen("output.txt", "ab"); // "w" 模式表示写入，并在文件不存在时创建文件
            if (file == NULL) {
               perror("Error opening file");
               return 1;
            }
            // 写入数据到文件
            fwrite(requestedheader->iov_base, sizeof(char), requestedheader->iov_len, file); 
            // 关闭文件
            fclose(file);
         */
         
         LOG(DEBUG)<<"Read Request 生成 \n";



         free(FileHandleID);
         free(origParentEntryID);
         filecnt++;
         break; //只open一个文件
      }
      else if(typelist[i]==DirEntryType_DIRECTORY)//还需要继续 readdir
      {

      }
   }







   //1.释放内存 路径解析阶段的
      free(ParentEntryID);
      free(currentDirEntryID);
      free(currentDirName);

   //2. 释放内存 namelist idlist typelist readdir阶段的
   
      //释放内存 namelist
      if(namelist!=nullptr)
         {  
               for (int i = 0; i < DirElemCnt; i++) {
                  free(namelist[i]); // 释放每个字符串的内存
               }
               free(namelist); // 释放指针数组
         }
   

      //释放内存 idlist
      if(idlist!=nullptr)
        {
            for (int i = 0; i < DirElemCnt; i++) {
                     free(idlist[i]); // 释放每个字符串的内存
               }
            free(idlist); // 释放指针数组
        }

      //释放内存 typelist

         free(typelist); // 释放指针数组
   //3.释放套接字
   NodeConnPool_releaseStreamSocket(sock);
        return 0;
}
