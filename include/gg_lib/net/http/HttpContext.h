// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_HTTPCONTEXT_H
#define GG_LIB_HTTPCONTEXT_H

#include "gg_lib/noncopyable.h"
#include "gg_lib/net/http/HttpRequest.h"

namespace gg_lib {
    namespace net {
        class HttpContext : noncopyable {
        public:
            typedef std::function<bool(const HttpRequest &)> checkHeaderCallback;
            enum HttpRequestParseState {
                kExpectRequestLine,
                kExpectHeaders,
                kExpectBody,
                kGotAll,
            };

            explicit HttpContext(checkHeaderCallback cb_)
            : state_(kExpectRequestLine), request_(), checkHeaderCallback_(std::move(cb_)) {}

            bool parseRequest(Buffer *buf, Timestamp receiveTime);

            bool gotAll() const { return state_ == kGotAll; }

            void reset() {
                state_ = kExpectRequestLine;
                HttpRequest dummy;
                request_.swap(dummy);
            }

            const HttpRequest& request() const { return request_; }

        private:
            bool processRequestLine(string_view sv);

            HttpRequestParseState state_;
            HttpRequest request_;
            checkHeaderCallback checkHeaderCallback_;
        };
    }
}

#endif //GG_LIB_HTTPCONTEXT_H
