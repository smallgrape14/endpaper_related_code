#ifndef READDIR_HPP
#define READDIR_HPP

// #include "open.hpp"
#include "new_read.hpp"

#define NETMSGTYPE_ListDirFromOffset               2029



struct ListDirFromOffsetMsg
{
   NetMessage netMessage;

   int64_t serverOffset;
   unsigned maxOutNames;
   bool filterDots;

   // for serialization
   const EntryInfo* entryInfoPtr; // not owned by this object!

   // for deserialization
   EntryInfo entryInfo;
};

void ListDirFromOffsetMsg_serializePayload(NetMessage* msg, SerializeCtx* ctx);

const struct NetMessageOps ListDirFromOffsetMsg_Ops = {
   .serializePayload = ListDirFromOffsetMsg_serializePayload,
//    .deserializePayload = _NetMessage_deserializeDummy,
//    .processIncoming = NetMessage_processIncoming,
//    .getSupportedHeaderFeatureFlagsMask = NetMessage_getSupportedHeaderFeatureFlagsMask,
};

void ListDirFromOffsetMsg_serializePayload(NetMessage* msg, SerializeCtx* ctx)
{
   ListDirFromOffsetMsg* thisCast = (ListDirFromOffsetMsg*)msg;

   // serverOffset
   Serialization_serializeInt64(ctx, thisCast->serverOffset);

   // maxOutNames
   Serialization_serializeUInt(ctx, thisCast->maxOutNames);

   // EntryInfo
   EntryInfo_serialize(ctx, thisCast->entryInfoPtr);

   // filterDots
   Serialization_serializeBool(ctx, thisCast->filterDots);
}


void ListDirFromOffsetMsg_init(ListDirFromOffsetMsg* msg)
{
   NetMessage_init(&msg->netMessage, NETMSGTYPE_ListDirFromOffset, &ListDirFromOffsetMsg_Ops);
}

/**
 * @param entryInfo just a reference, so do not free it as long as you use this object!
 * @param filterDots true if you don't want "." and ".." in the result list.
 */
void ListDirFromOffsetMsg_initFromEntryInfo(ListDirFromOffsetMsg* msg, const EntryInfo* entryInfo,
   int64_t serverOffset, unsigned maxOutNames, bool filterDots)
{
   ListDirFromOffsetMsg_init(msg);

   msg->entryInfoPtr = entryInfo;

   msg->serverOffset = serverOffset;

   msg->maxOutNames = maxOutNames;

   msg->filterDots = filterDots;
}


// 第一个参数 entryinfo


// 第二个参数： FsDirInfo

/*
    struct FsDirInfo
    {    
        // 用到的参数
        int64_t serverOffset; // offset for the next incremental list request to the server
                                    (equals last element of serverOffsets vector) 

        // 目前没用到的参数如下
        FsObjectInfo fsObjectInfo;

        StrCpyVec dirContents; // entry names
        UInt8Vec dirContentsTypes; // DirEntryType elements matching dirContents vector
        StrCpyVec entryIDs; // entryID elements matching dirContents vector
        Int64CpyVec serverOffsets; // dir entry offsets for telldir() matching dirContents vector
        
        
        size_t currentContentsPos; // current local pos in dirContents (>=0 && <dirContents_len)
        bool endOfDir; // true if server reached end of dir entries during last query
    };

    loff_t FsDirInfo_getServerOffset(FsDirInfo* info)
    {
        return info->serverOffset;
    }

*/


// 阶段 8 对接收到的信息的处理和判断


int ListDirFromOffsetRespMsg_getResult(ListDirFromOffsetRespMsg* msg)
{
   return msg->result;
}

uint64_t ListDirFromOffsetRespMsg_getNewServerOffset(ListDirFromOffsetRespMsg* msg)
{
   return msg->newServerOffset;
}


//反序列化-----------------------------------------------------------------------------

bool ListDirFromOffsetRespMsg_deserializePayload(NetMessage* msg, DeserializeCtx* ctx)
{
   const char* logContext = "ListDirFromOffsetRespMsg deserialization";
   LOG_printf(DEBUG,"Go into ListDirFromOffsetRespMsg_deserializePayload \n");
   ListDirFromOffsetRespMsg* thisCast = (ListDirFromOffsetRespMsg*)msg;

   // newServerOffset
   if(!Serialization_deserializeInt64(ctx, &thisCast->newServerOffset) )
     {
         LOG_printf(DEBUG,"[ListDirFromOffsetRespMsg_deserializePayload] Serialization_newServerOffset Failed\n");
         return false;
      }
      LOG_printf(DEBUG,"@@ thisCast->newServerOffset = %lu \n",thisCast->newServerOffset);

   // serverOffsets
   if(!Serialization_deserializeInt64CpyVecPreprocess(ctx, &thisCast->serverOffsetsList) )
     {
         LOG_printf(DEBUG,"[ListDirFromOffsetRespMsg_deserializePayload] Serialization_serverOffsetsList Failed\n");
         return false;
      }

   // result
   if(!Serialization_deserializeInt(ctx, &thisCast->result) )
   {
         LOG_printf(DEBUG,"[ListDirFromOffsetRespMsg_deserializePayload] Serialization_result Failed\n");
         return false;
   }

   // entryTypes
   if(!Serialization_deserializeUInt8VecPreprocess(ctx, &thisCast->entryTypesList) )
   {
         LOG_printf(DEBUG,"[ListDirFromOffsetRespMsg_deserializePayload] Serialization_entryTypesList Failed\n");
         return false;
   }

   // entryIDs
   if (!Serialization_deserializeStrCpyVecPreprocess(ctx, &thisCast->entryIDsList) )
   {
         LOG_printf(DEBUG,"[ListDirFromOffsetRespMsg_deserializePayload] Serialization_entryIDsList Failed\n");
         return false;
   }

   // names
   if(!Serialization_deserializeStrCpyVecPreprocess(ctx, &thisCast->namesList) )
   {
         LOG_printf(DEBUG,"[ListDirFromOffsetRespMsg_deserializePayload] Serialization_namesList Failed\n");
         return false;
   }

   // sanity check for equal list lengths
   if(
      (thisCast->entryTypesList.elemCount != thisCast->namesList.elemCount) ||
      (thisCast->entryTypesList.elemCount != thisCast->entryIDsList.elemCount) ||
      (thisCast->entryTypesList.elemCount != thisCast->serverOffsetsList.elemCount) ) 
   {
      // printk_fhgfs(KERN_INFO, "%s: Sanity check failed (number of list elements does not match)!\n",
         // logContext);
         LOG(DEBUG)<<"[ListDirFromOffsetRespMsg_deserializePayload] Sanity check failed (number of list elements does not match)! \n";
      return false;
   }


   return true;
}


// 接收消息 处理 
        struct StringList
        {
            PointerList pointerList;
        };

    struct StrCpyList
    {
        struct StringList stringList;
    };

struct StrCpyVec
{
   StrCpyList strCpyList;

   char** vecArray;
   size_t vecArrayLen;
};
            void PointerList_clear(PointerList* plist)
            {
                // free all elems
                PointerListElem* elem = plist->head;
                while(elem)
                {
                    PointerListElem* next = elem->next;
                    // kfree(elem);
                    // LOG(DEBUG)<<"1 free \n";
                    free(elem);
                    // LOG(DEBUG)<<"2 free \n";

                    elem = next;
                }

                // reset attributes
                plist->head = nullptr;
                plist->tail = nullptr;

                plist->length = 0;
            }

        void StringList_clear(StringList* slist)
        {
            PointerList_clear( (PointerList*)slist);
        }

    void StrCpyList_clear(StrCpyList* slist)
    {
        struct PointerListElem* elem = ( (PointerList*)slist)->head;
        while(elem)
        {
            struct PointerListElem* next = elem->next;
            // kfree(elem->valuePointer);
            // LOG(DEBUG)<<"free \n";
            // if(elem->valuePointer!=nullptr)
            free(elem->valuePointer);

            // LOG(DEBUG)<<"free after\n";

            elem = next;
        }

        StringList_clear( (StringList*)slist);
    }

void StrCpyVec_clear(StrCpyVec* vec)
{
   StrCpyList_clear( (StrCpyList*)vec);
}
// static inline void __PointerList_addTail(PointerList* list, PointerListElem* elem)
// {
//    if(list->length)
//    { // elements exist => replace tail
//       elem->prev = list->tail;
//       list->tail->next = elem;
//       list->tail = elem;
//    }
//    else
//    { // no elements exist yet
//       list->head = elem;
//       list->tail = elem;

//       elem->prev = nullptr;
//    }

//    elem->next = nullptr;
//    list->length++;
// }

// void PointerList_addTail(PointerList* list, void* valuePointer)
// {
// //    PointerListElem* elem = (PointerListElem*)os_kmalloc(sizeof(PointerListElem) );
//    //  LOG(DEBUG)<<" ------ 6613 11 ----------\n";
//    //  LOG(DEBUG)<<"sizeof(PointerListElem) = "<<sizeof(PointerListElem)<<"\n";
//    // LOG_printf(DEBUG,"1 The address stored in ptr is: %p\n", (void*)valuePointer);
//    //  void * tmp =(void *)malloc(1);
//    // LOG_printf(DEBUG,"2 The address stored in ptr is: %p\n", (void*)tmp);
    

//    PointerListElem* elem = (PointerListElem*)malloc(sizeof(PointerListElem) );
//    // LOG_printf(DEBUG,"3 The address stored in ptr is: %p\n", (void*)elem);

//    //  LOG(DEBUG)<<" ------ 6613 12 ----------\n";

//    elem->valuePointer = valuePointer;
//    //  LOG(DEBUG)<<" ------ 6613 13 ----------\n";

//    __PointerList_addTail(list, elem);
//    //  LOG(DEBUG)<<" ------ 6613 14 ----------\n";

// }


// void PointerList_append(PointerList* list, void* valuePointer)
// {
//    PointerList_addTail(list, valuePointer);
// }

void StringList_append(StringList* slist, char* valuePointer)
{
    LOG(DEBUG)<<" ------ 6613 1 ----------\n";
   PointerList_append( (PointerList*)slist, valuePointer);
    LOG(DEBUG)<<" ------ 6613 end ----------\n";

}

void StrCpyList_append(StrCpyList* slist, const char* valuePointer)
   
{   
    LOG(DEBUG)<<" ------ 661 0 ----------\n";
   size_t valueLen = strlen(valuePointer)+1;
//    size_t valueLen = strlen(valuePointer);

   LOG(DEBUG)<<"valueLen = "<<valueLen<<"\n";// 6 = 5+1 
   LOG(DEBUG)<<" ------ 661 1 ----------\n";
//    char* valueCopy = (char*)os_kmalloc(valueLen);
   char* valueCopy = (char*)malloc(valueLen);
   LOG(DEBUG)<<" ------ 661 2 ----------\n";
   LOG_printf(DEBUG,"The address stored in ptr is: %p\n", (void*)valueCopy);
   LOG_printf(DEBUG,"The address stored in ptr is: %p\n", (void*)valuePointer);

    // LOG(DEBUG)<<static_cast<void *>(valueCopy)<<"\n";
    // LOG(DEBUG)<<static_cast<void*>(valuePointer)<<"\n";

   memcpy(valueCopy, valuePointer, valueLen);//6B
   LOG(DEBUG)<<" ------ 661 3 ----------\n";

   StringList_append( (StringList*)slist, valueCopy);
   LOG(DEBUG)<<" ------ 661 4 ----------\n";

}

// size_t PointerList_length(const PointerList* list)
// {
//    return list->length;
// }

size_t StringList_length(StringList* list)
{
   return PointerList_length( (PointerList*)list);
}

size_t StrCpyList_length(StrCpyList* list)
{
   return StringList_length( (StringList*)list);
}

PointerListElem* PointerList_getTail(PointerList* list)
{
   return list->tail;
}

void StrCpyVec_append(StrCpyVec* vec, const char* valuePointer)
{
   PointerListElem* lastElem;
   char* lastElemValuePointer;
LOG(DEBUG)<<"------- 66 1 --------------\n";
   StrCpyList_append( (StrCpyList*)vec, valuePointer);
LOG(DEBUG)<<"------- 66 2 --------------\n";

   // check if we have enough buffer space for new elem
   if(StrCpyList_length( (StrCpyList*)vec) > vec->vecArrayLen)
   { // double vector array size (create new, copy, exchange, delete old)
    //   char** newVecArray = (char**)os_kmalloc(vec->vecArrayLen*sizeof(char*)*2);
LOG(DEBUG)<<"------- 66 3 --------------\n";

      char** newVecArray = (char**)malloc(vec->vecArrayLen*sizeof(char*)*2);
LOG(DEBUG)<<"------- 66 4 --------------\n";


      memcpy(newVecArray, vec->vecArray, vec->vecArrayLen*sizeof(char*) );
LOG(DEBUG)<<"------- 66 5 --------------\n";

    //   kfree(vec->vecArray);
      free(vec->vecArray);
LOG(DEBUG)<<"------- 66 6 --------------\n";

      vec->vecArrayLen = vec->vecArrayLen * 2;
      vec->vecArray = newVecArray;
LOG(DEBUG)<<"------- 66 7 --------------\n";

   }
LOG(DEBUG)<<"------- 66 8 --------------\n";

   // get last elem and add the valuePointer to the array
   lastElem = PointerList_getTail( (PointerList*)vec);

   lastElemValuePointer = (char*)lastElem->valuePointer;
LOG(DEBUG)<<"------- 66 9 --------------\n";

   (vec->vecArray)[StrCpyList_length( (StrCpyList*)vec)-1] = lastElemValuePointer;
LOG(DEBUG)<<"------- 66 10 --------------\n";

}


/**
 * Deserializes a StrCpyVec.
 * (requires pre-processing)
 *
 * @return false on error or inconsistency
 */
/*
bool Serialization_deserializeStrCpyVec(const RawList* inList, StrCpyVec* outVec)
{

   const char* listStart = inList->data;
   unsigned listBufLen = inList->length;
   unsigned i;

   // read each list element as a raw zero-terminated string
   // and make sure that we do not read beyond the specified end position
    LOG(DEBUG)<<"elemCount = "<<inList->elemCount<<"\t, length ="<<inList->length<<"\n";// 7 35
   for(i = 0; (i < inList->elemCount) && (listBufLen > 0); i++)
   {

      size_t elemLen = strlen(listStart);
      LOG(DEBUG)<<"elemLen = "<<elemLen<<"\n";
      LOG(DEBUG)<<"listStart = "<<listStart<<"\n";
      StrCpyVec_append(outVec, listStart);

      listStart += elemLen + 1; // +1 for the terminating zero 0 终止字符
      listBufLen -= elemLen + 1; // +1 for the terminating zero

   }

   return i == inList->elemCount;
}

void ListDirFromOffsetRespMsg_parseEntryIDs(ListDirFromOffsetRespMsg* msg, StrCpyVec* outEntryIDs)
{
   Serialization_deserializeStrCpyVec(&msg->entryIDsList, outEntryIDs);
}
*/
/**
 * Deserializes a StrCpyVec.
 * (requires pre-processing)
 *
 * @return false on error or inconsistency
 */
//entry
bool Serialization_deserializeStrCpyVec(const RawList* inList, char** namelist)
{

   const char* listStart = inList->data;
   unsigned listBufLen = inList->length;
   unsigned i;

   // read each list element as a raw zero-terminated string
   // and make sure that we do not read beyond the specified end position
    LOG(DEBUG)<<"elemCount = "<<inList->elemCount<<"\t, length ="<<inList->length<<"\n";// 7 35
   for(i = 0; (i < inList->elemCount) && (listBufLen > 0); i++)
   {

      size_t elemLen = strlen(listStart);
      LOG(DEBUG)<<"elemLen = "<<elemLen<<"\n";
      LOG(DEBUG)<<"input : listStart = "<<listStart<<"\n";
   
      strncpy(namelist[i],listStart,elemLen);
      namelist[i][elemLen] = '\0'; // 手动添加终止的空字符
      LOG(DEBUG)<<"output : namelist = "<<namelist[i]<<"\n";

                            //   StrCpyVec_append(outVec, listStart);

      listStart += elemLen + 1; // +1 for the terminating zero 0 终止字符
      listBufLen -= elemLen + 1; // +1 for the terminating zero

   }

   return i == inList->elemCount;
}

void ListDirFromOffsetRespMsg_parseEntryIDs(ListDirFromOffsetRespMsg* msg, char** idlist)
{
   Serialization_deserializeStrCpyVec(&msg->entryIDsList, idlist);
}


size_t StrCpyVec_length(StrCpyVec* vec)
{
   return StrCpyList_length( (StrCpyList*)vec);
}

struct UInt8List
{
   struct PointerList pointerList;
};


/**
 * Note: Derived from the corresponding list. Use the list iterator for read-only access
 */
struct UInt8Vec
{
   struct UInt8List UInt8List;

   uint8_t* vecArray;
   size_t vecArrayLen;
};

struct Int64CpyList
{
   struct PointerList pointerList;
};

struct Int64CpyVec
{
   struct Int64CpyList Int64CpyList;

   int64_t** vecArray;
   size_t vecArrayLen;
};

void Int64CpyList_clear(Int64CpyList* list)
{
   struct PointerListElem* elem = ( (PointerList*)list)->head;
   while(elem)
   {
      struct PointerListElem* next = elem->next;
    //   kfree(elem->valuePointer);
      free(elem->valuePointer);

      elem = next;
   }

   PointerList_clear( (PointerList*)list);
}

void Int64CpyVec_clear(Int64CpyVec* vec)
{
   Int64CpyList_clear( (Int64CpyList*)vec);
}

void UInt8List_clear(UInt8List* list)
{
   PointerList_clear( (PointerList*)list);
}

void UInt8Vec_clear(UInt8Vec* list)
{
   UInt8List_clear( (UInt8List*)list);
}

void UInt8List_append(UInt8List* list, uint8_t value)
{
   /* cast value directly to pointer type here to store value directly in the pointer variable
      without allocating extra mem */
   PointerList_append( (PointerList*)list, (void*)(size_t)value);
}

static inline size_t UInt8List_length(UInt8List* list)
{
   return PointerList_length( (PointerList*)list);
}


void UInt8Vec_append(UInt8Vec* thisvec, uint8_t value)
{
   size_t newListLen;

   UInt8List_append( (UInt8List*)thisvec, value);

   newListLen = UInt8List_length( (UInt8List*)thisvec);

   // check if we have enough buffer space for new elem

   if(newListLen > thisvec->vecArrayLen)
   { // double vector array size: alloc new, copy values, delete old, switch to new
    //   uint8_t* newVecArray = (uint8_t*)os_kmalloc(thisvec->vecArrayLen * sizeof(uint8_t) * 2);
      uint8_t* newVecArray = (uint8_t*)malloc(thisvec->vecArrayLen * sizeof(uint8_t) * 2);

      memcpy(newVecArray, thisvec->vecArray, thisvec->vecArrayLen * sizeof(uint8_t) );

    //   kfree(thisvec->vecArray);
      free(thisvec->vecArray);


      thisvec->vecArrayLen = thisvec->vecArrayLen * 2;
      thisvec->vecArray = newVecArray;
   }

   // add value to last array elem (determine last used index based on list length)

   (thisvec->vecArray)[newListLen-1] = value;
}

/**
 * Deserializes a UInt8Vec.
 * (requires pre-processing)
 *
 * @return false on error or inconsistency
 */
// bool Serialization_deserializeUInt8Vec(const RawList* inList, UInt8Vec* outVec) //实现的有点问题，直接注释掉
// bool Serialization_deserializeUInt8Vec(const RawList* inList, char** outVec)

// {

//    DeserializeCtx ctx = { inList->data, inList->length };
//    unsigned i;

//    // read each list element
// LOG(DEBUG)<<"inList->data = "<<inList->data<<"\n";
// for(int j=0;j<inList->length;j++)
// {  
//    char c=inList->data[j];
//    for (int i = 7; i >= 0; i--) {
//         LOG_printf(DEBUG,"%d", (c >> i) & 1);
//     }
//     LOG(DEBUG)<<"\n";
// }
// LOG(DEBUG)<<"inList->length = "<<inList->length<<"\n";

// // if(!Serialization_deserializeChar(&ctx, outVec[0]) )
// //          {
// //             LOG(DEBUG)<<"return false; \n";
// //             return false;
// //          }
// //          LOG(DEBUG)<<"len = "<<strlen(outVec[0])<<"\n";
// //     LOG(DEBUG)<<"outVec["<<i<<"] = "<<outVec[0]<<"\n";
// //     for(i=0; i < inList->elemCount; i++)
// //     {
// //       LOG(DEBUG)<<"---- "<<i<<"--- = "<<outVec[0][i]<<"\n";
// //     }
//    // for(i=0; i < inList->elemCount; i++)
//    // {
//    //  //   uint8_t value;
    
//    // // DeserializeCtx ctx = { inList->data, inList->length };
//    //  //   if(!Serialization_deserializeUInt8(&ctx, &value) )
//    //    if(!Serialization_deserializeChar(&ctx, outVec[i]) )
//    //       {
//    //          LOG(DEBUG)<<"return false; \n";
//    //          return false;
//    //       }
//    //       LOG(DEBUG)<<"len = "<<strlen(outVec[i])<<"\n";
//    //  LOG(DEBUG)<<"outVec["<<i<<"] = "<<outVec[i]<<"\n";
//    //  //   UInt8Vec_append(outVec, value);

//    // }

//    return true;
// }

// void ListDirFromOffsetRespMsg_parseEntryTypes(ListDirFromOffsetRespMsg* msg,UInt8Vec* outEntryTypes)
// void ListDirFromOffsetRespMsg_parseEntryTypes(ListDirFromOffsetRespMsg* msg,char** outEntryTypes)
void ListDirFromOffsetRespMsg_parseEntryTypes(ListDirFromOffsetRespMsg* msg,int * outEntryTypes)

{
// LOG(DEBUG)<<"------ 51 ---------\n";
// LOG(DEBUG)<<"inList->data = "<<msg->entryTypesList.data<<"\n";
for(int j=0;j<msg->entryTypesList.length;j++)
{  
   char c=msg->entryTypesList.data[j];
   // for (int i = 7; i >= 0; i--) {
   //      LOG_printf(DEBUG,"%d", (c >> i) & 1);
   //  }
    outEntryTypes[j]=c;

   //  LOG(DEBUG)<<"\n";
}
LOG(DEBUG)<<"inList->length = "<<msg->entryTypesList.length<<"\n";


   // Serialization_deserializeUInt8Vec(&msg->entryTypesList, outEntryTypes);
LOG(DEBUG)<<"------ 5 end---------\n";

}

// void ListDirFromOffsetRespMsg_parseNames(ListDirFromOffsetRespMsg* msg, StrCpyVec* outNames)
void ListDirFromOffsetRespMsg_parseNames(ListDirFromOffsetRespMsg* msg, char** outNames)
{
   Serialization_deserializeStrCpyVec(&msg->namesList, outNames);
}

void Int64CpyList_append(Int64CpyList* list, int64_t value)
{
//    int64_t* valueCopyPointer = (int64_t*)os_kmalloc(sizeof(int64_t) );
   int64_t* valueCopyPointer = (int64_t*)malloc(sizeof(int64_t) );


   *valueCopyPointer = value;

   PointerList_append( (PointerList*)list, valueCopyPointer);
}

static inline size_t Int64CpyList_length(Int64CpyList* list)
{
   return PointerList_length( (PointerList*)list);
}

void Int64CpyVec_append(Int64CpyVec* vec, int64_t value)
{
   PointerListElem* lastElem;
   int64_t* lastElemValuePointer;

   Int64CpyList_append( (Int64CpyList*)vec, value);

   // check if we have enough buffer space for new elem
   if(Int64CpyList_length( (Int64CpyList*)vec) > vec->vecArrayLen)
   { // double vector array size (create new, copy, exchange, delete old)
      int64_t** newVecArray =
         (int64_t**)malloc(vec->vecArrayLen*sizeof(int64_t*)*2);
      memcpy(newVecArray, vec->vecArray, vec->vecArrayLen*sizeof(int64_t*) );
      free(vec->vecArray);
      vec->vecArrayLen = vec->vecArrayLen * 2;
      vec->vecArray = newVecArray;
   }

   // get last elem and add the valuePointer to the array
   lastElem = PointerList_getTail( (PointerList*)vec);
   lastElemValuePointer = (int64_t*)lastElem->valuePointer;
   (vec->vecArray)[Int64CpyList_length( (Int64CpyList*)vec)-1] = lastElemValuePointer;
}


/**
 * Deserializes a Int64CpyVec.
 * (requires pre-processing)
 *
 * @return false on error or inconsistency
 */
// bool Serialization_deserializeInt64CpyVec(const RawList* inList, Int64CpyVec* outVec)
bool Serialization_deserializeInt64CpyVec(const RawList* inList, int64_t* outVec)

{
   DeserializeCtx ctx = { inList->data, inList->length };
   unsigned i;

   // read each list element
   for(i=0; i < inList->elemCount; i++)
   {
      int64_t value;

    //   if(!Serialization_deserializeInt64(&ctx, &value) )
      if(!Serialization_deserializeInt64(&ctx, &outVec[i]) )

         return false;
    LOG(DEBUG)<<"outVec["<<i<<"] = "<<outVec[i]<<"\n";
    //   Int64CpyVec_append(outVec, value);
   }

   return true;
}
// void ListDirFromOffsetRespMsg_parseServerOffsets(ListDirFromOffsetRespMsg* msg,Int64CpyVec* outServerOffsets)
void ListDirFromOffsetRespMsg_parseServerOffsets(ListDirFromOffsetRespMsg* msg,int64_t* outServerOffsets)
{
   Serialization_deserializeInt64CpyVec(&msg->serverOffsetsList, outServerOffsets);
}

size_t UInt8Vec_length(UInt8Vec* vec)
{
   return UInt8List_length( (UInt8List*)vec);
}

size_t Int64CpyVec_length(Int64CpyVec* vec)
{
   return Int64CpyList_length( (Int64CpyList*)vec);
}

// ----------------------------
/*
void FsDirInfo_setServerOffset(FsDirInfo* info, int64_t serverOffset)
{
   info->serverOffset = serverOffset;
}

void ListDirFromOffsetRespMsg_parseEntryTypes(ListDirFromOffsetRespMsg* msg,
   UInt8Vec* outEntryTypes)
{
   Serialization_deserializeUInt8Vec(&msg->entryTypesList, outEntryTypes);
}

void ListDirFromOffsetRespMsg_parseNames(ListDirFromOffsetRespMsg* msg, StrCpyVec* outNames)
{
   Serialization_deserializeStrCpyVec(&msg->namesList, outNames);
}



void ListDirFromOffsetRespMsg_parseServerOffsets(ListDirFromOffsetRespMsg* msg,
   Int64CpyVec* outServerOffsets)
{
   Serialization_deserializeInt64CpyVec(&msg->serverOffsetsList, outServerOffsets);
}
*/

//readdir.cpp 的关键函数实现在这里
// void readdir(Socket* sock,char* searchparentEntryID,char* searchentryID,char* searchDirName,DirEntryType searchentryType)
// void readdir(Socket* sock,char* searchparentEntryID,char* searchentryID,char* searchDirName,DirEntryType searchentryType,char** Outnamelist,char** Outidlist,int* Outtypelist,int &OutDirElemCnt)
void readdir(Socket* &sock,char* searchparentEntryID,char* searchentryID,char* searchDirName,DirEntryType searchentryType,char** &Outnamelist,char** &Outidlist,int* &Outtypelist,int &OutDirElemCnt,uint32_t* file_num)

{

   
   ssize_t respRes = 0;
   ssize_t sendRes;
    unsigned sendBufLen= sendBufLen = 1024; //@@ 设置必须大于 111
    unsigned int recvBufLen=2*1024*1024;//470*1024*1024;//TODO 1024;            //@@ 接收缓冲区长度，这个长度决定了 bufLen 的长度

   // rrArgs->outRespBuf = nullptr;
   // rrArgs->outRespMsg = nullptr;

    LOG_printf(DEBUG,"---------1. 创建套接字 -------------\n");
      
            if (sock == NULL) //传参的时候确实看到sock==NULL
                                                // sock = NodeConnPool_acquireStreamSocket(connPool);
                                                // NodeConnPool_acquireStreamSocketEx(connPool, true, NULL);
                                                // sock = (Socket*)StandardSocket_construct(PF_INET, SOCK_STREAM, IPPROTO_TCP);
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
     
     
        
                unsigned short port=8005;//@@
            NicAddress nicAddr[10]; 
            initNicAddressArray_meta(nicAddr);//@@ 
            bool connectRes;
            connectRes = sock->ops->connectByIP(sock, nicAddr[4].ipAddr, port);//@@ 224: nicAddr[8] ;162 :  nicAddr; [3] 192.168.5.210 ens4f0np0 ;[2] 192.168.0.216  ens5f0np0

        

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
     
    // 这个也可以改成直接赋值 Ps: 获取sendbuflen 可以直接赋值，而且我看函数逻辑其实这里返回的是消息头部的长度 但是 这个getmessagelength有另外的功能是对头部信息序列化
                    //NoAllocBufferStore* bufStore = app->msgBufStore;//初始化直接改成app->msgBuffStore的具体赋值
    NoAllocBufferStore* bufStore=(NoAllocBufferStore*)malloc(sizeof(NoAllocBufferStore));
    bufStore->numBufs = 3;
    bufStore->bufSize =recvBufLen; //@@ 40 就会消息太大（已经解决了） 设置必须大于 111 
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
            //if(rrArgs->respBufType == MessagingTkBufType_BufStore)//根据响应缓冲区类型，从缓冲区存储中获取或分配缓冲区，回溯前面的代码 确实响应缓冲区的类型是这个
        if(respBufType == MessagingTkBufType_BufStore)//根据响应缓冲区类型，从缓冲区存储中获取或分配缓冲区，回溯前面的代码 确实响应缓冲区的类型是这个
        { 
            

            bufLen = bufStore->bufSize;// 获取缓冲区长度

            if(bufLen < sendBufLen )// 如果缓冲区长度小于发送缓冲区长度
            { 
                LOG_printf(DEBUG,"buflen<sendBuflen\n");
            }
                // rrArgs->outRespBuf = NoAllocBufferStore_waitForBuf(bufStore);

            outRespBuf = NoAllocBufferStore_waitForBuf(bufStore);// 这个函数其实就是从 bufStore->bufArray[] 取上一个，我定义的时候是有10个256Byte的缓冲区

                //LOG_printf(DEBUG,"Buffer received: %p\n", (void*)rrArgs->outRespBuf);  // 打印获取的缓冲区地址
            LOG_printf(DEBUG,"Buffer received: %p\n", (void*)outRespBuf);  // 打印获取的缓冲区地址

        }
      
   LOG_printf(DEBUG,"--------------------5.准备请求信息-------------------\n");
    
     // requestMsg 赋值
     ListDirFromOffsetMsg requestMsg;
     
         
          // 第一个参数 
          EntryInfo entryInfo;
        //   EntryInfo_init(&entryInfo);
        bool isGroup=false;
        uint32_t NodeNumID=1;
        EntryInfo_init(&entryInfo,searchparentEntryID,searchentryID,searchDirName,searchentryType,isGroup, NodeNumID);
         

          // 第二个参数
            //serverOffset 向上追溯 dirInfo ，不行的话直接给它赋值 // int64_t serverOffset = FsDirInfo_getServerOffset(dirInfo);//传入参数
            // 直接赋值
            int64_t serverOffset = 0;

          // 第三个参数
        unsigned maxOutNames = 10450;
         
         // prepare request 在准备请求信息
   ListDirFromOffsetMsg_initFromEntryInfo(
      &requestMsg, &entryInfo, serverOffset, maxOutNames, false);


   LOG_printf(DEBUG,"--------------------6.请求消息序列化-------------------\n");

      //5. 序列化
      //在网络通信中，序列化用于将消息转换为字节流，以便在网络上发送。将rrArgs->requestMsg 序列化后存放在rrArgs->outRespBuf
            // NetMessage_serialize(rrArgs->requestMsg, rrArgs->outRespBuf, sendBufLen); //往缓冲区中放请求的信息
      NetMessage_serialize((NetMessage*)&requestMsg, outRespBuf, sendBufLen); //往缓冲区中放请求的信息
    unsigned int serialize_len =NetMessage_getMsgLength((NetMessage*)&requestMsg);
    LOG(DEBUG)<<"serialize_len = "<<serialize_len<<"sendBufLen = "<<sendBufLen<<"\n";

      //LOG(DEBUG)<<"msgHeader.msgLength = "<<((NetMessage*)&requestMsg)->msgHeader.msgLength<<"\n";//1729519185
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
            //  LOG_printf(DEBUG,"send request len is %d\n",sendBufLen); // 输出 是 1024
             LOG_printf(DEBUG,"send request len is %d\n",serialize_len); // 输出 是 1024
            

    //   struct iovec  *iter = STACK_ALLOC_BEEGFS_ITER_KVEC(outRespBuf, sendBufLen);
      struct iovec  *iter = STACK_ALLOC_BEEGFS_ITER_KVEC(outRespBuf, serialize_len);





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


      
      if(sendRes != (ssize_t)serialize_len)
         {
            LOG_printf(DEBUG,"sendRes != serialize_len \n");
         }
   
   LOG_printf(DEBUG,"--------------------7. 接收消息-------------------\n");
      //7. receive response
                // respRes = MessagingTk_recvMsgBuf(app, sock, rrArgs->outRespBuf, bufLen);
                LOG(DEBUG)<<"bufLen = "<<bufLen<<"\n";
                unsigned int msgLength = NetMessage_extractMsgLengthFromBuf(outRespBuf);
                LOG(DEBUG)<<"msgLength = "<<msgLength<<"\n";
      respRes = MessagingTk_recvMsgBuf(sock, outRespBuf, bufLen);

      if(respRes <= 0) 
      { 
         LOG_printf(DEBUG,"respRes<=0 \n");
      }
      else if(respRes>bufLen)
      {
         LOG_printf(DEBUG," respRes>bufLen  \n");
      }
   
   LOG_printf(DEBUG,"--------------------8. 反序列化解析-------------------\n");
      
      //8. 反序列化解析
            // rrArgs->outRespMsg = NetMessageFactory_createFromBuf(app, rrArgs->outRespBuf, respRes);
       // std::unique_ptr<NetMessage> outRespMsg;
         //NetMessage* outRespMsg=new NetMessage();
         LOG(DEBUG)<<"respRes = "<<respRes<<"\n";
      NetMessage* outRespMsg = NetMessageFactory_createFromBuf(outRespBuf, respRes);//这里面是反序列化的操作
      LOG(DEBUG)<<"完成反序列化 \n";
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
      //LOG(DEBUG)<<"outRespMsg->msgHeader.msgType = "<<outRespMsg->msgHeader.msgType<<"\n";
      if(outRespMsg->msgHeader.msgType!=NETMSGTYPE_OpenFileResp && outRespMsg->msgHeader.msgType!=NETMSGTYPE_ListDirFromOffsetResp)
      {
         LOG(DEBUG)<<"Received invalid response type:"<<outRespMsg->msgHeader.msgType<<"\n";
      }

    ListDirFromOffsetRespMsg* listDirResp = (ListDirFromOffsetRespMsg*)outRespMsg;
    if(listDirResp ==nullptr)
    {
        LOG(DEBUG)<<"listDirResp ==null \n";
    }

   int retVal = (FhgfsOpsErr)ListDirFromOffsetRespMsg_getResult(listDirResp);
   LOG(DEBUG)<<"retVal = "<<retVal<<"\n";
   if(retVal == FhgfsOpsErr_SUCCESS)//XYP_DEBUG
   {
      
      LOG(DEBUG)<<"retVal == FhgfsOpsErr_SUCCESS \n ";
    LOG(DEBUG)<<listDirResp->entryIDsList.elemCount<<"\n";
    LOG(DEBUG)<<listDirResp->entryTypesList.elemCount<<"\n";
    LOG(DEBUG)<<listDirResp->serverOffsetsList.elemCount<<"\n";

    LOG(DEBUG)<<"----------- 8-1 解析 namelist  ------------------------------------------------------------\n";


   //  char ** namelist=nullptr;
    int elemcnt=listDirResp->namesList.elemCount;
    OutDirElemCnt=elemcnt;
    LOG(DEBUG)<<"elemcnt = "<<elemcnt<<"\n";
// 分配指针数组
   if(Outnamelist==nullptr)
    {
      LOG(DEBUG)<<" 分配 namelist \n";
         Outnamelist = (char**)malloc(elemcnt * sizeof(char*)); // 为指针数组分配内存
      if (Outnamelist == nullptr) {
         // 如果内存分配失败，打印错误信息并退出
         std::cerr << "Memory allocation failed for namelist." << std::endl;
         return ;
      }
      for(int i=0;i<elemcnt;i++)
      {   
         Outnamelist[i]=(char *)malloc(sizeof(char)*25);
         if (Outnamelist[i] == nullptr) {
               // 如果内存分配失败，打印错误信息并释放之前分配的内存
               std::cerr << "Memory allocation failed for namelist[" << i << "]." << std::endl;

               // 释放已分配的内存
               for (int j = 0; j < i; j++) {
                  free(Outnamelist[j]);
               }
               free(Outnamelist); // 释放指针数组
               return ;
         }
      }
    }
    // 解析 namelist
      ListDirFromOffsetRespMsg_parseNames(listDirResp, Outnamelist);//如何解析出namelist
      if(Outnamelist==nullptr)
      {
         LOG(DEBUG)<<" Outnamelist==nullptr \n";
      }
      LOG(DEBUG)<<"outnamelist = "<<Outnamelist<<"\n";
    // //释放内存
   
    // for (int i = 0; i < elemcnt; i++) {
    //     free(namelist[i]); // 释放每个字符串的内存
    // }
    //     free(namelist); // 释放指针数组

    LOG(DEBUG)<<"----------- 8-2 解析 IDlist  ------------------------------------------------------------\n";
   //  char ** idlist=nullptr;
    elemcnt=listDirResp->entryIDsList.elemCount;
    LOG(DEBUG)<<"elemcnt = "<<elemcnt<<"\n";
    // 分配指针数组
    if(Outidlist==nullptr)
    {
         Outidlist = (char**)malloc(elemcnt * sizeof(char*)); // 为指针数组分配内存
      if (Outidlist == nullptr) {
         // 如果内存分配失败，打印错误信息并退出
         std::cerr << "Memory allocation failed for idlist." << std::endl;
         return ;
      }
      for(int i=0;i<elemcnt;i++)
      {   
         Outidlist[i]=(char *)malloc(sizeof(char)*25);
         if (Outidlist[i] == nullptr) {
               // 如果内存分配失败，打印错误信息并释放之前分配的内存
               std::cerr << "Memory allocation failed for idlist[" << i << "]." << std::endl;

               // 释放已分配的内存
               for (int j = 0; j < i; j++) {
                  free(Outidlist[j]);
               }
               free(Outidlist); // 释放指针数组
               return ;
         }
      }
    }
    // 解析 IDlist
      ListDirFromOffsetRespMsg_parseEntryIDs(listDirResp, Outidlist);
      LOG(DEBUG)<<"outnamelist2 = "<<Outnamelist<<"\n";


    

    

    LOG(DEBUG)<<"----------- 8-3 解析 typelist  ------------------------------------------------------------\n";
    //PS: 这个解析感觉有点问题，反序列化有点奇怪，用char * 字符的 反序列化是和 ctx->data="R" 是一样的，但是用 uint8_t 得到的值是空的没有输出
    //现在没有错误了 ，PS: 241212修改更正
                                                    {
                                                      // char** typelist=nullptr;
                                                    //     elemcnt=listDirResp->entryTypesList.elemCount;
                                                    //     LOG(DEBUG)<<"elemcnt = "<<elemcnt<<"\n";
                                                    //     // 分配指针数组
                                                    //     typelist = (char**)malloc(elemcnt * sizeof(char*)); // 为指针数组分配内存
                                                    //     if (typelist == nullptr) {
                                                    //         // 如果内存分配失败，打印错误信息并退出
                                                    //         std::cerr << "Memory allocation failed for typelist." << std::endl;
                                                    //         return ;
                                                    //     }
                                                    //     for(int i=0;i<elemcnt;i++)
                                                    //     {
                                                    //         typelist[i]=(char*)malloc(sizeof(char *)*25);
                                                    //         LOG(DEBUG)<<"len = "<<strlen(typelist[i])<<"\n";
                                                    //         if (typelist[i] == nullptr) {
                                                    //             // 如果内存分配失败，打印错误信息并释放之前分配的内存
                                                    //             std::cerr << "Memory allocation failed for typelist[" << i << "]." << std::endl;

                                                    //             // 释放已分配的内存
                                                    //             for (int j = 0; j < i; j++) {
                                                    //                 free(typelist[j]);
                                                    //             }
                                                    //             free(typelist); // 释放指针数组
                                                    //             return ;
                                                    //         }
                                                    //         // 初始化字符串为空
                                                    //         // typelist[i][0] = '\0'; // 设置第一个字符为 '\0'
                                                    //         LOG(DEBUG)<<"len = "<<strlen(typelist[i])<<"\n";

                                                    //     }
                                                    }
    
// int * Outtypelist=nullptr;
    elemcnt=listDirResp->entryTypesList.elemCount;
    LOG(DEBUG)<<"elemcnt = "<<elemcnt<<"\n";
    // 分配指针数组
    if(Outtypelist==nullptr)
    {
            Outtypelist = (int*)malloc(elemcnt * sizeof(int)); // 为指针数组分配内存
         if (Outtypelist == nullptr) {
            // 如果内存分配失败，打印错误信息并退出
            std::cerr << "Memory allocation failed for typelist." << std::endl;
            return ;
         }
    }
    
//解析 entrytype
      ListDirFromOffsetRespMsg_parseEntryTypes(listDirResp, Outtypelist);
    LOG(DEBUG)<<"-------print typelist \n";
    int file_cnt=0;//统计文件的数量
    for(int j=0;j<elemcnt;j++)
    {
      //   LOG(DEBUG)<<Outtypelist[j]<<" ";
        if(Outtypelist[j]==DirEntryType_REGULARFILE)
        {
            file_cnt++;
        }
    }
    *file_num=file_cnt;
    LOG(DEBUG)<<"\n";

      LOG(DEBUG)<<"outnamelist3 = "<<Outnamelist<<"\n";

//  LOG_printf(DEBUG,"------- 依次 open 目录中所有文件的句柄ID ------------");

//  for(int j=0;j<listDirResp->entryTypesList.elemCount;j++)
//  {
//     if(typelist[j]==DirEntryType_REGULARFILE)//正常的文件就
//     {

//     }
//     else if(typelist[j]==DirEntryType_DIRECTORY)
//     {
//         //如果是目录的话需要需要readdir 该目录
//     }
//  }




//统一释放空间 



    //释放内存 namelist
   
    /*
      for (int i = 0; i < elemcnt; i++) {
         free(namelist[i]); // 释放每个字符串的内存
      }
         free(namelist); // 释放指针数组
      //释放内存 idlist
      for (int i = 0; i < elemcnt; i++) {
               free(idlist[i]); // 释放每个字符串的内存
         }
      free(idlist); // 释放指针数组
   //释放内存 typelist
      // for (int i = 0; i < elemcnt; i++) {
      //             free(typelist[i]); // 释放每个字符串的内存
      //         }
      free(typelist); // 释放指针数组
    
    */

    LOG(DEBUG)<<"----------- 8-4 解析 serverOffsets  ------------------------------------------------------------\n";

    int64_t new_offset =listDirResp->newServerOffset;
    LOG(DEBUG)<<"new_offset = "<<new_offset<<"\n";

// 分配指针数组
    int64_t* offsetlist=nullptr;
    elemcnt=listDirResp->serverOffsetsList.elemCount;
    LOG(DEBUG)<<"elemcnt = "<<elemcnt<<"\n";

    
    offsetlist = (int64_t*)malloc(elemcnt * sizeof(int64_t)); // 为指针数组分配内存
    if (offsetlist == nullptr) {
        // 如果内存分配失败，打印错误信息并退出
        std::cerr << "Memory allocation failed for offsetlist." << std::endl;
        return ;
    }
    
//解析 serveroffset
      
    ListDirFromOffsetRespMsg_parseServerOffsets(listDirResp, offsetlist);

//释放内存
    free(offsetlist); // 释放指针数组

      LOG(DEBUG)<<"outnamelist4 = "<<Outnamelist<<"\n";


    
   
   }
   else
   {
        if(retVal == FhgfsOpsErr_PATHNOTEXISTS)
        {
            LOG(DEBUG)<<"retval = FhgfsOpsErr_PATHNOTEXISTS \n";
        }
        else
        {
            LOG(DEBUG)<<"retval = "<<retVal<<"\n";
        }
   }

      LOG(DEBUG)<<"outnamelist 5= "<<Outnamelist<<"\n";
    
     delete iter; 
   // 释放套接字
   // NodeConnPool_releaseStreamSocket(sock);
   
   // 释放资源
    for (size_t i = 0; i < bufStore->numBufs; ++i) {
        free(bufStore->bufArray[i]);  // 释放每个缓冲区的内存
    }
    free(bufStore->bufArray);  // 释放缓冲区数组
    Condition_destroy(&bufStore->newBufCond);  // 销毁条件变量
    
      LOG(DEBUG)<<"outnamelist 6= "<<Outnamelist<<"\n";
      for(int j=0;j<OutDirElemCnt;j++)
      {
         LOG(DEBUG)<<j<<"----"<<Outnamelist[j]<<"\n";
      }
   return ;
}

#endif
