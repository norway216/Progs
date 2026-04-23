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

class AtomicThreadPool {
public:
    explicit AtomicThreadPool(std::size_t thread_count = std::thread::hardware_concurrency())
        : stop_flag_(false),
          active_tasks_(0),
          idle_threads_(0) {
        if (thread_count == 0) {
            thread_count = 1;
        }

        workers_.reserve(thread_count);

        for (std::size_t i = 0; i < thread_count; ++i) {
            workers_.emplace_back([this]() {
                this->worker_loop();
            });
        }
    }

    AtomicThreadPool(const AtomicThreadPool&) = delete;
    AtomicThreadPool& operator=(const AtomicThreadPool&) = delete;

    AtomicThreadPool(AtomicThreadPool&&) = delete;
    AtomicThreadPool& operator=(AtomicThreadPool&&) = delete;

    ~AtomicThreadPool() {
        shutdown();
    }

public:
    template <typename F, typename... Args>
    auto submit(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>> {
        using ReturnType = std::invoke_result_t<F, Args...>;

        if (stop_flag_.load(std::memory_order_acquire)) {
            throw std::runtime_error("ThreadPool is stopped; cannot submit new tasks.");
        }

        auto task_ptr = std::make_shared<std::packaged_task<ReturnType()>>(
            [func = std::forward<F>(f),
             tup = std::make_tuple(std::forward<Args>(args)...)]() mutable -> ReturnType {
                return std::apply(std::move(func), std::move(tup));
            });

        std::future<ReturnType> result = task_ptr->get_future();

        {
            std::lock_guard<std::mutex> lock(queue_mutex_);

            if (stop_flag_.load(std::memory_order_relaxed)) {
                throw std::runtime_error("ThreadPool is stopped during submit.");
            }

            tasks_.emplace([task_ptr]() {
                (*task_ptr)();
            });
        }

        active_tasks_.fetch_add(1, std::memory_order_relaxed);
        cv_.notify_one();

        return result;
    }

    void shutdown() {
        bool expected = false;
        if (!stop_flag_.compare_exchange_strong(
                expected, true,
                std::memory_order_acq_rel,
                std::memory_order_acquire)) {
            return;
        }

        cv_.notify_all();

        for (std::thread& t : workers_) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    void wait_for_all() {
        std::unique_lock<std::mutex> lock(wait_mutex_);
        wait_cv_.wait(lock, [this]() {
            return active_tasks_.load(std::memory_order_acquire) == 0;
        });
    }

    [[nodiscard]] std::size_t thread_count() const noexcept {
        return workers_.size();
    }

    [[nodiscard]] std::size_t active_task_count() const noexcept {
        return active_tasks_.load(std::memory_order_acquire);
    }

    [[nodiscard]] std::size_t idle_thread_count() const noexcept {
        return idle_threads_.load(std::memory_order_acquire);
    }

private:
    void worker_loop() {
        while (true) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(queue_mutex_);

                idle_threads_.fetch_add(1, std::memory_order_relaxed);

                cv_.wait(lock, [this]() {
                    return stop_flag_.load(std::memory_order_acquire) || !tasks_.empty();
                });

                idle_threads_.fetch_sub(1, std::memory_order_relaxed);

                if (stop_flag_.load(std::memory_order_acquire) && tasks_.empty()) {
                    return;
                }

                task = std::move(tasks_.front());
                tasks_.pop();
            }

            try {
                task();
            } catch (...) {
                // 理论上 packaged_task 会自己捕获异常并传入 future，
                // 这里主要兜底，防止线程因异常退出。
            }

            const std::size_t remaining =
                active_tasks_.fetch_sub(1, std::memory_order_acq_rel) - 1;

            if (remaining == 0) {
                std::lock_guard<std::mutex> lock(wait_mutex_);
                wait_cv_.notify_all();
            }
        }
    }

private:
    std::vector<std::thread> workers_;

    std::queue<std::function<void()>> tasks_;
    mutable std::mutex queue_mutex_;
    std::condition_variable cv_;

    std::atomic<bool> stop_flag_;
    std::atomic<std::size_t> active_tasks_;
    std::atomic<std::size_t> idle_threads_;

    std::mutex wait_mutex_;
    std::condition_variable wait_cv_;
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