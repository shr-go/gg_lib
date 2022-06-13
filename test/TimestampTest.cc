// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/Timestamp.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unordered_set>

using namespace gg_lib;

TEST(TimestampTest, TestCreate) {
    Timestamp time1;
    EXPECT_EQ(time1.microSecondsSinceEpoch(), 0);
    Timestamp time2(10000);
    EXPECT_EQ(time2.microSecondsSinceEpoch(), 10000);
}

TEST(TimestampTest, TestToString) {
    Timestamp time(1654853474123456);
    EXPECT_EQ(time.toString(), "20220610 09:31:14.123456");
    EXPECT_EQ(time.toString(true), "09:31:14");
    Timestamp anotherTime = Timestamp::addTime(time, 10.1);
    EXPECT_EQ(anotherTime.toString(), "20220610 09:31:24.223456");
}

TEST(TimestampTest, TimeCompare) {
    Timestamp time1 = Timestamp::now();
    Timestamp time2 = Timestamp::addTime(time1, 10);
    Timestamp time3 = Timestamp::addTime(time1, -10);
    Timestamp time4 = time1;

    EXPECT_EQ(time1, time4);
    EXPECT_NE(time1, time2);

    EXPECT_LT(time1, time2);
    EXPECT_LE(time1, time2);
    EXPECT_LE(time1, time4);

    EXPECT_GT(time1, time3);
    EXPECT_GE(time1, time3);
    EXPECT_GE(time1, time4);

    std::vector<Timestamp> times = {time1, time2, time3, time4};
    std::sort(times.begin(), times.end());
    EXPECT_THAT(times, testing::ElementsAre(time3, time1, time4, time2));
}

TEST(TimestampTest, Timehash) {
    std::unordered_set<Timestamp> set;
    Timestamp time1 = Timestamp::now();
    Timestamp time2 = Timestamp::addTime(time1, 10);
    EXPECT_FALSE(set.count(time1));
    set.insert(time1);
    EXPECT_TRUE(set.count(time1));
    EXPECT_EQ(set.find(time2), set.end());
}