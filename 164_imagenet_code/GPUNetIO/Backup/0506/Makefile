# 定义编译器和链接器
NVCC = nvcc
CXX = g++

# 定义CUDA和DOCA的安装路径（根据实际情况修改）
CUDA_PATH = /usr/local/cuda

# 定义编译和链接选项
CXXFLAGS = -std=c++11 -I$(CUDA_PATH)/include 
# LDFLAGS = -L$(CUDA_PATH)/lib64 -ldl -lrt -lpthread -lcudart -ldoca_runtime
LDFLAGS = -L$(CUDA_PATH)/lib64 -ldl -lrt -lpthread -lcudart


# 定义源文件和目标文件
SRC = dataloader.cpp
OBJ = dataloader.o
TARGET = export_gpu_data

# 定义默认目标
all: $(TARGET)

# 编译目标文件
$(OBJ): $(SRC)
	$(NVCC) -c $(SRC) -o $(OBJ) $(CXXFLAGS)

# 链接目标文件生成可执行文件
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

# 清理生成的文件
clean:
	rm -f $(OBJ) $(TARGET)