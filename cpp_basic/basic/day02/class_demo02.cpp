// 使用C++的类，模板技术，实现一个简单的点类
#include <fmt/core.h>
#include <fmt/printf.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

// 1. 定义一个点类
template <typename T>
class Point {
public:
    // 实现无参构造函数
    Point ()
        : m_x {0} {
            fmt::print("无参数构造函数...\n");
    }

    // 实现含参数构造函数
    Point (const T& x)
        : m_x {x} {
            fmt::print("含参数构造函数...\n");
    }

    // 实现拷贝构造函数
    Point (const Point<T>& p)
        : m_x {p.m_x} {
            fmt::print("拷贝构造函数...\n");
    }

    // 实现赋值运算符函数
    Point<T>& operator== (const Point<T>& p) {
        if (this == &p) {
            return *this;
        }
        m_x = p.m_x;
        return *this;
    }

    // 实现析构函数
    ~Point () {
        fmt::print("析构函数...\n");
    }
public:
    // 打印当前成员
    virtual void print () {
        fmt::print("Point<T>::print -> x: {0}\n", m_x);
    }
protected:
    T m_x;
};

int main () {
    Point<int> p{4};
    p.print();

    return 0;
}