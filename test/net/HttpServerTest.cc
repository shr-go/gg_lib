// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/Buffer.h"
#include "gg_lib/net/http/HttpContext.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace gg_lib;
using namespace gg_lib::net;

TEST(HttpServerTest, ParseRequestTest) {
    HttpContext context([](const HttpRequest &request) -> bool { return true; });
    Buffer input;
    input.append("GET /index.html HTTP/1.1\r\n"
                 "Host: www.google.com\r\n"
                 "\r\n");
    EXPECT_TRUE(context.parseRequest(&input, Timestamp::now()));
    EXPECT_TRUE(context.gotAll());

    const HttpRequest& request = context.request();

    EXPECT_EQ(request.getMethod(), HttpRequest::kGet);
    EXPECT_EQ(request.getHeader("Host"), string("www.google.com"));
    EXPECT_EQ(request.getPath(), string("/index.html"));
    EXPECT_EQ(request.getVersion(), HttpRequest::kHttp11);
    EXPECT_EQ(request.getHeader("User-Agent"), string(""));
}

TEST(HttpServerTest, ParseRequestMultiTest) {
    string all("GET /index.html HTTP/1.1\r\n"
               "Host: www.google.com\r\n"
               "\r\n");
    for (size_t sz1 = 0; sz1 < all.size(); ++sz1) {
        HttpContext context([](const HttpRequest &request) -> bool { return true; });
        Buffer input;
        input.append(all.c_str(), sz1);

        EXPECT_TRUE(context.parseRequest(&input, Timestamp::now()));
        EXPECT_TRUE(!context.gotAll());

        size_t sz2 = all.size() - sz1;
        input.append(all.c_str() + sz1, sz2);

        EXPECT_TRUE(context.parseRequest(&input, Timestamp::now()));
        EXPECT_TRUE(context.gotAll());
        const HttpRequest& request = context.request();

        EXPECT_EQ(request.getMethod(), HttpRequest::kGet);
        EXPECT_EQ(request.getHeader("Host"), string("www.google.com"));
        EXPECT_EQ(request.getPath(), string("/index.html"));
        EXPECT_EQ(request.getVersion(), HttpRequest::kHttp11);
        EXPECT_EQ(request.getHeader("User-Agent"), string(""));
    }
}


