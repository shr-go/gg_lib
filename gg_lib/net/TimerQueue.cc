// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/Logging.h"

#include "gg_lib/net/TimerQueue.h"
#include "gg_lib/net/Timer.h"
#include "gg_lib/net/EventLoop.h"

#include <sys/timerfd.h>
#include <unistd.h>

namespace gg_lib {
    namespace net {
        namespace {
            int createTimerfd() {
                int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
                if (timerfd < 0) {
                    LOG_SYSFATAL << "Failed in timerfd_create";
                }
                return timerfd;
            }

            struct timespec howMuchTimeFromNow(Timestamp when) {
                int64_t microseconds = when.microSecondsSinceEpoch()
                                       - Timestamp::now().microSecondsSinceEpoch();
                if (microseconds < 100) {
                    microseconds = 100;
                }
                struct timespec ts;
                ts.tv_sec = static_cast<time_t>(
                        microseconds / Timestamp::kMicroSecondsPerSecond);
                ts.tv_nsec = static_cast<long>(
                        (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
                return ts;
            }

            void readTimerfd(int timerfd, Timestamp now) {
                uint64_t howmany;
                ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
                LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
                if (n != sizeof howmany) {
                    LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
                }
            }

            void resetTimerfd(int timerfd, Timestamp expiration) {
                // wake up loop by timerfd_settime()
                struct itimerspec newValue{};
                struct itimerspec oldValue{};
                newValue.it_value = howMuchTimeFromNow(expiration);
                int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
                if (ret) {
                    LOG_SYSERR << "timerfd_settime()";
                }
            }
        }
    }
}


using namespace gg_lib;
using namespace gg_lib::net;

AtomicInt64 Timer::s_numCreated_;

TimerQueue::TimerQueue(EventLoop *loop)
        : loop_(loop),
          timerfd_(createTimerfd()),
          timerfdChannel_(loop, timerfd_),
          timers_() {
    timerfdChannel_.setReadCallback(
            std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    ::close(timerfd_);
    // here we don't need to delete timers, since they are managed by shared_ptr.
}

TimerId TimerQueue::addTimer(TimerCallback cb,
                             Timestamp when,
                             double interval) {
    TimerPtr timer = std::make_shared<Timer>(std::move(cb), when, interval);
    TimerId timerId = timer->sequence();
    loop_->runInLoop(
            std::bind(&TimerQueue::addTimerInLoop, this, std::move(timer)));
    return timerId;
}

void TimerQueue::cancel(TimerId timerId) {
    loop_->runInLoop(
            std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::addTimerInLoop(TimerPtr &timer) {
    loop_->assertInLoopThread();
    auto expiration = timer->expiration();
    bool earliestChanged = insert(timer);
    if (earliestChanged) {
        resetTimerfd(timerfd_, expiration);
    }
}

void TimerQueue::cancelInLoop(TimerId timerId) {
    loop_->assertInLoopThread();
    activeTimers_.erase(timerId);
}

void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    Timestamp now = Timestamp::now();
    readTimerfd(timerfd_, now);
    std::vector<TimerPtr> expired = getExpired(now);
    for (const auto &it: expired) {
        it->run();
    }
    reset(expired, now);
}

std::vector<TimerQueue::TimerPtr> TimerQueue::getExpired(Timestamp now) {
    std::vector<TimerPtr> expired;
    auto end = timers_.upper_bound(now);
    assert(end == timers_.end() || now < end->first);
    auto begin = timers_.begin();
    for (auto iter = timers_.begin(); iter != end; ++iter) {
        WeakTimerPtr ptr = iter->second;
        if (TimerPtr timer = ptr.lock()) {
            expired.push_back(std::move(timer));
        }
    }
    timers_.erase(begin, end);
    return expired;
}

void TimerQueue::reset(const std::vector<TimerPtr> &expired, Timestamp now) {
    Timestamp nextExpire;
    for (const auto &timer: expired) {
        TimerId timerId = timer->sequence();
        // repeat timer may be deleted in its callback.
        if (timer->repeat() && activeTimers_.find(timerId) != activeTimers_.end()) {
            timer->restart(now);
            insert(timer);
        } else {
            activeTimers_.erase(timerId);
        }
    }
    while (!timers_.empty()) {
        auto timer = timers_.begin();
        if (timer->second.expired()) {
            timers_.erase(timer);
        } else {
            nextExpire = timer->first;
            break;
        }
    }

    if (nextExpire.valid()) {
        resetTimerfd(timerfd_, nextExpire);
    }
}

bool TimerQueue::insert(const TimerQueue::TimerPtr &timer) {
    loop_->assertInLoopThread();
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    auto begin = timers_.begin();
    if (begin == timers_.end() || when < begin->first) {
        earliestChanged = true;
    }
    timers_.emplace(when, WeakTimerPtr(timer));
    TimerId timerId = timer->sequence();
    if (activeTimers_.find(timerId) == activeTimers_.end()) {
        activeTimers_.emplace(timerId, timer);
    }
    return earliestChanged;
}

