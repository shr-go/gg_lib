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

    typedef std::atomic <int32_t> AtomicInt32;
    typedef std::atomic <int64_t> AtomicInt64;

    // helper struct for unpack Tuple.
    template<std::size_t... Ts>
    struct Index {};

    template <std::size_t N, std::size_t... Ts>
    struct GenSeq: GenSeq<N - 1, N - 1, Ts...> {};

    template <std::size_t... Ts>
    struct GenSeq<0, Ts...> : Index<Ts...> {};

    // format Helper
    template<typename... ARGS>
    class FmtHelper {
    public:
        explicit FmtHelper(const char *fmt, ARGS &&... args)
                : fmt_(fmt), args_(std::forward<ARGS>(args)...) {}

        inline void formatTo(char *buf) const {
            formatTo(buf, GenSeq<sizeof...(ARGS)>{});
        }

        inline std::size_t formatToN(char *buf, std::size_t n) const {
            return formatToN(buf, n, GenSeq<sizeof...(ARGS)>{});
        }

        inline std::size_t formatSize() const {
            return formatSize(GenSeq<sizeof...(ARGS)>{});
        }

    private:
        template<std::size_t... Is>
        inline void formatTo(char *buf, Index<Is...>) const {
            fmt::format_to(buf, fmt_, std::get<Is>(args_)...);
        }

        template<std::size_t... Is>
        inline std::size_t formatToN(char *buf, std::size_t n, Index<Is...>) const {
            auto result = fmt::format_to_n(buf, n, fmt_, std::get<Is>(args_)...);
            return result.out - buf;
        }

        template<std::size_t... Is>
        inline std::size_t formatSize(Index<Is...>) const {
            return fmt::formatted_size(fmt_, std::get<Is>(args_)...);
        }

        const char *fmt_;
        const std::tuple<ARGS...> args_;
    };

    template<typename ...ARGS>
    FmtHelper<ARGS...> Fmt(const char* fmt, ARGS &&... args) {
        return FmtHelper<ARGS...>(fmt, std::forward<ARGS>(args)...);
    }


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
