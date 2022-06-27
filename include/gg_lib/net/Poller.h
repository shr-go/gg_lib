// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_POLLER_H
#define GG_LIB_POLLER_H

#include "gg_lib/Timestamp.h"
#include "gg_lib/net/EventLoop.h"

#include <vector>
#include <unordered_map>


namespace gg_lib {
    namespace net {
        class Channel;

        class Poller : noncopyable {
        public:
            typedef std::vector<Channel *> ChannelList;

            Poller(EventLoop *loop);

            virtual ~Poller();

            /// Polls the I/O events.
            /// Must be called in the loop thread.
            virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;

            /// Changes the interested I/O events.
            /// Must be called in the loop thread.
            virtual void updateChannel(Channel *channel) = 0;

            /// Remove the channel, when it destructs.
            /// Must be called in the loop thread.
            virtual void removeChannel(Channel *channel) = 0;

            virtual bool hasChannel(Channel *channel) const;

            static Poller *newDefaultPoller(EventLoop *loop);

            void assertInLoopThread() const {
                ownerLoop_->assertInLoopThread();
            }


        protected:
            typedef std::unordered_map<int, Channel *> ChannelMap;
            ChannelMap channels_;
        private:
            EventLoop *ownerLoop_;
        };
    }
}

#endif //GG_LIB_POLLER_H
