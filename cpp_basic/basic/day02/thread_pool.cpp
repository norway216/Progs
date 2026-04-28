#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <atomic>
#include <mutex>
#include <condition_variable>

class ThreadPool {
public:
    ThreadPool(size_t numThreads) : stop(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        this->condition.wait(lock, [this] { return this->stop || !this->taskQueue.empty(); });
                        if (this->stop && this->taskQueue.empty()) {
                            return;
                        }
                        task = std::move(this->taskQueue.front());
                        this->taskQueue.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    template <class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            taskQueue.push(std::function<void()>(std::forward<F>(f)));
        }
        condition.notify_one();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> taskQueue;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

void exampleTask(int id) {
    std::cout << "Task " << id << " is being executed by thread: " << std::this_thread::get_id() << std::endl;
}

int main() {
    ThreadPool pool(4);  // 创建一个包含4个线程的线程池

    // 向线程池提交任务
    for (int i = 0; i < 10; ++i) {
        pool.enqueue([i] { exampleTask(i); });
    }

    // 给线程池一些时间来执行任务
    std::this_thread::sleep_for(std::chrono::seconds(2));

    return 0;
}