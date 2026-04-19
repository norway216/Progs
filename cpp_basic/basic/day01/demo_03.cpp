// 指针和引用的使用 指针（地址）
#include <iostream>
#include <string>
#include <memory>
#include <fmt/core.h>
#include <fmt/printf.h>
#include <array>
#include <vector>

// 1. 基本变量和指针
void func1 (bool flag) {
    if (flag) {
        int a, b, c;
        long la;
        a = 9, b = 6, c = 3;
        la = 12;

        char* p = (char*)&a;
        fmt::print("address of a: {0}, the value of a: {1}\n", fmt::ptr(p), *(int*)p);
        fmt::print("address of b: {0}, the value of b: {1}\n", fmt::ptr(&b), b);
        p = (char*)&la;
        fmt::print("address of la: {0}, value of la: {1}\n", fmt::ptr(p), *(int*)p);
        long address = (long)&la;
        fmt::print("address of la: {0}, value of la: {1}\n", fmt::ptr(&la), *(int*)address);

    }
    else {
        float f1;
        double d1;
        f1 = 19.3f;
        d1 = 92.5;
    }
}

int main () {
    bool flag = true;
    func1(flag);

    return 0;
}