#include <iostream>
#include <vector>
#include <array>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <cassert>
#include <new>
#include <cstring>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/mman.h>
    #include <unistd.h>
#endif

// ============================================================
// 高性能内存池：单文件版本
// 特点：
// 1. 支持小对象快速分配
// 2. 使用 size class 分级管理
// 3. 使用 thread_local 线程本地缓存减少锁竞争
// 4. 大块内存直接走系统 malloc/free
// 5. 支持 create<T>() / destroy<T>()
// 6. 支持 STL allocator
// ============================================================

class MemoryPool
{
public:
    static constexpr std::size_t ALIGNMENT = 8;
    static constexpr std::size_t MAX_SMALL_SIZE = 1024;
    static constexpr std::size_t NUM_SIZE_CLASSES = MAX_SMALL_SIZE / ALIGNMENT;
    static constexpr std::size_t PAGE_SIZE = 64 * 1024;
    static constexpr std::size_t LOCAL_CACHE_LIMIT = 64;

private:
    struct FreeNode
    {
        FreeNode* next;
    };

    struct CentralFreeList
    {
        FreeNode* head = nullptr;
        std::mutex mtx;
    };

    struct ThreadLocalFreeList
    {
        FreeNode* head = nullptr;
        std::size_t count = 0;
    };

    struct PageBlock
    {
        void* memory = nullptr;
        std::size_t size = 0;
        PageBlock* next = nullptr;
    };

private:
    std::array<CentralFreeList, NUM_SIZE_CLASSES> centralLists_;
    std::mutex pageMutex_;
    PageBlock* pageBlocks_ = nullptr;

private:
    MemoryPool() = default;

    ~MemoryPool()
    {
        releaseAllPages();
    }

    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

public:
    static MemoryPool& instance()
    {
        static MemoryPool pool;
        return pool;
    }

public:
    void* allocate(std::size_t size)
    {
        if (size == 0) {
            size = 1;
        }

        if (size > MAX_SMALL_SIZE) {
            return ::operator new(size);
        }

        const std::size_t index = sizeToIndex(size);
        auto& localList = getThreadLocalList(index);

        if (localList.head != nullptr) {
            FreeNode* node = localList.head;
            localList.head = node->next;
            --localList.count;
            return node;
        }

        refillLocalCache(index);

        if (localList.head != nullptr) {
            FreeNode* node = localList.head;
            localList.head = node->next;
            --localList.count;
            return node;
        }

        throw std::bad_alloc();
    }

    void deallocate(void* ptr, std::size_t size)
    {
        if (ptr == nullptr) {
            return;
        }

        if (size == 0) {
            size = 1;
        }

        if (size > MAX_SMALL_SIZE) {
            ::operator delete(ptr);
            return;
        }

        const std::size_t index = sizeToIndex(size);
        auto& localList = getThreadLocalList(index);

        FreeNode* node = static_cast<FreeNode*>(ptr);
        node->next = localList.head;
        localList.head = node;
        ++localList.count;

        if (localList.count > LOCAL_CACHE_LIMIT) {
            returnLocalCacheToCentral(index);
        }
    }

    template <typename T, typename... Args>
    T* create(Args&&... args)
    {
        void* mem = allocate(sizeof(T));

        try {
            return new (mem) T(std::forward<Args>(args)...);
        } catch (...) {
            deallocate(mem, sizeof(T));
            throw;
        }
    }

    template <typename T>
    void destroy(T* obj)
    {
        if (obj == nullptr) {
            return;
        }

        obj->~T();
        deallocate(obj, sizeof(T));
    }

private:
    static std::size_t alignUp(std::size_t size)
    {
        return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    }

    static std::size_t sizeToIndex(std::size_t size)
    {
        const std::size_t aligned = alignUp(size);
        return aligned / ALIGNMENT - 1;
    }

    static std::size_t indexToSize(std::size_t index)
    {
        return (index + 1) * ALIGNMENT;
    }

private:
    static ThreadLocalFreeList& getThreadLocalList(std::size_t index)
    {
        thread_local std::array<ThreadLocalFreeList, NUM_SIZE_CLASSES> localLists;
        return localLists[index];
    }

private:
    void refillLocalCache(std::size_t index)
    {
        constexpr std::size_t batchCount = 32;

        auto& central = centralLists_[index];
        auto& local = getThreadLocalList(index);

        {
            std::lock_guard<std::mutex> lock(central.mtx);

            std::size_t count = 0;

            while (central.head != nullptr && count < batchCount) {
                FreeNode* node = central.head;
                central.head = node->next;

                node->next = local.head;
                local.head = node;
                ++local.count;
                ++count;
            }

            if (count > 0) {
                return;
            }
        }

        allocateNewPage(index);

        {
            std::lock_guard<std::mutex> lock(central.mtx);

            std::size_t count = 0;

            while (central.head != nullptr && count < batchCount) {
                FreeNode* node = central.head;
                central.head = node->next;

                node->next = local.head;
                local.head = node;
                ++local.count;
                ++count;
            }
        }
    }

    void returnLocalCacheToCentral(std::size_t index)
    {
        constexpr std::size_t returnCount = 32;

        auto& local = getThreadLocalList(index);
        auto& central = centralLists_[index];

        std::lock_guard<std::mutex> lock(central.mtx);

        std::size_t count = 0;

        while (local.head != nullptr && count < returnCount) {
            FreeNode* node = local.head;
            local.head = node->next;
            --local.count;

            node->next = central.head;
            central.head = node;

            ++count;
        }
    }

    void allocateNewPage(std::size_t index)
    {
        const std::size_t blockSize = indexToSize(index);
        const std::size_t blocksPerPage = PAGE_SIZE / blockSize;

        void* page = systemAlloc(PAGE_SIZE);
        if (page == nullptr) {
            throw std::bad_alloc();
        }

        {
            std::lock_guard<std::mutex> lock(pageMutex_);

            PageBlock* block = static_cast<PageBlock*>(std::malloc(sizeof(PageBlock)));
            if (!block) {
                systemFree(page, PAGE_SIZE);
                throw std::bad_alloc();
            }

            block->memory = page;
            block->size = PAGE_SIZE;
            block->next = pageBlocks_;
            pageBlocks_ = block;
        }

        auto& central = centralLists_[index];

        std::lock_guard<std::mutex> lock(central.mtx);

        char* start = static_cast<char*>(page);

        for (std::size_t i = 0; i < blocksPerPage; ++i) {
            FreeNode* node = reinterpret_cast<FreeNode*>(start + i * blockSize);
            node->next = central.head;
            central.head = node;
        }
    }

private:
    static void* systemAlloc(std::size_t size)
    {
#ifdef _WIN32
        return VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
        void* ptr = mmap(
            nullptr,
            size,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS,
            -1,
            0
        );

        if (ptr == MAP_FAILED) {
            return nullptr;
        }

        return ptr;
#endif
    }

    static void systemFree(void* ptr, std::size_t size)
    {
#ifdef _WIN32
        (void)size;
        VirtualFree(ptr, 0, MEM_RELEASE);
#else
        munmap(ptr, size);
#endif
    }

    void releaseAllPages()
    {
        PageBlock* block = pageBlocks_;

        while (block != nullptr) {
            PageBlock* next = block->next;

            systemFree(block->memory, block->size);
            std::free(block);

            block = next;
        }

        pageBlocks_ = nullptr;
    }
};

// ============================================================
// STL allocator 适配器
// 可用于 std::vector / std::list / std::map 等容器
// ============================================================

template <typename T>
class PoolAllocator
{
public:
    using value_type = T;

public:
    PoolAllocator() noexcept = default;

    template <typename U>
    PoolAllocator(const PoolAllocator<U>&) noexcept {}

    T* allocate(std::size_t n)
    {
        return static_cast<T*>(
            MemoryPool::instance().allocate(n * sizeof(T))
        );
    }

    void deallocate(T* ptr, std::size_t n) noexcept
    {
        MemoryPool::instance().deallocate(ptr, n * sizeof(T));
    }

    template <typename U>
    bool operator==(const PoolAllocator<U>&) const noexcept
    {
        return true;
    }

    template <typename U>
    bool operator!=(const PoolAllocator<U>&) const noexcept
    {
        return false;
    }
};

// ============================================================
// 测试结构体
// ============================================================

struct TestObject
{
    int id;
    double value;
    char name[32];

    TestObject(int i, double v)
        : id(i), value(v)
    {
        std::snprintf(name, sizeof(name), "Object-%d", i);
    }

    void print() const
    {
        std::cout << "id=" << id
                  << ", value=" << value
                  << ", name=" << name
                  << std::endl;
    }
};

// ============================================================
// 单线程性能测试
// ============================================================

void benchmarkSingleThread()
{
    constexpr std::size_t N = 1'000'000;

    std::vector<void*> ptrs;
    ptrs.reserve(N);

    auto start = std::chrono::high_resolution_clock::now();

    for (std::size_t i = 0; i < N; ++i) {
        void* p = MemoryPool::instance().allocate(64);
        ptrs.push_back(p);
    }

    for (void* p : ptrs) {
        MemoryPool::instance().deallocate(p, 64);
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "[MemoryPool SingleThread] "
              << N << " alloc/free cost: "
              << cost << " ms"
              << std::endl;
}

void benchmarkMallocSingleThread()
{
    constexpr std::size_t N = 1'000'000;

    std::vector<void*> ptrs;
    ptrs.reserve(N);

    auto start = std::chrono::high_resolution_clock::now();

    for (std::size_t i = 0; i < N; ++i) {
        void* p = ::operator new(64);
        ptrs.push_back(p);
    }

    for (void* p : ptrs) {
        ::operator delete(p);
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "[System new/delete SingleThread] "
              << N << " alloc/free cost: "
              << cost << " ms"
              << std::endl;
}

// ============================================================
// 多线程性能测试
// ============================================================

void benchmarkMultiThread()
{
    constexpr std::size_t THREAD_COUNT = 8;
    constexpr std::size_t ALLOC_PER_THREAD = 300'000;

    std::atomic<std::size_t> finished{0};

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;

    for (std::size_t t = 0; t < THREAD_COUNT; ++t) {
        threads.emplace_back([&]() {
            std::vector<void*> ptrs;
            ptrs.reserve(ALLOC_PER_THREAD);

            for (std::size_t i = 0; i < ALLOC_PER_THREAD; ++i) {
                void* p = MemoryPool::instance().allocate(64);
                ptrs.push_back(p);
            }

            for (void* p : ptrs) {
                MemoryPool::instance().deallocate(p, 64);
            }

            ++finished;
        });
    }

    for (auto& th : threads) {
        th.join();
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "[MemoryPool MultiThread] "
              << THREAD_COUNT * ALLOC_PER_THREAD
              << " alloc/free cost: "
              << cost << " ms"
              << std::endl;
}

// ============================================================
// 功能测试
// ============================================================

void functionalTest()
{
    std::cout << "========== Functional Test ==========" << std::endl;

    auto& pool = MemoryPool::instance();

    TestObject* obj = pool.create<TestObject>(1, 3.14159);
    obj->print();
    pool.destroy(obj);

    int* number = pool.create<int>(12345);
    std::cout << "number = " << *number << std::endl;
    pool.destroy(number);

    std::vector<int, PoolAllocator<int>> vec;

    for (int i = 0; i < 10; ++i) {
        vec.push_back(i * 10);
    }

    std::cout << "vector with PoolAllocator: ";

    for (int v : vec) {
        std::cout << v << " ";
    }

    std::cout << std::endl;
}

// ============================================================
// main
// ============================================================

int main()
{
    functionalTest();

    std::cout << "\n========== Benchmark ==========" << std::endl;

    benchmarkSingleThread();
    benchmarkMallocSingleThread();
    benchmarkMultiThread();

    std::cout << "\nDone." << std::endl;

    return 0;
}