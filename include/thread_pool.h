#pragma once

#ifndef BUILD_WASM

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

class ThreadPool {
public:
    using Task = std::function<void()>;

    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency());
    ~ThreadPool();

    // Add task to the pool
    void enqueue(Task task);
    
    // Pool management
    void start();
    void stop();
    void resize(size_t new_size);
    
    // Status
    bool isRunning() const { return is_running_; }
    size_t getThreadCount() const { return threads_.size(); }
    size_t getQueueSize() const;

private:
    std::vector<std::thread> threads_;
    std::queue<Task> tasks_;
    
    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> is_running_;
    
    void workerLoop();
};

#endif // BUILD_WASM
