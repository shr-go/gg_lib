// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/ThreadPool.h"
#include "gg_lib/Logging.h"

#include <unistd.h>

using namespace gg_lib;

void print() {
    printf("thread=%s\n", CurrentThread::tidString());
}

void printString(const std::string &str) {
    LOG_INFO << str;
    usleep(100 * 1000);
}

void test1(int maxSize) {
    LOG_WARN << "Test ThreadPool with max queue size = " << maxSize;
    ThreadPool pool("MainThreadPool");
    pool.setMaxQueueSize(maxSize);
    pool.start(5);

    LOG_WARN << "Adding";
    pool.run(print);
    pool.run(print);
    for (int i = 0; i < 100; ++i) {
        char buf[32];
        snprintf(buf, sizeof buf, "task %d", i);
        pool.run(std::bind(printString, std::string(buf)));
    }
    LOG_WARN << "Done";

    CountDownLatch latch(1);
    pool.run(std::bind(&CountDownLatch::countDown, &latch));
    latch.wait();
    pool.stop();
}

void longTask(int num) {
    LOG_INFO << "longTask " << num;
    std::this_thread::sleep_for(std::chrono::seconds(3));
}

void test2() {
    LOG_WARN << "Test ThreadPool by stopping early.";
    ThreadPool pool("ThreadPool");
    pool.setMaxQueueSize(5);
    pool.start(3);

    Thread thread1([&pool]() {
        for (int i = 0; i < 20; ++i) {
            pool.run(std::bind(longTask, i));
        }
    }, "thread1");
    thread1.start();

    std::this_thread::sleep_for(std::chrono::seconds(5));
    LOG_WARN << "stop pool";
    pool.stop();  // early stop

    thread1.join();
    // run() after stop()
    pool.run(print);
    LOG_WARN << "test2 Done";
}

int main() {
    test1(0);
    test1(1);
    test1(5);
    test1(10);
    test1(50);
    test2();
}
