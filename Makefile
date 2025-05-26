# 编译器设置
CXX := g++
CXXFLAGS := -std=c++11 -Wall -Wextra -Iinclude
GTEST_CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude # gtest 需要 C++17
GTEST_LIBS := -lgtest -lgtest_main -pthread

# 静态分析工具设置
## Clang-Tidy 设置
TIDY := clang-tidy
TIDY_CHECKS := -checks=bugprone-*,clang-analyzer-*,modernize-*,performance-*,-modernize-use-trailing-return-type
TIDY_FLAGS := -p $(CURDIR) --header-filter="$(INC_DIR)/.*"
TIDY_REPORT := clang-tidy-report.txt
TIDY_FIXES := clang-tidy-fixes.yaml

# 项目目录结构
SRC_DIR := src
INC_DIR := include
BIN_DIR := bin
OBJ_DIR := obj
TEST_DIR := test
TEST_FILE := $(TEST_DIR)/test.txt

## Cppcheck 设置
CPPCHECK := cppcheck
CPPCHECK_FLAGS := --enable=all --std=c++11 --suppress=missingIncludeSystem -I $(INC_DIR) --inline-suppr --suppress=unmatchedSuppression
CPPCHECK_REPORT := cppcheck-report.xml

# 源文件和目标文件
SRCS := $(filter-out $(SRC_DIR)/bridge_test.cpp, $(wildcard $(SRC_DIR)/*.cpp))
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
EXEC := $(BIN_DIR)/text_graph

# gtest 相关设置
GTEST_SRC := $(SRC_DIR)/bridge_test.cpp $(SRC_DIR)/Graph.cpp $(SRC_DIR)/Tools.cpp
GTEST_EXEC := $(BIN_DIR)/bridge_test

# 主目标
all: $(EXEC)

# 链接主程序可执行文件
$(EXEC): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# 编译源文件
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 创建必要的目录
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# gtest 目标：编译和链接单元测试
gtest: $(GTEST_EXEC)

$(GTEST_EXEC): $(GTEST_SRC) | $(BIN_DIR)
	$(CXX) $(GTEST_CXXFLAGS) $(GTEST_SRC) $(GTEST_LIBS) -o $@

# 运行 gtest 单元测试
test-gtest: $(GTEST_EXEC)
	@echo "Running gtest unit tests..."
	@./$(GTEST_EXEC)

# 清理生成的文件
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(COMPILE_COMMANDS) \
        $(TIDY_REPORT) $(TIDY_FIXES) \
        $(CPPCHECK_REPORT) $(GTEST_EXEC)

# 重新编译
rebuild: clean all

# 测试目标（原有测试）
test: $(EXEC)
	@if [ ! -f "$(TEST_FILE)" ]; then \
		echo "Error: Test file $(TEST_FILE) does not exist"; \
		exit 1; \
	fi
	@echo "Running tests with $(TEST_FILE)..."
	@./$(EXEC) "$(TEST_FILE)"

# 编译数据库路径
COMPILE_COMMANDS := compile_commands.json

# Clang-Tidy 相关目标
tidy: $(COMPILE_COMMANDS)
	@echo "Running clang-tidy and saving report to $(TIDY_REPORT)..."
	@$(TIDY) $(TIDY_CHECKS) $(TIDY_FLAGS) $(SRCS) > $(TIDY_REPORT) 2>&1
	@echo "Report saved to $(TIDY_REPORT)"
	@echo "Use 'make view-tidy' to view the report"

tidy-fixes: $(COMPILE_COMMANDS)
	@echo "Running clang-tidy and generating fixes file..."
	@$(TIDY) -export-fixes=$(TIDY_FIXES) $(TIDY_CHECKS) $(TIDY_FLAGS) $(SRCS)
	@echo "Fixes saved to $(TIDY_FIXES)"

$(COMPILE_COMMANDS):
	@echo "Generating compile commands..."
	@bear -- $(MAKE) rebuild > /dev/null 2>&1

view-fixes:
	@if [ -f "$(TIDY_FIXES)" ]; then \
		less $(TIDY_FIXES); \
	else \
		echo "No clang-tidy fixes file found. Run 'make tidy-fixes' first."; \
	fi

# Cppcheck 相关目标
cppcheck:
	@echo "正在运行 cppcheck..."
	@if [ ! -d "$(INC_DIR)" ]; then \
		echo "错误：头文件目录 $(INC_DIR) 不存在"; \
		exit 1; \
	fi
	@$(CPPCHECK) $(CPPCHECK_FLAGS) --xml $(SRCS) 2> $(CPPCHECK_REPORT) || { \
		echo "Cppcheck 运行失败，请检查 $(CPPCHECK_REPORT) 获取详情"; \
		exit 1; \
	}
	@echo "XML 报告已保存至 $(CPPCHECK_REPORT)"

# 综合静态分析目标
static-analysis: tidy cppcheck
	@echo "Completed all static analysis"

# 伪目标声明
.PHONY: all clean rebuild test gtest test-gtest tidy tidy-fixes view-fixes \
        cppcheck static-analysis