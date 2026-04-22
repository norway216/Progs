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

// 6. 指针和数组：其实数组就是一段连续的内存空间，char是单字节，int就可以看成是char的数组，
// 可以这么理解，内存就是一块大树组
void f6 (int* arr, int len) {
    int sum = 0;
    for (int i = 0; i < len; ++i) {
        sum += arr[i];
    }
    fmt::print("sum: {0}\n", sum);
}


int main () {
    int a = 9;
    int* p = &a;
    f1 (p);
    f2 (&p);
    f3 (p);
    f4 (p);
    f5 (&a);

    int arr[] = {1, 2, 3,4 ,5};
    f6 (arr, 5);

    return 0;
}