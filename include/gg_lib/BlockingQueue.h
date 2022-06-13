// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_BLOCKINGQUEUE_H
#define GG_LIB_BLOCKINGQUEUE_H

#include <mutex>
#include <condition_variable>
#include <vector>

#include "gg_lib/noncopyable.h"

namespace gg_lib {
    template <typename T>
    class BlockingQueue: noncopyable {
    public:
        explicit BlockingQueue(int maxSize)
            : mutex_(), notEmpty_(), notFull_(), cur_(0), size_(maxSize) {

        }

    private:

    private:
        mutable std::mutex mutex_;
        std::condition_variable notEmpty_;
        std::condition_variable notFull_;
        std::vector<T> queue_;
        int cur_;
        int size_;
    };
}


#endif //GG_LIB_BLOCKINGQUEUE_H
