// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include <vector>
#include <memory>
#include "gg_lib/Logging.h"
#include "gg_lib/TimeZone.h"
#include "gg_lib/ThreadHelper.h"
#include "gg_lib/AsyncLogging.h"

using namespace gg_lib;

constexpr int threadNum = 8;
constexpr int n = 1000 * 1000;
constexpr int mb = 1024 * 1024;

AsyncLogging *g_asyncLog = nullptr;
AtomicInt64 g_total;

void threadFunc() {
    printf("%s\n", CurrentThread::tidString());
    for (int i = 0; i < n; ++i) {
        LOG_INFO << Fmt("{} {} {:08}", "Hello 0123456789", "abcdefghijklmnopqrstuvwxyz", i);
    }
}

void asyncOutput(const char *msg, int len) {
    g_total += len;
    g_asyncLog->append(msg, len);
}

void asyncFlush() {}

void bench() {
    std::vector<Thread> vt;
    for (int i = 0; i < threadNum; ++i) {
        vt.emplace_back(threadFunc);
    }
    Timestamp start(Timestamp::now());
    for (auto &pt: vt) {
        pt.start();
    }
    for (auto &pt: vt) {
        pt.join();
    }
    g_asyncLog->stop();
    Timestamp end(Timestamp::now());
    auto seconds = static_cast<double >(end.microSecondsSinceEpoch() - start.microSecondsSinceEpoch())
                   / Timestamp::kMicroSecondsPerSecond;
    int64_t g_total_ = g_total.load();
    printf("%f seconds, %d bytes, %10.2f msg/s, %.2f MiB/s\n",
           seconds, g_total_, n * threadNum / seconds, g_total_ / seconds / (1024 * 1024));
}

int main() {
    Logger::setLogLevel(Logger::LogLevel::DEBUG);
    TimeZone tz("/usr/share/zoneinfo/Asia/Shanghai");
    Logger::setTimeZone(tz);

    AsyncLogging async("/tmp/AsyncLogging", 1600 * mb);
    async.start();
    g_asyncLog = &async;

    Logger::setOutput(asyncOutput);
    Logger::setFlush(asyncFlush);
    bench();
}
