// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/http/HttpResponse.h"
#include "gg_lib/net/Buffer.h"

using namespace gg_lib;
using namespace gg_lib::net;


namespace gg_lib {
    namespace net {
        __thread char t_dateTime[64];
        __thread time_t t_dateLastSecond;
    }
}

static inline void refreshDateTime() {
    int64_t microSecondsSinceEpoch = Timestamp::now().microSecondsSinceEpoch();
    auto seconds = static_cast<time_t>(microSecondsSinceEpoch / Timestamp::kMicroSecondsPerSecond);
    if (seconds != t_dateLastSecond) {
        t_dateLastSecond = seconds;
        struct tm tm_time;
        gmtime_r(&seconds, &tm_time);
        size_t len = strftime(t_dateTime, sizeof(t_dateTime), "Date: %a, %d %b %Y %H:%M:%S GMT\r\n", &tm_time);
        assert(len == 37);
    }
}

void HttpResponse::appendToBuffer(Buffer *output) const {
    char buf[32];
    snprintf(buf, sizeof buf, "HTTP/1.1 %d ", statusCode_);
    output->append(buf);
    output->append(statusMessage_);
    output->append("\r\n");
    if (closeConnection_) {
        output->append("Connection: close\r\n");
    } else {
        snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", body_.size());
        output->append(buf);
        output->append("Connection: Keep-Alive\r\n");
    }
    for (const auto &header: headers_) {
        output->append(header.first);
        output->append(": ");
        output->append(header.second);
        output->append("\r\n");
    }
    refreshDateTime();
    output->append(t_dateTime);
    output->append("\r\n");
    output->append(body_);
}
