// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_THREADHELPER_H
#define GG_LIB_THREADHELPER_H

#include "gg_lib/noncopyable.h"
#include "gg_lib/Utils.h"
#include "gg_lib/CountDownLatch.h"
#include <functional>
#include <thread>
#include <atomic>
#include <sstream>

namespace gg_lib {
    class Thread : noncopyable {
    public:
        typedef std::function<void()> ThreadFunc;

        template<typename TF, typename TS = string>
        explicit Thread(TF &&func, TS &&name = TS())
                : started_(false),
                  joined_(false),
                  thread_(),
                  latch_(1) {
            static_assert(std::is_convertible<TF, ThreadFunc>::value, "TF is not a ThreadFunc.");
            static_assert(std::is_convertible<TS, string>::value, "TS can not convert to string.");
            func_ = std::forward<TF>(func);
            name_ = std::forward<TS>(name);
            setDefaultName();
        }

        Thread(Thread &&th) noexcept;

//        explicit Thread(ThreadFunc, string = "");

        ~Thread();

        void start();

        void join();

        bool started() const { return started_; }

        const string &name() const { return name_; }

        static int numCreated() { return numCreated_.load(); }


    private:
        void setDefaultName();

        bool started_;
        bool joined_;
        std::thread thread_;
        ThreadFunc func_;
        string name_;
        CountDownLatch latch_;
        static AtomicInt32 numCreated_;
    };

    namespace CurrentThread {
        extern __thread char t_tidString[32];
        extern __thread int t_tidStringLength;
        extern __thread const char *t_threadName;

        void cacheTid();

        inline const char *tidString() {
            if (__builtin_expect(t_tidStringLength == 0, false)) {
                cacheTid();
            }
            return t_tidString;
        }

        inline int tidStringLength() {
            return t_tidStringLength;
        }

        inline const char *name() {
            return t_threadName;
        }
    }
}

#endif //GG_LIB_THREADHELPER_H
