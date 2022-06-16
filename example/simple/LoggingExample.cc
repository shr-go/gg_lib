// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include <vector>
#include <memory>
#include "gg_lib/Logging.h"
#include "gg_lib/TimeZone.h"
#include "gg_lib/ThreadHelper.h"
#include "gg_lib/FileUtil.h"

using namespace gg_lib;

AtomicInt32 g_total;

void dummyOut(const char*, int len) {
    g_total += len;
}

void dummyFlush() {}

void threadFunc() {
    int n = 1000 * 1000;
    printf("%s\n", CurrentThread::tidString());

    for (int i = 0; i < n; ++i) {
        LOG_INFO
            << Fmt("{} {} {} {} {:030}", "Hello 0123456789", "abcdefghijklmnopqrstuvwxyz", "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
                   "ABCDEFGHIJKLMNopqrstuvwxyz", i);
    }
}

int main() {
    using namespace std::placeholders;
    Logger::setLogLevel(Logger::LogLevel::DEBUG);
    TimeZone tz("/usr/share/zoneinfo/Asia/Shanghai");
    Logger::setTimeZone(tz);

    Logger::setOutput(&dummyOut);
    Logger::setFlush(&dummyFlush);
    int n = 1000 * 1000;
    Timestamp start(Timestamp::now());

    std::vector<Thread> vt;
    for (int i = 0; i < 8; ++i) {
        vt.emplace_back(threadFunc);
    }
    for (auto &pt: vt) {
        pt.start();
    }
    for (auto &pt: vt) {
        pt.join();
    }

    Timestamp end(Timestamp::now());
    auto seconds = static_cast<double >(end.microSecondsSinceEpoch() - start.microSecondsSinceEpoch())
                   / Timestamp::kMicroSecondsPerSecond;
    int32_t g_total_ = g_total.load();
    printf("%f seconds, %d bytes, %10.2f msg/s, %.2f MiB/s\n",
           seconds, g_total_, n / seconds, g_total_ / seconds / (1024 * 1024));

}
