#include <gtest/gtest.h>
#include "thread_pool.h"
#include <atomic>
#include <chrono>
#include <thread>

#ifndef BUILD_WASM

class ThreadPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        pool = std::make_unique<ThreadPool>(4);
    }
    
    void TearDown() override {
        if (pool) {
            pool->stop();
        }
    }
    
    std::unique_ptr<ThreadPool> pool;
};

TEST_F(ThreadPoolTest, InitialState) {
    EXPECT_FALSE(pool->isRunning());
    EXPECT_EQ(pool->getThreadCount(), 4);
    EXPECT_EQ(pool->getQueueSize(), 0);
}

TEST_F(ThreadPoolTest, StartAndStop) {
    pool->start();
    EXPECT_TRUE(pool->isRunning());
    
    pool->stop();
    EXPECT_FALSE(pool->isRunning());
}

TEST_F(ThreadPoolTest, TaskExecution) {
    std::atomic<int> counter{0};
    
    pool->start();
    
    // Enqueue some tasks
    for (int i = 0; i < 10; ++i) {
        pool->enqueue([&counter]() {
            counter++;
        });
    }
    
    // Wait a bit for tasks to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_EQ(counter.load(), 10);
}

TEST_F(ThreadPoolTest, TaskExecutionOrder) {
    std::vector<int> results;
    std::mutex results_mutex;
    
    pool->start();
    
    // Enqueue tasks that will add to results in order
    for (int i = 0; i < 5; ++i) {
        pool->enqueue([&results, &results_mutex, i]() {
            std::lock_guard<std::mutex> lock(results_mutex);
            results.push_back(i);
        });
    }
    
    // Wait for tasks to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_EQ(results.size(), 5);
    // Note: Order is not guaranteed with multiple threads
}

TEST_F(ThreadPoolTest, QueueSize) {
    // Don't start the pool so tasks queue up
    EXPECT_EQ(pool->getQueueSize(), 0);
    
    pool->enqueue([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    });
    
    EXPECT_EQ(pool->getQueueSize(), 1);
    
    pool->enqueue([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    });
    
    EXPECT_EQ(pool->getQueueSize(), 2);
}

TEST_F(ThreadPoolTest, ResizePool) {
    EXPECT_EQ(pool->getThreadCount(), 4);
    
    pool->resize(8);
    EXPECT_EQ(pool->getThreadCount(), 8);
    
    pool->resize(2);
    EXPECT_EQ(pool->getThreadCount(), 2);
    
    // Test resize to 0 (should become 1)
    pool->resize(0);
    EXPECT_EQ(pool->getThreadCount(), 1);
}

TEST_F(ThreadPoolTest, ResizeWhileRunning) {
    std::atomic<int> counter{0};
    
    pool->start();
    EXPECT_TRUE(pool->isRunning());
    
    // Add some tasks
    for (int i = 0; i < 5; ++i) {
        pool->enqueue([&counter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            counter++;
        });
    }
    
    // Resize while running
    pool->resize(8);
    EXPECT_EQ(pool->getThreadCount(), 8);
    EXPECT_TRUE(pool->isRunning());
    
    // Add more tasks
    for (int i = 0; i < 5; ++i) {
        pool->enqueue([&counter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            counter++;
        });
    }
    
    // Wait for all tasks to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    EXPECT_EQ(counter.load(), 10);
}

TEST_F(ThreadPoolTest, ExceptionHandling) {
    std::atomic<int> completed_tasks{0};
    
    pool->start();
    
    // Add a task that throws an exception
    pool->enqueue([]() {
        throw std::runtime_error("Test exception");
    });
    
    // Add a normal task after the exception
    pool->enqueue([&completed_tasks]() {
        completed_tasks++;
    });
    
    // Wait for tasks to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // The pool should continue processing tasks even after an exception
    EXPECT_EQ(completed_tasks.load(), 1);
}

TEST_F(ThreadPoolTest, StopWithPendingTasks) {
    pool->start();
    
    // Add a long-running task
    pool->enqueue([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    });
    
    // Add more tasks
    for (int i = 0; i < 5; ++i) {
        pool->enqueue([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }
    
    // Stop should wait for all tasks to complete or interrupt them
    pool->stop();
    EXPECT_FALSE(pool->isRunning());
}

TEST_F(ThreadPoolTest, EnqueueWhenStopped) {
    std::atomic<int> counter{0};
    
    // Don't start the pool
    pool->enqueue([&counter]() {
        counter++;
    });
    
    // Wait a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Task shouldn't execute when pool is not running
    EXPECT_EQ(counter.load(), 0);
}

#endif // BUILD_WASM
