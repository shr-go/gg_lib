// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_TCPCONNECTION_H
#define GG_LIB_TCPCONNECTION_H

#include "gg_lib/net/NetUtils.h"
#include "gg_lib/net/SocketsHelper.h"
#include "gg_lib/net/Buffer.h"
#include "gg_lib/any.h"

#include <memory>

namespace gg_lib {
    namespace net {
        class Channel;

        class EventLoop;

        class Socket;

        class TcpConnection : noncopyable,
                              public std::enable_shared_from_this<TcpConnection> {
        public:
            TcpConnection(EventLoop *loop,
                          string name,
                          int sockfd,
                          const InetAddress &localAddr,
                          const InetAddress &peerAddr);

            ~TcpConnection();

            EventLoop *getLoop() const { return loop_; }

            const string &name() const { return name_; }

            const InetAddress &localAddress() const { return localAddr_; }

            const InetAddress &peerAddress() const { return peerAddr_; }

            bool connected() const { return state_ == kConnected; }

            bool disconnected() const { return state_ == kDisconnected; }

            bool getTcpInfo(struct tcp_info *tcpi) const { return socket_->getTcpInfo(tcpi); }

            void setTcpNoDelay(bool on) { socket_->setTcpNoDelay(on); }

            void setContext(any context) { context_ = std::move(context); }

            const any &getContext() const { return context_; }

            any &getContext() { return context_; }

            void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }

            void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }

            void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }

            void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, size_t highWaterMark) {
                highWaterMarkCallback_ = cb;
                highWaterMark_ = highWaterMark;
            }

            Buffer *inputBuffer() { return &inputBuffer_; }

            Buffer *outputBuffer() { return &outputBuffer_; }

            string getTcpInfoString() const;

            void send(const char *msg) { send(string_view(msg)); }

            void send(string &&message);

            void send(const void *message, size_t len);

            void send(const string_view &message);

            void send(Buffer &&message);

            void send(Buffer *message);

            void shutdown();

            void shutdownAndForceCloseAfter(double seconds);

            void forceClose();

            void forceCloseWithDelay(double seconds);

            void startRead();

            void stopRead();

            bool isReading() const { return reading_; };

            /// Internal use only.
            void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }

            // called when TcpServer accepts a new connection
            void connectEstablished();

            // called when TcpServer remove this connection in TcpServer map.
            void connectDestroyed();

        private:
            enum StateE {
                kDisconnected, kConnecting, kConnected, kDisconnecting
            };

            void handleRead(Timestamp receiveTime);

            void handleWrite();

            void handleClose();

            void handleError();

            void sendInLoop(const string_view &message);

            void sendInLoop(Buffer &message);

            void sendInLoop(const void *message, size_t len);

            void shutdownInLoop();

            void shutdownAndForceCloseInLoop(double seconds);

            void forceCloseInLoop();

            void setState(StateE s) { state_ = s; }

            const char *stateToString() const;

            void startReadInLoop();

            void stopReadInLoop();

            EventLoop *loop_;
            const string name_;
            std::atomic<StateE> state_;
            bool reading_;

            std::unique_ptr<Socket> socket_;
            std::unique_ptr<Channel> channel_;
            const InetAddress localAddr_;
            const InetAddress peerAddr_;
            ConnectionCallback connectionCallback_;
            MessageCallback messageCallback_;
            WriteCompleteCallback writeCompleteCallback_;
            HighWaterMarkCallback highWaterMarkCallback_;
            CloseCallback closeCallback_;
            size_t highWaterMark_;
            Buffer inputBuffer_;
            Buffer outputBuffer_;
            any context_;
        };
    }
}


#endif //GG_LIB_TCPCONNECTION_H
