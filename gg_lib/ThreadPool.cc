// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/ThreadPool.h"

using namespace gg_lib;

ThreadPool::ThreadPool(StringArg name)
        : mutex_(),
          notFull_(),
          notEmpty_(),
          name_(name.c_str()),
          maxQueueSize_(0),
          running_(false) {
}

ThreadPool::~ThreadPool() {
    if (running_)
        stop();
}

void ThreadPool::start(int numThreads) {
    assert(!running_ && threads_.empty());
    running_ = true;
    threads_.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i) {
        threads_.emplace_back(new Thread(std::bind(&ThreadPool::runInThread, this),
                                         name_ + std::to_string(i)));
        threads_[i]->start();
    }
    if (numThreads == 0 && threadInitCallback_) {
        threadInitCallback_();
    }
}

void ThreadPool::stop() {
    {
        std::lock_guard<std::mutex> lk(mutex_);
        running_ = false;
        notEmpty_.notify_all();
        notFull_.notify_all();
    }
    for (auto &thread: threads_) {
        thread->join();
    }
}

size_t ThreadPool::queueSize() const {
    std::lock_guard<std::mutex> lk(mutex_);
    return queue_.size();
}

ThreadPool::Task ThreadPool::take() {
    std::unique_lock<std::mutex> lk(mutex_);
    while (queue_.empty() && running_) {
        notEmpty_.wait(lk);
    }
    Task task;
    if (!queue_.empty()) {
        task = std::move(queue_.front());
        queue_.pop_front();
        if (maxQueueSize_ > 0) {
            notFull_.notify_one();
        }
    }
    return task;
}

bool ThreadPool::isFull() const {
    return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

void ThreadPool::runInThread() {
    try {
        if (threadInitCallback_) {
            threadInitCallback_();
        }
        while (running_) {
            Task task = take();
            if (task) {
                task();
            }
        }
    } catch (const std::exception& ex) {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
    }
}

