// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include <benchmark/benchmark.h>
#include "grisu3/grisu3_print.h"

#define FMT_HEADER_ONLY
#include <fmt/core.h>

const char digits[] = "9876543210123456789";
const char *zero = digits + 9;

template<typename T>
size_t convert(char buf[], T value) {
    T i = value;
    char *p = buf;

    do {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);
    if (value < 0) {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);
    return p - buf;
}

template size_t convert(char buf[], int value);
template size_t convert(char buf[], unsigned int value);

static void BM_INT_STRING_1(benchmark::State& state) {
    int num = INT32_MAX;
    char buf[64];
    for (auto _ : state) {
        convert(buf, num);
        benchmark::DoNotOptimize(buf);
    }
}

static void BM_INT_STRING_2(benchmark::State& state) {
    int num = INT32_MAX;
    char buf[64];
    for (auto _ : state) {
        snprintf(buf, 64, "%d", num);
        benchmark::DoNotOptimize(buf);
    }
}

static void BM_INT_STRING_3(benchmark::State& state) {
    int num = INT32_MAX;
    char buf[64];
    for (auto _ : state) {
        grisu3_i_to_str(num, buf);
        benchmark::DoNotOptimize(buf);
    }
}

static void BM_INT_STRING_4(benchmark::State& state) {
    int num = INT32_MAX;
    char buf[64];
    for (auto _ : state) {
        fmt::format_to(buf, "{}", num);
        benchmark::DoNotOptimize(buf);
    }
}

static void BM_INT_STRING_5(benchmark::State& state) {
    int num = INT32_MIN;
    char buf[64];
    for (auto _ : state) {
        convert(buf, num);
        benchmark::DoNotOptimize(buf);
    }
}

static void BM_INT_STRING_6(benchmark::State& state) {
    int num = INT32_MIN;
    char buf[64];
    for (auto _ : state) {
        snprintf(buf, 64, "%d", num);
        benchmark::DoNotOptimize(buf);
    }
}

static void BM_INT_STRING_7(benchmark::State& state) {
    int num = INT32_MIN;
    char buf[64];
    for (auto _ : state) {
        grisu3_i_to_str(num, buf);
        benchmark::DoNotOptimize(buf);
    }
}

static void BM_INT_STRING_8(benchmark::State& state) {
    int num = INT32_MIN;
    char buf[64];
    for (auto _ : state) {
        fmt::format_to(buf, "{}", num);
        benchmark::DoNotOptimize(buf);
    }
}

static void BM_INT_STRING_9(benchmark::State& state) {
    double num = 3.14;
    char buf[64];
    for (auto _ : state) {
        grisu3_print_double(num, buf);
        benchmark::DoNotOptimize(buf);
    }
}

static void BM_INT_STRING_10(benchmark::State& state) {
    double num = 3.14;
    char buf[64];
    for (auto _ : state) {
        snprintf(buf, 64, "%f", num);
        benchmark::DoNotOptimize(buf);
    }
}

static void BM_INT_STRING_11(benchmark::State& state) {
    double num = 3.14;
    char buf[64];
    for (auto _ : state) {
        fmt::format_to(buf, "{}", num);
        benchmark::DoNotOptimize(buf);
    }
}

BENCHMARK(BM_INT_STRING_1);
BENCHMARK(BM_INT_STRING_2);
BENCHMARK(BM_INT_STRING_3);
BENCHMARK(BM_INT_STRING_4);
BENCHMARK(BM_INT_STRING_5);
BENCHMARK(BM_INT_STRING_6);
BENCHMARK(BM_INT_STRING_7);
BENCHMARK(BM_INT_STRING_8);
BENCHMARK(BM_INT_STRING_9);
BENCHMARK(BM_INT_STRING_10);
BENCHMARK(BM_INT_STRING_11);


BENCHMARK_MAIN();
