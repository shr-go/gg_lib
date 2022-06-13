// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_UTILS_H
#define GG_LIB_UTILS_H

#define FMT_HEADER_ONLY

#include <stdint.h>
#include <string.h>
#include <string>
#include <string_view.h>
#include <fmt/core.h>

namespace gg_lib {
    using std::string;
    using nonstd::string_view;

    class StringArg {
    public:
        StringArg(const char *str) : str_(str) {};

        StringArg(const string &str) : str_(str.c_str()) {};

        const char *c_str() const { return str_; }

    private:
        const char *str_;
    };
}

#endif //GG_LIB_UTILS_H
