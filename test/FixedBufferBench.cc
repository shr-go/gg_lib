// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include <benchmark/benchmark.h>
#include "gg_lib/FixedBuffer.h"

using namespace gg_lib;

#define SIZE1 4088
#define SIZE2 4089

static void BM_S1_Buffer(benchmark::State& state) {
    char templ[SIZE1];
    memset(templ, 'x', SIZE1);
    for (auto _ : state) {
        FixedBuffer<SIZE1> fb;
        fb.append(templ, SIZE1);
        benchmark::DoNotOptimize(fb);
    }
}
// Register the function as a benchmark
BENCHMARK(BM_S1_Buffer);

// Define another benchmark
static void BM_S2_Buffer(benchmark::State& state) {
    char templ[SIZE2];
    memset(templ, 'x', SIZE2);
    for (auto _ : state) {
        FixedBuffer<SIZE2> fb;
        fb.append(templ, SIZE2);
        benchmark::DoNotOptimize(fb);
    }
}
BENCHMARK(BM_S2_Buffer);

BENCHMARK_MAIN();