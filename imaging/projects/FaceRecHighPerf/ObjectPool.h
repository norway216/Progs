#pragma once
#include <vector>
#include <queue>
#include <memory>
#include <mutex>

template<typename T, size_t N>
class ObjectPool {
    std::vector<std::unique_ptr<T>> pool_;
    std::queue<T*> freeQueue_;
    std::mutex mutex_;
public:
    ObjectPool() {
        for(size_t i=0;i<N;i++){
            auto obj = std::make_unique<T>();
            freeQueue_.push(obj.get());
            pool_.push_back(std::move(obj));
        }
    }
    T* acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        if(freeQueue_.empty()) return nullptr;
        T* obj = freeQueue_.front();
        freeQueue_.pop();
        return obj;
    }
    void release(T* obj){
        std::lock_guard<std::mutex> lock(mutex_);
        freeQueue_.push(obj);
    }
};