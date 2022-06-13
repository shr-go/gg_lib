// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/Timestamp.h"
#include <sys/time.h>

using namespace gg_lib;

string Timestamp::toString(bool Simplify) const {
    char buf[64] = {};
    auto seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);
    if (Simplify) {
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    } else {
        int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
                 microseconds);
    }
    return buf;
}

Timestamp Timestamp::now() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return Timestamp(tv.tv_sec * kMicroSecondsPerSecond + tv.tv_usec);
}
