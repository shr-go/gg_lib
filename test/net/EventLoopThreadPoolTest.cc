// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/EventLoopThreadPool.h"
#include "gg_lib/net/EventLoop.h"
#include "gg_lib/Logging.h"

#include <unistd.h>

using namespace gg_lib;
using namespace gg_lib::net;

void print(EventLoop *p = nullptr) {
    LOG_INFO << Fmt("pid = {}, loop = {}", getpid(), reinterpret_cast<void *>(p));
}

void init(EventLoop *p) {
    LOG_INFO << Fmt("pid = {}, loop = {}", getpid(), reinterpret_cast<void *>(p));
}

int main() {
    print();

    EventLoop loop;
    loop.runAfter(11, std::bind(&EventLoop::quit, &loop));

    {
        LOG_INFO << "Single thread " << &loop;
        EventLoopThreadPool model(&loop, "single");
        model.setThreadNum(0);
        model.start(init);
        assert(model.getNextLoop() == &loop);
        assert(model.getNextLoop() == &loop);
        assert(model.getNextLoop() == &loop);
    }

    {
        LOG_INFO << "Another thread";
        EventLoopThreadPool model(&loop, "another");
        model.setThreadNum(1);
        model.start(init);
        EventLoop *nextLoop = model.getNextLoop();
        nextLoop->runAfter(2, std::bind(print, nextLoop));
        assert(nextLoop != &loop);
        assert(nextLoop == model.getNextLoop());
        assert(nextLoop == model.getNextLoop());
        ::sleep(3);
    }

    {
        LOG_INFO << "Three thread";
        EventLoopThreadPool model(&loop, "three");
        model.setThreadNum(3);
        model.start(init);
        EventLoop *nextLoop = model.getNextLoop();
        nextLoop->runInLoop(std::bind(print, nextLoop));
        assert(nextLoop != &loop);
        assert(nextLoop != model.getNextLoop());
        assert(nextLoop != model.getNextLoop());
        assert(nextLoop == model.getNextLoop());
    }

    loop.loop();
}
