#include "../include/Tools.h"

// Convert to lowercase and normalize word
std::string normalizeWord(const std::string& word) {
    std::string result;
    for (char c : word) {
        if (std::isalpha(c)) {
            result += static_cast<char>(std::tolower(c));
        }
    }
    return result;
}

// Function to display shortest path on screen
void displayShortestPath(const std::pair<double, std::vector<std::string>>& pathInfo) {
    if (pathInfo.first == -1) {
        std::cout << RED << "No path exists between these words." << RESET << '\n';
        return;
    }

    std::cout << GREEN << "Shortest Path: " << RESET;
    for (size_t i = 0; i < pathInfo.second.size(); ++i) {
        if (i > 0) std::cout << " -> ";
        std::cout << pathInfo.second[i];
    }
    std::cout << '\n';

    std::cout << "Path Length: " << pathInfo.first << '\n';
}