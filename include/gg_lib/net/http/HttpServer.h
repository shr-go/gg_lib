// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_HTTPSERVER_H
#define GG_LIB_HTTPSERVER_H

#include "gg_lib/net/TcpServer.h"
#include <unordered_map>

namespace gg_lib {
    namespace net {
        class HttpRequest;

        class HttpResponse;

        class HttpServer : noncopyable {
        public:
            typedef std::function<void(const HttpRequest &, HttpResponse *)> HttpCallback;

            HttpServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       string name,
                       TcpServer::Option option = TcpServer::kNoReusePort);

            EventLoop *getLoop() const { return server_.getLoop(); }

            void setGetCallback(StringArg path, HttpCallback cb) {
                getCallback_[path.c_str()] = std::move(cb);
            }

            bool checkGetCallback(StringArg path) {
                return getCallback_.find(path.c_str()) != getCallback_.end();
            }

            const HttpCallback& getGetCallback(StringArg path) {
                static HttpCallback emptyHttpCallback;
                auto iter = getCallback_.find(path.c_str());
                if (iter != getCallback_.end()) {
                    return iter->second;
                }
                return emptyHttpCallback;
            }

            void setThreadNum(int numThreads) {
                server_.setThreadNum(numThreads);
            }

            void start();

        private:
            void onConnection(const TcpConnectionPtr &conn);

            void onMessage(const TcpConnectionPtr &conn,
                           Buffer *buf,
                           Timestamp receiveTime);

            void onRequest(const TcpConnectionPtr &, const HttpRequest &);

            TcpServer server_;
            std::unordered_map<string, HttpCallback> getCallback_;
            // TODO add post method support
        };
    }
}

#endif //GG_LIB_HTTPSERVER_H
