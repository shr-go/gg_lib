// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_HTTPRESPONSE_H
#define GG_LIB_HTTPRESPONSE_H

#include "gg_lib/noncopyable.h"
#include "gg_lib/net/NetUtils.h"
#include <unordered_map>
#include <utility>

namespace gg_lib {
    namespace net {
        class Buffer;

        class HttpResponse : noncopyable {
        public:
            enum HttpStatusCode {
                kUnknown,
                k200Ok = 200,
                k301MovedPermanently = 301,
                k400BadRequest = 400,
                k404NotFound = 404,
            };

            explicit HttpResponse(bool close)
                    : headers_(), statusCode_(kUnknown), closeConnection_(close),
                      statusMessage_(), body_() {}

            void setStatusCode(HttpStatusCode code) { statusCode_ = code; }

            void setStatusMessage(const string &message) { statusMessage_ = message; }

            void setCloseConnection(bool on) { closeConnection_ = on; }

            bool getCloseConnection() const { return closeConnection_; }

            void setContentType(StringArg contentType) { addHeader("Content-Type", contentType); }

            void addHeader(StringArg key, StringArg value) { headers_[key.c_str()] = value.c_str(); }

            void setBody(string body) { body_ = std::move(body); }

            void appendToBuffer(Buffer *output) const;

        private:
            std::unordered_map<string, string> headers_;
            HttpStatusCode statusCode_;
            string statusMessage_;
            string body_;
            bool closeConnection_;
        };
    }
}

#endif //GG_LIB_HTTPRESPONSE_H
