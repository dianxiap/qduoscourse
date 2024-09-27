#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <algorithm>

std::mutex mtx; 
std::condition_variable cv; 
std::queue<std::string> buffer; 
const int BUFFER_SIZE = 10; 
bool finished = false; 

// 生产者函数
void producer(const std::string& filename) {
    std::ifstream file(filename); // 打开文件
    std::string keyword;
    while (file >> keyword) { // 从文件中读取每个关键词
        std::unique_lock<std::mutex> lock(mtx); // 加锁
        cv.wait(lock, [] { return buffer.size() < BUFFER_SIZE; }); // 等待缓冲区有空位
        buffer.push(keyword); // 将关键词放入缓冲区
        lock.unlock(); // 解锁
        cv.notify_one(); // 通知消费者
    }
    finished = true; // 设置生产者完成标志
    cv.notify_all(); // 通知所有等待的线程
}

// 消费者函数
void consumer(const std::string& textfile) {
    std::ifstream file(textfile);
    std::string text((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>()); // 读取整个文本文件
    std::transform(text.begin(), text.end(), text.begin(), ::tolower); // 将文本转换为小写

    while (true) {
        std::unique_lock<std::mutex> lock(mtx); // 加锁
        cv.wait(lock, [] { return !buffer.empty() || finished; }); // 等待有关键词或生产者完成
        if (buffer.empty() && finished) break; // 如果缓冲区空且生产者完成，则退出
        std::string keyword = buffer.front(); // 获取缓冲区中的关键词
        buffer.pop(); // 移除关键词
        lock.unlock(); // 解锁
        cv.notify_one(); // 通知生产者

        // 关键词也转为小写匹配
        std::transform(keyword.begin(), keyword.end(), keyword.begin(), ::tolower);
        size_t pos = text.find(keyword); // 搜索关键词
        int count = 0;
        while (pos != std::string::npos) {
            count++;
            pos = text.find(keyword, pos + keyword.length()); // 更新搜索位置
        }
        std::cout << "关键词 '" << keyword << "' 出现次数: " << count << std::endl; // 打印关键词出现次数
    }
}

int main() {
    std::thread prod(producer, "keywords.txt"); // 创建生产者线程
    std::thread cons(consumer, "Bible.txt"); // 创建消费者线程

    prod.join(); 
    cons.join(); 

    return 0;
}
