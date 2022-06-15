// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_COUNTDOWNLATCH_H
#define GG_LIB_COUNTDOWNLATCH_H

#include "gg_lib/noncopyable.h"
#include <mutex>
#include <condition_variable>

namespace gg_lib {
    class CountDownLatch : noncopyable {
    public:
        explicit CountDownLatch(int count);

        void wait();

        void countDown();

        int getCount() const;

    private:
        mutable std::mutex mutex_;
        std::condition_variable condition_;
        int count_;
    };
}

#endif //GG_LIB_COUNTDOWNLATCH_H
