// C++基础知识之函数的使用
#include <fmt/core.h>
#include <fmt/printf.h>
#include <string>
#include <vector>
#include <string_view>

// 1. 创建一个函数
void f (const std::string& str) {
    fmt::print("string: {0}\n", str);
}

void f_sv (std::string_view& sv) {
    sv.remove_suffix(2);
    fmt::print("string_view: {0}\n", sv);
}

int main () {
    // 调用函数
    std::string s {"hello, c++!"};
    std::string_view sv {"hello, c++!!!"};
    f_sv (sv);
    f (s);

    return 0;
}