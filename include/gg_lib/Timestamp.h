// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_TIMESTAMP_H
#define GG_LIB_TIMESTAMP_H

#include "gg_lib/copyable.h"
#include "gg_lib/Utils.h"
#include "gg_lib/comparable.h"

#include <functional>

namespace gg_lib {
    class Timestamp : public copyable,
                      public EqualCompareT<Timestamp>,
                      public LessCompareT<Timestamp> {
    public:
        explicit Timestamp(int64_t microSecond = 0)
                : microSecondsSinceEpoch_(microSecond) {}

        /// Transfer a timestamp to string
        /// \param Simplify Choose to get a simplify timestamp string like 12:34:56
        /// or a timestamp string like `20010203 12:34:56.123456`
        /// \return string present the timestamp
        string toString(bool Simplify = false) const;

        bool valid() const { return microSecondsSinceEpoch_ > 0; }

        int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }

        time_t secondsSinceEpoch() const {
            return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
        }

        ///
        /// \return Get now present timestamp
        static Timestamp now();

        /// Get timestamp duration
        /// \param high, low
        /// \return duration in seconds
        static double timeDuration(Timestamp high, Timestamp low) {
            int64_t diff = high.microSecondsSinceEpoch_ - low.microSecondsSinceEpoch_;
            return static_cast<double>(diff) / kMicroSecondsPerSecond;
        }

        /// Add seconds to a timestamp
        /// \param timestamp
        /// \param seconds seconds is a double, so you can pass a seconds like 1.2 or -1.2
        /// \return A new timestamp
        static Timestamp addTime(Timestamp timestamp, double seconds) {
            auto delta = static_cast<int64_t>(seconds * kMicroSecondsPerSecond);
            return Timestamp(timestamp.microSecondsSinceEpoch_ + delta);
        }

        friend std::ostream &operator<<(std::ostream &os, const Timestamp &time) {
            return os << std::to_string(time.microSecondsSinceEpoch_);
        }

        static constexpr int kMicroSecondsPerSecond = 1000 * 1000;
    private:
        int64_t microSecondsSinceEpoch_;
    };

    inline bool operator<(Timestamp lhs, Timestamp rhs) {
        return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
    }

    inline bool operator==(Timestamp lhs, Timestamp rhs) {
        return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
    }
}

template<>
struct std::hash<gg_lib::Timestamp> {
    size_t operator()(const gg_lib::Timestamp &time) const {
        return hash<long long>{}(static_cast<long long>(time.microSecondsSinceEpoch()));
    }
};

#endif //GG_LIB_TIMESTAMP_H
