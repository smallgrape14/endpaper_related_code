# Makefile

# 编译器
CC = g++

# 编译选项
CFLAGS = -Wall -Wextra -pedantic -pthread

# 源文件列表
# SOURCES = read.cpp read_perfile.cpp read_vary_iosize.cpp
SOURCES = read.cpp


# 可执行文件列表
TARGETS = $(SOURCES:.cpp=)

# 默认目标
all: $(TARGETS)

# 编译规则
%: %.cpp
	$(CC) $(CFLAGS) -o $@ $<

# 清理生成的文件
clean:
	rm -f $(TARGETS)

# # Makefile

# # 编译器
# CC = g++

# # 编译选项
# CFLAGS = -Wall -Wextra -pedantic -pthread

# # 目标文件
# TARGET = read

# # 源文件
# SOURCES = read.cpp

# # 默认目标
# all: $(TARGET)

# # 编译目标
# $(TARGET): $(SOURCES)
# 	$(CC) $(CFLAGS) -o $@ $^

# # 清理生成的文件
# clean:
# 	rm -f $(TARGET)