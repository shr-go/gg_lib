# benchmark part
find_package(benchmark REQUIRED)

set(BenchmarkSrc
        NumStringBench.cc
        FixedBufferBench.cc
        )

foreach(File IN LISTS BenchmarkSrc)
    get_filename_component(ExeName ${File} NAME_WE)
    add_executable(${ExeName} ${File})
    target_link_libraries(${ExeName} gg_lib benchmark::benchmark)
endforeach()

# test part
enable_testing()
find_package(GTest REQUIRED)
include(GoogleTest)

set(TestSrc
        FixedBufferTest.cc
        BlockingQueueTest.cc
        TimestampTest.cc
        LogStreamTest.cc
        DateTest.cc
        )

foreach(File IN LISTS TestSrc)
    get_filename_component(ExeName ${File} NAME_WE)
    add_executable(${ExeName} ${File})
    target_link_libraries(${ExeName} gg_lib GTest::gtest GTest::gtest_main)
    gtest_discover_tests(${ExeName})
endforeach()

# example part
set(ExampleSrc
        LoggingExample.cc
        )

foreach(File IN LISTS ExampleSrc)
    get_filename_component(ExeName ${File} NAME_WE)
    add_executable(${ExeName} ${File})
    target_link_libraries(${ExeName} gg_lib)
endforeach()
