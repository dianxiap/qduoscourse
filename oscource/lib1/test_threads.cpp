#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <regex>
#include <queue>
#include <sstream> // 用于字符串流操作

std::mutex mtx;
std::unordered_map<std::string, int> global_word_count;
int global_total_words = 0;

void process_file(const std::string& filename) {
    std::ifstream file(filename);
    std::unordered_map<std::string, int> word_count;
    std::string word;
    std::regex word_regex("\\b[a-zA-Z]+\\b");

    while (file >> word) {
        std::smatch match;
        if (std::regex_search(word, match, word_regex)) {
            word = match.str();
            std::transform(word.begin(), word.end(), word.begin(), ::tolower);
            word_count[word]++;
            mtx.lock();
            global_total_words++;
            mtx.unlock();
        }
    }

    mtx.lock();
    for (const auto& w : word_count) {
        global_word_count[w.first] += w.second;
    }
    mtx.unlock();
    file.close();
}

std::vector<std::pair<std::string, int>> get_top_n_words(int n) {
    std::vector<std::pair<std::string, int>> result;
    std::priority_queue<std::pair<int, std::string>> pq;
    for (const auto& p : global_word_count) {
        pq.push({p.second, p.first});
    }
    for (int i = 0; i < n && !pq.empty(); ++i) {
        result.push_back({pq.top().second, pq.top().first});
        pq.pop();
    }
    return result;
}

int main() {
    std::vector<std::string> filenames;
    int num_files = 10; // 假设有10个文件
    std::string directory = "test_file/"; // 文件夹路径

    // 使用循环生成文件名
    for (int i = 1; i <= num_files; ++i) {
        std::ostringstream oss;
        oss << directory << i << ".txt"; // 将文件夹路径和文件名组合
        filenames.push_back(oss.str());
    }

    std::vector<std::thread> threads;

    // 为每个文件创建一个新线程
    for (const auto& filename : filenames) {
        threads.push_back(std::thread(process_file, filename));
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 输出统计结果
    std::cout << "总单词数: " << global_total_words << std::endl;

    auto top_words = get_top_n_words(10);
    std::cout << "Top 10 热词:" << std::endl;
    for (const auto& word : top_words) {
        std::cout << word.first << " (" << word.second << "次)" << std::endl;
    }

    return 0;
}
