// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include <benchmark/benchmark.h>
#include "gg_lib/Timestamp.h"

using namespace gg_lib;

static void BMTimestampNow(benchmark::State& state) {
    for (auto _ : state) {
        Timestamp now = Timestamp::now();
        benchmark::DoNotOptimize(now);
    }
}

static void BMTimeNow(benchmark::State& state) {
    for (auto _ : state) {
        time_t now = ::time(nullptr);
        benchmark::DoNotOptimize(now);
    }
}

BENCHMARK(BMTimestampNow);
BENCHMARK(BMTimeNow);

BENCHMARK_MAIN();
