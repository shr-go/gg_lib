// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_EVENTLOOP_H
#define GG_LIB_EVENTLOOP_H

#include "gg_lib/Utils.h"
#include "gg_lib/noncopyable.h"
#include "gg_lib/Timestamp.h"

#include <boost/any.hpp>
#include <vector>

namespace gg_lib {
    namespace net {
        class Channel;
        class Poller;
        class TimerQueue;

        class EventLoop: noncopyable {
        public:

        private:
            typedef std::vector<Channel*> ChannelList;
            bool looping_;
            bool quit_;
            bool eventHandling_;
            bool callingPendingFunctors_;

            int64_t iteration_;
            Timestamp pollReturnTime_;
            std::unique_ptr<Poller> poller_;
            std::unique_ptr<TimerQueue> timerQueue_;
            int wakeupFD_;
            std::unique_ptr<Channel> wakeupChannel_;
            boost::any context_;
        };
    }
}

#endif //GG_LIB_EVENTLOOP_H
