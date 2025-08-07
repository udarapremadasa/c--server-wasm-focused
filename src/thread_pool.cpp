#include "thread_pool.h"

#ifndef BUILD_WASM

ThreadPool::ThreadPool(size_t num_threads) : is_running_(false) {
    resize(num_threads);
}

ThreadPool::~ThreadPool() {
    stop();
}

void ThreadPool::start() {
    if (is_running_) {
        return;
    }
    
    is_running_ = true;
    
    for (auto& thread : threads_) {
        thread = std::thread(&ThreadPool::workerLoop, this);
    }
}

void ThreadPool::stop() {
    if (!is_running_) {
        return;
    }
    
    is_running_ = false;
    condition_.notify_all();
    
    for (auto& thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void ThreadPool::enqueue(Task task) {
    if (!is_running_) {
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        tasks_.push(std::move(task));
    }
    
    condition_.notify_one();
}

void ThreadPool::resize(size_t new_size) {
    if (new_size == 0) {
        new_size = 1;
    }
    
    bool was_running = is_running_;
    
    if (was_running) {
        stop();
    }
    
    threads_.clear();
    threads_.resize(new_size);
    
    if (was_running) {
        start();
    }
}

size_t ThreadPool::getQueueSize() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return tasks_.size();
}

void ThreadPool::workerLoop() {
    while (is_running_) {
        Task task;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            condition_.wait(lock, [this] { return !tasks_.empty() || !is_running_; });
            
            if (!is_running_) {
                break;
            }
            
            if (!tasks_.empty()) {
                task = std::move(tasks_.front());
                tasks_.pop();
            }
        }
        
        if (task) {
            try {
                task();
            } catch (const std::exception& e) {
                // Log exception if needed
                // For now, just continue processing
            }
        }
    }
}

#endif // BUILD_WASM
