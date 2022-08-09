// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/http/HttpServer.h"
#include "gg_lib/net/http/HttpContext.h"
#include "gg_lib/net/http/HttpResponse.h"
#include "gg_lib/Logging.h"

#include <utility>

using namespace gg_lib;
using namespace gg_lib::net;

HttpServer::HttpServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       string name,
                       TcpServer::Option option)
        : server_(loop, listenAddr, std::move(name), option),
          getCallback_() {
    server_.setConnectionCallback([this](const TcpConnectionPtr &conn) {
        this->onConnection(conn);
    });
    server_.setMessageCallback([this](const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime) {
        this->onMessage(conn, buf, receiveTime);
    });
}

void HttpServer::start() {
    LOG_INFO << Fmt("HttpServer[{}] starts listening on {}", server_.name(), server_.ipPort());
    server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr &conn) {
    if (conn->connected()) {
        conn->setContext(std::make_shared<HttpContext>([this](const HttpRequest &request) -> bool {
            auto method = request.getMethod();
            if (method == HttpRequest::kGet) {
                const string &path = request.getPath();
                if (this->checkGetCallback(path)) {
                    return true;
                }
            }
            return false;
        }));
    }
    conn->setHighWaterMarkCallback(optionalHighWaterMarkCallback, 256 * 1024);
}

void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime) {
    auto &context = any_cast<std::shared_ptr<HttpContext> &>(conn->getContext());
    Buffer outBuf;
    HttpResponse response;
    bool needClose = false;
    while (!needClose) {
        if (!context->parseRequest(buf, receiveTime)) {
            outBuf.append("HTTP/1.1 400 Bad Request\r\n\r\n");
            context->reset();
            needClose = true;
        } else if (context->gotAll()) {
            onRequest(context->request(), &response);
            response.appendToBuffer(&outBuf);
            context->reset();
            if (response.getCloseConnection()) {
                needClose = true;
            }
            response.reset();
        } else {
            break;
        }
    }
    conn->send(std::move(outBuf));
    if (needClose) {
        conn->forceClose();
    }
}

void HttpServer::onRequest(const HttpRequest &req, HttpResponse *resp) {
    const string &connection = req.getHeader("Connection");
    bool close = connection == "close" ||
                 (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
    resp->setCloseConnection(close);
    auto method = req.getMethod();
    if (method == HttpRequest::kGet) {
        const auto &cb = getCallback(req.getPath());
        if (cb) {
            cb(req, resp);
        }
    }
}


