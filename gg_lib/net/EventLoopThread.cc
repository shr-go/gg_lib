// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/EventLoopThread.h"
#include "gg_lib/net/EventLoop.h"

using namespace gg_lib;
using namespace gg_lib::net;

EventLoopThread::EventLoopThread(const EventLoopThread::ThreadInitCallback &cb,
                                 const string &name)
        : loop_(nullptr),
          exiting_(false),
          thread_(std::bind(&EventLoopThread::threadFunc, this), name),
          callback_(cb) {}


EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_ != nullptr) {
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::startLoop() {
    assert(!thread_.started());
    thread_.start();
    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lk(mutex_);
        while (loop_ == nullptr) {
            cond_.wait(lk);
        }
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    if (callback_) {
        callback_(&loop);
    }
    {
        std::lock_guard<std::mutex> lk(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.loop();
    std::lock_guard<std::mutex> lk(mutex_);
    loop_ = nullptr;
}
