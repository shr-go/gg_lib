// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/Acceptor.h"
#include "gg_lib/net/EventLoop.h"
#include "gg_lib/Logging.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

using namespace gg_lib;
using namespace gg_lib::net;

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport)
        : loop_(loop),
          acceptSocket_(sockets::createNonblockingOrDie(listenAddr.family())),
          acceptChannel_(loop, acceptSocket_.fd()),
          listening_(false),
          idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
    assert(idleFd_ >= 0);
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(
            std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    sockets::close(idleFd_);
}

void Acceptor::listen() {
    loop_->assertInLoopThread();
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
    loop_->assertInLoopThread();
    for (;;) {
        InetAddress peerAddr;
        int connfd = acceptSocket_.accept(&peerAddr);
        if (connfd >= 0) {
            LOG_TRACE << Fmt("Accepts of {}", peerAddr.toIpPort());
            if (newConnectionCallback_) {
                newConnectionCallback_(connfd, peerAddr);
            } else {
                sockets::close(connfd);
            }
        } else if (errno == EAGAIN) {
            break;
        } else {
            LOG_SYSERR << "in Acceptor::handleRead";
            if (errno == EMFILE) {
                ::close(idleFd_);
                idleFd_ = ::accept(acceptSocket_.fd(), nullptr, nullptr);
                ::close(idleFd_);
                idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
            }
        }
    }
}
