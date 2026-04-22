// 指针和引用的基本使用
#include <fmt/core.h>
#include <fmt/printf.h>
#include <iostream>
#include <vector>
#include <string>

// 1. 函数参数为指针
void f1 (int* p) {
    fmt::print("address: {0}, value: {1}\n", fmt::ptr(p), *p);
}

// 2. 函数参数为二级指针
void f2 (int** p) {
    fmt::print("address: {0}, value: {1}\n", fmt::ptr(p), fmt::ptr(*p));
    fmt::print("address: {0}, value: {1}\n", fmt::ptr(*p), **p);
}

// 3. 函数参数为指针的引用
void f3 (int*& p) {  // 和直接使用指针没有太大的区别
    fmt::print("address: {0}, value: {1}\n", fmt::ptr(p), *p);
}

// 4. 函数参数为指针的const引用  和直接使用指针没有太大的区别
void f4 (int* const& p) {
    fmt::print("address: {0}, value: {1}\n", fmt::ptr(p), *p);
}

// 5. 函数参数为指针的右值引用
void f5 (int* && p) {
    fmt::print("address: {0}, value: {1}\n", fmt::ptr(p), *p);
}


int main () {
    int a = 9;
    int* p = &a;
    f1 (p);
    f2 (&p);
    f3 (p);
    f4 (p);
    f5 (&a);

    return 0;
}