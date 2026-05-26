# 编译器配置
CC = gcc
CFLAGS = -Wall -Wextra -g -I.
LDFLAGS = 
LDLIBS = -lrt  # 链接实时库（用于gettimeofday）

# 源文件列表
SRC = test.c test1.c avg.c test_decom.c test_decom_all.c

# 自动生成目标文件列表（与源文件同名）
TARGETS = $(SRC:.c=)

# 默认构建目标
all: $(TARGETS)

# 通用编译规则
%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

# 清理构建产物
clean:
	rm -f $(TARGETS)

# 安装到系统路径
install: $(TARGETS)
	cp $(TARGETS) /usr/local/bin/

# 卸载程序
uninstall:
	rm -f /usr/local/bin/$(TARGETS)

.PHONY: all clean install uninstall