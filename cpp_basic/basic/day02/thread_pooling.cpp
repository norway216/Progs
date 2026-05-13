#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <exception>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <chrono>

#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <sys/mman.h>
#include <unistd.h>

class LinuxMemoryPool
{
private:
    static constexpr std::size_t ALIGNMENT = 16;
    static constexpr std::uint64_t BLOCK_MAGIC = 0x20260508CAFEBABEULL;
    static constexpr std::uint64_t BLOCK_FREED = 0xDEADBEEFCAFEBABEULL;

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
        if (size == 0) size = 1;

        std::size_t blockSize = alignUp(size);
        Bucket* bucket = getOrCreateBucket(blockSize);

        // 优先从 free list 获取
        {
            std::lock_guard<std::mutex> lock(bucket->mutex);
            if (bucket->freeList)
            {
                FreeNode* node = bucket->freeList;
                bucket->freeList = node->next;
                bucket->freeCount--;

                BlockHeader* header = getHeaderFromUserPointer(node);
                assert(header->magic == BLOCK_MAGIC || header->magic == BLOCK_FREED);
                header->magic = BLOCK_MAGIC;
                header->requestedSize = size;

                return static_cast<void*>(node);
            }
        }

        // 分配新页
        allocateNewPage(blockSize);

        // 再次尝试
        {
            std::lock_guard<std::mutex> lock(bucket->mutex);
            if (bucket->freeList)
            {
                FreeNode* node = bucket->freeList;
                bucket->freeList = node->next;
                bucket->freeCount--;

                BlockHeader* header = getHeaderFromUserPointer(node);
                header->magic = BLOCK_MAGIC;
                header->requestedSize = size;

                return static_cast<void*>(node);
            }
        }

        throw std::bad_alloc();
    }

    void deallocate(void* ptr)
    {
        if (!ptr) return;

        BlockHeader* header = getHeaderFromUserPointer(ptr);

        if (header->magic != BLOCK_MAGIC)
        {
            std::cerr << "[MemoryPool Error] Invalid pointer or double free detected." << std::endl;
            std::abort();
        }

        header->magic = BLOCK_FREED;

        Bucket* bucket = getOrCreateBucket(header->blockSize);
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
        if (!obj) return;
        obj->~T();
        deallocate(obj);
    }

    void printStats()
    {
        std::lock_guard<std::mutex> mapLock(bucketMapMutex_);
        std::lock_guard<std::mutex> pageLock(pageMutex_);

        std::cout << "\n========== Memory Pool Stats ==========" << std::endl;

        std::size_t totalPageBytes = 0;
        for (const auto& page : pages_) totalPageBytes += page.pageSize;

        std::cout << "Page count      : " << pages_.size() << std::endl;
        std::cout << "Total page bytes: " << totalPageBytes << std::endl;
        std::cout << "Bucket count    : " << buckets_.size() << std::endl;

        for (const auto& kv : buckets_)
        {
            Bucket* bucket = kv.second;
            std::lock_guard<std::mutex> bucketLock(bucket->mutex);
            std::cout << "BlockSize = " << kv.first
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
        return reinterpret_cast<BlockHeader*>(static_cast<char*>(userPtr) - sizeof(BlockHeader));
    }

    static void* getUserPointerFromHeader(BlockHeader* header)
    {
        return reinterpret_cast<void*>(reinterpret_cast<char*>(header) + sizeof(BlockHeader));
    }

private:
    Bucket* getOrCreateBucket(std::size_t blockSize)
    {
        // 双重检查锁，避免重复创建 bucket
        {
            std::lock_guard<std::mutex> lock(bucketMapMutex_);
            auto it = buckets_.find(blockSize);
            if (it != buckets_.end()) return it->second;
        }

        Bucket* bucket = new Bucket();
        {
            std::lock_guard<std::mutex> lock(bucketMapMutex_);
            auto [it, inserted] = buckets_.emplace(blockSize, bucket);
            if (!inserted) { delete bucket; return it->second; }
        }
        return bucket;
    }

    std::size_t choosePageSize(std::size_t blockSize)
    {
        const std::size_t unitSize = sizeof(BlockHeader) + blockSize;
        const std::size_t pageSize = static_cast<std::size_t>(::getpagesize());
        std::size_t targetBlockCount = 0;

        if (blockSize <= 64) targetBlockCount = 1024;
        else if (blockSize <= 256) targetBlockCount = 512;
        else if (blockSize <= 1024) targetBlockCount = 256;
        else if (blockSize <= 4096) targetBlockCount = 128;
        else if (blockSize <= 64*1024) targetBlockCount = 32;
        else if (blockSize <= 1024*1024) targetBlockCount = 4;
        else targetBlockCount = 1;

        std::size_t rawSize = unitSize * targetBlockCount;
        if (rawSize < pageSize) rawSize = pageSize;
        return alignUpToPage(rawSize);
    }

    void allocateNewPage(std::size_t blockSize)
    {
        Bucket* bucket = getOrCreateBucket(blockSize);

        const std::size_t unitSize = sizeof(BlockHeader) + blockSize;
        const std::size_t pageSize = choosePageSize(blockSize);
        const std::size_t blockCount = pageSize / unitSize;

        void* page = ::mmap(nullptr, pageSize, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (page == MAP_FAILED) throw std::bad_alloc();

        {
            std::lock_guard<std::mutex> lock(pageMutex_);
            pages_.push_back({page, pageSize, blockSize, blockCount});
        }

        char* current = static_cast<char*>(page);
        {
            std::lock_guard<std::mutex> lock(bucket->mutex);
            for (std::size_t i = 0; i < blockCount; ++i)
            {
                BlockHeader* header = reinterpret_cast<BlockHeader*>(current);
                header->magic = BLOCK_MAGIC;
                header->requestedSize = 0; // 用户请求大小初始化为 0
                header->blockSize = blockSize;

                FreeNode* node = static_cast<FreeNode*>(getUserPointerFromHeader(header));
                node->next = bucket->freeList;
                bucket->freeList = node;
                bucket->freeCount++;
                bucket->totalCount++;

                current += unitSize;
            }
        }
    }

    void releaseAllPages()
    {
        std::lock_guard<std::mutex> lock(pageMutex_);
        for (auto& page : pages_)
            ::munmap(page.pageAddress, page.pageSize);
        pages_.clear();
    }

    void releaseAllBuckets()
    {
        std::lock_guard<std::mutex> lock(bucketMapMutex_);
        for (auto& kv : buckets_) delete kv.second;
        buckets_.clear();
    }
};

static int heavy_compute(int x) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return x * x;
}

int main() {
    AtomicThreadPool pool(4);

    std::vector<std::future<int>> futures;
    futures.reserve(8);

    for (int i = 1; i <= 8; ++i) {
        futures.emplace_back(pool.submit(heavy_compute, i));
    }

    for (auto& f : futures) {
        std::cout << "result = " << f.get() << '\n';
    }

    pool.wait_for_all();

    std::cout << "all tasks done\n";
    std::cout << "thread count = " << pool.thread_count() << '\n';
    std::cout << "active tasks = " << pool.active_task_count() << '\n';
    std::cout << "idle threads = " << pool.idle_thread_count() << '\n';

    pool.shutdown();
    return 0;
}