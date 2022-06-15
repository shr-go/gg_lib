// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_TIMEZONE_H
#define GG_LIB_TIMEZONE_H

#include "gg_lib/copyable.h"
#include <memory>
#include <time.h>

namespace gg_lib {
    class TimeZone : public copyable {
    public:
        explicit TimeZone(const char *zonefile);

        TimeZone(int eastOfUtc, const char *tzname);

        TimeZone() = default;

        explicit operator bool() const {
            return static_cast<bool>(data_);
        }

        struct tm toLocalTime(time_t secondsSinceEpoch) const;

        time_t fromLocalTime(const struct tm &) const;

        static struct tm toUtcTime(time_t secondsSinceEpoch, bool yday = false);

        static time_t fromUtcTime(const struct tm &);

        static time_t fromUtcTime(int year, int month, int day,
                                  int hour, int minute, int seconds);

        const char* getZoneAbbr() const;

        struct Data;
    private:
        std::shared_ptr<Data> data_;
    };
}

#endif //GG_LIB_TIMEZONE_H
