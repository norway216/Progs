#include <iostream>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include <sys/mman.h>
#include <unistd.h>

// ============================================================
// Linux 高性能内存池
//
// 设计目标：
// 1. 只支持 Linux
// 2. 使用 mmap 向系统申请内存页
// 3. 支持不同大小的内存页
// 4. 按申请大小进行分桶
// 5. 申请内存时优先查找相同 size 的空闲块
// 6. 如果没有相同 size 的空闲块，则向系统申请新页
// 7. 用户释放内存后，不还给系统，而是放回内存池
// 8. 用户释放时不需要传 size
// 9. 支持 create<T>() / destroy<T>()
//
// 适用场景：
// - 超声图像帧 buffer
// - 高频小块内存申请
// - 图像算法临时缓存
// - 网络包缓存
// - 消息队列节点缓存
// ============================================================

class LinuxMemoryPool
{
private:
    static constexpr std::size_t ALIGNMENT = 16;
    static constexpr std::uint64_t BLOCK_MAGIC = 0x20260508CAFEBABEULL;

private:
    struct BlockHeader
    {
        std::uint64_t magic;
        std::size_t requestedSize;
        std::size_t blockSize;
    };

    struct FreeNode
    {
        FreeNode* next;
    };

    struct PageInfo
    {
        void* pageAddress;
        std::size_t pageSize;
        std::size_t blockSize;
        std::size_t blockCount;
    };

    struct Bucket
    {
        FreeNode* freeList = nullptr;
        std::size_t freeCount = 0;
        std::size_t totalCount = 0;
        std::mutex mutex;
    };

private:
    std::unordered_map<std::size_t, Bucket*> buckets_;
    std::vector<PageInfo> pages_;

    std::mutex bucketMapMutex_;
    std::mutex pageMutex_;

private:
    LinuxMemoryPool() = default;

    ~LinuxMemoryPool()
    {
        releaseAllPages();
        releaseAllBuckets();
    }

    LinuxMemoryPool(const LinuxMemoryPool&) = delete;
    LinuxMemoryPool& operator=(const LinuxMemoryPool&) = delete;

public:
    static LinuxMemoryPool& instance()
    {
        static LinuxMemoryPool pool;
        return pool;
    }

public:
    void* allocate(std::size_t size)
    {
        if (size == 0) {
            size = 1;
        }

        const std::size_t blockSize = alignUp(size);
        Bucket* bucket = getOrCreateBucket(blockSize);

        {
            std::lock_guard<std::mutex> lock(bucket->mutex);

            if (bucket->freeList != nullptr) {
                FreeNode* node = bucket->freeList;
                bucket->freeList = node->next;
                bucket->freeCount--;

                BlockHeader* header = getHeaderFromUserPointer(node);
                assert(header->magic == BLOCK_MAGIC);
                assert(header->blockSize == blockSize);

                header->requestedSize = size;

                return static_cast<void*>(node);
            }
        }

        allocateNewPage(blockSize);

        {
            std::lock_guard<std::mutex> lock(bucket->mutex);

            if (bucket->freeList != nullptr) {
                FreeNode* node = bucket->freeList;
                bucket->freeList = node->next;
                bucket->freeCount--;

                BlockHeader* header = getHeaderFromUserPointer(node);
                assert(header->magic == BLOCK_MAGIC);
                assert(header->blockSize == blockSize);

                header->requestedSize = size;

                return static_cast<void*>(node);
            }
        }

        throw std::bad_alloc();
    }

    void deallocate(void* ptr)
    {
        if (ptr == nullptr) {
            return;
        }

        BlockHeader* header = getHeaderFromUserPointer(ptr);

        if (header->magic != BLOCK_MAGIC) {
            std::cerr << "[MemoryPool Error] Invalid pointer or memory corruption detected."
                      << std::endl;
            std::abort();
        }

        const std::size_t blockSize = header->blockSize;
        Bucket* bucket = getOrCreateBucket(blockSize);

        FreeNode* node = static_cast<FreeNode*>(ptr);

        {
            std::lock_guard<std::mutex> lock(bucket->mutex);
            node->next = bucket->freeList;
            bucket->freeList = node;
            bucket->freeCount++;
        }
    }

    template <typename T, typename... Args>
    T* create(Args&&... args)
    {
        void* mem = allocate(sizeof(T));

        try {
            return new (mem) T(std::forward<Args>(args)...);
        } catch (...) {
            deallocate(mem);
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
        deallocate(obj);
    }

public:
    void printStats()
    {
        std::lock_guard<std::mutex> mapLock(bucketMapMutex_);
        std::lock_guard<std::mutex> pageLock(pageMutex_);

        std::cout << "\n========== Memory Pool Stats ==========" << std::endl;

        std::size_t totalPageBytes = 0;
        for (const auto& page : pages_) {
            totalPageBytes += page.pageSize;
        }

        std::cout << "Page count      : " << pages_.size() << std::endl;
        std::cout << "Total page bytes: " << totalPageBytes << std::endl;
        std::cout << "Bucket count    : " << buckets_.size() << std::endl;

        for (const auto& kv : buckets_) {
            std::size_t blockSize = kv.first;
            Bucket* bucket = kv.second;

            std::lock_guard<std::mutex> bucketLock(bucket->mutex);

            std::cout << "BlockSize = " << blockSize
                      << " bytes, total = " << bucket->totalCount
                      << ", free = " << bucket->freeCount
                      << ", used = " << bucket->totalCount - bucket->freeCount
                      << std::endl;
        }

        std::cout << "=======================================\n" << std::endl;
    }

private:
    static std::size_t alignUp(std::size_t size)
    {
        return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    }

    static std::size_t alignUpToPage(std::size_t size)
    {
        const std::size_t pageSize = static_cast<std::size_t>(::getpagesize());
        return (size + pageSize - 1) & ~(pageSize - 1);
    }

    static BlockHeader* getHeaderFromUserPointer(void* userPtr)
    {
        return reinterpret_cast<BlockHeader*>(
            static_cast<char*>(userPtr) - sizeof(BlockHeader)
        );
    }

    static void* getUserPointerFromHeader(BlockHeader* header)
    {
        return reinterpret_cast<void*>(
            reinterpret_cast<char*>(header) + sizeof(BlockHeader)
        );
    }

private:
    Bucket* getOrCreateBucket(std::size_t blockSize)
    {
        {
            std::lock_guard<std::mutex> lock(bucketMapMutex_);

            auto it = buckets_.find(blockSize);
            if (it != buckets_.end()) {
                return it->second;
            }

            Bucket* bucket = new Bucket();
            buckets_[blockSize] = bucket;
            return bucket;
        }
    }

    std::size_t choosePageSize(std::size_t blockSize)
    {
        const std::size_t unitSize = sizeof(BlockHeader) + blockSize;

        const std::size_t systemPageSize = static_cast<std::size_t>(::getpagesize());

        std::size_t targetBlockCount = 0;

        if (blockSize <= 64) {
            targetBlockCount = 1024;
        } else if (blockSize <= 256) {
            targetBlockCount = 512;
        } else if (blockSize <= 1024) {
            targetBlockCount = 256;
        } else if (blockSize <= 4096) {
            targetBlockCount = 128;
        } else if (blockSize <= 64 * 1024) {
            targetBlockCount = 32;
        } else if (blockSize <= 1024 * 1024) {
            targetBlockCount = 4;
        } else {
            targetBlockCount = 1;
        }

        std::size_t rawPageSize = unitSize * targetBlockCount;

        if (rawPageSize < systemPageSize) {
            rawPageSize = systemPageSize;
        }

        return alignUpToPage(rawPageSize);
    }

    void allocateNewPage(std::size_t blockSize)
    {
        Bucket* bucket = getOrCreateBucket(blockSize);

        const std::size_t unitSize = sizeof(BlockHeader) + blockSize;
        const std::size_t pageSize = choosePageSize(blockSize);
        const std::size_t blockCount = pageSize / unitSize;

        void* page = ::mmap(
            nullptr,
            pageSize,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS,
            -1,
            0
        );

        if (page == MAP_FAILED) {
            throw std::bad_alloc();
        }

        {
            std::lock_guard<std::mutex> pageLock(pageMutex_);

            pages_.push_back(PageInfo{
                page,
                pageSize,
                blockSize,
                blockCount
            });
        }

        char* current = static_cast<char*>(page);

        {
            std::lock_guard<std::mutex> bucketLock(bucket->mutex);

            for (std::size_t i = 0; i < blockCount; ++i) {
                BlockHeader* header = reinterpret_cast<BlockHeader*>(current);

                header->magic = BLOCK_MAGIC;
                header->requestedSize = blockSize;
                header->blockSize = blockSize;

                void* userPtr = getUserPointerFromHeader(header);

                FreeNode* node = static_cast<FreeNode*>(userPtr);
                node->next = bucket->freeList;
                bucket->freeList = node;

                bucket->freeCount++;
                bucket->totalCount++;

                current += unitSize;
            }
        }
    }

private:
    void releaseAllPages()
    {
        std::lock_guard<std::mutex> lock(pageMutex_);

        for (const auto& page : pages_) {
            ::munmap(page.pageAddress, page.pageSize);
        }

        pages_.clear();
    }

    void releaseAllBuckets()
    {
        std::lock_guard<std::mutex> lock(bucketMapMutex_);

        for (auto& kv : buckets_) {
            delete kv.second;
        }

        buckets_.clear();
    }
};

// ============================================================
// 测试对象
// ============================================================

struct ImageFrame
{
    int frameId;
    int width;
    int height;
    char name[64];

    ImageFrame(int id, int w, int h)
        : frameId(id), width(w), height(h)
    {
        std::snprintf(name, sizeof(name), "Frame-%d", id);
    }

    void print() const
    {
        std::cout << "ImageFrame: "
                  << "id=" << frameId
                  << ", width=" << width
                  << ", height=" << height
                  << ", name=" << name
                  << std::endl;
    }
};

// ============================================================
// 功能测试
// ============================================================

void functionalTest()
{
    std::cout << "========== Functional Test ==========" << std::endl;

    auto& pool = LinuxMemoryPool::instance();

    void* p1 = pool.allocate(128);
    void* p2 = pool.allocate(128);
    void* p3 = pool.allocate(1024);

    std::memset(p1, 0x11, 128);
    std::memset(p2, 0x22, 128);
    std::memset(p3, 0x33, 1024);

    std::cout << "p1 = " << p1 << std::endl;
    std::cout << "p2 = " << p2 << std::endl;
    std::cout << "p3 = " << p3 << std::endl;

    pool.deallocate(p1);
    pool.deallocate(p2);
    pool.deallocate(p3);

    void* p4 = pool.allocate(128);
    std::cout << "p4 = " << p4 << "  reuse 128-byte block" << std::endl;
    pool.deallocate(p4);

    ImageFrame* frame = pool.create<ImageFrame>(1, 640, 480);
    frame->print();
    pool.destroy(frame);

    pool.printStats();
}

// ============================================================
// 模拟超声图像 buffer 申请
// ============================================================

void ultrasoundBufferTest()
{
    std::cout << "========== Ultrasound Buffer Test ==========" << std::endl;

    auto& pool = LinuxMemoryPool::instance();

    constexpr std::size_t FRAME_COUNT = 16;
    constexpr std::size_t RF_BUFFER_SIZE = 1024 * 1024;
    constexpr std::size_t IMAGE_BUFFER_SIZE = 640 * 480;

    std::vector<void*> rfBuffers;
    std::vector<void*> imageBuffers;

    for (std::size_t i = 0; i < FRAME_COUNT; ++i) {
        void* rf = pool.allocate(RF_BUFFER_SIZE);
        void* img = pool.allocate(IMAGE_BUFFER_SIZE);

        rfBuffers.push_back(rf);
        imageBuffers.push_back(img);
    }

    for (void* p : rfBuffers) {
        pool.deallocate(p);
    }

    for (void* p : imageBuffers) {
        pool.deallocate(p);
    }

    pool.printStats();
}

// ============================================================
// 性能测试：内存池 vs malloc/free
// ============================================================

void benchmarkMemoryPool()
{
    std::cout << "========== Benchmark MemoryPool ==========" << std::endl;

    constexpr std::size_t N = 1'000'000;
    constexpr std::size_t SIZE = 256;

    auto& pool = LinuxMemoryPool::instance();

    std::vector<void*> ptrs;
    ptrs.reserve(N);

    auto begin = std::chrono::high_resolution_clock::now();

    for (std::size_t i = 0; i < N; ++i) {
        ptrs.push_back(pool.allocate(SIZE));
    }

    for (void* p : ptrs) {
        pool.deallocate(p);
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

    std::cout << "MemoryPool allocate/deallocate "
              << N << " blocks, size = "
              << SIZE << ", cost = "
              << cost << " ms"
              << std::endl;
}

void benchmarkMalloc()
{
    std::cout << "========== Benchmark malloc/free ==========" << std::endl;

    constexpr std::size_t N = 1'000'000;
    constexpr std::size_t SIZE = 256;

    std::vector<void*> ptrs;
    ptrs.reserve(N);

    auto begin = std::chrono::high_resolution_clock::now();

    for (std::size_t i = 0; i < N; ++i) {
        ptrs.push_back(std::malloc(SIZE));
    }

    for (void* p : ptrs) {
        std::free(p);
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

    std::cout << "malloc/free allocate/deallocate "
              << N << " blocks, size = "
              << SIZE << ", cost = "
              << cost << " ms"
              << std::endl;
}

// ============================================================
// 多线程测试
// ============================================================

void multiThreadTest()
{
    std::cout << "========== Multi Thread Test ==========" << std::endl;

    constexpr std::size_t THREAD_COUNT = 8;
    constexpr std::size_t LOOP_COUNT = 200000;
    constexpr std::size_t SIZE = 512;

    auto& pool = LinuxMemoryPool::instance();

    auto begin = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    std::atomic<std::size_t> doneCount{0};

    for (std::size_t t = 0; t < THREAD_COUNT; ++t) {
        threads.emplace_back([&pool, &doneCount]() {
            std::vector<void*> ptrs;
            ptrs.reserve(LOOP_COUNT);

            for (std::size_t i = 0; i < LOOP_COUNT; ++i) {
                void* p = pool.allocate(SIZE);
                ptrs.push_back(p);
            }

            for (void* p : ptrs) {
                pool.deallocate(p);
            }

            doneCount++;
        });
    }

    for (auto& th : threads) {
        th.join();
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

    std::cout << "MultiThread MemoryPool "
              << THREAD_COUNT * LOOP_COUNT
              << " blocks, size = "
              << SIZE
              << ", cost = "
              << cost << " ms"
              << std::endl;

    pool.printStats();
}

// ============================================================
// main
// ============================================================

int main()
{
    functionalTest();

    ultrasoundBufferTest();

    benchmarkMemoryPool();

    benchmarkMalloc();

    multiThreadTest();

    std::cout << "Done." << std::endl;

    return 0;
}