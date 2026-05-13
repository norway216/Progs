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

class LinuxMemoryPool {
private:
    static constexpr std::size_t ALIGNMENT = 16;
    static constexpr std::uint64_t BLOCK_MAGIC = 0xCAFEBABE20260508ULL;
    static constexpr std::uint64_t BLOCK_FREED = 0xDEADBEEFCAFEBABEULL;

    struct BlockHeader {
        std::uint64_t magic;
        std::size_t requestedSize;
        std::size_t blockSize;
    };

    struct FreeNode {
        FreeNode* next;
    };

    struct PageInfo {
        void* pageAddress;
        std::size_t pageSize;
        std::size_t blockSize;
        std::size_t blockCount;
    };

    struct Bucket {
        FreeNode* freeList = nullptr;
        std::size_t freeCount = 0;
        std::size_t totalCount = 0;
        std::mutex mutex;
    };

    std::unordered_map<std::size_t, Bucket*> buckets_;
    std::vector<PageInfo> pages_;
    std::mutex bucketMapMutex_;
    std::mutex pageMutex_;

    LinuxMemoryPool() = default;
    ~LinuxMemoryPool() {
        releaseAllPages();
        releaseAllBuckets();
    }
    LinuxMemoryPool(const LinuxMemoryPool&) = delete;
    LinuxMemoryPool& operator=(const LinuxMemoryPool&) = delete;

public:
    static LinuxMemoryPool& instance() {
        static LinuxMemoryPool pool;
        return pool;
    }

    void* allocate(std::size_t size) {
        if (size == 0) size = 1;
        size = alignUp(size);

        Bucket* bucket = getOrCreateBucket(size);

        {
            std::lock_guard<std::mutex> lock(bucket->mutex);
            if (bucket->freeList) {
                FreeNode* node = bucket->freeList;
                bucket->freeList = node->next;
                bucket->freeCount--;

                BlockHeader* header = getHeaderFromUserPointer(node);
                header->magic = BLOCK_MAGIC;
                header->requestedSize = size;
                return static_cast<void*>(node);
            }
        }

        // 如果 free list 为空，则分配新 page
        allocateNewPage(size);

        {
            std::lock_guard<std::mutex> lock(bucket->mutex);
            assert(bucket->freeList && "New page allocation should add blocks to free list");
            FreeNode* node = bucket->freeList;
            bucket->freeList = node->next;
            bucket->freeCount--;

            BlockHeader* header = getHeaderFromUserPointer(node);
            header->magic = BLOCK_MAGIC;
            header->requestedSize = size;
            return static_cast<void*>(node);
        }
    }

    void deallocate(void* ptr) {
        if (!ptr) return;
        BlockHeader* header = getHeaderFromUserPointer(ptr);

        if (header->magic != BLOCK_MAGIC) {
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

    template<typename T, typename... Args>
    T* create(Args&&... args) {
        void* mem = allocate(sizeof(T));
        try {
            return new(mem) T(std::forward<Args>(args)...);
        } catch (...) {
            deallocate(mem);
            throw;
        }
    }

    template<typename T>
    void destroy(T* obj) {
        if (!obj) return;
        obj->~T();
        deallocate(obj);
    }

    void printStats() {
        std::lock_guard<std::mutex> mapLock(bucketMapMutex_);
        std::lock_guard<std::mutex> pageLock(pageMutex_);

        std::cout << "\n========== Memory Pool Stats ==========" << std::endl;
        std::size_t totalPageBytes = 0;
        for (auto& page : pages_) totalPageBytes += page.pageSize;

        std::cout << "Page count: " << pages_.size() << ", total bytes: " << totalPageBytes << std::endl;
        std::cout << "Bucket count: " << buckets_.size() << std::endl;

        for (auto& kv : buckets_) {
            Bucket* bucket = kv.second;
            std::lock_guard<std::mutex> lock(bucket->mutex);
            std::cout << "BlockSize=" << kv.first
                      << ", total=" << bucket->totalCount
                      << ", free=" << bucket->freeCount
                      << ", used=" << bucket->totalCount - bucket->freeCount
                      << std::endl;
        }
        std::cout << "=======================================\n" << std::endl;
    }

private:
    static std::size_t alignUp(std::size_t size) {
        return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    }

    static std::size_t alignUpToPage(std::size_t size) {
        const std::size_t pageSize = static_cast<std::size_t>(::getpagesize());
        return (size + pageSize - 1) & ~(pageSize - 1);
    }

    static BlockHeader* getHeaderFromUserPointer(void* ptr) {
        return reinterpret_cast<BlockHeader*>(static_cast<char*>(ptr) - sizeof(BlockHeader));
    }

    static void* getUserPointerFromHeader(BlockHeader* header) {
        return reinterpret_cast<void*>(reinterpret_cast<char*>(header) + sizeof(BlockHeader));
    }

    Bucket* getOrCreateBucket(std::size_t blockSize) {
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

    std::size_t choosePageSize(std::size_t blockSize) {
        const std::size_t unitSize = sizeof(BlockHeader) + blockSize;
        std::size_t blockCount = 0;

        if (blockSize <= 64) blockCount = 1024;
        else if (blockSize <= 256) blockCount = 512;
        else if (blockSize <= 1024) blockCount = 256;
        else if (blockSize <= 4096) blockCount = 128;
        else if (blockSize <= 64*1024) blockCount = 32;
        else if (blockSize <= 1024*1024) blockCount = 4;
        else blockCount = 1;

        std::size_t rawSize = unitSize * blockCount;
        if (rawSize < static_cast<std::size_t>(::getpagesize()))
            rawSize = ::getpagesize();
        return alignUpToPage(rawSize);
    }

    void allocateNewPage(std::size_t blockSize) {
        Bucket* bucket = getOrCreateBucket(blockSize);
        const std::size_t unitSize = sizeof(BlockHeader) + blockSize;
        const std::size_t pageSize = choosePageSize(blockSize);
        const std::size_t blockCount = pageSize / unitSize;

        void* page = ::mmap(nullptr, pageSize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        if (page == MAP_FAILED) throw std::bad_alloc();

        {
            std::lock_guard<std::mutex> lock(pageMutex_);
            pages_.push_back({page, pageSize, blockSize, blockCount});
        }

        char* current = static_cast<char*>(page);
        std::lock_guard<std::mutex> lock(bucket->mutex);
        for (std::size_t i = 0; i < blockCount; ++i) {
            BlockHeader* header = reinterpret_cast<BlockHeader*>(current);
            header->magic = BLOCK_MAGIC;
            header->requestedSize = 0;
            header->blockSize = blockSize;

            FreeNode* node = static_cast<FreeNode*>(getUserPointerFromHeader(header));
            node->next = bucket->freeList;
            bucket->freeList = node;
            bucket->freeCount++;
            bucket->totalCount++;

            current += unitSize;
        }
    }

    void releaseAllPages() {
        std::lock_guard<std::mutex> lock(pageMutex_);
        for (auto& page : pages_)
            ::munmap(page.pageAddress, page.pageSize);
        pages_.clear();
    }

    void releaseAllBuckets() {
        std::lock_guard<std::mutex> lock(bucketMapMutex_);
        for (auto& kv : buckets_) delete kv.second;
        buckets_.clear();
    }
};

// ==============================
// 测试 main
// ==============================
int main() {
    auto& pool = LinuxMemoryPool::instance();

    // 单线程 benchmark
    constexpr size_t N = 1'000'000;
    constexpr size_t SIZE = 256;
    std::vector<void*> ptrs; ptrs.reserve(N);

    auto t0 = std::chrono::high_resolution_clock::now();
    for (size_t i=0;i<N;i++) ptrs.push_back(pool.allocate(SIZE));
    for (auto p : ptrs) pool.deallocate(p);
    auto t1 = std::chrono::high_resolution_clock::now();
    std::cout << "MemoryPool single-thread " << N << " blocks, cost="
              << std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count()
              << " ms\n";

    // 多线程 benchmark
    constexpr size_t THREADS = 8;
    constexpr size_t LOOP = 200000;
    std::atomic<size_t> doneCount{0};
    std::vector<std::thread> threads;
    auto t2 = std::chrono::high_resolution_clock::now();
    for (size_t i=0;i<THREADS;i++) {
        threads.emplace_back([&]() {
            std::vector<void*> local; local.reserve(LOOP);
            for (size_t j=0;j<LOOP;j++) local.push_back(pool.allocate(SIZE));
            for (auto p : local) pool.deallocate(p);
            doneCount++;
        });
    }
    for (auto& th : threads) th.join();
    auto t3 = std::chrono::high_resolution_clock::now();
    std::cout << "MemoryPool multi-thread " << THREADS*LOOP
              << " blocks, cost="
              << std::chrono::duration_cast<std::chrono::milliseconds>(t3-t2).count()
              << " ms\n";

    pool.printStats();
    return 0;
}