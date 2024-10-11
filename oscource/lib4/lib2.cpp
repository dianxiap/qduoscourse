#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <unistd.h> // 用于usleep函数

using namespace std;

class PageReplacementSimulator {
private:
    int memory_size;        // 内存容量
    int page_length;        // 每个页面的大小
    int process_capacity;   // 进程容量
    int page_count;         // 页面数 (process_capacity / page_length)
    int resident_size;      // 驻留集大小
    vector<int> page_sequence;  // 页面访问序列
    vector<int> memory;     // 内存驻留集
    unordered_map<int, int> page_access_count; // 页面访问次数 (用于LFU)
    int time_counter;       // 访问时间计数器
    int page_faults;        // 缺页次数
    unordered_map<int, int> last_access_time; // 页面上次访问时间 (用于LRU)

public:
    PageReplacementSimulator(int mem_size, int page_len, int proc_cap, int res_size)
        : memory_size(mem_size), page_length(page_len), process_capacity(proc_cap), 
          page_count(proc_cap / page_len), resident_size(res_size), time_counter(0), page_faults(0) {
        memory.reserve(resident_size);
    }

    void input_page_sequence() {
        char choice;
        cout << "是否随机生成页面访问序列？(y/n): ";
        cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            for (int i = 0; i < page_count; ++i) {
                page_sequence.push_back(rand() % page_count);
            }
        } else {
            cout << "请手动输入页面访问序列（" << page_count << " 个页面）: ";
            for (int i = 0; i < page_count; ++i) {
                int page;
                cin >> page;
                page_sequence.push_back(page);
            }
        }
    }

    void simulate_FIFO() {
        cout << "开始FIFO页面置换模拟...\n";
        queue<int> fifo_queue;
        page_faults = 0;
        for (int page : page_sequence) {
            cout << "访问页面: " << page << " -> ";
            if (find(memory.begin(), memory.end(), page) != memory.end()) {
                cout << "命中，耗时：" << rand() % 10 << " 单位时间\n";
                usleep(rand() % 10000); // 模拟命中的时间消耗
                continue;
            }
            page_faults++;
            if (memory.size() < resident_size) {
                memory.push_back(page);
                cout << "直接装入，耗时：" << rand() % 100 << " 单位时间\n";
                usleep(rand() % 100000); // 模拟直接装入的时间消耗
            } else {
                int oldest_page = fifo_queue.front();
                fifo_queue.pop();
                replace(memory.begin(), memory.end(), oldest_page, page);
                cout << "缺页，替换，耗时：" << rand() % 100 << " 单位时间\n";
                usleep(rand() % 100000); // 模拟缺页的时间消耗
            }
            fifo_queue.push(page);
        }
        cout << "FIFO缺页次数: " << page_faults << endl;
    }

    void simulate_LRU() {
        cout << "开始LRU页面置换模拟...\n";
        page_faults = 0;
        for (int page : page_sequence) {
            cout << "访问页面: " << page << " -> ";
            time_counter++;
            if (find(memory.begin(), memory.end(), page) != memory.end()) {
                cout << "命中，耗时：" << rand() % 10 << " 单位时间\n";
                usleep(rand() % 10000); // 模拟命中的时间消耗
                last_access_time[page] = time_counter;
                continue;
            }
            page_faults++;
            if (memory.size() < resident_size) {
                memory.push_back(page);
                cout << "直接装入，耗时：" << rand() % 100 << " 单位时间\n";
                usleep(rand() % 100000); // 模拟直接装入的时间消耗
                last_access_time[page] = time_counter;
            } else {
                int lru_page = *min_element(memory.begin(), memory.end(), 
                    [&](int a, int b) { return last_access_time[a] < last_access_time[b]; });
                replace(memory.begin(), memory.end(), lru_page, page);
                cout << "缺页，替换，耗时：" << rand() % 100 << " 单位时间\n";
                usleep(rand() % 100000); // 模拟缺页的时间消耗
                last_access_time[page] = time_counter;
            }
        }
        cout << "LRU缺页次数: " << page_faults << endl;
    }

    void simulate_OPT() {
        cout << "开始OPT页面置换模拟...\n";
        page_faults = 0;
        for (int i = 0; i < page_sequence.size(); ++i) {
            int page = page_sequence[i];
            cout << "访问页面: " << page << " -> ";
            if (find(memory.begin(), memory.end(), page) != memory.end()) {
                cout << "命中，耗时：" << rand() % 10 << " 单位时间\n";
                usleep(rand() % 10000); // 模拟命中的时间消耗
                continue;
            }
            page_faults++;
            if (memory.size() < resident_size) {
                memory.push_back(page);
                cout << "直接装入，耗时：" << rand() % 100 << " 单位时间\n";
                usleep(rand() % 100000); // 模拟直接装入的时间消耗
            } else {
                int farthest_page = -1;
                int farthest_distance = -1;
                for (int m : memory) {
                    int next_use = -1;
                    for (int j = i + 1; j < page_sequence.size(); ++j) {
                        if (page_sequence[j] == m) {
                            next_use = j;
                            break;
                        }
                    }
                    if (next_use == -1) {
                        farthest_page = m;
                        break;
                    }
                    if (next_use > farthest_distance) {
                        farthest_distance = next_use;
                        farthest_page = m;
                    }
                }
                replace(memory.begin(), memory.end(), farthest_page, page);
                cout << "缺页，替换，耗时：" << rand() % 100 << " 单位时间\n";
                usleep(rand() % 100000); // 模拟缺页的时间消耗
            }
        }
        cout << "OPT缺页次数: " << page_faults << endl;
    }

    void simulate_Clock() {
        cout << "开始Clock页面置换模拟...\n";
        vector<bool> use_bit(memory.size(), false);
        page_faults = 0;
        for (int i = 0; i < page_sequence.size(); ++i) {
            int page = page_sequence[i];
            cout << "访问页面: " << page << " -> ";
            int j = 0;
            bool found = false;
            while (j < memory.size()) {
                if (memory[j] == page) {
                    cout << "命中，耗时：" << rand() % 10 << " 单位时间\n";
                    usleep(rand() % 10000); // 模拟命中的时间消耗
                    use_bit[j] = true;
                    found = true;
                    break;
                }
                if (!use_bit[j] && memory.size() >= resident_size) {
                    use_bit[j] = true;
                    memory[j] = page;
                    cout << "缺页，替换，耗时：" << rand() % 100 << " 单位时间\n";
                    usleep(rand() % 100000); // 模拟缺页的时间消耗
                    page_faults++;
                    found = true;
                    break;
                }
                j = (j + 1) % memory.size();
            }
            if (!found) {
                memory.push_back(page);
                use_bit.push_back(true);
                cout << "直接装入，耗时：" << rand() % 100 << " 单位时间\n";
                usleep(rand() % 100000); // 模拟直接装入的时间消耗
            }
            for (int k = 0; k < memory.size(); ++k) {
                use_bit[k] = false;
            }
        }
        cout << "Clock缺页次数: " << page_faults << endl;
    }


    void simulate_LFU() {
        cout << "开始LFU页面置换模拟...\n";
        page_faults = 0;
        for (int page : page_sequence) {
            cout << "访问页面: " << page << " -> ";
            if (find(memory.begin(), memory.end(), page) != memory.end()) {
                cout << "命中，耗时：" << rand() % 10 << " 单位时间\n";
                usleep(rand() % 10000); // 模拟命中的时间消耗
                page_access_count[page]++;
                continue;
            }
            page_faults++;
            if (memory.size() < resident_size) {
                memory.push_back(page);
                page_access_count[page] = 1;
                cout << "直接装入，耗时：" << rand() % 100 << " 单位时间\n";
                usleep(rand() % 100000); // 模拟直接装入的时间消耗
            } else {
                int lfu_page = *min_element(memory.begin(), memory.end(), 
                    [&](int a, int b) { return page_access_count[a] < page_access_count[b]; });
                replace(memory.begin(), memory.end(), lfu_page, page);
                cout << "缺页，替换，耗时：" << rand() % 100 << " 单位时间\n";
                usleep(rand() % 100000); // 模拟缺页的时间消耗
                page_access_count[lfu_page] = 0; // 重置被替换页面的访问次数
                page_access_count[page] = 1;
            }
        }
        cout << "LFU缺页次数: " << page_faults << endl;
    }

    void calculate_and_display_page_fault_rate(const string& algorithm_name) {
        double fault_rate = static_cast<double>(page_faults) / page_sequence.size();
        cout << algorithm_name << "缺页率: " << fault_rate << endl;
    }
};

int main() {
    srand(time(0));
    int memory_size, page_length, process_capacity, resident_size;

    cout << "请输入内存容量: ";
    cin >> memory_size;
    
    cout << "请输入页面长度: ";
    cin >> page_length;

    cout << "请输入进程容量（字节数）: ";
    cin >> process_capacity;
    
    cout << "请输入驻留集大小: ";
    cin >> resident_size;

    // 确保进程容量可以被页面长度整除
    if (process_capacity % page_length != 0) {
        cout << "错误：进程容量必须是页面长度的整数倍！" << endl;
        return -1;
    }

    // 创建页面置换模拟器
    PageReplacementSimulator simulator(memory_size, page_length, process_capacity, resident_size);

    // 输入页面访问序列
    simulator.input_page_sequence();

    // 运行FIFO模拟
    simulator.simulate_FIFO();
    simulator.calculate_and_display_page_fault_rate("FIFO");

    // 运行LRU模拟
    simulator.simulate_LRU();
    simulator.calculate_and_display_page_fault_rate("LRU");

    // 运行OPT模拟
    simulator.simulate_OPT();
    simulator.calculate_and_display_page_fault_rate("OPT");

    // 运行Clock模拟
    simulator.simulate_Clock();
    simulator.calculate_and_display_page_fault_rate("Clock");

    // 运行LFU模拟
    simulator.simulate_LFU();
    simulator.calculate_and_display_page_fault_rate("LFU");

    return 0;
}