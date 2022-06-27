// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/EventLoopThread.h"
#include "gg_lib/net/EventLoop.h"
#include "gg_lib/Logging.h"

#include <unistd.h>

using namespace gg_lib;
using namespace gg_lib::net;

void print(EventLoop *p = nullptr) {
    LOG_INFO << Fmt("pid = {}, loop = {}", getpid(), reinterpret_cast<void*>(p));
}

void quit(EventLoop *p) {
    print(p);
    p->quit();
}

int main() {
    LOG_INFO << Fmt("pid = {}", getpid());
    {
        EventLoopThread thr1;  // never start
    }
    {
        EventLoopThread thr2;
        EventLoop* loop = thr2.startLoop();
        loop->runInLoop(std::bind(print, loop));
        std::this_thread::sleep_for(std::chrono::microseconds(500 * 1000));
    }
    {
        EventLoopThread thr3;
        EventLoop* loop = thr3.startLoop();
        loop->runInLoop(std::bind(quit, loop));
        std::this_thread::sleep_for(std::chrono::microseconds(500 * 1000));
    }
}
