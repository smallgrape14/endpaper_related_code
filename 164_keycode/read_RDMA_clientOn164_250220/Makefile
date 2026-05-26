# 定义编译器
CXX = g++

# 定义编译器标志
CFLAGS = -w -std=c++11 -O3
CFLAGS += -DBEEGFS_NVFS
# CFLAGS += -DBEEGFS_RDMA

# CFLAGS = -w -std=c++11 -O3 # 关闭所有的警告信息
# CFLAGS = -Wall -std=c++11 -O3
# CFLAGS = -Wall -std=gnu++11 -O3 //for use typeof


# 定义链接库
# LIBS = -lpthread -lm -lrdmacm -libverbs -BEEGFS_RDMA -BEEGFS_NVFS
# LIBS = -lpthread -lm -lrdmacm -libverbs 
LIBS = -lpthread -lm -lrdmacm -libverbs 



# 定义需要包含的头文件目录
# INCLUDES = -I/usr/src/linux-5.10.213/include/  -I.
INCLUDES = -I.
# INCLUDES = 


# -I/usr/src/linux-5.10.213/include/  -I.
# /usr/src/linux-5.10.213/include/ 
# /usr/src/linux-headers-6.8.0-40-generic/include

# 定义目标文件名
TARGET = test2

# 定义源文件
# SRC = read.cpp new_read.hpp open.hpp   # TCP 
# SRC = read.cpp readfilevec.hpp new_read.hpp RDMAConnection.hpp IBVSocket.hpp open.hpp	 #RDMA
SRC = read.cpp new_read.hpp read_pre.hpp RDMAConnection.hpp IBVSocket.hpp open.hpp	 #RDMA


# new_read.hpp open.hpp
#read.cpp read_communicate.cpp read.h open.hpp 

# 默认目标
all: $(TARGET)

# 链接目标
$(TARGET): $(SRC)
	$(CXX) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

# 清理目标
clean:
	rm -f $(TARGET)

# 确保当make被调用时，目录存在
.PHONY: all clean

# g++ -Wall -std=c++11 -O3 -I/usr/src/linux-5.10.213/include/  -o test2 open完整.cpp -lpthread -lm 

# g++ -Wall -std=c++11 -O3 -I/usr/src/linux-5.10.213/include/  -o open open.cpp -lpthread -lm 
