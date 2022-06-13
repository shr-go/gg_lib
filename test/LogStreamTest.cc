// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/LogStream.h"
#include <gtest/gtest.h>

using namespace gg_lib;

TEST(LogStreamTest, TestBooleans) {
    LogStream os;
    auto& buf = os.buffer();
    EXPECT_EQ(buf.toString(), "");
    os << true;
    EXPECT_EQ(buf.toString(), "true");
    os << '\n' << false;
    EXPECT_EQ(buf.toString(), "true\nfalse");
    os << '\n';
    os << Fmt("{}-{}", true, false);
    EXPECT_EQ(buf.toString(), "true\nfalse\ntrue-false");
}

TEST(LogStreamTest, TestInteger) {
    LogStream os;
    auto& buf = os.buffer();
    EXPECT_EQ(buf.toString(), "");
    os << 100;
    EXPECT_EQ(buf.toString(), "100");
    os << 0;
    EXPECT_EQ(buf.toString(), "1000");
    os << -100;
    EXPECT_EQ(buf.toString(), "1000-100");
    os << Fmt("\n{}{}", 100, -100);
    EXPECT_EQ(buf.toString(), "1000-100\n100-100");
}

TEST(LogStreamTest, TestIntegerLimit) {
    LogStream os;
    auto& buf = os.buffer();
    os << -2147483647;
    EXPECT_EQ(buf.toString(), string("-2147483647"));
    os << static_cast<int>(-2147483647 - 1);
    EXPECT_EQ(buf.toString(), string("-2147483647-2147483648"));
    os << ' ';
    os << 2147483647;
    EXPECT_EQ(buf.toString(), string("-2147483647-2147483648 2147483647"));
    os.resetBuffer();

    os << std::numeric_limits<int16_t>::min();
    EXPECT_EQ(buf.toString(), string("-32768"));
    os.resetBuffer();

    os << std::numeric_limits<int16_t>::max();
    EXPECT_EQ(buf.toString(), string("32767"));
    os.resetBuffer();

    os << std::numeric_limits<uint16_t>::min();
    EXPECT_EQ(buf.toString(), string("0"));
    os.resetBuffer();

    os << std::numeric_limits<uint16_t>::max();
    EXPECT_EQ(buf.toString(), string("65535"));
    os.resetBuffer();

    os << std::numeric_limits<int32_t>::min();
    EXPECT_EQ(buf.toString(), string("-2147483648"));
    os.resetBuffer();

    os << std::numeric_limits<int32_t>::max();
    EXPECT_EQ(buf.toString(), string("2147483647"));
    os.resetBuffer();

    os << std::numeric_limits<uint32_t>::min();
    EXPECT_EQ(buf.toString(), string("0"));
    os.resetBuffer();

    os << std::numeric_limits<uint32_t>::max();
    EXPECT_EQ(buf.toString(), string("4294967295"));
    os.resetBuffer();

    os << std::numeric_limits<int64_t>::min();
    EXPECT_EQ(buf.toString(), string("-9223372036854775808"));
    os.resetBuffer();

    os << std::numeric_limits<int64_t>::max();
    EXPECT_EQ(buf.toString(), string("9223372036854775807"));
    os.resetBuffer();

    os << std::numeric_limits<uint64_t>::min();
    EXPECT_EQ(buf.toString(), string("0"));
    os.resetBuffer();

    os << std::numeric_limits<uint64_t>::max();
    EXPECT_EQ(buf.toString(), string("18446744073709551615"));
    os.resetBuffer();

    int16_t a = 0;
    int32_t b = 0;
    int64_t c = 0;
    os << a << b << c;
    EXPECT_EQ(buf.toString(), string("000"));
}

TEST(LogStreamTest, TestFloatLimit) {
    LogStream os;
    auto& buf = os.buffer();

    os << 0.0;
    EXPECT_EQ(buf.toString(), "0");
    os.resetBuffer();

    os << 1.0;
    EXPECT_EQ(buf.toString(), "1");
    os.resetBuffer();

    os << 0.11;
    EXPECT_EQ(buf.toString(), "0.11");
    os.resetBuffer();

    os << -0.11;
    EXPECT_EQ(buf.toString(), "-0.11");
    os.resetBuffer();

    double a = 0.1;
    os << a;
    EXPECT_EQ(buf.toString(), "0.1");
    os.resetBuffer();

    double b = 0.05;
    os << b;
    EXPECT_EQ(buf.toString(), "0.05");
    os.resetBuffer();

    double c = 0.15;
    os << c;
    EXPECT_EQ(buf.toString(), "0.15");
    os.resetBuffer();

    os << 1.23456789;
    EXPECT_EQ(buf.toString(), "1.23456789");
    os.resetBuffer();

    os << 1.234567;
    EXPECT_EQ(buf.toString(), "1.234567");
    os.resetBuffer();

    os << -123.456;
    EXPECT_EQ(buf.toString(), "-123.456");
    os.resetBuffer();
}

TEST(LogStreamTest, TestVoid) {
    LogStream os;
    auto& buf = os.buffer();

    os << static_cast<void*>(0);
    EXPECT_EQ(buf.toString(), "0x0");
    os.resetBuffer();

    os << reinterpret_cast<void*>(8888);
    EXPECT_EQ(buf.toString(), "0x22B8");
    os.resetBuffer();
}

TEST(LogStreamTest, TestString) {
    LogStream os;
    auto& buf = os.buffer();

    os << "Hello ";
    EXPECT_EQ(buf.toString(), "Hello ");
    os << "World";
    EXPECT_EQ(buf.toString(), "Hello World");
    os << Fmt("{}{}", " ", "Again");
    EXPECT_EQ(buf.toString(), "Hello World Again");
}