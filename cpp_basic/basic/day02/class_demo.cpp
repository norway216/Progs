// 创建一个类
/* 
数组
指针
引用
结构体
动态内存
*/
#include <fmt/core.h>
#include <fmt/printf.h>
#include <iostream>
#include <string>
#include <vector>

// 数组，指针，引用，指针的引用
void f (int arr[], int len) {
    int sum = 0;
    for (int i = 0; i < len; ++i) {
        sum += arr[i];
    }
    fmt::print("sum = {0}\n", sum);
}

void f1 (int* arr, int len) {
    int sum = 0;
    for (int i = 0; i < len; ++i) {
        sum += arr[i];
    }
    fmt::print("sum = {0}\n", sum);
}

void f2 (int*&& p) {
    // p 仅仅是一个指针  *&& 指针的右值引用
    fmt::print("addres of p: {0}, value is: {1}\n", fmt::ptr(p), *p);
    int a = 90;
    p = &a;
    fmt::print("address of p: {0}, value is: {1}\n", fmt::ptr(p), *p);
}

// 指针的引用：
// 1. 指针的左值引用，指的是指针变量的引用；
// 2. 另外一种是指针的右值引用，可以是某个变量的地址的直接引用
void f3 (int*& p) {
    fmt::print("address: {0}, value: {1}\n", fmt::ptr(p), *p);
    int* b = new int(3);
    p = b;
    fmt::print("address: {0}, value: {1}\n", fmt::ptr(p), *p);
    *p = 999;
    fmt::print("address: {0}, value: {1}\n", fmt::ptr(p), *p);
}

// const引用，可以接收右值类型  const修饰的是&引用
void f4 ( int* const & p) {
    fmt::print("address: {0}, value: {1}\n", fmt::ptr(p), *p);
}

int main () {
    int arr[] = {3, 6, 7, 2, 9};
    f (arr, sizeof(arr)/sizeof(arr[0]));
    f1 (arr, sizeof(arr)/sizeof(arr[0]));
    int a = 8;
    f2 (&a);
    int* p = &a;
    f3 (p);
    f4(&a);

    return 0;
}