// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/http/HttpResponse.h"
#include "gg_lib/Logging.h"
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
    output->append(Fmt("HTTP/1.1 {} {}\r\n", statusCode_, statusMessage_));
    if (closeConnection_) {
        output->append("Connection: close\r\n");
    } else {
        output->append(Fmt("Content-Length: {}\r\nConnection: Keep-Alive\r\n", body_.size()));
    }
    for (const auto &header: headers_) {
        output->append(Fmt("{}: {}\r\n", header.first, header.second));
    }
    refreshDateTime();
    output->append(Fmt("{}\r\n{}", t_dateTime, body_));
}

void HttpResponse::reset() {
    headers_.clear();
    statusMessage_.clear();
    body_.clear();
    statusCode_ = kUnknown;
    closeConnection_ = false;
}
