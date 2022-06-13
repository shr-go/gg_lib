// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_DATE_H
#define GG_LIB_DATE_H

#include "gg_lib/Utils.h"
#include "gg_lib/copyable.h"
#include "gg_lib/comparable.h"

namespace gg_lib {
    class Date : public copyable,
                 public LessCompareT<Date>,
                 public EqualCompareT<Date> {
    public:
        struct YearMonthDay {
            int year;
            int month;
            int day;
        };

        static constexpr int kDaysPerWeek = 7;
        static const int kJulianDayOf1970_01_01;

        Date() : julianDay_(0) {};

        Date(int year, int month, int day);

        explicit Date(int julianDat) : julianDay_(julianDat) {};

        explicit Date(const struct tm &);

        void swap(Date &that) {
            std::swap(julianDay_, that.julianDay_);
        }

        explicit operator bool() const {
            return julianDay_ > 0;
        }

        string toIsoString() const;

        struct YearMonthDay yearMonthDay() const;

        int year() const {
            return yearMonthDay().year;
        }

        int month() const {
            return yearMonthDay().month;
        }

        int day() const {
            return yearMonthDay().day;
        }

        int weekDay() const {
            return (julianDay_ + 1) % kDaysPerWeek;
        }

        int julianDay() const {
            return julianDay_;
        }

        Date operator+(int days) const {
            return Date(julianDay_ + days);
        }

    private:
        int julianDay_;
    };


    inline bool operator<(Date lhs, Date rhs) {
        return lhs.julianDay() < rhs.julianDay();
    }

    inline bool operator==(Date lhs, Date rhs) {
        return lhs.julianDay() == rhs.julianDay();
    }

    class LogStream;
    LogStream& operator<<(LogStream& s, Date date);
}

#endif //GG_LIB_DATE_H
