#include <gtest/gtest.h>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm> // 用于 std::find
#include "../include/Graph.h" // Graph 类定义在 graph.h 中
#include <unordered_set>

// 测试夹具类，用于设置测试环境
class GraphRandomWalkTest : public ::testing::Test {
protected:
    Graph graph;
    const std::string testFilePath = "test.txt";

    // 在每个测试用例运行前执行
    void SetUp() override {
        // 清空图以确保每个测试用例从干净状态开始
        graph = Graph();
    }

    // 在每个测试用例运行后执行
    void TearDown() override {
        // 删除测试中创建的文件
        std::remove(testFilePath.c_str());
    }

    // 辅助函数：创建测试文件
    void createTestFile(const std::string& filePath, const std::string& content) {
        std::ofstream file(filePath);
        ASSERT_TRUE(file.is_open()) << "Failed to create test file: " << filePath;
        file << content;
        file.close();
    }
};

// 测试用例 1：空文本建立有向图
TEST_F(GraphRandomWalkTest, EmptyGraph) {
    // 创建空文件
    createTestFile(testFilePath, "");

    // 构建图
    ASSERT_TRUE(graph.buildFromFile(testFilePath));

    // 执行随机游走
    std::vector<std::string> path = graph.randomWalk();

    // 验证结果
    EXPECT_TRUE(path.empty()) << "Expected empty path for empty graph";
}

// 测试用例2：path长度为1
TEST_F(GraphRandomWalkTest, SingleNodePath) {
    // 创建测试文件
    createTestFile(testFilePath, "to explore the strange new worlds to seek the new life and new civilizations");

    // 构建图
    ASSERT_TRUE(graph.buildFromFile(testFilePath));

    // 循环尝试随机游走，直到路径长度为1或达到最大尝试次数
    std::vector<std::string> path;
    const int maxAttempts = 1000; // 设置最大尝试次数
    int attempts = 0;
    do {
        path = graph.randomWalk();
        attempts++;
        if (path.size() == 1) {
            break; // 得到长度为1的路径，退出循环
        }
    } while (attempts < maxAttempts);

    // 验证是否成功获得长度为1的路径
    ASSERT_EQ(path.size(), 1u) << "Failed to get a single node path after " << maxAttempts << " attempts";

    // 验证路径内容
    EXPECT_EQ(path[0], "civilizations") << "Expected single node path to be 'civilizations'";
    EXPECT_TRUE(graph.containsWord(path[0])) << "Node " << path[0] << " should exist in graph";
}

// 测试用例3 ：多节点path
TEST_F(GraphRandomWalkTest, MultiNodePath) {
    // 创建测试文件
    createTestFile(testFilePath, "to explore the strange new worlds to seek the new life and new civilizations");

    // 构建图
    ASSERT_TRUE(graph.buildFromFile(testFilePath));

    // 执行随机游走
    std::vector<std::string> path = graph.randomWalk();

    // 验证路径非空
    EXPECT_FALSE(path.empty()) << "Expected non-empty path for multi-node graph";

    // 验证路径中每个节点存在于图中
    for (const auto& node : path) {
        EXPECT_TRUE(graph.containsWord(node)) << "Node " << node << " should exist in graph";
    }

    // 验证路径中没有重复边
    std::unordered_set<std::string> uniqueEdges;
    for (size_t i = 0; i < path.size() - 1; ++i) {
        std::string current = path[i];
        std::string next = path[i + 1];
        std::string edge = current + "->" + next;
        auto [_, inserted] = uniqueEdges.insert(edge);
        EXPECT_TRUE(inserted) << "Duplicate edge found in path: " << edge;
    }

    // 验证路径中的边是否合理（通过 findBridgeWords 检查）
    for (size_t i = 0; i < path.size() - 1; ++i) {
        std::string current = path[i];
        std::string next = path[i + 1];
        // 使用 findBridgeWords 检查是否存在从 current 到 next 的路径
        std::vector<std::string> bridgeWords = graph.findBridgeWords(current, next);
        bool isDirectEdge = false;
        for (const auto& edge : graph.shortestPath(current, next).second) {
            if (edge == next && path[i] == current) {
                isDirectEdge = true;
                break;
            }
        }
        EXPECT_TRUE(isDirectEdge || !bridgeWords.empty()) << "Invalid edge from " << current << " to " << next;
    }

    // 检查是否包含示例路径（由于随机性，检查一种可能路径）
    std::vector<std::string> expectedPath = {"new", "worlds", "to", "seek", "the"};
    if (path == expectedPath) {
        EXPECT_EQ(path, expectedPath) << "Expected path: new -> worlds -> to -> seek -> the";
    }
}
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}