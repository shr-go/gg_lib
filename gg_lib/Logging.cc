// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/Logging.h"
#include "gg_lib/Timestamp.h"
#include "gg_lib/TimeZone.h"
#include "gg_lib/ThreadHelper.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <thread>

namespace gg_lib {
    __thread char t_time[64];
    __thread time_t t_lastSecond;
    __thread char t_zoneAbbr[16] = "GMT";

    Logger::LogLevel initLogLevel() {
        if (::getenv("LOG_TRACE"))
            return Logger::TRACE;
        else if (::getenv("LOG_DEBUG"))
            return Logger::DEBUG;
        else
            return Logger::INFO;
    }

    Logger::LogLevel g_logLevel = initLogLevel();

    const char *LogLevelName[Logger::NUM_LOG_LEVELS] = {
            "TRACE ",
            "DEBUG ",
            "INFO  ",
            "WARN  ",
            "ERROR ",
            "FATAL ",
    };

    class StrHelper {
    public:
        StrHelper(const char *str, int len) : str_(str), len_(len) {
            assert(strlen(str) == len_);
        }

        const char *str_;
        const int len_;
    };

    inline LogStream &operator<<(LogStream &s, StrHelper v) {
        s.append(v.str_, v.len_);
        return s;
    }

    inline LogStream &operator<<(LogStream &s, const Logger::SourceFile &v) {
        s.append(v.data_, v.size_);
        return s;
    }

    void defaultOutput(const char *msg, int len) {
        size_t n = fwrite(msg, 1, len, stdout);
    }

    void defaultFlush() {
        fflush(stdout);
    }

    Logger::OutputFunc g_output = defaultOutput;
    Logger::FlushFunc g_flush = defaultFlush;
    TimeZone g_logTimeZone;
}

using namespace gg_lib;

Logger::Logger(SourceFile file, int line)
        : impl_(INFO, 0, file, line) {};

Logger::Logger(SourceFile file, int line, LogLevel level)
        : impl_(level, 0, file, line) {}

Logger::Logger(SourceFile file, int line, LogLevel level, const char *func)
        : impl_(level, 0, file, line) {
    impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, bool toAbort)
        : impl_(toAbort ? FATAL : ERROR, errno, file, line) {}

Logger::~Logger() {
    impl_.finish();
    const LogStream::Buffer &buf(stream().buffer());
    g_output(buf.data(), buf.length());
    if (impl_.level_ == FATAL) {
        g_flush();
        abort();
    }
}

void Logger::setLogLevel(Logger::LogLevel level) {
    g_logLevel = level;
}

void Logger::setOutput(OutputFunc out) {
    g_output = out;
}

void Logger::setFlush(FlushFunc flush) {
    g_flush = flush;
}

void Logger::setTimeZone(const TimeZone &tz) {
    g_logTimeZone = tz;
    strncpy(t_zoneAbbr, tz.getZoneAbbr(), sizeof t_zoneAbbr - 1);
}

Logger::Impl::Impl(LogLevel level, int savedErrno, const SourceFile &file, int line)
        : time_(Timestamp::now()),
          stream_(),
          level_(level),
          line_(line),
          basename_(file) {
    formatTime();
    stream_ << StrHelper(CurrentThread::tidString(), CurrentThread::tidStringLength());
    stream_ << StrHelper(LogLevelName[level], 6);
    if (savedErrno != 0) {
        stream_ << strerrordesc_np(savedErrno) << Fmt(" (errno={}) ", savedErrno);
    }

}

void Logger::Impl::formatTime() {
    int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
    auto seconds = static_cast<time_t>(microSecondsSinceEpoch / Timestamp::kMicroSecondsPerSecond);
    auto microseconds = static_cast<int>(microSecondsSinceEpoch % Timestamp::kMicroSecondsPerSecond);
    if (seconds != t_lastSecond) {
        t_lastSecond = seconds;
        struct tm tm_time;
        if (g_logTimeZone) {
            tm_time = g_logTimeZone.toLocalTime(seconds);
        } else {
            ::gmtime_r(&seconds, &tm_time);
        }
        int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
                           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                           tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        assert(len == 17);
        (void) len;
    }
    stream_ << Fmt("{}.{:06}({}) ", t_time, microseconds, t_zoneAbbr);
//    if (g_logTimeZone) {
//        stream_ << Fmt("{}.{:06} ", t_time, microseconds);
//    } else {
//        stream_ << Fmt("{}.{:06} ", t_time, microseconds);
//    }
}


void Logger::Impl::finish() {
    stream_ << Fmt(" - {}:{}\n", basename_.data_, line_);
}


