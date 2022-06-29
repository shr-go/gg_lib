// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_LOGSTREAM_H
#define GG_LIB_LOGSTREAM_H

#include "gg_lib/Utils.h"
#include "gg_lib/noncopyable.h"
#include "gg_lib/FixedBuffer.h"

namespace gg_lib {
    class LogStream : noncopyable {
        typedef LogStream self;
    public:
        typedef FixedBuffer<kSmallBuffer> Buffer;

        self &operator<<(bool v) {
            if (v)
                buffer_.append("true", 4);
            else
                buffer_.append("false", 5);
            return *this;
        }

        self &operator<<(short);

        self &operator<<(unsigned short);

        self &operator<<(int);

        self &operator<<(unsigned int);

        self &operator<<(long);

        self &operator<<(unsigned long);

        self &operator<<(long long);

        self &operator<<(unsigned long long);

        self &operator<<(const void *);

        self &operator<<(float v) {
            *this << static_cast<double>(v);
            return *this;
        }

        self &operator<<(double);

        self &operator<<(char v) {
            buffer_.append(&v, 1);
            return *this;
        }

        self &operator<<(const char *str) {
            if (str) {
                buffer_.append(str, strlen(str));
            } else {
                buffer_.append("(NULL)", 6);
            }
            return *this;
        }

        self &operator<<(const unsigned char *str) {
            return operator<<(reinterpret_cast<const char *>(str));
        }

        self &operator<<(const string &v) {
            buffer_.append(v.c_str(), v.size());
            return *this;
        }

        self &operator<<(const string_view &sv) {
            buffer_.append(sv.data(), sv.size());
            return *this;
        }

        self &operator<<(const Buffer &v) {
            *this << v.toStringView();
            return *this;
        }

        void append(const char *data, int len) {
            buffer_.append(data, len);
        }

        const Buffer &buffer() const {
            return buffer_;
        }

        void resetBuffer() {
            buffer_.reset();
        }


    private:
        template<typename T>
        void formatInteger(T);

        Buffer buffer_;
        static const int kMaxNumericSize = 48;
    };

    class Fmt {
    public:
        template<typename... ARGS>
        explicit Fmt(const char *fmt, ARGS &&... args) {
            data_ = fmt::format(fmt, std::forward<ARGS>(args)...);
        }

        string_view toStringView() const {
            return data_;
        }

    private:
        string data_;
    };

    inline LogStream &operator<<(LogStream &s, const Fmt &fmt) {
        s << fmt.toStringView();
        return s;
    }
}

#endif //GG_LIB_LOGSTREAM_H
