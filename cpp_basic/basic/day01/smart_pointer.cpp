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
    // 释放资源
    ptr1.release();
}

struct BufferHeader {
    int head_size;
    int current_size;
    int capacity;
};

// 设计一个内存管理的函数
void f1 (char* ptr, const size_t buffer_size) {
    if (buffer_size <= 0) {
        fmt::print("错误：函数参数不为负数...\n");
        return;
    }
    ptr = new char[buffer_size];
    memset(ptr, 0, buffer_size);

    BufferHeader bh{(int)sizeof(BufferHeader), 0, (int)(buffer_size - (unsigned long)sizeof(BufferHeader))};
    memcpy(ptr, &bh, sizeof(BufferHeader));

    BufferHeader* bh_ptr = (BufferHeader*)ptr;
    fmt::print("test: bufferheader size: {0}, current size: {1}, capacity: {2}\n",
        bh_ptr->head_size, bh_ptr->current_size, bh_ptr->capacity);
}

int main () {
    std::unique_ptr<Point> p = std::make_unique<Point>(Point{1, 2, 3});
    f(p);

    char* buffer = nullptr;
    f1 (buffer, 1024);

    if (buffer != nullptr) {
        delete[] buffer;
        buffer = nullptr;
    }

    return 0;
}