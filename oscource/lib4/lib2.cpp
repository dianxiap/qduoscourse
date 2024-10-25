#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h> // 用于共享内存

using namespace std;

class PageReplacementSimulator
{
private:
    int memory_size;      // 内存容量
    int page_length;      // 每个页面的大小
    int process_capacity; // 进程容量
    int page_count;       // 页面数 (process_capacity / page_length)
    int resident_size;    // 驻留集大小
    int *page_sequence;   // 页面访问序列，改为共享内存指针

public:
    PageReplacementSimulator(int mem_size, int page_len, int proc_cap, int res_size, int *page_seq)
        : memory_size(mem_size), page_length(page_len), process_capacity(proc_cap),
          page_count(proc_cap / page_len), resident_size(res_size), page_sequence(page_seq)
    {
        // 空构造函数
    }

    void input_page_sequence()
    {
        char choice;
        cout << "是否随机生成页面访问序列？(y/n): ";
        cin >> choice;

        // 为 page_sequence 分配共享内存
        page_sequence = (int *)mmap(NULL, page_count * sizeof(int),
                                    PROT_READ | PROT_WRITE,
                                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (page_sequence == MAP_FAILED)
        {
            cerr << "共享内存分配失败！" << endl;
            exit(1);
        }

        if (choice == 'y' || choice == 'Y')
        {
            for (int i = 0; i < page_count; ++i)
            {
                page_sequence[i] = rand() % page_count;
            }
        }
        else
        {
            cout << "请手动输入页面访问序列（" << page_count << " 个页面）: ";
            for (int i = 0; i < page_count; ++i)
            {
                int page;
                cin >> page;
                page_sequence[i] = page;
            }
        }
    }

    void simulate_FIFO()
    {
        vector<int> memory; // 内存驻留集
        queue<int> fifo_queue;
        int page_faults = 0;

        cout << "开始FIFO页面置换模拟...\n";

        for (int i = 0; i < page_count; ++i)
        {
            int page = page_sequence[i];
            int time_cost = rand() % 100; // 模拟耗时

            cout << "访问页面: " << page << " -> ";
            if (find(memory.begin(), memory.end(), page) != memory.end())
            {
                cout << "命中，耗时：" << time_cost << " 单位时间\n";
                usleep(time_cost * 100); // 模拟命中的时间消耗
                continue;
            }
            page_faults++;
            if (memory.size() < resident_size)
            {
                memory.push_back(page);
                cout << "直接装入，耗时：" << time_cost << " 单位时间\n";
                usleep(time_cost * 100); // 模拟直接装入的时间消耗
            }
            else
            {
                int oldest_page = fifo_queue.front();
                fifo_queue.pop();
                replace(memory.begin(), memory.end(), oldest_page, page);
                cout << "缺页，替换，耗时：" << time_cost << " 单位时间\n";
                usleep(time_cost * 100); // 模拟缺页的时间消耗
            }
            fifo_queue.push(page);
        }
        cout << "FIFO缺页次数: " << page_faults << endl;
    }

    void simulate_LRU()
    {
        vector<int> memory;
        unordered_map<int, int> last_access_time;
        int page_faults = 0;
        int time_counter = 0;

        cout << "开始LRU页面置换模拟...\n";

        for (int i = 0; i < page_count; ++i)
        {
            int page = page_sequence[i];
            time_counter++;
            int time_cost = rand() % 100; // 模拟耗时

            cout << "访问页面: " << page << " -> ";
            if (find(memory.begin(), memory.end(), page) != memory.end())
            {
                cout << "命中，耗时：" << time_cost << " 单位时间\n";
                usleep(time_cost * 100); // 模拟命中的时间消耗
                last_access_time[page] = time_counter;
                continue;
            }
            page_faults++;
            if (memory.size() < resident_size)
            {
                memory.push_back(page);
                cout << "直接装入，耗时：" << time_cost << " 单位时间\n";
                usleep(time_cost * 100); // 模拟直接装入的时间消耗
                last_access_time[page] = time_counter;
            }
            else
            {
                int lru_page = *min_element(memory.begin(), memory.end(),
                                            [&](int a, int b)
                                            { return last_access_time[a] < last_access_time[b]; });
                replace(memory.begin(), memory.end(), lru_page, page);
                cout << "缺页，替换，耗时：" << time_cost << " 单位时间\n";
                usleep(time_cost * 100); // 模拟缺页的时间消耗
                last_access_time[page] = time_counter;
            }
        }
        cout << "LRU缺页次数: " << page_faults << endl;
    }

    void simulate_OPT()
    {
        vector<int> memory;
        int page_faults = 0;

        cout << "开始OPT页面置换模拟...\n";

        for (int i = 0; i < page_count; ++i)
        {
            int page = page_sequence[i];
            int time_cost = rand() % 100; // 模拟耗时

            cout << "访问页面: " << page << " -> ";
            if (find(memory.begin(), memory.end(), page) != memory.end())
            {
                cout << "命中，耗时：" << time_cost << " 单位时间\n";
                usleep(time_cost * 100); // 模拟命中的时间消耗
                continue;
            }
            page_faults++;
            if (memory.size() < resident_size)
            {
                memory.push_back(page);
                cout << "直接装入，耗时：" << time_cost << " 单位时间\n";
                usleep(time_cost * 100); // 模拟直接装入的时间消耗
            }
            else
            {
                int farthest_page = -1;
                int farthest_distance = -1;
                for (int m : memory)
                {
                    int next_use = -1;
                    for (int j = i + 1; j < page_count; ++j)
                    {
                        if (page_sequence[j] == m)
                        {
                            next_use = j;
                            break;
                        }
                    }
                    if (next_use == -1)
                    {
                        farthest_page = m;
                        break;
                    }
                    if (next_use > farthest_distance)
                    {
                        farthest_distance = next_use;
                        farthest_page = m;
                    }
                }
                replace(memory.begin(), memory.end(), farthest_page, page);
                cout << "缺页，替换，耗时：" << time_cost << " 单位时间\n";
                usleep(time_cost * 100); // 模拟缺页的时间消耗
            }
        }
        cout << "OPT缺页次数: " << page_faults << endl;
    }

    void simulate_Clock()
    {
        vector<int> memory;          // 驻留集
        vector<bool> reference_bits; // 访问位
        int page_faults = 0;
        int clock_pointer = 0; // 时钟指针

        cout << "开始Clock页面置换模拟...\n";

        for (int i = 0; i < page_count; ++i)
        {
            int page = page_sequence[i];
            int time_cost = rand() % 100; // 模拟耗时

            cout << "访问页面: " << page << " -> ";

            // 如果页面在内存中，则更新访问位
            auto it = find(memory.begin(), memory.end(), page);
            if (it != memory.end())
            {
                int index = distance(memory.begin(), it);
                reference_bits[index] = true; // 设置访问位
                cout << "命中，耗时：" << time_cost << " 单位时间\n";
                usleep(time_cost * 100); // 模拟命中的时间消耗
                continue;
            }

            // 如果页面不在内存中，缺页
            page_faults++;

            if (memory.size() < resident_size)
            {
                // 如果内存未满，直接加入
                memory.push_back(page);
                reference_bits.push_back(true); // 新加入的页面访问位为1
                cout << "直接装入，耗时：" << time_cost << " 单位时间\n";
                usleep(time_cost * 100); // 模拟直接装入的时间消耗
            }
            else
            {
                // 启动时钟替换算法
                while (reference_bits[clock_pointer])
                {
                    reference_bits[clock_pointer] = false;
                    clock_pointer = (clock_pointer + 1) % resident_size;
                }
                // 替换当前指针指向的页面
                memory[clock_pointer] = page;
                reference_bits[clock_pointer] = true; // 新页面的访问位为1
                cout << "缺页，替换，耗时：" << time_cost << " 单位时间\n";
                usleep(time_cost * 100); // 模拟缺页的时间消耗

                // 移动指针
                clock_pointer = (clock_pointer + 1) % resident_size;
            }
        }
        cout << "Clock缺页次数: " << page_faults << endl;
    }
    void simulate_LFU()
    {
        vector<int> memory;                 // 驻留集
        unordered_map<int, int> freq_count; // 记录每个页面的使用频率
        int page_faults = 0;

        cout << "开始LFU页面置换模拟...\n";

        for (int i = 0; i < page_count; ++i)
        {
            int page = page_sequence[i];
            int time_cost = rand() % 100; // 模拟耗时

            cout << "访问页面: " << page << " -> ";

            // 如果页面在内存中，增加其访问频率
            auto it = find(memory.begin(), memory.end(), page);
            if (it != memory.end())
            {
                freq_count[page]++;
                cout << "命中，耗时：" << time_cost << " 单位时间\n";
                usleep(time_cost * 100); // 模拟命中的时间消耗
                continue;
            }

            // 如果页面不在内存中，缺页
            page_faults++;

            if (memory.size() < resident_size)
            {
                // 如果内存未满，直接加入
                memory.push_back(page);
                freq_count[page] = 1; // 新页面的访问频率为1
                cout << "直接装入，耗时：" << time_cost << " 单位时间\n";
                usleep(time_cost * 100); // 模拟直接装入的时间消耗
            }
            else
            {
                // 找到使用频率最少的页面
                int lfu_page = memory[0];
                for (int m : memory)
                {
                    if (freq_count[m] < freq_count[lfu_page])
                    {
                        lfu_page = m;
                    }
                }
                // 替换使用频率最少的页面
                replace(memory.begin(), memory.end(), lfu_page, page);
                freq_count.erase(lfu_page); // 移除旧页面的记录
                freq_count[page] = 1;       // 新页面的访问频率为1
                cout << "缺页，替换，耗时：" << time_cost << " 单位时间\n";
                usleep(time_cost * 100); // 模拟缺页的时间消耗
            }
        }
        cout << "LFU缺页次数: " << page_faults << endl;
    }
};

void run_simulation(PageReplacementSimulator &simulator, void (PageReplacementSimulator::*simulate_method)())
{
    (simulator.*simulate_method)();
}

int main()
{
    srand(static_cast<unsigned>(time(0)));
    int memory_size, page_length, process_capacity, resident_size;

    cout << "请输入内存容量: ";
    cin >> memory_size;

    cout << "请输入页面长度: ";
    cin >> page_length;

    cout << "请输入进程容量（字节数）: ";
    cin >> process_capacity;

    cout << "请输入驻留集大小: ";
    cin >> resident_size;

    if (process_capacity % page_length != 0)
    {
        cout << "错误：进程容量必须是页面长度的整数倍！" << endl;
        return -1;
    }

    // 创建共享内存用于页面访问序列
    int *shared_page_sequence = (int *)mmap(nullptr, (process_capacity / page_length) * sizeof(int),
                                            PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);


    PageReplacementSimulator simulator(memory_size, page_length, process_capacity, resident_size, shared_page_sequence);

    simulator.input_page_sequence();

    // 使用多进程模拟
    pid_t pid1 = fork();
    if (pid1 == 0)
    {
        run_simulation(simulator, &PageReplacementSimulator::simulate_FIFO);
        _exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 == 0)
    {
        run_simulation(simulator, &PageReplacementSimulator::simulate_LRU);
        _exit(0);
    }

    pid_t pid3 = fork();
    if (pid3 == 0)
    {
        run_simulation(simulator, &PageReplacementSimulator::simulate_OPT);
        _exit(0);
    }

    pid_t pid4 = fork();
    if (pid4 == 0)
    {
        run_simulation(simulator, &PageReplacementSimulator::simulate_Clock);
        _exit(0);
    }

    pid_t pid5 = fork();
    if (pid5 == 0)
    {
        run_simulation(simulator, &PageReplacementSimulator::simulate_LFU);
        _exit(0);
    }

    // 等待所有子进程结束
    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);
    waitpid(pid3, nullptr, 0);
    waitpid(pid4, nullptr, 0);
    waitpid(pid5, nullptr, 0);

    // 解除共享内存映射
    munmap(shared_page_sequence, (process_capacity / page_length) * sizeof(int));

    return 0;
}
