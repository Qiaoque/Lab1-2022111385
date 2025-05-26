#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <algorithm>
#include <cctype>
#include <random>
#include <ctime>
#include <limits>
#include <stack>
#include <iomanip>

// For graph visualization
#include <fstream>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define BLUE    "\033[34m"
#define YELLOW  "\033[33m"

class Graph {
private:
    struct Edge {
        std::string dest;
        int weight;

        Edge(std::string d, int w) : dest(std::move(d)), weight(w) {} // 按值传递并使用 std::move
        bool operator<(const Edge& other) const {
            return dest < other.dest;
        }
    };
    // Adjacency list representation
    std::map<std::string, std::vector<Edge>> adjacencyList;
    // Random number generator
    std::mt19937 rng;

public:
    Graph() : rng(static_cast<unsigned int>(time(nullptr))) {}
    bool buildFromFile(const std::string& filePath);
    void addEdge(const std::string& src, const std::string& dest);
    void displayGraph() const;
    bool saveGraphToFile(const std::string& filename) const;
    std::vector<std::string> findBridgeWords(const std::string& word1, const std::string& word2) const;
    std::string generateTextWithBridges(const std::string& inputText);
    std::pair<double, std::vector<std::string>> shortestPath(const std::string& start, const std::string& end) const;
    std::map<std::string, std::pair<double, std::vector<std::string>>> shortestPathsFromSource(const std::string& start) const;
    std::map<std::string, double> calculatePageRank(double dampingFactor = 0.85, 
        std::map<std::string, double> customInitialRanks = std::map<std::string, double>(), int iterations = 100) const;
    std::map<std::string, double> calculateTfIdfRanks(const std::string& filePath) const;
    std::map<std::string, double> calculatePageRankWithTfIdf(const std::string& filePath,
        double dampingFactor,
        int iterations) const;
    std::vector<std::string> randomWalk();
    bool containsWord(const std::string& word) const;
    // std::vector<std::string> getAllVertices() const;
};

#endif // GRAPH_H