#include "../include/Tools.h"

// Main function to handle user interface
int main(int argc, const char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <text_file>" << '\n';
        return 1;
    }

    std::string fileName = argv[1];
    Graph graph;

    std::cout << "Reading file: " << fileName << '\n';
    if (!graph.buildFromFile(fileName)) {
        std::cerr << "Failed to build graph from file." << '\n';
        return 1;
    }

    std::cout << BLUE << "Graph built successfully!" << RESET << '\n';

    // Main menu loop
    int choice;
    std::string input, word1, word2;

    while (true) {
        std::cout << "\n" << YELLOW << "===== Text Graph Processing System =====" << RESET << '\n';
        std::cout << "1. Display Graph" << '\n';
        std::cout << "2. Save Graph to File" << '\n';
        std::cout << "3. Find Bridge Words" << '\n';
        std::cout << "4. Generate Text with Bridge Words" << '\n';
        std::cout << "5. Find Shortest Path" << '\n';
        std::cout << "6. Calculate PageRank" << '\n';
        std::cout << "7. Random Walk" << '\n';
        std::cout << "0. Exit" << '\n';
        std::cout << "Enter your choice: ";
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear input buffer

        switch (choice) {
        case 0:
            std::cout << "Exiting program. Goodbye!" << '\n';
            return 0;

        case 1:
            graph.displayGraph();
            break;

        case 2: {
            std::string outputFile;
            std::cout << "Enter output file name (e.g., graph.dot): ";
            std::getline(std::cin, outputFile);
            graph.saveGraphToFile(outputFile);
            break;
        }

        case 3: {
            std::cout << "Enter first word: ";
            std::getline(std::cin, word1);
            std::cout << "Enter second word: ";
            std::getline(std::cin, word2);

            std::string normalizedWord1 = normalizeWord(word1);
            std::string normalizedWord2 = normalizeWord(word2);

            if (!graph.containsWord(normalizedWord1) || !graph.containsWord(normalizedWord2)) {
                std::string errorMsg = "No ";
                if (!graph.containsWord(normalizedWord1) && !graph.containsWord(normalizedWord2)) {
                    errorMsg += normalizedWord1;
                    errorMsg += " or ";
                    errorMsg += normalizedWord2;
                } else {
                    errorMsg += !graph.containsWord(normalizedWord1) ? normalizedWord1 : normalizedWord2;
                }
                errorMsg += " in the graph!";
                std::cout << RED << errorMsg << RESET << '\n';
            }
            else {
                std::vector<std::string> bridges = graph.findBridgeWords(normalizedWord1, normalizedWord2);

                if (bridges.empty()) {
                    std::cout << YELLOW << "No bridge words from " << normalizedWord1 << " to " << normalizedWord2 << "!" << RESET << '\n';
                }
                else {
                    std::cout << GREEN << "The bridge words from " << normalizedWord1 << " to " << normalizedWord2 << " are: ";
                    for (size_t i = 0; i < bridges.size(); ++i) {
                        if (i > 0) {
                            if (i == bridges.size() - 1) {
                                std::cout << (bridges.size() > 2 ? ", and " : " and ");
                            }
                            else {
                                std::cout << ", ";
                            }
                        }
                        std::cout << bridges[i];
                    }
                    std::cout << "." << RESET << '\n';
                }
            }
            break;
        }

        case 4: {
            std::cout << "Enter text to process: ";
            std::getline(std::cin, input);
            std::string newText = graph.generateTextWithBridges(input);
            std::cout << GREEN << "Generated text: " << RESET << newText << '\n';
            break;
        }

        case 5: {
            std::cout << "Enter first word (or press Enter for all paths): ";
            std::getline(std::cin, word1);

            if (word1.empty()) {
                std::cout << RED << "Please enter at least one word." << RESET << '\n';
                break;
            }

            std::string normalizedWord1 = normalizeWord(word1);

            if (!graph.containsWord(normalizedWord1)) {
                std::cout << RED << "No " << normalizedWord1 << " in the graph!" << RESET << '\n';
                break;
            }

            std::cout << "Enter second word (or press Enter for all paths): ";
            std::getline(std::cin, word2);

            if (word2.empty()) {
                // Calculate paths from source to all destinations
                std::map<std::string, std::pair<double, std::vector<std::string>>> paths =
                    graph.shortestPathsFromSource(normalizedWord1);

                if (paths.empty()) {
                    std::cout << YELLOW << "No paths found from " << normalizedWord1 << "." << RESET << '\n';
                }
                else {
                    std::cout << BLUE << "Shortest paths from " << normalizedWord1 << " to all words:" << RESET << '\n';
                    for (const auto& entry : paths) {
                        std::cout << GREEN << "To " << entry.first << ": " << RESET;
                        displayShortestPath(entry.second);
                        std::cout << '\n';
                    }
                }
            }
            else {
                // Calculate path from source to destination
                std::string normalizedWord2 = normalizeWord(word2);

                if (!graph.containsWord(normalizedWord2)) {
                    std::cout << RED << "No " << normalizedWord2 << " in the graph!" << RESET << '\n';
                    break;
                }

                std::pair<double, std::vector<std::string>> path =
                    graph.shortestPath(normalizedWord1, normalizedWord2);

                std::cout << BLUE << "Shortest path from " << normalizedWord1 << " to " << normalizedWord2 << ":" << RESET << '\n';
                displayShortestPath(path);
            }
            break;
        }

        case 6: {
            int prMethod;
            std::cout << YELLOW << "选择 PageRank 计算方法：" << RESET << '\n';
            std::cout << "1. 标准 PageRank (均匀初始值)" << '\n';
            std::cout << "2. 基于 TF-IDF 的 PageRank" << '\n';
            std::cout << "请输入选择 (1-2): ";
            std::cin >> prMethod;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 清除输入缓冲区

            double dampingFactor = 0.85; // 默认阻尼因子
            int iterations = 100;       // 默认迭代次数

            std::cout << "是否要自定义参数？(y/n): ";
            char customParams;
            std::cin >> customParams;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (customParams == 'y' || customParams == 'Y') {
                std::cout << "输入阻尼因子 (0.1-0.9，推荐 0.85): ";
                std::cin >> dampingFactor;
                std::cout << "输入迭代次数 (10-1000，推荐 100): ";
                std::cin >> iterations;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }

            // 根据用户选择计算 PageRank
            std::map<std::string, double> pageRanks;
            if (prMethod == 2) {
                std::cout << BLUE << "使用 TF-IDF 作为初始 PageRank 值..." << RESET << '\n';
                pageRanks = graph.calculatePageRankWithTfIdf(fileName, dampingFactor, iterations);
            }
            else {
                std::cout << BLUE << "使用标准 PageRank 计算..." << RESET << '\n';
                pageRanks = graph.calculatePageRank(dampingFactor, std::map<std::string, double>(), iterations);
            }

            // 按 PageRank 值排序 (从高到低)
            std::vector<std::pair<std::string, double>> sortedRanks;
            sortedRanks.reserve(pageRanks.size());
            for (const auto& entry : pageRanks) {
                sortedRanks.emplace_back(entry);
            }

            std::sort(sortedRanks.begin(), sortedRanks.end(),
                [](const std::pair<std::string, double>& a, const std::pair<std::string, double>& b) {
                    return a.second > b.second;
                });

            // 显示结果的选项
            int displayCount = 20; // 默认显示前 20 个结果
            std::cout << "显示前多少个结果 (默认 20): ";
            std::string displayInput;
            std::getline(std::cin, displayInput);
            if (!displayInput.empty()) {
                displayCount = std::stoi(displayInput);
            }

            std::cout << BLUE << "PageRank 值 (前 " << displayCount << "):" << RESET << '\n';
            std::cout << std::setw(15) << "单词" << std::setw(15) << "PageRank 值" << '\n';
            std::cout << std::string(30, '-') << '\n';

            // 显示排序后的结果
            for (size_t i = 0; i < std::min(sortedRanks.size(), static_cast<size_t>(displayCount)); ++i) {
                std::cout << std::setw(15) << sortedRanks[i].first
                    << std::setw(15) << std::fixed << std::setprecision(8) << sortedRanks[i].second << '\n';
            }

            // 保存 PageRank 结果到文件
            std::cout << "是否保存结果到文件？(y/n): ";
            char saveToFile;
            std::cin >> saveToFile;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (saveToFile == 'y' || saveToFile == 'Y') {
                std::string outputFile;
                std::cout << "输入输出文件名 (如 pagerank_results.txt): ";
                std::getline(std::cin, outputFile);

                std::ofstream outFile(outputFile);
                if (outFile.is_open()) {
                    outFile << "单词,PageRank值\n";
                    for (const auto& entry : sortedRanks) {
                        outFile << entry.first << "," << std::fixed << std::setprecision(6) << entry.second << "\n";
                    }
                    outFile.close();
                    std::cout << GREEN << "PageRank 结果已保存到 " << outputFile << RESET << '\n';
                }
                else {
                    std::cerr << RED << "无法打开文件保存结果。" << RESET << '\n';
                }
            }
            break;
        }

        case 7: {
            std::vector<std::string> walkPath = graph.randomWalk();

            if (walkPath.empty()) {
                std::cout << RED << "Random walk could not be performed on the graph." << RESET << '\n';
                break;
            }
            else {
                std::cout << GREEN << "Random Walk Path:" << RESET << '\n';
                for (size_t i = 0; i < walkPath.size(); ++i) {
                    if (i > 0) std::cout << " -> ";
                    std::cout << walkPath[i];
                }
                std::cout << '\n';

                // Save walk to file
                std::ofstream walkFile("random_walk.txt");
                if (walkFile.is_open()) {
                    for (size_t i = 0; i < walkPath.size(); ++i) {
                        walkFile << walkPath[i];
                        if (i < walkPath.size() - 1) walkFile << " ";
                    }
                    walkFile.close();
                    std::cout << "Random walk saved to random_walk.txt" << '\n';
                }
                else {
                    std::cerr << "Could not save random walk to file." << '\n';
                }
            }
            break;
        }

        default:
            std::cout << RED << "Invalid choice. Please try again." << RESET << '\n';
        }
    }

    return 0;
}