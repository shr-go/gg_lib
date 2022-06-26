// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_EVENTLOOP_H
#define GG_LIB_EVENTLOOP_H

#include "gg_lib/any.h"
#include "gg_lib/noncopyable.h"
#include "gg_lib/Timestamp.h"
#include "gg_lib/ThreadHelper.h"
#include "gg_lib/net/NetUtils.h"

#include <mutex>
#include <vector>

namespace gg_lib {
    namespace net {
        class Channel;

        class Poller;

        class TimerQueue;

        class EventLoop : noncopyable {
        public:
            typedef std::function<void()> Functor;

            EventLoop();

            ~EventLoop();

            void loop();

            void quit();

            Timestamp pollReturnTime() const { return pollReturnTime_; }

            int64_t iteration() const { return iteration_; }

            void runInLoop(Functor cb);

            void queueInLoop(Functor cb);

            TimerId runAt(Timestamp time, TimerCallback cb);

            TimerId runAfter(double delay, TimerCallback cb);

            TimerId runEvery(double interval, TimerCallback cb);

            void cancel(TimerId timerId);

            // internal usage
            void wakeup() const;

            void updateChannel(Channel *channel);

            void removeChannel(Channel *channel);

            bool hasChannel(Channel *channel);

            void assertInLoopThread() {
                if (!isInLoopThread()) {
                    abortNotInLoopThread();
                }
            }

            bool isInLoopThread() const {
                return CurrentThread::tid() == threadId_;
            }

            void setContext(any context) {
                context_ = std::move(context);
            }

            any &getContext() {
                return context_;
            }

            static EventLoop *getEventLoopOfCurrentThread();

        private:
            void abortNotInLoopThread();

            void handleRead() const;

            void doPendingFunctors();

            void printActiveChannels() const; // DEBUG

            typedef std::vector<Channel *> ChannelList;
            std::atomic<bool> quit_;
            bool looping_;
            bool eventHandling_;
            bool callingPendingFunctors_;

            int64_t iteration_;
            const std::thread::id threadId_;
            Timestamp pollReturnTime_;
            std::unique_ptr<Poller> poller_;
            std::unique_ptr<TimerQueue> timerQueue_;
            int wakeupFd_;
            std::unique_ptr<Channel> wakeupChannel_;
            any context_;

            ChannelList activeChannels_;
            Channel *currentActiveChannel_;

            std::mutex mutex_;
            std::vector<Functor> pendingFunctors_;
        };
    }
}

#endif //GG_LIB_EVENTLOOP_H
