// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/CountDownLatch.h"

using namespace gg_lib;

CountDownLatch::CountDownLatch(int count)
        : mutex_(), condition_(), count_(count) {}

void CountDownLatch::wait() {
    std::unique_lock<std::mutex> lk(mutex_);
    while (count_ > 0) {
        condition_.wait(lk);
    }
}

void CountDownLatch::countDown() {
    std::unique_lock<std::mutex> lk(mutex_);
    --count_;
    if (count_ == 0) {
        condition_.notify_all();
    }
}

int CountDownLatch::getCount() const {
    std::unique_lock<std::mutex> lk(mutex_);
    return count_;
}


