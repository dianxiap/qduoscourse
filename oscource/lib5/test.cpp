#pragma execution_character_set("utf-8")
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

const int BLOCK_SIZE = 512; // 每个磁盘块大小
const int DISK_BLOCKS = 100; // 总磁盘块数量
const int INODE_BLOCK_START = 1; // 索引节点起始块
const int INODE_BLOCKS = 10; // 索引节点块数
const int DATA_BLOCK_START = 11; // 数据块起始块
const int SUPER_BLOCK = 0; // 超级块编号

struct Inode {
    char fileName[32]; // 文件名
    int fileSize; // 文件大小
    int dataBlock; // 数据块编号
    bool used; // 是否被使用
};

std::fstream disk; // 模拟磁盘文件
std::vector<Inode> inodes; // 索引节点数组

// 初始化文件系统
void initFileSystem() {
    disk.open("disk.txt", std::ios::in | std::ios::out | std::ios::binary);
    if (!disk.is_open()) {
        // 如果磁盘文件不存在，创建并初始化
        disk.open("disk.txt", std::ios::out | std::ios::binary);
        // 初始化超级块
        disk.seekp(SUPER_BLOCK * BLOCK_SIZE);
        disk.write("Super Block", 12);
        
        // 初始化索引节点块
        for (int i = INODE_BLOCK_START; i < INODE_BLOCK_START + INODE_BLOCKS; ++i) {
            disk.seekp(i * BLOCK_SIZE);
            Inode emptyInode = {"", 0, 0, false};
            for (int j = 0; j < BLOCK_SIZE / sizeof(Inode); ++j) {
                disk.write(reinterpret_cast<char*>(&emptyInode), sizeof(Inode));
            }
        }

        // 初始化数据块
        char emptyDataBlock[BLOCK_SIZE] = {0};
        for (int i = DATA_BLOCK_START; i < DISK_BLOCKS; ++i) {
            disk.seekp(i * BLOCK_SIZE);
            disk.write(emptyDataBlock, BLOCK_SIZE);
        }

        disk.close();
        std::cout << "磁盘初始化完成！" << std::endl;
    } else {
        std::cout << "磁盘已存在！" << std::endl;
    }
}

// 创建文件
void mkFile(const std::string& fileName) {
    disk.open("disk.txt", std::ios::in | std::ios::out | std::ios::binary);
    if (!disk.is_open()) {
        std::cerr << "无法打开磁盘文件" << std::endl;
        return;
    }

    // 找到一个空闲的索引节点
    for (int i = INODE_BLOCK_START; i < INODE_BLOCK_START + INODE_BLOCKS; ++i) {
        disk.seekg(i * BLOCK_SIZE);
        for (int j = 0; j < BLOCK_SIZE / sizeof(Inode); ++j) {
            Inode inode;
            disk.read(reinterpret_cast<char*>(&inode), sizeof(Inode));
            if (!inode.used) {
                // 填写文件信息
                strncpy(inode.fileName, fileName.c_str(), sizeof(inode.fileName) - 1); // 确保不会溢出
                inode.fileSize = 0;
                inode.dataBlock = DATA_BLOCK_START; // 分配数据块
                inode.used = true;

                // 更新磁盘上的索引节点
                disk.seekp(i * BLOCK_SIZE + j * sizeof(Inode));
                disk.write(reinterpret_cast<char*>(&inode), sizeof(Inode));
                disk.close();
                std::cout << "文件 " << fileName << " 创建成功！" << std::endl;
                return;
            }
        }
    }
    std::cerr << "没有可用的索引节点" << std::endl;
    disk.close();
}

// 删除文件
void deleteFile(const std::string& fileName) {
    disk.open("disk.txt", std::ios::in | std::ios::out | std::ios::binary);
    if (!disk.is_open()) {
        std::cerr << "无法打开磁盘文件" << std::endl;
        return;
    }

    for (int i = INODE_BLOCK_START; i < INODE_BLOCK_START + INODE_BLOCKS; ++i) {
        disk.seekg(i * BLOCK_SIZE);
        for (int j = 0; j < BLOCK_SIZE / sizeof(Inode); ++j) {
            Inode inode;
            disk.read(reinterpret_cast<char*>(&inode), sizeof(Inode));
            if (inode.used && fileName == inode.fileName) {
                // 删除文件，清空索引节点
                inode.used = false;
                disk.seekp(i * BLOCK_SIZE + j * sizeof(Inode));
                disk.write(reinterpret_cast<char*>(&inode), sizeof(Inode));
                disk.close();
                std::cout << "文件 " << fileName << " 删除成功！" << std::endl;
                return;
            }
        }
    }
    std::cerr << "找不到文件 " << fileName << std::endl;
    disk.close();
}

// 显示文件列表
void ls() {
    disk.open("disk.txt", std::ios::in | std::ios::binary);
    if (!disk.is_open()) {
        std::cerr << "无法打开磁盘文件" << std::endl;
        return;
    }
    
    for (int i = INODE_BLOCK_START; i < INODE_BLOCK_START + INODE_BLOCKS; ++i) {
        disk.seekg(i * BLOCK_SIZE);
        for (int j = 0; j < BLOCK_SIZE / sizeof(Inode); ++j) {
            Inode inode;
            disk.read(reinterpret_cast<char*>(&inode), sizeof(Inode));
            if (inode.used) {
                std::cout << "文件: " << inode.fileName << std::endl;
            }
        }
    }
    disk.close();
}

// 打开文件
void openFile(const std::string& fileName) {
    disk.open("disk.txt", std::ios::in | std::ios::binary);
    if (!disk.is_open()) {
        std::cerr << "无法打开磁盘文件" << std::endl;
        return;
    }

    for (int i = INODE_BLOCK_START; i < INODE_BLOCK_START + INODE_BLOCKS; ++i) {
        disk.seekg(i * BLOCK_SIZE);
        for (int j = 0; j < BLOCK_SIZE / sizeof(Inode); ++j) {
            Inode inode;
            disk.read(reinterpret_cast<char*>(&inode), sizeof(Inode));
            if (inode.used && fileName == inode.fileName) {
                // 打印文件的基本信息（暂时没有文件内容）
                std::cout << "文件: " << inode.fileName << std::endl;
                std::cout << "文件大小: " << inode.fileSize << " 字节" << std::endl;
                std::cout << "数据块位置: " << inode.dataBlock << std::endl;
                disk.close();
                return;
            }
        }
    }
    std::cerr << "找不到文件 " << fileName << std::endl;
    disk.close();
}

// 主函数：命令行模拟文件操作
int main() {
    initFileSystem();
    std::string command;
    while (true) {
        std::cout << "输入命令(mk <filename>, delete <filename>, ls, open <filename>, exit): ";
        std::getline(std::cin, command);

        if (command.substr(0, 3) == "mk ") {
            mkFile(command.substr(3));
        } else if (command.substr(0, 7) == "delete ") {
            deleteFile(command.substr(7));
        } else if (command == "ls") {
            ls();
        } else if (command.substr(0, 5) == "open ") {
            openFile(command.substr(5));
        } else if (command == "exit") {
            break;
        } else {
            std::cerr << "无效命令，请重新输入" << std::endl;
        }
    }
    return 0;
}
        