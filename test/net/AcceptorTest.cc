// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/EventLoop.h"
#include "gg_lib/net/SocketsHelper.h"
#include "gg_lib/net/Acceptor.h"

#include "gg_lib/Logging.h"

using namespace gg_lib;
using namespace gg_lib::net;

EventLoop* g_loop;

void newConnection1(int sockfd, const InetAddress& peerAddr) {
    LOG_INFO << Fmt("accepted a new connection from {}", peerAddr.toIpPort());
    sockets::write(sockfd, "Listen on 9981\n", 15);
    sockets::close(sockfd);
}

void newConnection2(int sockfd, const InetAddress& peerAddr) {
    LOG_INFO << Fmt("accepted a new connection from {}", peerAddr.toIpPort());
    sockets::write(sockfd, "Listen on 9982\n", 15);
    sockets::close(sockfd);
    g_loop->quit();
}

int main() {
    Logger::setLogLevel(Logger::TRACE);
    EventLoop loop;
    g_loop = &loop;

    InetAddress listenAddr1(9981);
    Acceptor acceptor1(&loop, listenAddr1);
    acceptor1.setNewConnectionCallback(newConnection1);
    acceptor1.listen();

    InetAddress listenAddr2(9982);
    Acceptor acceptor2(&loop, listenAddr2);
    acceptor2.setNewConnectionCallback(newConnection2);
    acceptor2.listen();

    loop.loop();
    loop.loop();
}
