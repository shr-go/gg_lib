// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include <unistd.h>
#include "gg_lib/net/EventLoop.h"
#include "gg_lib/net/TcpServer.h"
#include "gg_lib/Logging.h"
#include "gg_lib/AsyncLogging.h"
#include "gg_lib/TimeZone.h"

using namespace gg_lib;
using namespace gg_lib::net;

int numThreads = 0;

class EchoServer {
public:
    EchoServer(EventLoop *loop, const InetAddress &listenAddr)
            : loop_(loop),
              server_(loop, listenAddr, "EchoServer") {
        server_.setConnectionCallback(&EchoServer::onConnection);
        server_.setMessageCallback(&EchoServer::onMessage);
        server_.setThreadNum(numThreads);
    }

    void start() {
        server_.start();
    }

    void debug() {
#ifndef NDEBUG
        server_.checkConnAlive();
#endif
    }

private:
    static void onConnection(const TcpConnectionPtr &conn) {
        LOG_TRACE << Fmt("{} -> {} is {}",
                         conn->peerAddress().toIpPort(),
                         conn->localAddress().toIpPort(),
                         conn->connected() ? "UP" : "DOWN");
        conn->setHighWaterMarkCallback(&EchoServer::onHighWaterMark, 256 * 1024);
//        conn->send("hello\n");
    }

    static void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time) {
        LOG_TRACE << Fmt("{} recv {} bytes at {}",
                         conn->name(), buf->readableBytes(), time.toString());
        conn->send(buf);
    }

    static void onHighWaterMark(const TcpConnectionPtr &conn, size_t size) {
        conn->stopRead();
        conn->setWriteCompleteCallback(&EchoServer::onWriteComplete);
    }

    static void onWriteComplete(const TcpConnectionPtr& conn) {
        conn->startRead();
        conn->setWriteCompleteCallback(WriteCompleteCallback());
    }

    EventLoop *loop_;
    TcpServer server_;
};

EventLoop *gLoop = nullptr;

void quit() {
    gLoop->quit();
}

int main(int argc, char **argv) {
//    TimeZone tz("/usr/share/zoneinfo/Asia/Shanghai");
//    Logger::setTimeZone(tz);
//    AsyncLogging async("/tmp/EchoServer", 1 << 30);
//    async.start();
//    Logger::setOutput([&async](const char *msg, int len) { async.append(msg, len); });
    Logger::setOutput([](const char*, int) {});
    Logger::setFlush([] {});

    LOG_INFO << "pid = " << getpid() << ", thread = " << CurrentThread::tidString();
    LOG_INFO << "sizeof TcpConnection = " << sizeof(TcpConnection);
    if (argc > 1) {
        numThreads = atoi(argv[1]);
    }
    bool ipv6 = argc > 2;
    {
        EventLoop loop;
        gLoop = &loop;
        InetAddress listenAddr(9999, false, ipv6);
        EchoServer server(&loop, listenAddr);

        server.start();
//        loop.runEvery(2, std::bind(&EchoServer::debug, &server));
        loop.loop();
    }
}
