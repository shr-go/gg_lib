find_package(benchmark REQUIRED)
enable_testing()
find_package(GTest REQUIRED)

# benchmark part

#add_executable(FixedBufferBench FixedBufferBench.cpp)
#target_link_libraries(FixedBufferBench benchmark::benchmark)

add_executable(NumStringBench NumStringBench.cc)
target_link_libraries(NumStringBench benchmark::benchmark)

# test part

add_executable(FixedBufferTest FixedBufferTest.cc)
target_link_libraries(FixedBufferTest GTest::gtest GTest::gtest_main)

add_executable(BlockingQueueTest BlockingQueueTest.cc)
target_link_libraries(BlockingQueueTest GTest::gtest GTest::gtest_main)

add_executable(TimestampTest TimestampTest.cc)
target_link_libraries(TimestampTest gg_lib GTest::gtest GTest::gtest_main)

add_executable(LogStreamTest LogStreamTest.cc)
target_link_libraries(LogStreamTest gg_lib GTest::gtest GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(FixedBufferTest BlockingQueueTest)
