# 定义编译器
CC = gcc

# 定义编译选项
CFLAGS = -Wall -O2 -g
LDFLAGS = -libverbs -lrdmacm -lpthread

# 定义目标文件
TARGETS = server client

# 定义源文件和目标文件
SRCS = server.c client.c
OBJS = $(SRCS:.c=.o)

# 默认目标
all: $(TARGETS)

# 编译服务端
server: server.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# 编译客户端
client: client.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# 编译.c文件为.o文件
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理生成的文件
clean:
	rm -f $(TARGETS) $(OBJS)

# 默认运行目标
.PHONY: all clean