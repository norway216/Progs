// 时间库的使用
#include <fmt/core.h>
#include <fmt/printf.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>

// 测试一个排序函数的时间消耗
template <typename T>
void f (std::vector<T>& data) {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    std::sort(data.begin(), data.end());
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    fmt::print("time: {0}\n", std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());
}

int main () {
    std::vector<int> data;
    for (int  i = 0; i < 10000; ++i) {
        if (i & 2 == 0) {
            data.push_back(i * 12);
        }
        else {
            data.push_back(i * (-3));
        }
    }

    f<int> (data);

    return 0;
}