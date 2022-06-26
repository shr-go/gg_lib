// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/Poller.h"
#include "gg_lib/net/Chnanel.h"

using namespace gg_lib;
using namespace gg_lib::net;

Poller::Poller(EventLoop *loop)
        : ownerLoop_(loop) {}

Poller::~Poller() = default;

bool Poller::hasChannel(Channel *channel) const {
    assertInLoopThread();
    ChannelMap::const_iterator it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}
