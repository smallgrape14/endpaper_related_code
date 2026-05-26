# GDS性能测试Makefile - 修复版本
CC = gcc-11
CXX = g++-11
NVCC = nvcc
CFLAGS = -std=c++14 -O2 -g
NVCC_FLAGS = -std=c++14 -O2 -g -arch=sm_70 -ccbin /usr/bin/gcc-11 --compiler-options -Wall
LIBS = -lcufile -lcudart
TARGET = gds_benchmark
SRCS = test1.cu
OBJS = $(SRCS:.cu=.o)

# 默认目标
all: check-gds $(TARGET)

# 检查GDS支持
check-gds:
	@echo "=== 检查GDS支持环境 ==="
	@if ! command -v $(NVCC) >/dev/null 2>&1; then \
		echo "错误: nvcc未找到，请安装CUDA工具包"; \
		exit 1; \
	fi
	@if ! command -v gcc-11 >/dev/null 2>&1; then \
		echo "安装兼容的GCC版本..."; \
		sudo apt-get update && sudo apt-get install -y gcc-11 g++-11; \
	fi
	@echo "GCC版本: $$(gcc-11 --version | head -1)"
	@echo "CUDA版本: $$($(NVCC) --version | tail -1)"
	@echo "环境检查完成"

# 链接目标文件生成可执行文件
$(TARGET): $(OBJS)
	$(NVCC) $(NVCC_FLAGS) -o $@ $(OBJS) $(LIBS)

# 编译规则
%.o: %.cu
	$(NVCC) $(NVCC_FLAGS) -c $< -o $@

# 调试版本
debug: NVCC_FLAGS += -DDEBUG -G
debug: $(TARGET)

# 发布版本（优化）
release: NVCC_FLAGS += -DNDEBUG -O3
release: $(OBJS)
	$(NVCC) $(NVCC_FLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# 性能分析版本
profile: NVCC_FLAGS += -pg -lineinfo
profile: LIBS += -pg
profile: $(TARGET)

# 清理生成的文件
clean:
	rm -f $(OBJS) $(TARGET) *.csv gmon.out core*

# 安装依赖（Ubuntu/Debian）
install-deps:
	@echo "安装必要的依赖包..."
	sudo apt-get update
	sudo apt-get install -y gcc-11 g++-11 nvidia-cuda-toolkit
	@echo "依赖安装完成"

# 运行基本测试（修复重复定义）
run-test: $(TARGET)
	@echo "=== 运行GDS基本测试 ==="
	./$(TARGET) . gds_test_results.csv

# 运行性能基准测试
run-benchmark: $(TARGET)
	@echo "=== 运行GDS性能基准测试 ==="
	@echo "测试当前目录..."
	./$(TARGET) . gds_benchmark_results.csv

# 运行10次性能测试
run-10x: $(TARGET)
	@echo "=== 运行10次GDS性能测试 ==="
	@for i in 1 2 3 4 5 6 7 8 9 10; do \
		echo "第 $$i 次运行..."; \
		./$(TARGET) . gds_performance_run_$$i.csv; \
	done

# 生成汇总报告
generate-report: run-10x
	@echo "=== 生成性能汇总报告 ==="
	@if [ -f gds_performance_run_1.csv ]; then \
		echo "汇总10次测试结果..."; \
		head -5 gds_performance_run_1.csv > gds_summary_report.csv; \
		for i in 1 2 3 4 5 6 7 8 9 10; do \
			tail -n +2 gds_performance_run_$$i.csv | head -1 >> gds_summary_report.csv; \
		done; \
		echo "汇总报告已生成: gds_summary_report.csv"; \
	else \
		echo "请先运行 'make run-10x' 生成测试数据"; \
	fi

# 系统信息检查
system-info:
	@echo "=== 系统环境信息 ==="
	@echo "主机名: $$(hostname)"
	@echo "操作系统: $$(uname -a)"
	@echo "GPU信息:"
	@nvidia-smi --query-gpu=name,driver_version,memory.total --format=csv,noheader 2>/dev/null || echo "NVIDIA驱动未安装"
	@echo "CUDA设备:"
	@nvidia-smi -L 2>/dev/null || echo "无法获取GPU列表"
	@echo "存储设备:"
	@df -h | grep -E "(^Filesystem|/dev/sd|/dev/nvme)" | head -5

# 验证GDS功能
verify-gds: $(TARGET)
	@echo "=== 验证GDS功能 ==="
	@echo "创建测试文件..."
	@mkdir -p test_data
	@dd if=/dev/zero of=test_data/test_file_1GB.bin bs=1M count=1024 status=progress 2>/dev/null
	@echo "运行GDS测试..."
	./$(TARGET) test_data gds_verification_results.csv
	@echo "清理测试文件..."
	@rm -rf test_data

# 帮助信息
help:
	@echo "GDS性能测试Makefile目标:"
	@echo "  all             - 编译程序（包含环境检查）"
	@echo "  check-gds       - 检查GDS支持环境"
	@echo "  install-deps    - 安装必要的依赖包"
	@echo "  run-test        - 运行基本测试"
	@echo "  run-benchmark   - 运行性能基准测试"
	@echo "  run-10x         - 运行10次性能测试"
	@echo "  generate-report - 生成汇总报告"
	@echo "  system-info     - 显示系统环境信息"
	@echo "  verify-gds      - 验证GDS功能"
	@echo "  clean           - 清理生成的文件"
	@echo "  debug           - 编译调试版本"
	@echo "  release         - 编译发布版本"
	@echo "  profile         - 编译性能分析版本"

# 声明伪目标
.PHONY: all clean check-gds install-deps run-test run-benchmark run-10x generate-report system-info verify-gds help debug release profile