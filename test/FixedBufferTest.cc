// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/FixedBuffer.h"
#include <gtest/gtest.h>

using namespace gg_lib;

TEST(FixedBufferTest, TestAppend) {
    FixedBuffer<kSmallBuffer> buffer;
    buffer.append("a", 2);
    EXPECT_STREQ("a", buffer.data());
    buffer.reset();
    EXPECT_EQ(buffer.length(), 0);
    EXPECT_EQ(buffer.avail(), kSmallBuffer);
    char str[2 * kSmallBuffer] = {};
    memset(str, 2 * kSmallBuffer - 1, 'x');
    buffer.append(str, 2 * kSmallBuffer);
    EXPECT_EQ(buffer.length(), kSmallBuffer);
    EXPECT_EQ(buffer.avail(), 0);
    std::string s1(str, 100);
    std::string s2(buffer.data(), 100);
    EXPECT_EQ(s1, s2);
    buffer.reset();
    buffer.append("a", 1);
    buffer.append("bc", 2);
    buffer.append("def", 3);
    buffer.append(" hello", 6);
    std::string s3 = "abcdef hello";
    std::string s4(buffer.data(), buffer.length());
    EXPECT_EQ(s3, s4);
}