// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_LOGGING_H
#define GG_LIB_LOGGING_H

#include "gg_lib/LogStream.h"
#include "gg_lib/Timestamp.h"

namespace gg_lib {
    class TimeZone;

    const char *strerror_tr(int savedErrno);

    class Logger {
    public:
        enum LogLevel {
            TRACE,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
            NUM_LOG_LEVELS,
        };

        class SourceFile {
        public:
            template<int N>
            SourceFile(const char (&arr)[N])
                    : data_(arr), size_(N - 1) {
                const char *slash = strrchr(data_, '/');
                if (slash) {
                    data_ = slash + 1;
                    size_ -= static_cast<int>(data_ - arr);
                }
            }

            explicit SourceFile(const char *filename)
                    : data_(filename) {
                const char *slash = strrchr(filename, '/');
                if (slash) {
                    data_ = slash + 1;
                }
                size_ = static_cast<int>(strlen(data_));
            }

            const char *data_;
            int size_;
        };

        Logger(SourceFile file, int line);

        Logger(SourceFile file, int line, LogLevel level);

        Logger(SourceFile file, int line, LogLevel level, const char *func);

        Logger(SourceFile file, int line, bool toAbort);

        ~Logger();

        LogStream &stream() { return impl_.stream_; }

        static LogLevel logLevel();

        static void setLogLevel(LogLevel level);

        typedef void(*OutputFunc)(const char *msg, int len);

        typedef void (*FlushFunc)();

        static void setOutput(OutputFunc);

        static void setFlush(FlushFunc);

        static void setTimeZone(const TimeZone &tz);


    private:
        class Impl {
        public:
            typedef Logger::LogLevel LogLevel;

            Impl(LogLevel level, int old_errno, const SourceFile &file, int line);

            void formatTime();

            void finish();

            Timestamp time_;
            LogStream stream_;
            LogLevel level_;
            int line_;
            SourceFile basename_;
        };

        Impl impl_;
    };

    extern Logger::LogLevel g_logLevel;

    inline Logger::LogLevel Logger::logLevel() {
        return g_logLevel;
    }

    template<typename T>
    T *CheckNotNull(Logger::SourceFile file, int line, const char *names, T *ptr) {
        if (ptr == NULL) {
            Logger(file, line, Logger::FATAL).stream() << names;
        }
        return ptr;
    }
}

#define LOG_TRACE if (gg_lib::Logger::logLevel() <= gg_lib::Logger::TRACE) \
  gg_lib::Logger(__FILE__, __LINE__, gg_lib::Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (gg_lib::Logger::logLevel() <= gg_lib::Logger::DEBUG) \
  gg_lib::Logger(__FILE__, __LINE__, gg_lib::Logger::DEBUG, __func__).stream()
#define LOG_INFO if (gg_lib::Logger::logLevel() <= gg_lib::Logger::INFO) \
  gg_lib::Logger(__FILE__, __LINE__, gg_lib::Logger::INFO, __func__).stream()
#define LOG_WARN gg_lib::Logger(__FILE__, __LINE__, gg_lib::Logger::WARN).stream()
#define LOG_ERROR gg_lib::Logger(__FILE__, __LINE__, gg_lib::Logger::ERROR).stream()
#define LOG_FATAL gg_lib::Logger(__FILE__, __LINE__, gg_lib::Logger::FATAL).stream()
#define LOG_SYSERR gg_lib::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL gg_lib::Logger(__FILE__, __LINE__, true).stream()
#define CHECK_NOTNULL(val) \
  gg_lib::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))


#endif //GG_LIB_LOGGING_H
