// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/http/HttpServer.h"
#include "gg_lib/net/http/HttpRequest.h"
#include "gg_lib/net/http/HttpResponse.h"
#include "gg_lib/net/EventLoop.h"
#include "gg_lib/Logging.h"

using namespace gg_lib;
using namespace gg_lib::net;

int main(int argc, char **argv) {
    Logger::setOutput([](const char*, int) {});
    Logger::setFlush([] {});
    Logger::setLogLevel(Logger::WARN);

    int numThreads = 0;
    if (argc > 1) {
        numThreads = atoi(argv[1]);
    }
    EventLoop loop;
    HttpServer server(&loop, InetAddress(3012), "dummy");
    server.setGetCallback("/", [](const HttpRequest& req, HttpResponse* resp){
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/plain");
        resp->addHeader("Server", "gg_lib");
        resp->setBody("hello, world!\n");
    });
    server.setThreadNum(numThreads);
    server.start();
    loop.loop();
}
