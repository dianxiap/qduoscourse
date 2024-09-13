#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <sstream>
#include <string>
#include <cctype>

// 函数：将单词转换为小写
std::string toLower(const std::string &word) {
    std::string lowerWord = word;
    for (char &ch : lowerWord) {
        ch = std::tolower(ch);
    }
    return lowerWord;
}

// 函数：处理文件，统计单词频率
void processFile(const std::string &filename, std::unordered_map<std::string, int> &wordCount, int &totalWords) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return;
    }

    std::string word;
    while (file >> word) {
        // 去掉标点符号并转换为小写
        word.erase(std::remove_if(word.begin(), word.end(), [](char c) { return std::ispunct(c); }), word.end());
        word = toLower(word);

        // 统计单词频率
        if (!word.empty()) {
            wordCount[word]++;
            totalWords++;
        }
    }
    file.close();
}

// 函数：找出Top-N热词
void findTopNWords(const std::unordered_map<std::string, int> &wordCount, int N) {
    std::vector<std::pair<std::string, int>> words(wordCount.begin(), wordCount.end());

    // 根据频率进行排序
    std::sort(words.begin(), words.end(), [](const auto &a, const auto &b) {
        return a.second > b.second;
    });

    // 输出Top-N热词
    std::cout << "Top " << N << " 热词:" << std::endl;
    for (int i = 0; i < N && i < words.size(); ++i) {
        std::cout << words[i].first << ": " << words[i].second << std::endl;
    }
}

int main() {
    std::unordered_map<std::string, int> wordCount;
    int totalWords = 0;
    int N = 10; 

    std::vector<std::string> filenames;
    int num_files = 10; // 假设有10个文件
    std::string directory = "test_file/"; // 文件夹路径

    // 使用循环生成文件名
    for (int i = 1; i <= num_files; ++i) {
        std::ostringstream oss;
        oss << directory << i << ".txt"; // 将文件夹路径和文件名组合
        filenames.push_back(oss.str());
    }
    
    // 处理每个文件
    for (const auto &filename : filenames) {
        processFile(filename, wordCount, totalWords);
    }

    // 输出单词总数
    std::cout << "单词总数: " << totalWords << std::endl;

    // 找出Top-N热词
    findTopNWords(wordCount, N);

    return 0;
}
