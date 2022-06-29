// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_TCPSERVER_H
#define GG_LIB_TCPSERVER_H

#include "gg_lib/net/NetUtils.h"
#include "gg_lib/net/TcpConnection.h"

#include "gg_lib/noncopyable.h"

#include <unordered_map>
#include <utility>

namespace gg_lib {
    namespace net {
        class Acceptor;

        class EventLoop;

        class EventLoopThreadPool;

        class TcpServer : noncopyable {
        public:
            typedef std::function<void(EventLoop *)> ThreadInitCallback;
            enum Option {
                kNoReusePort,
                kReusePort,
            };

            TcpServer(EventLoop *loop,
                      const InetAddress &listenAddr,
                      string nameArg,
                      Option option = kNoReusePort);

            ~TcpServer();

            const string &ipPort() const { return ipPort_; }

            const string &name() const { return name_; }

            EventLoop *getLoop() const { return loop_; }

            /// @brief Set the number of threads for handling input.
            /// @param numThreads
            /// - 0 means all I/O in loop's thread, no thread will created, this is the default value.
            /// - 1 means all I/O in another thread.
            /// - N means a thread pool with N threads, new connections are assigned on a round-robin basis.
            void setThreadNum(int numThreads);

            void setThreadInitCallback(ThreadInitCallback cb) { threadInitCallback_ = std::move(cb); }

            std::shared_ptr<EventLoopThreadPool> threadPool() { return threadPool_; }

            /// Thread safe
            void start();

            void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); }

            void setMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb); }

            void setWriteCompleteCallback(WriteCompleteCallback cb) { writeCompleteCallback_ = std::move(cb); }
#ifndef NDEBUG
            void checkConnAlive();
#endif

        private:
            void newConnection(int sockfd, const InetAddress& peerAddr);

            /// Thread safe.
            void removeConnection(const TcpConnectionPtr& conn);

            void removeConnectionInLoop(const TcpConnectionPtr& conn);

            typedef std::unordered_map<string, TcpConnectionPtr> ConnectionMap;

            EventLoop *loop_;
            const string ipPort_;
            const string name_;
            std::unique_ptr<Acceptor> acceptor_;
            std::shared_ptr<EventLoopThreadPool> threadPool_;
            ConnectionCallback connectionCallback_;
            MessageCallback messageCallback_;
            WriteCompleteCallback writeCompleteCallback_;
            ThreadInitCallback threadInitCallback_;
            AtomicInt32 started_;
            int nextConnId_;
            ConnectionMap connections_;
#ifndef NDEBUG
            typedef std::weak_ptr<TcpConnection> WeakTcpConnectionPtr;
            typedef std::vector<WeakTcpConnectionPtr> WeakVec;
            WeakVec weakVec_;
#endif
        };
    }
}

#endif //GG_LIB_TCPSERVER_H
