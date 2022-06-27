// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/EventLoop.h"
#include "gg_lib/Logging.h"

#include <unistd.h>

using namespace gg_lib;
using namespace gg_lib::net;

void callback() {
    LOG_INFO << "pid = " << getpid();
    // will abort due to an EventLoop already exists.
    EventLoop anotherLoop;
}

void threadFunc() {
    LOG_INFO << "pid = " << getpid();
    assert(EventLoop::getEventLoopOfCurrentThread() == nullptr);
    EventLoop loop;
    assert(EventLoop::getEventLoopOfCurrentThread() == &loop);
    loop.runAfter(1.0, callback);
    loop.loop();
}

int main() {
    LOG_INFO << "pid = " << getpid();
    assert(EventLoop::getEventLoopOfCurrentThread() == nullptr);
    EventLoop loop;
    assert(EventLoop::getEventLoopOfCurrentThread() == &loop);

    Thread thread(threadFunc);
    thread.start();
    loop.loop();
}
