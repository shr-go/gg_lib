# benchmark part
find_package(benchmark REQUIRED)

set(BenchmarkSrc
        NumStringBench.cc
        FixedBufferBench.cc
        TimestampBench.cc
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
        AnyTest.cc
        BlockingQueueTest.cc
        DateTest.cc
        FixedBufferTest.cc
        LogStreamTest.cc
        TimestampTest.cc
        net/BufferTest.cc
        )

foreach(File IN LISTS TestSrc)
    get_filename_component(ExeName ${File} NAME_WE)
    add_executable(${ExeName} ${File})
    target_link_libraries(${ExeName} gg_lib GTest::gtest GTest::gtest_main)
    gtest_discover_tests(${ExeName})
endforeach()

# test binary
set(TestBinarySrc
        ThreadTest.cc
        ThreadPoolTest.cc
        WeakCallbackTest.cc
        net/AcceptorTest.cc
        net/EventLoopTest.cc
        net/EventLoopThreadPoolTest.cc
        net/EventLoopThreadTest.cc
        net/TimerQueueTest.cc
        )

foreach(File IN LISTS TestBinarySrc)
    get_filename_component(ExeName ${File} NAME_WE)
    add_executable(${ExeName} ${File})
    target_link_libraries(${ExeName} gg_lib)
endforeach()
