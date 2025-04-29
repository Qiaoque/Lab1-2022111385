#include "../include/Tools.h"

// Process text file and build graph
bool Graph::buildFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return false;
    }

    // Process the entire file as a single string, replacing newlines with spaces
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    // Replace all punctuation with spaces
    // ispunct() is a function that returns true if the given character is a punctuation mark（标点符号）
    for (char& c : content) {
        if (std::ispunct(c) || c == '\n' || c == '\r') {
            c = ' ';
        }
    }

    std::stringstream ss(content);
    std::string word, prevWord;
    bool firstWord = true;

    // Process words
    while (ss >> word) {
        std::string normalizedWord = normalizeWord(word);
        if (normalizedWord.empty()) continue;

        if (!firstWord) {
            // Add edge from prevWord to current word
            addEdge(prevWord, normalizedWord);
        }
        else {
            firstWord = false;
        }

        prevWord = normalizedWord;
    }

    return true;
}

// Add edge or increase weight if it already exists
void Graph::addEdge(const std::string& src, const std::string& dest) {
    // Ensure src is in the adjacency list
    if (adjacencyList.find(src) == adjacencyList.end()) {
        adjacencyList[src] = std::vector<Edge>();
    }

    // Check if edge already exists
    bool found = false;
    for (Edge& edge : adjacencyList[src]) {
        if (edge.dest == dest) {
            edge.weight++;
            found = true;
            break;
        }
    }

    // If edge doesn't exist, add it
    if (!found) {
        adjacencyList[src].push_back(Edge(dest, 1));
    }

    // Ensure dest is in the adjacency list (even if it has no outgoing edges)
    if (adjacencyList.find(dest) == adjacencyList.end()) {
        adjacencyList[dest] = std::vector<Edge>();
    }
}

// Display the graph
void Graph::displayGraph() const {
    std::cout << BLUE << "\n=== Directed Graph Representation ===" << RESET << std::endl;

    for (const auto& entry : adjacencyList) {
        const std::string& vertex = entry.first;
        const std::vector<Edge>& edges = entry.second;

        std::cout << GREEN << vertex << RESET << " -> ";

        if (edges.empty()) {
            std::cout << "(no outgoing edges)";
        }
        else {
            bool first = true;
            for (const Edge& edge : edges) {
                if (!first) std::cout << ", ";
                std::cout << edge.dest << " (weight: " << edge.weight << ")";
                first = false;
            }
        }
        std::cout << std::endl;
    }
}

// Save graph as DOT file for visualization with Graphviz
bool Graph::saveGraphToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return false;
    }

    // Write DOT format
    file << "digraph TextGraph {\n";
    file << "  node [shape=box, style=filled, fillcolor=lightblue];\n";
    file << "  edge [color=gray];\n";

    for (const auto& entry : adjacencyList) {
        const std::string& vertex = entry.first;
        const std::vector<Edge>& edges = entry.second;

        for (const Edge& edge : edges) {
            file << "  \"" << vertex << "\" -> \"" << edge.dest << "\" [label=\"" << edge.weight << "\"];\n";
        }
    }

    file << "}\n";
    file.close();

    std::cout << "Graph saved to " << filename << " (DOT format)" << std::endl;
    std::cout << "To visualize: install Graphviz and run 'dot -Tpng " << filename << " -o graph.png'" << std::endl;

    return true;
}

// Find bridge words between two words
std::vector<std::string> Graph::findBridgeWords(const std::string& word1, const std::string& word2) const {
    std::vector<std::string> bridges;
    std::string normalizedWord1 = normalizeWord(word1);
    std::string normalizedWord2 = normalizeWord(word2);

    // Check if both words exist in the graph
    if (adjacencyList.find(normalizedWord1) == adjacencyList.end() ||
        adjacencyList.find(normalizedWord2) == adjacencyList.end()) {
        return bridges; // Empty vector indicates words not found
    }

    // Find potential bridge words
    for (const auto& entry : adjacencyList) {
        const std::string& potentialBridge = entry.first;

        // Check if word1 connects to potentialBridge
        bool word1ConnectsToBridge = false;
        for (const Edge& edge : adjacencyList.at(normalizedWord1)) {
            if (edge.dest == potentialBridge) {
                word1ConnectsToBridge = true;
                break;
            }
        }

        if (!word1ConnectsToBridge) continue;

        // Check if potentialBridge connects to word2
        for (const Edge& edge : adjacencyList.at(potentialBridge)) {
            if (edge.dest == normalizedWord2) {
                bridges.push_back(potentialBridge);
                break;
            }
        }
    }

    return bridges;
}

// Generate new text with bridge words
std::string Graph::generateTextWithBridges(const std::string& inputText) {
    std::stringstream ss(inputText);
    std::string word;
    std::vector<std::string> words;

    // Split input text into words
    while (ss >> word) {
        std::string normalizedWord = normalizeWord(word);
        if (!normalizedWord.empty()) {
            words.push_back(normalizedWord);
        }
    }

    if (words.size() < 2) {
        return inputText; // Not enough words to process
    }

    std::stringstream result;
    result << words[0]; // Add first word

    // Process word pairs and insert bridge words
    for (size_t i = 0; i < words.size() - 1; ++i) {
        const std::string& currentWord = words[i];
        const std::string& nextWord = words[i + 1];

        // Find bridge words
        std::vector<std::string> bridges = findBridgeWords(currentWord, nextWord);

        // Insert a random bridge word if any exist
        if (!bridges.empty()) {
            std::uniform_int_distribution<size_t> dist(0, bridges.size() - 1);
            size_t randomIndex = dist(rng);
            result << " " << bridges[randomIndex];
        }

        // Add the next word
        result << " " << nextWord;
    }

    return result.str();
}

// Find shortest path using Dijkstra's algorithm
std::pair<double, std::vector<std::string>> Graph::shortestPath(const std::string& start, const std::string& end) const {
    std::string normalizedStart = normalizeWord(start);
    std::string normalizedEnd = normalizeWord(end);

    // Check if both words exist in the graph
    if (adjacencyList.find(normalizedStart) == adjacencyList.end() ||
        adjacencyList.find(normalizedEnd) == adjacencyList.end()) {
        return { -1, {} }; // Indicate words not found
    }

    // Initialize distances with infinity
    std::map<std::string, double> distance;
    std::map<std::string, std::string> previous;
    std::set<std::string> unvisited;

    for (const auto& entry : adjacencyList) {
        const std::string& vertex = entry.first;
        distance[vertex] = std::numeric_limits<double>::infinity();
        unvisited.insert(vertex);
    }

    distance[normalizedStart] = 0;

    // Dijkstra's algorithm
    while (!unvisited.empty()) {
        // Find vertex with minimum distance
        std::string current;
        double minDist = std::numeric_limits<double>::infinity();

        for (const std::string& vertex : unvisited) {
            if (distance[vertex] < minDist) {
                minDist = distance[vertex];
                current = vertex;
            }
        }

        if (minDist == std::numeric_limits<double>::infinity()) {
            break; // No path exists
        }

        // Remove current from unvisited
        unvisited.erase(current);

        // If we reached the end, break
        if (current == normalizedEnd) {
            break;
        }

        // Update distances to neighbors
        if (adjacencyList.find(current) != adjacencyList.end()) {
            for (const Edge& edge : adjacencyList.at(current)) {
                if (unvisited.find(edge.dest) != unvisited.end()) {
                    // double alt = distance[current] + (1.0 / edge.weight); // Use inverse of weight for shortest path
                    double alt = distance[current] + edge.weight;
                    if (alt < distance[edge.dest]) {
                        distance[edge.dest] = alt;
                        previous[edge.dest] = current;
                    }
                }
            }
        }
    }

    // Reconstruct path if end was reached
    if (distance[normalizedEnd] != std::numeric_limits<double>::infinity()) {
        std::vector<std::string> path;
        std::string current = normalizedEnd;

        while (current != normalizedStart) {
            path.push_back(current);
            current = previous[current];
        }

        path.push_back(normalizedStart);
        std::reverse(path.begin(), path.end());

        return { distance[normalizedEnd], path };
    }

    return { -1, {} }; // No path found
}

// Compute shortest paths from a single source to all other vertices
std::map<std::string, std::pair<double, std::vector<std::string>>> Graph::shortestPathsFromSource(const std::string& start) const {
    std::map<std::string, std::pair<double, std::vector<std::string>>> result;
    std::string normalizedStart = normalizeWord(start);

    // Check if start word exists in the graph
    if (adjacencyList.find(normalizedStart) == adjacencyList.end()) {
        return result; // Empty map indicates word not found
    }

    // Find shortest path to each vertex
    for (const auto& entry : adjacencyList) {
        const std::string& dest = entry.first;
        if (dest != normalizedStart) {
            std::pair<double, std::vector<std::string>> path = shortestPath(normalizedStart, dest);
            if (path.first != -1) {
                result[dest] = path;
            }
        }
    }

    return result;
}

// // Calculate PageRank
// std::map<std::string, double> Graph::calculatePageRank(double dampingFactor, int iterations) const {
//     std::map<std::string, double> pageRank;

//     // Initialize PageRank values
//     double initialRank = 1.0 / adjacencyList.size();
//     for (const auto& entry : adjacencyList) {
//         pageRank[entry.first] = initialRank;
//     }

//     // Iterate to refine PageRank values
//     for (int i = 0; i < iterations; ++i) {
//         std::map<std::string, double> newRank;

//         // Initialize new ranks with (1-d)/N
//         double baseRank = (1.0 - dampingFactor) / adjacencyList.size();
//         for (const auto& entry : adjacencyList) {
//             newRank[entry.first] = baseRank;
//         }

//         // Calculate influence from each vertex
//         for (const auto& entry : adjacencyList) {
//             const std::string& vertex = entry.first;
//             const std::vector<Edge>& edges = entry.second;

//             if (!edges.empty()) {
//                 // Calculate total weight of outgoing edges
//                 double totalWeight = 0;
//                 for (const Edge& edge : edges) {
//                     totalWeight += edge.weight;
//                 }

//                 // Distribute rank to neighbors
//                 for (const Edge& edge : edges) {
//                     double contribution = dampingFactor * pageRank[vertex] * (edge.weight / totalWeight);
//                     newRank[edge.dest] += contribution;
//                 }
//             }
//         }

//         // Update PageRank values
//         pageRank = newRank;
//     }

//     return pageRank;
// }

// Perform random walk on the graph
std::vector<std::string> Graph::randomWalk() {
    if (adjacencyList.empty()) {
        return {}; // Empty graph
    }

    std::vector<std::string> path;
    std::set<std::pair<std::string, std::string>> visitedEdges;

    // Choose a random starting vertex
    std::vector<std::string> vertices;
    for (const auto& entry : adjacencyList) {
        vertices.push_back(entry.first);
    }

    std::uniform_int_distribution<size_t> dist(0, vertices.size() - 1);
    std::string current = vertices[dist(rng)];
    path.push_back(current);

    while (true) {
        // Check if current vertex has outgoing edges
        if (adjacencyList.at(current).empty()) {
            break; // Stop if no outgoing edges
        }

        // Choose a random outgoing edge
        const std::vector<Edge>& edges = adjacencyList.at(current);
        std::uniform_int_distribution<size_t> edgeDist(0, edges.size() - 1);
        const Edge& edge = edges[edgeDist(rng)];

        // Check if this edge has been visited
        std::pair<std::string, std::string> edgePair = { current, edge.dest };
        if (visitedEdges.find(edgePair) != visitedEdges.end()) {
            break; // Stop if edge already visited
        }

        // Add edge to visited and add destination to path
        visitedEdges.insert(edgePair);
        current = edge.dest;
        path.push_back(current);
    }

    return path;
}

// Check if a word exists in the graph
bool Graph::containsWord(const std::string& word) const {
    std::string normalizedWord = normalizeWord(word);
    return adjacencyList.find(normalizedWord) != adjacencyList.end();
}

// Get all vertices (words) in the graph
std::vector<std::string> Graph::getAllVertices() const {
    std::vector<std::string> vertices;
    for (const auto& entry : adjacencyList) {
        vertices.push_back(entry.first);
    }
    return vertices;
}

// Calculate PageRank with custom initial ranks
std::map<std::string, double> Graph::calculatePageRank(double dampingFactor, int iterations,
    std::map<std::string, double> customInitialRanks) const {
    std::map<std::string, double> pageRank;

    // Initialize PageRank values - use custom ranks if provided, or default to uniform distribution
    if (!customInitialRanks.empty()) {
        // Use provided custom initial ranks
        double sum = 0.0;
        // First copy the custom ranks for existing vertices
        for (const auto& entry : adjacencyList) {
            const std::string& vertex = entry.first;
            if (customInitialRanks.find(vertex) != customInitialRanks.end()) {
                pageRank[vertex] = customInitialRanks.at(vertex);
                sum += pageRank[vertex];
            }
            else {
                // Default value for vertices without custom rank
                pageRank[vertex] = 0.5;
                sum += 0.5;
            }
        }

        // Normalize to make sure sum equals 1.0
        for (auto& entry : pageRank) {
            entry.second /= sum;
        }
    }
    else {
        // Use traditional uniform distribution
        double initialRank = 1.0 / adjacencyList.size();
        printf("initialRank: %f\n", initialRank);
        for (const auto& entry : adjacencyList) {
            pageRank[entry.first] = initialRank;
        }
    }

    // Get the total number of vertices in the graph
    size_t totalVertices = adjacencyList.size();

    // Iterate to refine PageRank values
    for (int i = 0; i < iterations; ++i) {
        std::map<std::string, double> newRank;
        double danglingSum = 0.0;

        // Initialize new ranks with (1-d)/N
        double baseRank = (1.0 - dampingFactor) / totalVertices;
        for (const auto& entry : adjacencyList) {
            newRank[entry.first] = baseRank;
        }

        // First, collect the sum of PageRank values from dangling nodes (nodes with no outgoing edges)
        for (const auto& entry : adjacencyList) {
            const std::string& vertex = entry.first;
            const std::vector<Edge>& edges = entry.second;

            if (edges.empty()) {
                // This is a dangling node, add its contribution to danglingSum
                danglingSum += pageRank[vertex];
            }
        }

        // Distribute the dangling node PageRank values evenly to all vertices
        double danglingContribution = dampingFactor * danglingSum / totalVertices;
        for (auto& entry : newRank) {
            entry.second += danglingContribution;
        }

        // Calculate influence from each non-dangling vertex
        for (const auto& entry : adjacencyList) {
            const std::string& vertex = entry.first;
            const std::vector<Edge>& edges = entry.second;

            if (!edges.empty()) {
                // Calculate total weight of outgoing edges
                double totalWeight = 0;
                for (const Edge& edge : edges) {
                    totalWeight += edge.weight;
                }

                // Distribute rank to neighbors
                for (const Edge& edge : edges) {
                    double contribution = dampingFactor * pageRank[vertex] * (edge.weight / totalWeight);
                    newRank[edge.dest] += contribution;
                }
            }
        }

        // Update PageRank values
        pageRank = newRank;
    }

    return pageRank;
}

// Helper function to calculate TF-IDF initial ranks
std::map<std::string, double> Graph::calculateTfIdfRanks(const std::string& filePath) const {
    std::map<std::string, double> tfIdfRanks;
    std::map<std::string, int> termFrequency; // Term frequency
    std::map<std::string, int> docFrequency;  // Document frequency

    // Use the original text to calculate TF-IDF
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filePath << " for TF-IDF calculation." << std::endl;
        return tfIdfRanks;
    }

    // Count term frequencies
    std::string line;
    std::vector<std::string> sentences;

    // Read the file line by line
    while (std::getline(file, line)) {
        if (!line.empty()) {
            sentences.push_back(line);
        }
    }
    file.close();

    // Handle case with only one line/document
    int numDocs = sentences.size();
    if (numDocs == 0) {
        return tfIdfRanks; // Return empty map if no content
    }

    // If only one line, split it into multiple virtual sentences
    if (numDocs == 1) {
        std::string content = sentences[0];
        sentences.clear();

        // Split by punctuation or every N words to create virtual documents
        std::stringstream ss(content);
        std::string word;
        std::string currentSentence;
        int wordCount = 0;
        const int maxWordsPerSentence = 5; // Adjust as needed

        while (ss >> word) {
            currentSentence += word + " ";
            wordCount++;

            if (wordCount >= maxWordsPerSentence) {
                sentences.push_back(currentSentence);
                currentSentence = "";
                wordCount = 0;
            }
        }

        // Add any remaining words as the last sentence
        if (!currentSentence.empty()) {
            sentences.push_back(currentSentence);
        }

        // Update numDocs
        numDocs = sentences.size();
    }

    // Process each sentence as a document
    for (const auto& sentence : sentences) {
        std::stringstream ss(sentence);
        std::string word;
        std::set<std::string> uniqueWordsInDoc;

        while (ss >> word) {
            std::string normalizedWord = normalizeWord(word);
            if (!normalizedWord.empty()) {
                termFrequency[normalizedWord]++;
                uniqueWordsInDoc.insert(normalizedWord);
            }
        }

        // Update document frequency
        for (const auto& uniqueWord : uniqueWordsInDoc) {
            docFrequency[uniqueWord]++;
        }
    }

    // Calculate TF-IDF for each word
    for (const auto& entry : adjacencyList) {
        const std::string& vertex = entry.first;

        // Default value for cases where TF-IDF calculation isn't reliable
        double tfidf = 0.5; // Start with a reasonable default

        if (termFrequency.find(vertex) != termFrequency.end()) {
            double tf = static_cast<double>(termFrequency[vertex]);

            // Avoid division by zero and log(1) = 0 issues
            if (docFrequency.find(vertex) != docFrequency.end() && docFrequency[vertex] > 0 && numDocs > 1) {
                double idf = log(static_cast<double>(numDocs) / docFrequency[vertex]);
                tfidf = tf * idf;
            }
            else {
                // Use term frequency directly when IDF isn't reliable
                tfidf = tf;
            }

            // Ensure we never have zero or negative values
            if (tfidf <= 0) {
                tfidf = 0.1;
            }
        }

        tfIdfRanks[vertex] = tfidf;
    }

    // Normalize the ranks so they sum to 1.0
    double sum = 0.0;
    for (const auto& entry : tfIdfRanks) {
        sum += entry.second;
    }

    if (sum > 0) {
        for (auto& entry : tfIdfRanks) {
            entry.second /= sum;
            printf("\"%s\" TF-IDF initialRank: %f\n", entry.first.c_str(), entry.second);
        }
    }
    else {
        // Fallback to uniform distribution if sum is 0
        double uniformRank = 1.0 / tfIdfRanks.size();
        for (auto& entry : tfIdfRanks) {
            entry.second = uniformRank;
        }
    }

    return tfIdfRanks;
}

// Calculate PageRank with TF-IDF as initial ranks
std::map<std::string, double> Graph::calculatePageRankWithTfIdf(const std::string& filePath,
    double dampingFactor,
    int iterations) const {
    std::map<std::string, double> tfIdfRanks = calculateTfIdfRanks(filePath);
    return calculatePageRank(dampingFactor, iterations, tfIdfRanks);
}
