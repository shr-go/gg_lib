// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/Date.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace gg_lib;

TEST(DateTest, CommonTest) {
    Date date(2022, 6, 13);
    EXPECT_EQ(date.toIsoString(), "2022-06-13");
    EXPECT_EQ(date.julianDay(), 2459744);
    Date anotherDate = date + 2000;
    EXPECT_EQ(anotherDate.toIsoString(), "2027-12-04");
    EXPECT_GE(anotherDate, date);
    EXPECT_NE(anotherDate, date);
}