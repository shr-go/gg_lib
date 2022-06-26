// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_TIMER_H
#define GG_LIB_TIMER_H

#include "gg_lib/Timestamp.h"
#include "gg_lib/net/NetUtils.h"

namespace gg_lib {
    namespace net {
        class Timer : noncopyable {
        public:
            Timer(TimerCallback cb, Timestamp when, double interval)
                    : callback_(std::move(cb)),
                      expiration_(when),
                      interval_(interval),
                      repeat_(interval > 0.0),
                      sequence_(s_numCreated_.fetch_add(1)) {}

            void run() const {
                callback_();
            }

            Timestamp expiration() const { return expiration_; }

            bool repeat() const { return repeat_; }

            TimerId sequence() const { return sequence_; }

            void restart(Timestamp now) {
                if (__builtin_expect(repeat_, true)) {
                    expiration_ = Timestamp::addTime(now, interval_);
                } else {
                    expiration_ = Timestamp();
                }
            }

            static int64_t numCreated() { return s_numCreated_.load(); }


        private:
            const TimerCallback callback_;
            Timestamp expiration_;
            const double interval_;
            const bool repeat_;
            const TimerId sequence_;

            static AtomicInt64 s_numCreated_;
        };
    }
}

#endif //GG_LIB_TIMER_H
