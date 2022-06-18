// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/AsyncLogging.h"
#include "gg_lib/FileUtil.h"
#include "gg_lib/Timestamp.h"
#include <chrono>

using namespace gg_lib;

AsyncLogging::AsyncLogging(StringArg basename,
                           off_t rollSize,
                           int flushInterval)
        : basename_(basename.c_str()),
          flushInterval_(flushInterval),
          rollSize_(rollSize),
          running_(false),
          thread_(std::bind(&AsyncLogging::threadFunc, this), "AsyncLogging"),
          latch_(1),
          mutex_(),
          cond_(),
          currentBuffer_(new Buffer),
          nextBuffer_(new Buffer),
          buffers_() {
    buffers_.reserve(16);
#ifdef PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP
    /// try use hybrid mutex for better performance
    *mutex_.native_handle() = PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP;
#endif
}

void AsyncLogging::append(const char *logline, int len) {
    std::lock_guard<std::mutex> lk(mutex_);
    if (__builtin_expect(currentBuffer_->avail() > len, true)) {
        currentBuffer_->append(logline, len);
    } else {
        buffers_.push_back(std::move(currentBuffer_));
        if (nextBuffer_) {
            currentBuffer_ = std::move(nextBuffer_);
        } else {
            currentBuffer_.reset(new Buffer);
        }
        currentBuffer_->append(logline, len);
        cond_.notify_one();
    }
}

void AsyncLogging::threadFunc() {
    assert(running_);
    latch_.countDown();
    LogFile output(basename_, rollSize_, false);
    BufferPtr Buffer1(new Buffer);
    BufferPtr Buffer2(new Buffer);
    BufferVector toWrite;
    toWrite.reserve(16);
    while (running_) {
        assert(Buffer1 && Buffer1->length() == 0);
        assert(Buffer2 && Buffer2->length() == 0);
        assert(toWrite.empty());
        {
            std::unique_lock<std::mutex> lk(mutex_);
            if (buffers_.empty()) {
                cond_.wait_for(lk, std::chrono::seconds(flushInterval_));
            }
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(Buffer1);
            toWrite.swap(buffers_);
            if (!nextBuffer_) {
                nextBuffer_ = std::move(Buffer2);
            }
        }

        assert(!toWrite.empty());

        if (toWrite.size() > 16) {
            char buf[256];
            snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
                     Timestamp::now().toString().c_str(),
                     toWrite.size() - 2);
            fputs(buf, stderr);
            output.append(buf, static_cast<int>(strlen(buf)));
            toWrite.resize(2);
        }

        for (const auto& buffer: toWrite) {
            //fixme: may use writev for better performance
            output.append(buffer->data(), buffer->length());
        }

        if (toWrite.size() > 2) {
            toWrite.resize(2);
        }

        if (!Buffer1) {
            assert(!toWrite.empty());
            Buffer1 = std::move(toWrite.back());
            toWrite.pop_back();
            Buffer1->reset();
        }

        if (!Buffer2) {
            assert(!toWrite.empty());
            Buffer2 = std::move(toWrite.back());
            toWrite.pop_back();
            Buffer2->reset();
        }

        toWrite.clear();
        output.flush();
    }
    output.flush();
}
