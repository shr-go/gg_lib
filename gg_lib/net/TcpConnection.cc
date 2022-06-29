// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/TcpConnection.h"
#include "gg_lib/net/EventLoop.h"
#include "gg_lib/net/Chnanel.h"

#include <utility>

#include "gg_lib/Logging.h"
#include "gg_lib/WeakCallback.h"

using namespace gg_lib;
using namespace gg_lib::net;

void gg_lib::net::defaultConnectionCallback(const TcpConnectionPtr &conn) {
    LOG_TRACE << Fmt("{} -> {} is {}",
                     conn->localAddress().toIpPort(),
                     conn->peerAddress().toIpPort(),
                     conn->connected() ? "UP" : "DOWN");
}

void gg_lib::net::defaultMessageCallback(const TcpConnectionPtr &conn,
                                         Buffer *buffer,
                                         Timestamp receiveTime) {
    buffer->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop *loop,
                             string name,
                             int sockfd,
                             const InetAddress &localAddr,
                             const InetAddress &peerAddr)
        : loop_(CHECK_NOTNULL(loop)),
          name_(std::move(name)),
          state_(kConnecting),
          reading_(true),
          socket_(new Socket(sockfd)),
          channel_(new Channel(loop, sockfd)),
          localAddr_(localAddr),
          peerAddr_(peerAddr),
          highWaterMark_(8 * 1024 * 1024) {
    channel_->setReadCallback(
            std::bind(&TcpConnection::handleRead, this, _1));
    channel_->setWriteCallback(
            std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(
            std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(
            std::bind(&TcpConnection::handleError, this));
    LOG_DEBUG << Fmt("TcpConnection::ctor[{}] at {} fd={}",
                     name_, static_cast<void *>(this), sockfd);
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
    LOG_DEBUG << Fmt("TcpConnection::dtor[{}] at fd={} statr={}",
                     name_, channel_->fd(), stateToString());
    assert(state_ == kDisconnected);
}

string TcpConnection::getTcpInfoString() const {
    char buf[1024]{};
    socket_->getTcpInfoString(buf, sizeof buf - 1);
    return buf;
}

/// FIXME consider using weak callback?
void TcpConnection::send(string &&message) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(string_view(message));
        } else {
            void (TcpConnection::*fp)(const string_view &message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(
                    std::bind(fp, this, std::move(message)));
        }
    }
}

void TcpConnection::send(const void *message, size_t len) {
    send(string_view(static_cast<const char *>(message), len));
}

void TcpConnection::send(const string_view &message) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message);
        } else {
            /// here we copy the message since sendInLoop won't call immediately.
            void (TcpConnection::*fp)(const string_view &message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(
                    std::bind(fp, this, message.to_string()));
        }
    }
}

void TcpConnection::send(Buffer &&message) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message.peek(), message.readableBytes());
        } else {
            void (TcpConnection::*fp)(Buffer &message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(
                    std::bind(fp, this, std::move(message)));
        }
    }
}

void TcpConnection::send(Buffer *message) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message->peek(), message->readableBytes());
            message->retrieveAll();
        } else {
            void (TcpConnection::*fp)(const string_view &message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(
                    std::bind(fp, this, message->retrieveAllAsString()));
        }
    }
}

void TcpConnection::shutdown() {
    if (state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if (!channel_->isWriting()) {
        socket_->shutdownWrite();
    }
}

void TcpConnection::shutdownAndForceCloseAfter(double seconds) {
    if (state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownAndForceCloseInLoop, this, seconds));
    }
}

void TcpConnection::shutdownAndForceCloseInLoop(double seconds) {
    loop_->assertInLoopThread();
    if (!channel_->isWriting()) {
        socket_->shutdownWrite();
    }
    loop_->runAfter(
            seconds,
            makeWeakCallback(weak_from_this(),
                             &TcpConnection::forceCloseInLoop));
}

void TcpConnection::forceClose() {
    if (state_ == kConnected || state_ == kDisconnecting) {
        setState(kDisconnecting);
        loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::forceCloseWithDelay(double seconds) {
    if (state_ == kConnected || state_ == kDisconnecting) {
        setState(kDisconnecting);
        loop_->runAfter(
                seconds,
                makeWeakCallback(shared_from_this(),
                                 &TcpConnection::forceClose));
    }
}

void TcpConnection::forceCloseInLoop() {
    loop_->assertInLoopThread();
    if (state_ == kConnected || state_ == kDisconnecting) {
        handleClose();
    }
}

void TcpConnection::startRead() {
    loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::stopRead() {
    loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->tie(weak_from_this());
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    loop_->assertInLoopThread();
    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime) {
    loop_->assertInLoopThread();
    int saveErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &saveErrno);
    if (n > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    } else if (n == 0) {
        handleClose();
    } else {
        errno = saveErrno;
        LOG_SYSERR << "TcpConnection::handleRead";
        handleError();
    }
}

void TcpConnection::handleWrite() {
    loop_->assertInLoopThread();
    if (channel_->isWriting()) {
        ssize_t n = sockets::write(channel_->fd(),
                                   outputBuffer_.peek(),
                                   outputBuffer_.readableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (writeCompleteCallback_) {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        } else {
            LOG_SYSERR << "TcpConnection::handleWrite";
        }
    } else {
        LOG_TRACE << Fmt("Connection fd = {} is down, no more writing", channel_->fd());
    }
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    LOG_TRACE << Fmt("fd = {} state = {}", channel_->fd(), stateToString());
    assert(state_ == kConnected || state_ == kDisconnecting);
    setState(kDisconnected);
    channel_->disableAll();
    TcpConnectionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);
    closeCallback_(guardThis);
}

void TcpConnection::handleError() {
    int err = sockets::getSocketError(channel_->fd());
    LOG_ERROR << Fmt("TcpConnection::handleError [{}] - SO_ERROR = {} {}",
                     name_, err, strerror_tr(err));
}

void TcpConnection::sendInLoop(const string_view &message) {
    sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(Buffer &message) {
    sendInLoop(message.peek(), message.readableBytes());
    message.retrieveAll();
}

void TcpConnection::sendInLoop(const void *message, size_t len) {
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if (state_ == kDisconnected) {
        LOG_WARN << "disconnected, give up writing";
        return;
    }
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = sockets::write(channel_->fd(), message, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_) {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_SYSERR << "TcpConnection::sendInLoop";
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }
    assert(remaining <= len);
    if (!faultError && remaining > 0) {
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_) {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        outputBuffer_.append(static_cast<const char *>(message) + nwrote, remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

const char *TcpConnection::stateToString() const {
    StateE state = state_;
    switch (state) {
        case kDisconnected:
            return "kDisconnected";
        case kConnecting:
            return "kConnecting";
        case kConnected:
            return "kConnected";
        case kDisconnecting:
            return "kDisconnecting";
        default:
            return "unknown state";
    }
}

void TcpConnection::startReadInLoop() {
    loop_->assertInLoopThread();
    if (!reading_ || !channel_->isReading()) {
        channel_->enableReading();
        reading_ = true;
    }
}

void TcpConnection::stopReadInLoop() {
    loop_->assertInLoopThread();
    if (reading_ || channel_->isReading()) {
        channel_->disableReading();
        reading_ = false;
    }
}
