// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_POLLPOLLER_H
#define GG_LIB_POLLPOLLER_H

#include "gg_lib/net/Poller.h"

struct pollfd;

namespace gg_lib {
    namespace net {
        ///
        /// @brief IO Multiplexing with poll(2).
        ///
        class PollPoller : public Poller {
        public:
            PollPoller(EventLoop *loop);

            ~PollPoller() override = default;

            Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;

            void updateChannel(Channel *channel) override;

            void removeChannel(Channel *channel) override;

        private:
            void fillActiveChannels(int numEvents,
                                    ChannelList *activeChannels) const;

            typedef std::vector<struct pollfd> PollFdList;
            PollFdList pollfds_;
        };
    }
}

#endif //GG_LIB_POLLPOLLER_H
