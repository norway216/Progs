#include <iostream>
#include <fmt/printf.h>
#include <fmt/core.h>
#include <string>

// 创建一个类
template <typename T>
class Point {
public:
    // 无参构造函数
    Point () 
        : m_x {0} {
        fmt::print("Point<T>::Point -> 调用无参数构造函数...\n");
    }
    // 含参数构造函数
    Point (const T& x)
        : m_x {x} {
        fmt::print("Point<T>::Point(T) -> 调用含参数构造函数...\n");
    }
    // 拷贝构造函数
    Point (const Point<T>& p)
        : m_x {p.m_x} {
        fmt::print("Point<T>::Point(Point<T>&) -> 调用拷贝构造函数...\n");
    }
    // // 另外一种写法 这两种写法有什么区别 这两种效果一样
    // Point (const Point& p)
    //     : m_x {p.m_x} {
    //     fmt::print("Point<T>::Point(Point&) -> 调用拷贝构造函数...\n");
    // }
    // 析构函数
    ~Point () {
        fmt::print("调用析构函数...\n");
    }
public:
    virtual void print () {
        fmt::print("Point<T>::print -> x: {0}\n", m_x);
    }

protected:
    T m_x;
};

// 实现2D点
template <typename T>
class Point2D : public Point<T> {
public:
    Point2D () 
        : m_y {0}, Point<T>(0) {
        fmt::print("Point2D<T>::Point2D 无参数构造函数...\n");
    }

    Point2D (const T& y)
        : m_y {y}, Point<T>{0} {
        fmt::print("Point2D<T>::Point2D(T&) 含参数构造函数...\n");
    }
    Point2D (const T& x, const T& y)
        : m_y {y}, Point<T>{x} {
        fmt::print("Point2D<T>::Point2D(T&, T&) 含参数构造函数...\n");
    }
    
    Point2D (const Point2D<T>& p)
        : m_y {p.m_y}, Point<T>{p.m_x} {
        fmt::print("Point2D<T>::Point2D(Point2D&) 拷贝构造函数...\n");
    }

    ~Point2D () {
        fmt::print("Point2D<T>::~Point2D 析构函数...\n");
    }
public:
    void print () {
        fmt::print("Point2D<T>::print -> x: {0}, y: {1}\n", this->m_x, m_y);
    }
protected:
    T m_y;
};

int main () {
    Point<int> p;
    p.print();

    Point<int> p1{10};
    p1.print();

    Point<int> p2{p1};
    p2.print();

    Point2D<int> p3{1, 10};
    p3.print();

    Point2D<int> p4(p3);
    p4.print();

    return 0;
}