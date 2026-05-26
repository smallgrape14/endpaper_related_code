#ifndef RDMA_MESSAGES_H
#define RDMA_MESSAGES_H

const char *DEFAULT_PORT = "12345";
const size_t BUFFER_SIZE = 10 * 1024 * 1024;//10MB
#define MAX_FILE_NAME 256
#define MAX_FILE_NUM 10
const size_t MAX_FILE_SIZE = 50*1024*1024;//100MB
enum message_id
{
  MSG_INVALID = 0,
  MSG_MR,
  MSG_READY,
  MSG_DONE
};

struct message
{
  int id;
  char file_name[MAX_FILE_NAME];
  union
  {
    struct
    {
      uint64_t addr;
      uint32_t rkey;
    } mr;
  } data;

};

#endif
