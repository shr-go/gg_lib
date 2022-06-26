// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/Poller.h"
#include "gg_lib/net/poller/PollPoller.h"

using namespace gg_lib::net;

Poller *Poller::newDefaultPoller(EventLoop *loop) {
    // FIXME
    return new PollPoller(loop);
//    if (::getenv("USE_POLL")) {
//        return new PollPoller(loop);
//    } else {
//        return new EPollPoller(loop);
//    }
}
