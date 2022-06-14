// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/TimeZone.h"
#include "gg_lib/noncopyable.h"
#include "gg_lib/Date.h"
#include "gg_lib/FileUtil.h"

#include <vector>
#include <algorithm>

namespace gg_lib {
    struct Transition {
        time_t gmttime;
        time_t localtime;
        int localtimeIdx;

        Transition(time_t t, time_t l, int localIdx)
                : gmttime(t), localtime(l), localtimeIdx(localIdx) {}
    };

    struct Comp {
        bool compareGmt;

        Comp(bool gmt)
                : compareGmt(gmt) {}

        bool operator()(const Transition &lhs, const Transition &rhs) const {
            if (compareGmt)
                return lhs.gmttime < rhs.gmttime;
            else
                return lhs.localtime < rhs.localtime;
        }

        bool equal(const Transition &lhs, const Transition &rhs) const {
            if (compareGmt)
                return lhs.gmttime == rhs.gmttime;
            else
                return lhs.localtime == rhs.localtime;
        }
    };

    struct Localtime {
        time_t gmtOffset;
        bool isDst;
        int arrbIdx;

        Localtime(time_t offset, bool dst, int arrb)
                : gmtOffset(offset), isDst(dst), arrbIdx(arrb) {}
    };

    inline void fillHMS(unsigned seconds, struct tm *utc) {
        utc->tm_sec = static_cast<int>(seconds % 60);
        unsigned minutes = seconds / 60;
        utc->tm_min = static_cast<int>(minutes % 60);
        utc->tm_hour = static_cast<int>(minutes / 60);
    }

    constexpr int kSecondsPerDay = 24 * 60 * 60;

    struct TimeZone::Data {
        std::vector<Transition> transitions;
        std::vector<Localtime> localtimes;
        std::vector<string> names;
        string abbreviation;
    };

    bool readTimeZoneFile(StringArg zonefile, struct TimeZone::Data *data) {
        ReadFile f(zonefile);
        if (!f)
            return false;
        try {
            string head = f.readBytes(4);
            if (head != "TZif")
                throw std::logic_error("bad head");
            string version = f.readBytes(1);
            f.passBytes(15);
            int32_t isgmtcnt = be32toh(f.readInt32());
            int32_t isstdcnt = be32toh(f.readInt32());
            int32_t leapcnt = be32toh(f.readInt32());
            int32_t timecnt = be32toh(f.readInt32());
            int32_t typecnt = be32toh(f.readInt32());
            int32_t charcnt = be32toh(f.readInt32());
            std::vector<int32_t> trans;
            std::vector<int> localtimes;
            trans.reserve(timecnt);
            for (int i = 0; i < timecnt; ++i) {
                trans.push_back(be32toh(f.readInt32()));
            }
            for (int i = 0; i < timecnt; ++i) {
                localtimes.push_back(f.readUInt8());
            }
            for (int i = 0; i < typecnt; ++i) {
                int32_t gmtoff = be32toh(f.readInt32());
                uint8_t isdst = f.readUInt8();
                uint8_t abbrind = f.readUInt8();

                data->localtimes.emplace_back(gmtoff, isdst, abbrind);
            }
            for (int i = 0; i < timecnt; ++i) {
                int localIdx = localtimes[i];
                time_t localtime = trans[i] + data->localtimes[localIdx].gmtOffset;
                data->transitions.emplace_back(trans[i], localtime, localIdx);
            }
            data->abbreviation = f.readBytes(charcnt);
        } catch (std::logic_error &e) {
            fprintf(stderr, "%s\n", e.what());
            return false;
        }
        return true;
    }

    const Localtime *findLocalTime(const TimeZone::Data &data, Transition sentry, Comp comp) {
        const Localtime *local = nullptr;
        if (data.transitions.empty() || comp(sentry, data.transitions.front())) {
            local = &data.localtimes.front();
        } else {
            auto transI = std::lower_bound(data.transitions.begin(),
                                           data.transitions.end(), sentry, comp);
            if (transI != data.transitions.end()) {
                if (!comp.equal(sentry, *transI)) {
                    assert(transI != data.transitions.begin());
                    --transI;
                }
                local = &data.localtimes[transI->localtimeIdx];
            } else {
                local = &data.localtimes[data.transitions.back().localtimeIdx];
            }
        }
        return local;
    }
}

using namespace gg_lib;

TimeZone::TimeZone(const char *zonefile)
        : data_(new TimeZone::Data) {
    if (!readTimeZoneFile(zonefile, data_.get())) {
        data_.reset();
    }
}

TimeZone::TimeZone(int eastOfUtc, const char *name)
        : data_(new TimeZone::Data) {
    data_->localtimes.emplace_back(eastOfUtc, false, 0);
    data_->abbreviation = name;
}

struct tm TimeZone::toLocalTime(time_t seconds) const {
    struct tm localTime{};
    assert(data_ != NULL);
    const Data &data(*data_);

    Transition sentry(seconds, 0, 0);
    const Localtime *local = findLocalTime(data, sentry, Comp(true));

    if (local) {
        time_t localSeconds = seconds + local->gmtOffset;
        ::gmtime_r(&localSeconds, &localTime);
        localTime.tm_isdst = local->isDst;
        localTime.tm_gmtoff = local->gmtOffset;
        localTime.tm_zone = &data.abbreviation[local->arrbIdx];
    }

    return localTime;
}

time_t TimeZone::fromLocalTime(const struct tm &localTm) const {
    assert(data_ != NULL);
    const Data &data(*data_);

    struct tm tmp = localTm;
    time_t seconds = ::timegm(&tmp);
    Transition sentry(0, seconds, 0);
    const Localtime *local = findLocalTime(data, sentry, Comp(false));
    if (localTm.tm_isdst) {
        struct tm tryTm = toLocalTime(seconds - local->gmtOffset);
        if (!tryTm.tm_isdst
            && tryTm.tm_hour == localTm.tm_hour
            && tryTm.tm_min == localTm.tm_min) {
            seconds -= 3600;
        }
    }
    return seconds - local->gmtOffset;
}

struct tm TimeZone::toUtcTime(time_t secondsSinceEpoch, bool yday) {
    struct tm utc{};
    utc.tm_zone = "GMT";
    int seconds = static_cast<int>(secondsSinceEpoch % kSecondsPerDay);
    int days = static_cast<int>(secondsSinceEpoch / kSecondsPerDay);
    if (seconds < 0) {
        seconds += kSecondsPerDay;
        --days;
    }
    fillHMS(seconds, &utc);
    Date date(days + Date::kJulianDayOf1970_01_01);
    Date::YearMonthDay ymd = date.yearMonthDay();
    utc.tm_year = ymd.year - 1900;
    utc.tm_mon = ymd.month - 1;
    utc.tm_mday = ymd.day;
    utc.tm_wday = date.weekDay();

    if (yday) {
        Date startOfYear(ymd.year, 1, 1);
        utc.tm_yday = date.julianDay() - startOfYear.julianDay();
    }
    return utc;
}

time_t TimeZone::fromUtcTime(const struct tm &utc) {
    return fromUtcTime(utc.tm_year + 1900, utc.tm_mon + 1, utc.tm_mday,
                       utc.tm_hour, utc.tm_min, utc.tm_sec);
}

time_t TimeZone::fromUtcTime(int year, int month, int day,
                             int hour, int minute, int seconds) {
    Date date(year, month, day);
    int secondsInDay = hour * 3600 + minute * 60 + seconds;
    time_t days = date.julianDay() - Date::kJulianDayOf1970_01_01;
    return days * kSecondsPerDay + secondsInDay;
}



