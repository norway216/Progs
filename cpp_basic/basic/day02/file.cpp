// 文件操作相关
#include <fmt/core.h>
#include <fmt/printf.h>
#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <fstream>

// 1. 文件写入操作
void write_file (const std::string& file_name, std::vector<int>& nums) {
    std::fstream osm;
    osm.open(file_name, std::ios::out | std::ios::app);
    if (!osm.is_open()) {
        fmt::print("错误：不能打开文件...\n");
        return;
    }
    for (int i = 0; i < nums.size(); ++i) {
        osm << nums[i] << "\n";
    }
    osm.close();
    fmt::print("成功写入文件...\n");
}

// 2. 文件读取操作
void read_file(const std::string& file_name, std::vector<int>& nums) {
    std::ifstream osm(file_name); // 使用 ifstream 专门读取
    if (!osm.is_open()) {
        fmt::print("错误：无法打开文件...\n");
        return;
    }

    std::string line;
    // 直接在循环条件中读取，这样读取失败或到末尾会立即停止
    while (std::getline(osm, line)) {
        // 跳过空行，防止 stoi 崩溃
        if (line.empty()) continue; 
        
        try {
            nums.push_back(std::stoi(line));
        } catch (const std::exception& e) {
            fmt::print("转换错误: {} 内容: '{}'\n", e.what(), line);
        }
    }
    osm.close();

    // 打印结果 (注意 fmt::print 索引从 0 开始或直接打印)
    for (const auto& n : nums) {
        fmt::print("{} ", n);
    }
    fmt::print("\n");
}

int main () {
    std::string file_name {"data.dat"};
    std::vector<int> nums {1, 2, 3, 4,5};
    // write_file (file_name, nums);
    std::vector<int> data;
    read_file (file_name, data);

    return 0;
}