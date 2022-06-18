// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_THREADPOOL_H
#define GG_LIB_THREADPOOL_H

#include "gg_lib/ThreadHelper.h"
#include <deque>
#include <vector>

namespace gg_lib {
    class ThreadPool: noncopyable {
    public:
        typedef std::function<void ()> Task;
        explicit ThreadPool(StringArg name = "ThreadPool");
        ~ThreadPool();

        void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
        void setThreadInitCallback(const Task& cb) { threadInitCallback_ = cb; }

        void start(int numThreads);
        void stop();

        const string& name() const { return name_; }

        size_t queueSize() const;

        template<typename T>
        void run(T &&task) {
            static_assert(std::is_convertible<T, Task>::value, "T isn't a task.");
            if (threads_.empty()) {
                task();
            } else {
                std::unique_lock<std::mutex> lk(mutex_);
                while (isFull() && running_) {
                    notFull_.wait(lk);
                }
                if (!running_) return;
                assert(!isFull());
                queue_.push_back(std::forward<T>(task));
                notEmpty_.notify_one();
            }
        }


    private:
        bool isFull() const;
        void runInThread();
        Task take();

        mutable std::mutex mutex_;
        std::condition_variable notEmpty_;
        std::condition_variable notFull_;
        string name_;
        Task threadInitCallback_;
        std::vector<std::unique_ptr<Thread> > threads_;
        std::deque<Task> queue_;
        size_t maxQueueSize_;
        bool running_;
    };
}

#endif //GG_LIB_THREADPOOL_H
