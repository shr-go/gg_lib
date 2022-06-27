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
        net/Acceptor.cc
        net/Channel.cc
        net/EventLoop.cc
        net/EventLoopThread.cc
        net/EventLoopThreadPool.cc
        net/Poller.cc
        net/SocketsHelper.cc
        net/TcpConnection.cc
        net/TcpServer.cc
        net/TimerQueue.cc
        net/poller/DefaultPoller.cc
        net/poller/PollPoller.cc
        )

add_library(gg_lib ${GG_LIB_SRC})
target_link_libraries(gg_lib pthread rt)
