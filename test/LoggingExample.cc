// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/Logging.h"
#include "gg_lib/TimeZone.h"

int g_total;
FILE *out = nullptr;

void fileOutput(const char *msg, int len) {
    g_total += len;
    size_t n = fwrite(msg, 1, len, out);
}

int main() {
    using namespace gg_lib;
    out = fopen("/tmp/test.log", "w");
    Logger::setLogLevel(Logger::LogLevel::DEBUG);
    Logger::setOutput(fileOutput);
    TimeZone tz("/usr/share/zoneinfo/Asia/Shanghai");
    Logger::setTimeZone(tz);
    int n = 1000 * 1000;
    Timestamp start(Timestamp::now());
    for (int i = 0; i < n; ++i) {
        LOG_INFO << Fmt("{} {} {}", "Hello 0123456789", "abcdefghijklmnopqrstuvwxyz", i);
    }
    Timestamp end(Timestamp::now());
    auto seconds = static_cast<double >(end.microSecondsSinceEpoch() - start.microSecondsSinceEpoch())
                   / Timestamp::kMicroSecondsPerSecond;
    printf("%f seconds, %d bytes, %10.2f msg/s, %.2f MiB/s\n",
           seconds, g_total, n / seconds, g_total / seconds / (1024 * 1024));

}