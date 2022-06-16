// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_ASYNCLOGGING_H
#define GG_LIB_ASYNCLOGGING_H

#include "gg_lib/noncopyable.h"
#include "gg_lib/Utils.h"
#include "gg_lib/CountDownLatch.h"
#include "gg_lib/ThreadHelper.h"
#include "gg_lib/LogStream.h"

#include <memory>
#include <mutex>
#include <condition_variable>
#include <vector>

namespace gg_lib {
    class AsyncLogging : noncopyable {
    public:
        AsyncLogging(StringArg basename, off_t rollSize, int flushInterval = 3);
        ~AsyncLogging() {
            if (running_)
                stop();
        }

        void append(const char* logline, int len);

        void start() {
            running_ = true;
            thread_.start();
            latch_.wait();
        }

        void stop() {
            running_ = false;
            cond_.notify_one();
            thread_.join();
        }
    private:
        void threadFunc();

        typedef FixedBuffer<kLargeBuffer> Buffer;
        typedef std::unique_ptr<Buffer> BufferPtr;
        typedef std::vector<BufferPtr> BufferVector;

        const string basename_;
        const int flushInterval_;
        const off_t rollSize_;

        std::atomic<bool> running_;
        Thread thread_;
        CountDownLatch latch_;
        std::mutex mutex_;
        std::condition_variable cond_;
        BufferPtr currentBuffer_;
        BufferPtr nextBuffer_;
        BufferVector buffers_;
    };
}

#endif //GG_LIB_ASYNCLOGGING_H
