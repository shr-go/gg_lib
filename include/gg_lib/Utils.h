// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_UTILS_H
#define GG_LIB_UTILS_H

#define FMT_HEADER_ONLY

#include <stdint.h>
#include <string.h>
#include <string>
#include <atomic>
#include <functional>
#include <string_view.h>
#include <fmt/core.h>

namespace gg_lib {
    using std::string;
    using nonstd::string_view;
    using namespace std::placeholders;

    class StringArg {
    public:
        StringArg(const char *str) : str_(str) {};

        StringArg(const string &str) : str_(str.c_str()) {};

        const char *c_str() const { return str_; }

    private:
        const char *str_;
    };

    typedef std::atomic<int32_t> AtomicInt32;
    typedef std::atomic<int64_t> AtomicInt64;
}

#if __cplusplus < 201402L

template<class T, class... Args>
std::unique_ptr<T> make_unique(Args &&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#else
using std::make_unique;
#endif

#endif //GG_LIB_UTILS_H
