// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/http/HttpContext.h"
#include "gg_lib/net/Buffer.h"

using namespace gg_lib;
using namespace gg_lib::net;

bool HttpContext::parseRequest(Buffer *buf, Timestamp receiveTime) {
    bool ok = true;
    bool hasMore = true;
    while (hasMore) {
        const char *crlf;
        switch (state_) {
            case kExpectRequestLine:
                crlf = buf->findCRLF();
                if (crlf) {
                    string_view sv(buf->peek(), crlf - (buf->peek()));
                    if (processRequestLine(sv) && checkHeaderCallback_(request_)) {
                        request_.setReceiveTime(receiveTime);
                        buf->retrieveUntil(crlf + 2);
                        state_ = kExpectHeaders;
                    } else {
                        ok = false;
                        hasMore = false;
                    }
                } else {
                    hasMore = false;
                }
                break;
            case kExpectHeaders:
                crlf = buf->findCRLF();
                if (crlf) {
                    string_view sv(buf->peek(), crlf - (buf->peek()));
                    auto colon = sv.find(':');
                    auto valueBegin = sv.find_first_not_of(' ', colon + 1);
                    auto valueEnd = sv.find_last_not_of(' ');
                    if (colon != string_view::npos) {
                        request_.addHeader(sv.substr(0, colon), sv.substr(valueBegin, valueEnd - valueBegin + 1));
                    } else {
                        // The header end.
                        // TODO: Add http body
                        state_ = kGotAll;
                        hasMore = false;
                    }
                    buf->retrieveUntil(crlf + 2);
                } else {
                    hasMore = false;
                }
            case kExpectBody:
                break;
            case kGotAll:
                break;
        }
    }
    return ok;
}

bool HttpContext::processRequestLine(string_view sv) {
    bool succeed = false;
    auto space = sv.find(' ');
    if (space != string_view::npos && request_.setMethod(sv.substr(0, space))) {
        auto start = space + 1;
        space = sv.find(' ', start);
        if (space != string_view::npos) {
            auto pathQuery = sv.substr(start, space - start);
            auto questionMark = pathQuery.find('?');
            if (questionMark != string_view::npos) {
                request_.setPath(pathQuery.substr(0, questionMark));
                request_.setQuery(pathQuery.substr(questionMark));
            } else {
                request_.setPath(pathQuery);
            }
            auto version = sv.substr(space + 1);
            if (version.length() == 8 && version.starts_with("HTTP/1.")) {
                succeed = true;
                if (version[7] == '1') {
                    request_.setVersion(HttpRequest::kHttp11);
                } else if (version[7] == '0') {
                    request_.setVersion(HttpRequest::kHttp10);
                } else {
                    succeed = false;
                }
            }
        }
    }
    return succeed;
}
