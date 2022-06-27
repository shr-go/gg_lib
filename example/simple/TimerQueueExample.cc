// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/EventLoop.h"
#include "gg_lib/net/TimerQueue.h"
#include "gg_lib/Logging.h"

#include <stdio.h>
#include <unistd.h>
#include <random>

using namespace gg_lib;
using namespace gg_lib::net;

int cnt = 0;
EventLoop *g_loop;

void addCnt(int n) {
    cnt += n;
    g_loop->runAfter(1, std::bind(&addCnt, 1));
}

void print() {
    LOG_INFO << Fmt("Cnt = {}", cnt);
}

void stop(TimerId timer) {
    LOG_INFO << Fmt("Stop Timer = {}", timer);
    g_loop->cancel(timer);
}

void cancel() {
    LOG_INFO << "Cancel loop";
    g_loop->quit();
}

int main() {
    EventLoop loop;
    g_loop = &loop;
    for (int i = 0; i < 2000000; ++i) {
        loop.runAfter((1.0 / (i + 1)), std::bind(&addCnt, 1));
    }
    TimerId timer = loop.runEvery(1, &print);
    LOG_INFO << Fmt("Begin Timer = {}", timer);
    loop.runAfter(14, std::bind(&stop, timer));
    loop.runAfter(16, &cancel);
    loop.loop();
    LOG_INFO << "main loop exits";
}
