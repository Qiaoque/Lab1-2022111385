# 编译器设置
CXX := g++
CXXFLAGS := -std=c++11 -Wall -Wextra -Iinclude

# 项目目录结构
SRC_DIR := src
INC_DIR := include
BIN_DIR := bin
OBJ_DIR := obj
TEST_DIR := test
TEST_FILE := $(TEST_DIR)/test.txt

# 源文件和目标文件
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
EXEC := $(BIN_DIR)/text_graph

# 主目标
all: $(EXEC)

# 链接可执行文件
$(EXEC): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# 编译源文件
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 创建必要的目录
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# 清理生成的文件
clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(BIN_DIR)

# 重新编译
rebuild: clean all

# 测试目标 - 先构建程序，然后运行测试
test: $(EXEC)
	@if [ ! -f "$(TEST_FILE)" ]; then \
		echo "Error: Test file $(TEST_FILE) does not exist"; \
		exit 1; \
	fi
	@echo "Running tests with $(TEST_FILE)..."
	@./$(EXEC) "$(TEST_FILE)"

# 伪目标声明
.PHONY: all clean rebuild test