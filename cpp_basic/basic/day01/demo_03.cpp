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

// 2. 结构体和指针
struct Point {
    double x, y, z;
};

void func2 (Point* p) {
    if (p == nullptr) {
        return;
    }
    *p = {10, 12, 14};
    fmt::print("Point: x {0}, y {1}, z {2}\n", p->x, p->y, p->z);
    fmt::print("address: {0}\n", fmt::ptr(p));
}
// 3. 函数和指针
typedef void(*func_ptr)(Point* );
void f (Point* p) {
    fmt::print("Point: ( {0}, {1}, {2})\n", p->x, p->y, p->z);
}
void func3 () {
    // 测试函数和函数指针
    func_ptr fp = f;
    Point p{1, 2, 3};
    fp(&p);
}

int main () {
    bool flag = true;
    func1(flag);

    Point p;
    func2(&p);

    func3();

    return 0;
}