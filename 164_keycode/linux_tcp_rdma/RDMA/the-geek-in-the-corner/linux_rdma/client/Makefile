# 设置编译器
CC = gcc

# 设置目标文件名
TARGET = client

# 设置源文件
SOURCES = client.c

# 设置编译器标志
CFLAGS = -Wall -g
LDFLAGS = -lrdmacm -libverbs -lpthread

# 默认目标
all: $(TARGET)

# 编译目标文件
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

# 清理生成的文件
clean:
	rm -f $(TARGET)