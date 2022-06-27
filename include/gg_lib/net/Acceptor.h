// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_ACCEPTOR_H
#define GG_LIB_ACCEPTOR_H

#include "gg_lib/net/Chnanel.h"
#include "gg_lib/net/SocketsHelper.h"

namespace gg_lib {
    namespace net {
        class EventLoop;

        class InetAddress;

        class Acceptor : noncopyable {
        public:
            typedef std::function<void(int sockfd, const InetAddress &)> NewConnectionCallback;

            Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport = true);

            ~Acceptor();

            void setNewConnectionCallback(const NewConnectionCallback &cb) { newConnectionCallback_ = cb; }

            void listen();

            bool listening() const { return listening_; }

        private:
            void handleRead();

            EventLoop *loop_;
            Socket acceptSocket_;
            Channel acceptChannel_;
            NewConnectionCallback newConnectionCallback_;
            bool listening_;
            int idleFd_;
        };
    }
}

#endif //GG_LIB_ACCEPTOR_H
