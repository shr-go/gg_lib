// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/EventLoopThreadPool.h"
#include "gg_lib/net/EventLoop.h"
#include "gg_lib/net/EventLoopThread.h"

using namespace gg_lib;
using namespace gg_lib::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const string &nameArg)
        : baseLoop_(baseLoop),
          name_(nameArg),
          started_(false),
          numThreads_(0),
          next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() = default;


void EventLoopThreadPool::start(const ThreadInitCallback &cb) {
    assert(!started_);
    assert(numThreads_ >= 0);
    baseLoop_->assertInLoopThread();
    started_ = true;

    for (int i = 0; i < numThreads_; ++i) {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s-%d", name_.c_str(), i);
        threads_.push_back(std::make_unique<EventLoopThread>(cb, string(buf)));
        loops_.push_back(threads_.back()->startLoop());
    }
    if (numThreads_ == 0 && cb) {
        cb(baseLoop_);
    }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    assert(started_);
    EventLoop *loop = baseLoop_;
    if (!loops_.empty()) {
        loop = loops_[next_++];
        if (static_cast<size_t>(next_) >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop;
}

EventLoop *EventLoopThreadPool::getLoopForHash(size_t hash) {
    baseLoop_->assertInLoopThread();
    EventLoop *loop = baseLoop_;
    if (!loops_.empty()) {
        loop = loops_[hash % loops_.size()];
    }
    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops() {
    baseLoop_->assertInLoopThread();
    assert(started_);
    if (loops_.empty()) {
        return std::vector<EventLoop*>{baseLoop_};
    } else {
        return loops_;
    }
}
