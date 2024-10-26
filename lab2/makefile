# 编译器
CXX = g++

# 编译选项，使用 C++11 标准
CXXFLAGS = -Wall -g -std=c++11

# 输出目录
OUT_DIR = out

# 目标文件
TARGETS = $(OUT_DIR)/server $(OUT_DIR)/client

# 源文件
SRCS = client.cpp server.cpp util.cpp

# 头文件
HEADERS = util.h

# 生成目标文件
all: $(TARGETS)

# 创建输出目录
$(OUT_DIR):
	mkdir -p $(OUT_DIR)

# server 目标文件
$(OUT_DIR)/server: $(OUT_DIR)/server.o $(OUT_DIR)/util.o | $(OUT_DIR)
	$(CXX) $(CXXFLAGS) -o $(OUT_DIR)/server $(OUT_DIR)/server.o $(OUT_DIR)/util.o

# client 目标文件
$(OUT_DIR)/client: $(OUT_DIR)/client.o $(OUT_DIR)/util.o | $(OUT_DIR)
	$(CXX) $(CXXFLAGS) -o $(OUT_DIR)/client $(OUT_DIR)/client.o $(OUT_DIR)/util.o

# 生成 .o 文件
$(OUT_DIR)/server.o: server.cpp util.h | $(OUT_DIR)
	$(CXX) $(CXXFLAGS) -c server.cpp -o $(OUT_DIR)/server.o

$(OUT_DIR)/client.o: client.cpp util.h | $(OUT_DIR)
	$(CXX) $(CXXFLAGS) -c client.cpp -o $(OUT_DIR)/client.o

$(OUT_DIR)/util.o: util.cpp util.h | $(OUT_DIR)
	$(CXX) $(CXXFLAGS) -c util.cpp -o $(OUT_DIR)/util.o

# 清理目标文件和可执行文件
clean:
	rm -f $(OUT_DIR)/*.o $(TARGETS)

