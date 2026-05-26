# Makefile

# 编译器
CC = gcc

# 编译选项
CFLAGS = -Wall -Wextra -pedantic

# 目标文件
TARGET = test

# 源文件
SOURCES = test.c

# 默认目标
all: $(TARGET)

# 编译目标
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^

# 清理生成的文件
clean:
	rm -f $(TARGET)