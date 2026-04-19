// C++ 基础知识：变量，名称空间，运算符，枚举，结构体，结构体，条件语句，函数等
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <fmt/core.h>
#include <fmt/printf.h>

// 枚举类型
enum Type {
    B = 0,
    C = 1,
    PW = 2,
    CW = 3,
    M = 4
};
// 强枚举类型 enum class
enum class EType : int {
    B = 0,
    C = 1,
    PW = 2,
    CW = 3,
    M = 4
};

// 测试函数
void func1 (const std::string& str) {
    fmt::print("string: {0}\n", str);
    int a, b, c;
    long d;
    std::cin >> a >> b >> c;
    std::cin >> d;
    fmt::print("a: {0}, b: {1}, c: {2}\n", a, b, c);
    fmt::print("d: {0}\n", d);

    Type type(Type::CW);
    fmt::print("Type: {0}\n", type);

    EType etype(EType::PW);
    fmt::print("EType: {0}\n", static_cast<int>(etype));
}

int main () {

    std::string str{"Google"};

    func1(str);

    return 0;
}