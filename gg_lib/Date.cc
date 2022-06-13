// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/Date.h"
#include "gg_lib/LogStream.h"

namespace gg_lib {
    int getJulianDay(int year, int month, int day) {
        int a = (14 - month) / 12;
        int y = year + 4800 - a;
        int m = month + 12 * a - 3;
        return day + (153 * m + 2) / 5 + y * 365 + y / 4 - y / 100 + y / 400 - 32045;
    }

    struct Date::YearMonthDay getYearMonthDay(int julianDay) {
        int a = julianDay + 32044;
        int b = (4 * a + 3) / 146097;
        int c = a - ((b * 146097) / 4);
        int d = (4 * c + 3) / 1461;
        int e = c - ((1461 * d) / 4);
        int m = (5 * e + 2) / 153;
        int day = e - ((153 * m + 2) / 5) + 1;
        int month = m + 3 - 12 * (m / 10);
        int year = b * 100 + d - 4800 + (m / 10);
        return Date::YearMonthDay{year, month, day};
    }

    const int Date::kJulianDayOf1970_01_01 = getJulianDay(1970, 1, 1);
}

using namespace gg_lib;

Date::Date(int year, int month, int day)
        : julianDay_(getJulianDay(year, month, day)) {}

Date::Date(const struct tm &t) : julianDay_(getJulianDay(
        t.tm_year + 1900,
        t.tm_mon + 1,
        t.tm_mday)) {}

string Date::toIsoString() const {
    char buf[32];
    YearMonthDay ymd = yearMonthDay();
    snprintf(buf, sizeof buf, "%4d-%02d-%02d", ymd.year, ymd.month, ymd.day);
    return buf;
}

Date::YearMonthDay Date::yearMonthDay() const {
    return getYearMonthDay(julianDay_);
}

inline LogStream& operator<<(LogStream& s, Date date) {
    s << date.toIsoString();
}
