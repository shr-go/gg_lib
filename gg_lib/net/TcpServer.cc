// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/TcpServer.h"
#include "gg_lib/net/Acceptor.h"
#include "gg_lib/net/EventLoop.h"
#include "gg_lib/net/EventLoopThreadPool.h"

#include <utility>

#include "gg_lib/Logging.h"

using namespace gg_lib;
using namespace gg_lib::net;

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, string nameArg, TcpServer::Option option)
        : loop_(CHECK_NOTNULL(loop)),
          ipPort_(listenAddr.toIpPort()),
          name_(std::move(nameArg)),
          acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
          threadPool_(new EventLoopThreadPool(loop, name_)),
          connectionCallback_(defaultConnectionCallback),
          messageCallback_(defaultMessageCallback),
          nextConnId_(1) {
    acceptor_->setNewConnectionCallback(
            std::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer() {
    loop_->assertInLoopThread();
    LOG_TRACE << Fmt("TcpServer::~TcpServer [{}] destructing", name_);
    for (auto &item: connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(
                std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads) {
    assert(numThreads >= 0);
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
    if (started_.exchange(1) == 0) {
        threadPool_->start(threadInitCallback_);
        assert(!acceptor_->listening());
        loop_->runInLoop(
                std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
    loop_->assertInLoopThread();
    EventLoop *ioLoop = threadPool_->getNextLoop();
    string connName = fmt::format("{}-{}#{}", name_, ipPort_, nextConnId_++);
    LOG_INFO << Fmt("TcpServer::newConnection [{}] - new connection [{}] from {}",
                    name_, connName, peerAddr.toIpPort());
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    TcpConnectionPtr conn =
            std::make_shared<TcpConnection>(ioLoop, connName, sockfd, localAddr, peerAddr);
    connections_[connName] = conn;
#ifndef NDEBUG
    weakVec_.push_back(conn);
#endif
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    // FIXME: unsafe
    conn->setCloseCallback(
            std::bind(&TcpServer::removeConnection, this, _1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

/// here may be not safe cause TcpConnectionPtr may live after TcpServer destroy,
/// however, in most case, TcpServer live longer than TcpConnection.
void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    // FIXME: unsafe
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    loop_->assertInLoopThread();
    LOG_INFO << Fmt("TcpServer::removeConnectionInLoop [{}] - connection {}", name_, conn->name());
    size_t n = connections_.erase(conn->name());
    assert(n == 1);
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn));
}

#ifndef NDEBUG
void TcpServer::checkConnAlive() {
    auto iter = weakVec_.begin();
    int expired = 0, total = 0;
    while (iter != weakVec_.end()) {
        total += 1;
        auto weakPtr = *iter;
        if (weakPtr.expired()) {
            expired += 1;
        }
        ++iter;
    }
    fprintf(stderr, "expired/total = %d/%d\n", expired, total);
}
#endif
