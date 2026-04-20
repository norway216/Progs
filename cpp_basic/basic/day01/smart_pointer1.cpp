// 智能指针的基本用法
#include <fmt/core.h>
#include <fmt/printf.h>
#include <iostream>
#include <vector>
#include <string>
#include <memory>

// 定义点结构
template <typename T>
struct Point2D {
    T x, y;
};

// 1. 智能指针的基本用法
template <typename T>
void f1 (std::unique_ptr<T>& ptr) {
    fmt::print("ptr: {0}, {1}\n", ptr->x, ptr->y);
}

// 2. 指针指向另外一个对象
template <typename T>
void f2 (std::unique_ptr<T>& ptr) {
    fmt::print("ptr: {0}, {1}\n", ptr->x, ptr->y);
    ptr.reset(new Point2D<int>{5, 6});
    fmt::print("ptr reset: {0}, {1}\n", ptr->x, ptr->y);
}

struct Addr {
    Point2D<int>* p;
};

// 3. 指针指向一个地址，使用模板怎么实现
template <typename ADDR>
void f3 (std::unique_ptr<ADDR>& ptr) {
    fmt::print("ptr: {0}\n", fmt::ptr(ptr));
    Point2D<int> p1{6, 9};
    ptr.reset(new Addr{&p1});
    fmt::print("addr: {0}, x: {1}, y: {2}\n",
        fmt::ptr(ptr), ptr->p->x, ptr->p->y);
}

int main () {
    Point2D<int> p{1, 2};
    // 使用智能指针托管p
    std::unique_ptr<Point2D<int>> ptr = std::make_unique<Point2D<int>>(p);
    std::unique_ptr<Point2D<int>> ptr1{new Point2D<int>{2, 5}};
    f1 (ptr);
    f1 (ptr1);

    f2 (ptr);

    std::unique_ptr<Addr> p_addr {new Addr {&p}};
    f3 (p_addr);

    return 0;
}