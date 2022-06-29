// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_EPOLLPOLLER_H
#define GG_LIB_EPOLLPOLLER_H

#include "gg_lib/net/Poller.h"
#include <vector>

struct epoll_event;

namespace gg_lib {
    namespace net {
        class EPollPoller : public Poller {
        public:
            EPollPoller(EventLoop *loop);

            ~EPollPoller() override;

            Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;

            void updateChannel(Channel *channel) override;

            void removeChannel(Channel *channel) override;

        private:
            static constexpr int kInitEventListSize = 64;

            static const char *operationToString(int op);

            void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

            void update(int operation, Channel *channel);

            typedef std::vector<struct epoll_event> EventList;

            int epollfd_;
            EventList events_;
        };
    }
}

#endif //GG_LIB_EPOLLPOLLER_H
