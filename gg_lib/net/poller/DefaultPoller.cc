// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/Poller.h"
#include "gg_lib/net/poller/PollPoller.h"
#include "gg_lib/net/poller/EPollPoller.h"

using namespace gg_lib::net;

Poller *Poller::newDefaultPoller(EventLoop *loop) {
    if (::getenv("USE_POLL")) {
        return new PollPoller(loop);
    } else {
        return new EPollPoller(loop);
    }
}
