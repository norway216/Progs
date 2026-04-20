#include <fmt/core.h>
#include <fmt/printf.h>
#include <string>
#include <memory>

struct Point {
    double x, y, z;
};

// 使用智能指针
void f (const std::unique_ptr<Point>& p) {
    fmt::print("f -> {0}, {1}, {2}, {3}\n", fmt::ptr(p.get()), p->x, p->y, p->z);
    Point* ptr = p.get();
    fmt::print("ptr: {0}, {1}, {2}\n", ptr->x, ptr->y, ptr->z);

    Point p1{3, 6, 9}, p2 {2,4,6};
    std::unique_ptr<Point> ptr1 = std::make_unique<Point>(p1);
    fmt::print("p1: {0}, {1}, {2}\n", ptr1->x, ptr1->y, ptr1->z);
    ptr1.reset(new Point{1,2,6});
    fmt::print("p1: {0}, {1}, {2}\n", ptr1->x, ptr1->y, ptr1->z);
}

int main () {
    std::unique_ptr<Point> p = std::make_unique<Point>(Point{1, 2, 3});
    f(p);

    return 0;
}