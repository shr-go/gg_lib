set(GG_LIB_SRC
        AsyncLogging.cc
        CountDownLatch.cc
        Date.cc
        FileUtil.cc
        Logging.cc
        LogStream.cc
        ThreadHelper.cc
        ThreadPool.cc
        Timestamp.cc
        TimeZone.cc
        net/EventLoop.cc
        )

add_library(gg_lib ${GG_LIB_SRC})
target_link_libraries(gg_lib pthread rt)
