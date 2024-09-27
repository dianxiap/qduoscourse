#include <iostream>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>

using namespace std;

const size_t TOTAL_MEMORY = 50;  // 总内存大小
pthread_mutex_t memory_mutex;     // 互斥锁保护内存分配

ofstream log_file("memory_log.txt");  // 打开日志文件

struct MemoryBlock {
    size_t start;
    size_t size;
    bool is_free;

    // 带有三个参数的构造函数
    MemoryBlock(size_t s, size_t sz, bool free) : start(s), size(sz), is_free(free) {}

    // 定义 operator==，用于比较两个 MemoryBlock 对象
    bool operator==(const MemoryBlock& other) const {
        return start == other.start && size == other.size && is_free == other.is_free;
    }
};


vector<MemoryBlock> memory_blocks;  // 内存块列表

// 打印内存使用情况到日志文件
void log_memory_status() {
    pthread_mutex_lock(&memory_mutex);
    log_file << "当前内存状态：" << endl;
    for (const auto& block : memory_blocks) {
        log_file << "起始地址: " << block.start
                 << ", 大小: " << block.size
                 << ", " << (block.is_free ? "空闲" : "已占用") << endl;
    }
    log_file << "----------------------" << endl;
    pthread_mutex_unlock(&memory_mutex);
}

// 初始化内存，将整个内存初始化为一个大块空闲区
void init_memory() {
    memory_blocks.push_back(MemoryBlock(0, TOTAL_MEMORY, true));
    log_file << "初始化 " << TOTAL_MEMORY << " 大小的内存。" << endl;
}

// 内存分配算法 - 首次适应（First Fit）
MemoryBlock* first_fit(size_t request_size) {
    for (auto& block : memory_blocks) {
        if (block.is_free && block.size >= request_size) {
            return &block;
        }
    }
    return nullptr;
}

// 内存分配算法 - 最佳适应（Best Fit）
MemoryBlock* best_fit(size_t request_size) {
    MemoryBlock* best_block = nullptr;
    for (auto& block : memory_blocks) {
        if (block.is_free && block.size >= request_size) {
            if (best_block == nullptr || block.size < best_block->size) {
                best_block = &block;
            }
        }
    }
    return best_block;
}

// 内存分配算法 - 最坏适应（Worst Fit）
MemoryBlock* worst_fit(size_t request_size) {
    MemoryBlock* worst_block = nullptr;
    for (auto& block : memory_blocks) {
        if (block.is_free && block.size >= request_size) {
            if (worst_block == nullptr || block.size > worst_block->size) {
                worst_block = &block;
            }
        }
    }
    return worst_block;
}

// 分配内存
// 分配内存函数
MemoryBlock* allocate_memory(size_t request_size, int algorithm) {
    pthread_mutex_lock(&memory_mutex);
    MemoryBlock* allocated_block = nullptr;

    // 选择分配算法
    switch (algorithm) {
        case 0: // 首次适应
            allocated_block = first_fit(request_size);
            break;
        case 1: // 最佳适应
            allocated_block = best_fit(request_size);
            break;
        case 2: // 最坏适应
            allocated_block = worst_fit(request_size);
            break;
        default:
            log_file << "无效的算法选择" << endl;
    }

    // 如果找到合适的内存块，进行分配
    if (allocated_block) {
        if (allocated_block->size > request_size) {
            // 如果空闲块比请求块大，分割空闲块
            auto it = std::find(memory_blocks.begin(), memory_blocks.end(), *allocated_block);
            if (it != memory_blocks.end()) {
                memory_blocks.insert(it + 1, MemoryBlock(allocated_block->start + request_size, allocated_block->size - request_size, true));
            }
        }
        allocated_block->size = request_size;
        allocated_block->is_free = false;
        log_file << "分配了 " << request_size << " 大小的内存。" << endl;
    } 
    else 
    {
        log_file << "无法分配 " << request_size << " 大小的内存。" << endl;
    }
    pthread_mutex_unlock(&memory_mutex);
    log_memory_status();
    return allocated_block;
}

// 释放内存
void free_memory(MemoryBlock* block) {
    pthread_mutex_lock(&memory_mutex);
    block->is_free = true;
    log_file << "释放了 " << block->size << " 大小的内存。" << endl;

    // 合并相邻的空闲块
    for (size_t i = 0; i < memory_blocks.size(); i++) {
        if (memory_blocks[i].is_free) {
            if (i + 1 < memory_blocks.size() && memory_blocks[i + 1].is_free) {
                memory_blocks[i].size += memory_blocks[i + 1].size;
                memory_blocks.erase(memory_blocks.begin() + i + 1);
                i--; // 重新检查当前合并后的块
            }
        }
    }
    pthread_mutex_unlock(&memory_mutex);
    log_memory_status();
}

// 碎片整理，将空闲块移到一端
void defragment_memory() {
    pthread_mutex_lock(&memory_mutex);
    vector<MemoryBlock> allocated_blocks, free_blocks;
    for (const auto& block : memory_blocks) {
        if (block.is_free) {
            free_blocks.push_back(block);
        } else {
            allocated_blocks.push_back(block);
        }
    }

    memory_blocks.clear();
    memory_blocks.insert(memory_blocks.end(), allocated_blocks.begin(), allocated_blocks.end());
    memory_blocks.insert(memory_blocks.end(), free_blocks.begin(), free_blocks.end());

    // 更新每个块的起始地址
    size_t current_address = 0;
    for (auto& block : memory_blocks) {
        block.start = current_address;
        current_address += block.size;
    }

    log_file << "进行了内存碎片整理。" << endl;
    pthread_mutex_unlock(&memory_mutex);
    log_memory_status();
}

// 线程函数，模拟作业的执行
void* job_thread(void* arg) {
    int job_id = *((int*)arg);
    size_t memory_needed = rand() % 20 + 1;  // 随机生成需要的内存大小

    // 调用分配算法（0:FF, 1:BF, 2:WF）
    MemoryBlock* allocated_block = allocate_memory(memory_needed, rand() % 3);

    if (allocated_block != nullptr) {
        // 模拟作业执行
        sleep(rand() % 5 + 1);

        // 作业完成，释放内存
        free_memory(allocated_block);
    }

    return nullptr;
}

int main() {
    srand(time(nullptr));
    pthread_mutex_init(&memory_mutex, NULL);
    init_memory();  // 初始化内存

    const int NUM_JOBS = 5;
    pthread_t threads[NUM_JOBS];

    // 创建并执行作业线程
    for (int i = 0; i < NUM_JOBS; i++) {
        int* job_id = new int(i);
        pthread_create(&threads[i], NULL, job_thread, job_id);
    }

    // 等待所有线程结束
    for (int i = 0; i < NUM_JOBS; i++) {
        pthread_join(threads[i], NULL);
    }

    // 碎片整理
    defragment_memory();

    pthread_mutex_destroy(&memory_mutex);
    log_file.close();  // 关闭日志文件
    return 0;
}
