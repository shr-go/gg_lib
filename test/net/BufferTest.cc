// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/Buffer.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>

using namespace gg_lib;
using namespace gg_lib::net;

static constexpr size_t kCheapPrepend = 8;
static constexpr size_t kInitialSize = 1024;

TEST(BufferTest, AppendRetrieve) {
    Buffer buf;

    EXPECT_EQ(buf.readableBytes(), 0);
    EXPECT_EQ(buf.writableBytes(), kInitialSize);
    EXPECT_EQ(buf.prependableBytes(), kCheapPrepend);

    const std::string str(200, 'x');
    buf.append(str);
    EXPECT_EQ(buf.readableBytes(), str.size());
    EXPECT_EQ(buf.writableBytes(), kInitialSize - str.size());
    EXPECT_EQ(buf.prependableBytes(), kCheapPrepend);

    const string str2 =  buf.retrieveAsString(50);
    EXPECT_EQ(str2.size(), 50);
    EXPECT_EQ(buf.readableBytes(), str.size() - str2.size());
    EXPECT_EQ(buf.writableBytes(), kInitialSize - str.size());
    EXPECT_EQ(buf.prependableBytes(), kCheapPrepend + str2.size());
    EXPECT_EQ(str2, string(50, 'x'));

    buf.append(str);
    EXPECT_EQ(buf.readableBytes(), 2*str.size() - str2.size());
    EXPECT_EQ(buf.writableBytes(), kInitialSize - 2*str.size());
    EXPECT_EQ(buf.prependableBytes(), kCheapPrepend + str2.size());

    const string str3 =  buf.retrieveAllAsString();
    EXPECT_EQ(str3.size(), 350);
    EXPECT_EQ(buf.readableBytes(), 0);
    EXPECT_EQ(buf.writableBytes(), kInitialSize);
    EXPECT_EQ(buf.prependableBytes(), kCheapPrepend);
    EXPECT_EQ(str3, string(350, 'x'));
}

TEST(BufferTest, BufferInsideGrow) {
    Buffer buf;
    buf.append(string(800, 'y'));
    EXPECT_EQ(buf.readableBytes(), 800);
    EXPECT_EQ(buf.writableBytes(), kInitialSize-800);

    buf.retrieve(500);
    EXPECT_EQ(buf.readableBytes(), 300);
    EXPECT_EQ(buf.writableBytes(), kInitialSize-800);
    EXPECT_EQ(buf.prependableBytes(), kCheapPrepend+500);

    buf.append(string(300, 'z'));
    EXPECT_EQ(buf.readableBytes(), 600);
    EXPECT_EQ(buf.writableBytes(), kInitialSize-600);
    EXPECT_EQ(buf.prependableBytes(), kCheapPrepend);
}

TEST(BufferTest, BufferShrink) {
    Buffer buf;
    buf.append(string(2000, 'y'));
    EXPECT_EQ(buf.readableBytes(), 2000);
    EXPECT_EQ(buf.writableBytes(), 0);
    EXPECT_EQ(buf.prependableBytes(), kCheapPrepend);

    buf.retrieve(1500);
    EXPECT_EQ(buf.readableBytes(), 500);
    EXPECT_EQ(buf.writableBytes(), 0);
    EXPECT_EQ(buf.prependableBytes(), kCheapPrepend+1500);

    buf.shrink(0);
    EXPECT_EQ(buf.readableBytes(), 500);
    EXPECT_EQ(buf.writableBytes(), kInitialSize-500);
    EXPECT_EQ(buf.retrieveAllAsString(), string(500, 'y'));
    EXPECT_EQ(buf.prependableBytes(), kCheapPrepend);
}

TEST(BufferTest, BufferPrepend) {
    Buffer buf;
    buf.append(string(200, 'y'));
    EXPECT_EQ(buf.readableBytes(), 200);
    EXPECT_EQ(buf.writableBytes(), kInitialSize-200);
    EXPECT_EQ(buf.prependableBytes(), kCheapPrepend);

    int x = 0;
    buf.prepend(&x, sizeof x);
    EXPECT_EQ(buf.readableBytes(), 204);
    EXPECT_EQ(buf.writableBytes(), kInitialSize-200);
    EXPECT_EQ(buf.prependableBytes(), kCheapPrepend - 4);
}

TEST(BufferTest, BufferReadInt) {
    Buffer buf;
    buf.append("HTTP");

    EXPECT_EQ(buf.readableBytes(), 4);
    EXPECT_EQ(buf.peekInt8(), 'H');
    int top16 = buf.peekInt16();
    EXPECT_EQ(top16, 'H'*256 + 'T');
    EXPECT_EQ(buf.peekInt32(), top16*65536 + 'T'*256 + 'P');

    EXPECT_EQ(buf.readInt8(), 'H');
    EXPECT_EQ(buf.readInt16(), 'T'*256 + 'T');
    EXPECT_EQ(buf.readInt8(), 'P');
    EXPECT_EQ(buf.readableBytes(), 0);
    EXPECT_EQ(buf.writableBytes(), kInitialSize);

    buf.appendInt8(-1);
    buf.appendInt16(-2);
    buf.appendInt32(-3);
    EXPECT_EQ(buf.readableBytes(), 7);
    EXPECT_EQ(buf.readInt8(), -1);
    EXPECT_EQ(buf.readInt16(), -2);
    EXPECT_EQ(buf.readInt32(), -3);
}

TEST(BufferTest, BufferFindEOL) {
    Buffer buf;
    buf.append(string(100000, 'x'));
    const char* null = nullptr;
    EXPECT_EQ(buf.findEOL(), null);
    EXPECT_EQ(buf.findEOL(buf.peek()+90000), null);
}

void output(Buffer&& buf, const void* inner)
{
    Buffer newbuf(std::move(buf));
    EXPECT_EQ(inner, newbuf.peek());
}

TEST(BufferTest, BufferMove) {
    Buffer buf;
    buf.append("hello", 5);
    const void* inner = buf.peek();
    output(std::move(buf), inner);
}
