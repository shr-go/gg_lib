// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/ThreadHelper.h"

#include <unistd.h>

using namespace gg_lib;

void threadFunc1() {
    printf("thread=%s\n", CurrentThread::name());
}

void threadFunc2(int x) {
    printf("thread=%s, x=%d\n", CurrentThread::name(), x);
}

void threadFunc3() {
    printf("thread=%s\n", CurrentThread::name());
    sleep(1);
}

class Foo {
public:
    explicit Foo(int x): x_(x) {};
    void memberFunc1() {
        printf("thread=%s, Foo::x_=%d\n", CurrentThread::name(), x_);
    }
    void memberFunc2(const std::string& text) {
        printf("thread=%s, Foo::x_=%dm text=%s\n", CurrentThread::name(), x_, text.c_str());
    }
private:
    int x_;
};

int main() {
    printf("thread=%s\n", CurrentThread::name());
    Thread t1(threadFunc1);
    t1.start();
    t1.join();

    Thread t2(std::bind(threadFunc2, 42),"thread for free function with argument");
    t2.start();
    t2.join();

    Foo foo(100);
    Thread t3(std::bind(&Foo::memberFunc1, &foo),"thread for member function without argument");
    t3.start();
    t3.join();

    Thread t4(std::bind(&Foo::memberFunc2, &foo, std::string("Thread4")));
    t4.start();
    t4.join();

    {
        Thread t5(threadFunc3);
        t5.start();
    }
    sleep(2);
    {
        Thread t6(threadFunc3);
        t6.start();
        sleep(2);
    }
    sleep(2);
    printf("number of created threads %d\n", Thread::numCreated());
}
