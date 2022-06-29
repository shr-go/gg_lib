// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/poller/EPollPoller.h"
#include "gg_lib/net/Channel.h"
#include "gg_lib/Logging.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

using namespace gg_lib;
using namespace gg_lib::net;

namespace {
    constexpr int kNew = -1;
    constexpr int kAdded = 1;
    constexpr int kDeleted = 2;
}

EPollPoller::EPollPoller(EventLoop *loop)
        : Poller(loop),
          epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
          events_(kInitEventListSize) {
    if (epollfd_ < 0) {
        LOG_SYSFATAL << "EPollPoller::EPollPoller";
    }
}

EPollPoller::~EPollPoller() {
    ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, Poller::ChannelList *activeChannels) {
    LOG_TRACE << Fmt("fd total count {}", channels_.size());
    int numEvents = ::epoll_wait(epollfd_,
                                 events_.data(),
                                 static_cast<int>(events_.size()),
                                 timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        LOG_TRACE << Fmt("{}  events happened", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if (static_cast<size_t>(numEvents) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (numEvents == 0) {
        LOG_TRACE << "nothing happened";
    } else {
        if (savedErrno != EINTR) {
            errno = savedErrno;
            LOG_SYSERR << "EPollPoller::poll()";
        }
    }
    return now;
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const {
    assert(static_cast<size_t>(numEvents) <= events_.size());
    activeChannels->reserve(numEvents);
    for (int i = 0; i < numEvents; ++i) {
        auto channel = static_cast<Channel *>(events_[i].data.ptr);
#ifndef NDEBUG
        int fd = channel->fd();
        auto it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);
#endif
        channel->set_revents(static_cast<int>(events_[i].events));
        activeChannels->push_back(channel);
    }
}

void EPollPoller::updateChannel(Channel *channel) {
    Poller::assertInLoopThread();
    const int index = channel->index();
    LOG_TRACE << Fmt("fd = {} events = {} index = {}", channel->fd(), channel->events(), index);
    if (index == kNew || index == kDeleted) {
        int fd = channel->fd();
        if (index == kNew) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        } else {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        int fd = channel->fd();
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel *channel) {
    Poller::assertInLoopThread();
    int fd = channel->fd();
    LOG_TRACE << Fmt("fd = {}", fd);
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    size_t n = channels_.erase(fd);
    assert(n == 1);
    if (index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void EPollPoller::update(int operation, Channel *channel) {
    struct epoll_event event{};
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    LOG_TRACE << Fmt("epoll_ctl op = {} fd = {} event = {{{}}}",
                     operationToString(operation), fd, channel->eventsToString());
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_SYSERR << Fmt("epoll_ctl op ={} fd ={}", operationToString(operation), fd);
        } else {
            LOG_SYSFATAL << Fmt("epoll_ctl op ={} fd ={}", operationToString(operation), fd);
        }
    }
}

const char *EPollPoller::operationToString(int op) {
    switch (op) {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        case EPOLL_CTL_MOD:
            return "MOD";
        default:
            assert(false && "ERROR op");
            return "Unknown Operation";
    }
}
