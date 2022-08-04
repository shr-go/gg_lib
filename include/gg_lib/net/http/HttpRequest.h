// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_HTTPREQUEST_H
#define GG_LIB_HTTPREQUEST_H

#include "gg_lib/noncopyable.h"
#include "gg_lib/net/NetUtils.h"

#include <unordered_map>

namespace gg_lib {
    namespace net {
        class HttpRequest : noncopyable {
        public:
            enum Method {
                kInvalid, kGet, kPost, kHead, kPut, kDelete
            };
            enum Version {
                kUnknown, kHttp10, kHttp11
            };

            HttpRequest() : method_(kInvalid), version_(kUnknown) {}

            void setVersion(Version v) { version_ = v; }

            Version getVersion() const { return version_;};

            bool setMethod(string_view m) {
//                assert(method_ == kInvalid);
                if (m == "GET") {
                    method_ = kGet;
                } else if (m == "POST") {
                    method_ = kPost;
                } else if (m == "HEAD") {
                    method_ = kHead;
                } else if (m == "PUT") {
                    method_ = kPut;
                } else if (m == "HEAD") {
                    method_ = kHead;
                }
                return method_ != kInvalid;
            }

            Method getMethod() const { return method_; }

            const char *methodString() const {
                const char *result;
                switch (method_) {
                    case kGet:
                        result = "GET";
                        break;
                    case kPost:
                        result = "POST";
                        break;
                    case kHead:
                        result = "HEAD";
                        break;
                    case kPut:
                        result = "PUT";
                        break;
                    case kDelete:
                        result = "DELETE";
                        break;
                    default:
                        result = "UNKNOWN";
                        break;
                }
                return result;
            }

            void setPath(string_view path) { path_ = path.to_string(); }

            const string &getPath() const { return path_; }

            void setQuery(string_view query) { query_ = query.to_string(); }

            const string &getQuery() const { return query_; }

            void setReceiveTime(Timestamp t) { receiveTime_ = t; }

            Timestamp getReceiveTime(Timestamp t) { return receiveTime_; }

            void addHeader(string_view key, string_view value) {
                headers_[key.to_string()] = value.to_string();
            }

            const string &getHeader(const string &key) const {
                static string empty;
                auto iter = headers_.find(key);
                if (iter != headers_.end())
                    return iter->second;
                return empty;
            }

            const std::unordered_map<string, string> &headers() { return headers_; }

            void swap(HttpRequest &rhs) noexcept {
                std::swap(method_, rhs.method_);
                std::swap(version_, rhs.version_);
                std::swap(path_, rhs.path_);
                std::swap(query_, rhs.query_);
                receiveTime_.swap(rhs.receiveTime_);
                std::swap(headers_, rhs.headers_);
            }

        private:
            Method method_;
            Version version_;
            string path_;
            string query_;
            Timestamp receiveTime_;
            std::unordered_map<string, string> headers_;
        };
    }
}

#endif //GG_LIB_HTTPREQUEST_H
