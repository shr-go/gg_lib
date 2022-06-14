// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/Date.h"
#include "gg_lib/TimeZone.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace gg_lib;

struct tm getTm(const char *str) {
    struct tm gmt{};
    strptime(str, "%F %T", &gmt);
    return gmt;
}

time_t getGmt(const char *str) {
    struct tm gmt = getTm(str);
    return timegm(&gmt);
}


struct TestCase {
    const char *gmt;
    const char *local;
    bool isdst;
};


void TestTimeZone(const TimeZone &tz, TestCase tc) {
    time_t gmt = getGmt(tc.gmt);
    {
        struct tm local = tz.toLocalTime(gmt);
        char buf[256];
        strftime(buf, sizeof buf, "%F %T%z(%Z)", &local);
        EXPECT_STREQ(buf, tc.local);
        EXPECT_EQ(tc.isdst, local.tm_isdst);
    }
    {
        struct tm local = getTm(tc.local);
        local.tm_isdst = tc.isdst;
        time_t result = tz.fromLocalTime(local);
        EXPECT_EQ(result, gmt);
    }
}

TEST(DateTest, CommonTest) {
    Date date(2022, 6, 13);
    EXPECT_EQ(date.toIsoString(), "2022-06-13");
    EXPECT_EQ(date.julianDay(), 2459744);
    Date anotherDate = date + 2000;
    EXPECT_EQ(anotherDate.toIsoString(), "2027-12-04");
    EXPECT_GE(anotherDate, date);
    EXPECT_NE(anotherDate, date);
}

TEST(DateTest, TestTimeZoneNewYork) {
    TimeZone tz("/usr/share/zoneinfo/America/New_York");
    TestCase cases[] = {

            {"2006-03-07 00:00:00", "2006-03-06 19:00:00-0500(EST)", false},
            {"2006-04-02 06:59:59", "2006-04-02 01:59:59-0500(EST)", false},
            {"2006-04-02 07:00:00", "2006-04-02 03:00:00-0400(EDT)", true},
            {"2006-05-01 00:00:00", "2006-04-30 20:00:00-0400(EDT)", true},
            {"2006-05-02 01:00:00", "2006-05-01 21:00:00-0400(EDT)", true},
            {"2006-10-21 05:00:00", "2006-10-21 01:00:00-0400(EDT)", true},
            {"2006-10-29 05:59:59", "2006-10-29 01:59:59-0400(EDT)", true},
            {"2006-10-29 06:00:00", "2006-10-29 01:00:00-0500(EST)", false},
            {"2006-10-29 06:59:59", "2006-10-29 01:59:59-0500(EST)", false},
            {"2006-12-31 06:00:00", "2006-12-31 01:00:00-0500(EST)", false},
            {"2007-01-01 00:00:00", "2006-12-31 19:00:00-0500(EST)", false},

            {"2007-03-07 00:00:00", "2007-03-06 19:00:00-0500(EST)", false},
            {"2007-03-11 06:59:59", "2007-03-11 01:59:59-0500(EST)", false},
            {"2007-03-11 07:00:00", "2007-03-11 03:00:00-0400(EDT)", true},
            {"2007-05-01 00:00:00", "2007-04-30 20:00:00-0400(EDT)", true},
            {"2007-05-02 01:00:00", "2007-05-01 21:00:00-0400(EDT)", true},
            {"2007-10-31 05:00:00", "2007-10-31 01:00:00-0400(EDT)", true},
            {"2007-11-04 05:59:59", "2007-11-04 01:59:59-0400(EDT)", true},
            {"2007-11-04 06:00:00", "2007-11-04 01:00:00-0500(EST)", false},
            {"2007-11-04 06:59:59", "2007-11-04 01:59:59-0500(EST)", false},
            {"2007-12-31 06:00:00", "2007-12-31 01:00:00-0500(EST)", false},
            {"2008-01-01 00:00:00", "2007-12-31 19:00:00-0500(EST)", false},

            {"2009-03-07 00:00:00", "2009-03-06 19:00:00-0500(EST)", false},
            {"2009-03-08 06:59:59", "2009-03-08 01:59:59-0500(EST)", false},
            {"2009-03-08 07:00:00", "2009-03-08 03:00:00-0400(EDT)", true},
            {"2009-05-01 00:00:00", "2009-04-30 20:00:00-0400(EDT)", true},
            {"2009-05-02 01:00:00", "2009-05-01 21:00:00-0400(EDT)", true},
            {"2009-10-31 05:00:00", "2009-10-31 01:00:00-0400(EDT)", true},
            {"2009-11-01 05:59:59", "2009-11-01 01:59:59-0400(EDT)", true},
            {"2009-11-01 06:00:00", "2009-11-01 01:00:00-0500(EST)", false},
            {"2009-11-01 06:59:59", "2009-11-01 01:59:59-0500(EST)", false},
            {"2009-12-31 06:00:00", "2009-12-31 01:00:00-0500(EST)", false},
            {"2010-01-01 00:00:00", "2009-12-31 19:00:00-0500(EST)", false},

            {"2010-03-13 00:00:00", "2010-03-12 19:00:00-0500(EST)", false},
            {"2010-03-14 06:59:59", "2010-03-14 01:59:59-0500(EST)", false},
            {"2010-03-14 07:00:00", "2010-03-14 03:00:00-0400(EDT)", true},
            {"2010-05-01 00:00:00", "2010-04-30 20:00:00-0400(EDT)", true},
            {"2010-05-02 01:00:00", "2010-05-01 21:00:00-0400(EDT)", true},
            {"2010-11-06 05:00:00", "2010-11-06 01:00:00-0400(EDT)", true},
            {"2010-11-07 05:59:59", "2010-11-07 01:59:59-0400(EDT)", true},
            {"2010-11-07 06:00:00", "2010-11-07 01:00:00-0500(EST)", false},
            {"2010-11-07 06:59:59", "2010-11-07 01:59:59-0500(EST)", false},
            {"2010-12-31 06:00:00", "2010-12-31 01:00:00-0500(EST)", false},
            {"2011-01-01 00:00:00", "2010-12-31 19:00:00-0500(EST)", false},

            {"2011-03-01 00:00:00", "2011-02-28 19:00:00-0500(EST)", false},
            {"2011-03-13 06:59:59", "2011-03-13 01:59:59-0500(EST)", false},
            {"2011-03-13 07:00:00", "2011-03-13 03:00:00-0400(EDT)", true},
            {"2011-05-01 00:00:00", "2011-04-30 20:00:00-0400(EDT)", true},
            {"2011-05-02 01:00:00", "2011-05-01 21:00:00-0400(EDT)", true},
            {"2011-11-06 05:59:59", "2011-11-06 01:59:59-0400(EDT)", true},
            {"2011-11-06 06:00:00", "2011-11-06 01:00:00-0500(EST)", false},
            {"2011-11-06 06:59:59", "2011-11-06 01:59:59-0500(EST)", false},
            {"2011-12-31 06:00:00", "2011-12-31 01:00:00-0500(EST)", false},
            {"2012-01-01 00:00:00", "2011-12-31 19:00:00-0500(EST)", false},
    };
    for (const auto &c: cases) {
        TestTimeZone(tz, c);
    }
}

TEST(DateTest, TestTimeZoneHongKong) {
    TimeZone tz("/usr/share/zoneinfo/Asia/Hong_Kong");
    TestCase cases[] = {
            {"2011-04-03 00:00:00", "2011-04-03 08:00:00+0800(HKT)", false},
    };

    for (const auto &c: cases) {
        TestTimeZone(tz, c);
    }
}

TEST(DateTest, TestTimeZoneFixed) {
    TimeZone tz(8 * 3600, "FIXED");
    TestCase cases[] = {
            {"2014-04-03 00:00:00", "2014-04-03 08:00:00+0800(FIXED)", false},
    };
    for (const auto& c : cases)
    {
        TestTimeZone(tz, c);
    }
}