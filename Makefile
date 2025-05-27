# 编译器设置
CXX := g++
CXXFLAGS := -std=c++11 -Wall -Wextra -Iinclude --coverage
GTEST_CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude --coverage
GTEST_LIBS := -lgtest -lgtest_main -pthread

# 项目目录结构
SRC_DIR := src
INC_DIR := include
BIN_DIR := bin
OBJ_DIR := obj
TEST_DIR := test
GTEST_DIR := gtest
TEST_FILE := $(TEST_DIR)/test.txt

# 静态分析工具设置
## Clang-Tidy 设置
TIDY := clang-tidy
TIDY_CHECKS := -checks=bugprone-*,clang-analyzer-*,modernize-*,performance-*,-modernize-use-trailing-return-type
TIDY_FLAGS := -p $(CURDIR) --header-filter="$(INC_DIR)/.*"
TIDY_REPORT := clang-tidy-report.txt
TIDY_FIXES := clang-tidy-fixes.yaml

## Cppcheck 设置
CPPCHECK := cppcheck
CPPCHECK_FLAGS := --enable=all --std=c++11 --suppress=missingIncludeSystem -I $(INC_DIR) --inline-suppr --suppress=unmatchedSuppression
CPPCHECK_REPORT := cppcheck-report.xml

# 覆盖率报告相关设置
COV_DIR := coverage
COV_INFO := coverage.info
COV_REPORT := $(COV_DIR)/index.html

# 源文件和目标文件
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
EXEC := $(BIN_DIR)/text_graph

# gtest 相关设置
GTEST_SRC := $(wildcard $(GTEST_DIR)/*.cpp) $(SRC_DIR)/Graph.cpp $(SRC_DIR)/Tools.cpp
GTEST_OBJS := $(patsubst $(GTEST_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(wildcard $(GTEST_DIR)/*.cpp)) $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_DIR)/Graph.cpp $(SRC_DIR)/Tools.cpp)
GTEST_EXECS := $(BIN_DIR)/bridge_test $(BIN_DIR)/randomwalk_test

# 主目标
all: $(EXEC)

# 链接主程序可执行文件
$(EXEC): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# 编译源文件
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 编译 gtest 源文件
$(OBJ_DIR)/%.o: $(GTEST_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(GTEST_CXXFLAGS) -c $< -o $@

# 创建必要的目录
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# gtest 目标：编译和链接单元测试
gtest: $(GTEST_EXECS)

# 链接 bridge_test 可执行文件
$(BIN_DIR)/bridge_test: $(OBJ_DIR)/bridge_test.o $(OBJ_DIR)/Graph.o $(OBJ_DIR)/Tools.o | $(BIN_DIR)
	$(CXX) $(GTEST_CXXFLAGS) $^ $(GTEST_LIBS) -o $@

# 链接 randomwalk_test 可执行文件
$(BIN_DIR)/randomwalk_test: $(OBJ_DIR)/randomwalk_test.o $(OBJ_DIR)/Graph.o $(OBJ_DIR)/Tools.o | $(BIN_DIR)
	$(CXX) $(GTEST_CXXFLAGS) $^ $(GTEST_LIBS) -o $@

# 运行 gtest 单元测试
test-gtest: $(GTEST_EXECS)
	@echo "Running gtest unit tests..."
	@for exec in $(GTEST_EXECS); do \
		echo "Running $$exec..."; \
		./$$exec || exit 1; \
	done

# 清理生成的文件
clean: clean-coverage
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(COMPILE_COMMANDS) \
        $(TIDY_REPORT) $(TIDY_FIXES) \
        $(CPPCHECK_REPORT) $(GTEST_EXECS)

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

# 生成覆盖率报告
coverage: test-gtest
	@echo "Generating coverage report..."
	@mkdir -p $(COV_DIR)
	@lcov --capture --directory $(OBJ_DIR) --output-file $(COV_INFO) --rc branch_coverage=1 --ignore-errors mismatch
	@lcov --remove $(COV_INFO) '/usr/*' '$(GTEST_DIR)/*' --output-file $(COV_INFO) --rc branch_coverage=1
	@genhtml $(COV_INFO) --output-directory $(COV_DIR) --rc branch_coverage=1
	@echo "Coverage report generated at $(COV_REPORT)"

# 清理覆盖率相关文件
clean-coverage:
	rm -rf $(COV_DIR) $(COV_INFO) $(OBJ_DIR)/*.gcda $(OBJ_DIR)/*.gcno

# 综合静态分析目标
static-analysis: tidy cppcheck
	@echo "Completed all static analysis"

# 伪目标声明
.PHONY: all clean rebuild test gtest test-gtest tidy tidy-fixes view-fixes \
        cppcheck static-analysis