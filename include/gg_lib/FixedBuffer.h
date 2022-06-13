// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_FIXEDBUFFER_H
#define GG_LIB_FIXEDBUFFER_H

#include <cstring>
#include "gg_lib/Utils.h"
#include "gg_lib/noncopyable.h"

namespace gg_lib {
    constexpr int kSmallBuffer = 4000;
    constexpr int kMiddleBuffer = 8000;
    constexpr int kLargeBuffer = 4000 * 1000;

    template<int SIZE>
    class FixedBuffer : noncopyable {
    public:
        FixedBuffer() : cur_(data_) {};

        ~FixedBuffer() = default;

        void append(const char *buf, size_t len) {
            if (static_cast<size_t>(avail()) < len)
                len = avail();
            memcpy(cur_, buf, len);
            cur_ += len;
        }

        string_view toStringView() const {
            return string_view(data_, length());
        }

        string toString() const {
            return string(data_, length());
        }


        const char *data() const { return data_; }

        int length() const { return static_cast<int>(cur_ - data_); }

        char *current() { return cur_; }

        int avail() const { return static_cast<int>(end() - cur_); }

        void add(size_t len) { cur_ += len; }

        void reset() { cur_ = data_; }

    private:
        const char *end() const { return data_ + sizeof data_; }

    private:
        char data_[SIZE];
        char *cur_;
    };
}

#endif //GG_LIB_FIXEDBUFFER_H
