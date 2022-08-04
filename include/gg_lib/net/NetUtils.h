// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_NETUTILS_H
#define GG_LIB_NETUTILS_H

#include "gg_lib/Utils.h"
#include "gg_lib/Timestamp.h"
#include <functional>
#include <memory>


namespace gg_lib {
    namespace net {
        class Buffer;

        class TcpConnection;

        typedef uint64_t TimerId;

        typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
        typedef std::function<void()> TimerCallback;
        typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
        typedef std::function<void(const TcpConnectionPtr &)> CloseCallback;
        typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;
        typedef std::function<void(const TcpConnectionPtr &, size_t)> HighWaterMarkCallback;

        typedef std::function<void(const TcpConnectionPtr &,
                                   Buffer *,
                                   Timestamp)> MessageCallback;

        void defaultConnectionCallback(const TcpConnectionPtr &conn);

        void defaultMessageCallback(const TcpConnectionPtr &conn,
                                    Buffer *buffer,
                                    Timestamp receiveTime);

        void optionalHighWaterMarkCallback(const TcpConnectionPtr &conn, size_t);
        void optionalDoneHighWaterMarkCallback(const TcpConnectionPtr &conn);
    }
}

#endif //GG_LIB_NETUTILS_H
