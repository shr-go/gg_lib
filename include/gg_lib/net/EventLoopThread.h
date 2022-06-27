// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_EVENTLOOPTHREAD_H
#define GG_LIB_EVENTLOOPTHREAD_H

#include "gg_lib/ThreadHelper.h"

namespace gg_lib {
    namespace net {
        class EventLoop;

        class EventLoopThread : noncopyable {
        public:
            typedef std::function<void(EventLoop *)> ThreadInitCallback;

            explicit EventLoopThread(ThreadInitCallback cb = ThreadInitCallback(),
                            const string &name = string());

            ~EventLoopThread();
            EventLoop* startLoop();

        private:
            void threadFunc();

            EventLoop *loop_;
            bool exiting_;
            Thread thread_;
            std::mutex mutex_;
            std::condition_variable cond_;
            ThreadInitCallback callback_;
        };
    }
}

#endif //GG_LIB_EVENTLOOPTHREAD_H
