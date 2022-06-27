// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_TIMERQUEUE_H
#define GG_LIB_TIMERQUEUE_H

#include <vector>
#include <typeinfo>
#include <memory>

#include "gg_lib/Timestamp.h"
#include "gg_lib/net/Chnanel.h"
#include "gg_lib/net/NetUtils.h"

#include <unordered_map>
#include <queue>

namespace gg_lib {
    namespace net {
        class EventLoop;

        class Timer;

        class TimerQueue : noncopyable {
        public:
            // FIXME use unique_ptr instead.
            typedef std::shared_ptr<Timer> TimerPtr;

            explicit TimerQueue(EventLoop *loop);

            ~TimerQueue();

            TimerId addTimer(TimerCallback cb,
                             Timestamp when,
                             double interval);

            void cancel(TimerId timerId);

        private:
            typedef std::pair<Timestamp, TimerId> Entry;
            typedef std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> TimerMap;
            typedef std::unordered_map<TimerId, TimerPtr> ActiveTimerMap;

            void addTimerInLoop(TimerPtr &timer);

            void cancelInLoop(TimerId timerId);

            void handleRead();

            std::vector<TimerPtr> getExpired(Timestamp now);

            void reset(const std::vector<TimerPtr> &expired, Timestamp now);

            bool insert(const TimerPtr &timer);

            EventLoop *loop_;
            const int timerfd_;
            Channel timerfdChannel_;

            // Btree map sorted by expiration
            TimerMap timers_;

            // Own all timers
            ActiveTimerMap activeTimers_;
        };
    }
}

#endif //GG_LIB_TIMERQUEUE_H
