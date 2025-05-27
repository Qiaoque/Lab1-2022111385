#include <gtest/gtest.h>
#include <fstream>
#include <algorithm> // 用于 std::sort
#include "../include/Graph.h" // 假设 Graph 类定义在 graph.h 中

class GraphTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建临时测试文件
        std::ofstream testFile("test.txt");
        testFile << "to explore the strange new worlds to seek the new life and new civilizations";
        testFile.close();

        // 构建有向图
        graph.buildFromFile("test.txt");
    }

    void TearDown() override {
        // 删除临时测试文件
        std::remove("test.txt");
    }

    Graph graph; // Graph 对象
};

// 测试用例 1: word1="explore", word2="strange", 期望输出: ["the"]
TEST_F(GraphTest, TestBridgeWords_ExploreToStrange) {
    std::vector<std::string> expected = { "the" };
    std::vector<std::string> result = graph.findBridgeWords("explore", "strange");
    EXPECT_EQ(result, expected) << "Expected bridge word: the";
}

// 测试用例 2: word1="to", word2="the", 期望输出: ["explore", "seek"]
TEST_F(GraphTest, TestBridgeWords_ToToThe) {
    std::vector<std::string> expected = { "explore", "seek" };
    std::vector<std::string> result = graph.findBridgeWords("to", "the");
    std::sort(result.begin(), result.end()); // 排序以确保比较一致
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(result, expected) << "Expected bridge words: explore, seek";
}

// 测试用例 3: word1="seek", word2="life", 期望输出: []
TEST_F(GraphTest, TestBridgeWords_SeekToLife) {
    std::vector<std::string> expected = {};
    std::vector<std::string> result = graph.findBridgeWords("seek", "life");
    EXPECT_EQ(result, expected) << "Expected no bridge words";
}

// 测试用例 4: word1="to12", word2="seek", 期望输出: []
TEST_F(GraphTest, TestBridgeWords_NonExistentWords) {
    std::vector<std::string> expected = {};
    std::vector<std::string> result = graph.findBridgeWords("to12", "seek");
    EXPECT_EQ(result, expected) << "Expected no bridge words for non-existent word1";
}

// 测试用例 5: word1="", word2="seek", 期望输出: []
TEST_F(GraphTest, TestBridgeWords_EmptyWord1) {
    std::vector<std::string> expected = {};
    std::vector<std::string> result = graph.findBridgeWords("", "seek");
    EXPECT_EQ(result, expected) << "Expected no bridge words for empty word1";
}

// 测试用例 6: word1="xy", word2="seek", 期望输出: []
TEST_F(GraphTest, TestBridgeWords_NonExistentWord1) {
    std::vector<std::string> expected = {};
    std::vector<std::string> result = graph.findBridgeWords("xy", "seek");
    EXPECT_EQ(result, expected) << "Expected no bridge words for non-existent word1";
}

// 测试用例 7: word1="strange", word2="xy", 期望输出: []
TEST_F(GraphTest, TestBridgeWords_NonExistentWord2) {
    std::vector<std::string> expected = {};
    std::vector<std::string> result = graph.findBridgeWords("strange", "xy");
    EXPECT_EQ(result, expected) << "Expected no bridge words for non-existent word2";
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}