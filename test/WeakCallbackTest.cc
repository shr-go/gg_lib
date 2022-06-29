// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/WeakCallback.h"
#include "gg_lib/Logging.h"
#include <memory>
#include <functional>

using namespace gg_lib;
using namespace std::placeholders;

typedef std::function<void()> Func;

class Foo: public std::enable_shared_from_this<Foo> {
public:
    explicit Foo(int foo_) :foo(foo_) {}
    void print() {
        LOG_INFO << Fmt("Foo={}", foo);
    }
    Func getPrint() {
        return std::bind(&Foo::print, this);
    }
    Func getWeakPrint() {
        return makeWeakCallback(shared_from_this(), &Foo::print);
    }

private:
    int foo;
};

int main() {
    Logger::setLogLevel(Logger::TRACE);
    std::function<void()> func;
    {
        auto f1 = std::make_shared<Foo>(10);
        func = f1->getWeakPrint();
        func();
    }
    char buf1[1024];
    Foo f2(20);
    Foo f3(30);
    Foo f4(40);
    char buf2[1024];
    func();
}
