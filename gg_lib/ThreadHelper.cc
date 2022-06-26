// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/ThreadHelper.h"
#include "gg_lib/Logging.h"
#include <pthread.h>
#include <utility>
#include <sys/prctl.h>

namespace gg_lib {
    void afterFork() {
        gg_lib::CurrentThread::t_tidStringLength = 0;
        gg_lib::CurrentThread::t_threadName = "main";
        CurrentThread::cacheTid();
    }

    struct ThreadData {
        typedef Thread::ThreadFunc ThreadFunc;
        ThreadFunc func_;
        string name_;
        CountDownLatch *latch_;

        ThreadData(ThreadFunc &func,
                   string name,
                   CountDownLatch *latch)
                : func_(std::move(func)),
                  name_(std::move(name)),
                  latch_(latch) {}

        void runInThread() {
            CurrentThread::cacheTid();
            latch_->countDown();
            latch_ = nullptr;
            CurrentThread::t_threadName = name_.empty() ? "ggThread" : name_.c_str();
#ifdef PR_SET_NAME
            ::prctl(PR_SET_NAME, CurrentThread::t_threadName);
#endif
            try {
                func_();
                CurrentThread::t_threadName = "finished";
            } catch (const std::exception &ex) {
                CurrentThread::t_threadName = "crashed";
                fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
                fprintf(stderr, "reason: %s\n", ex.what());
                abort();
            } catch (...) {
                CurrentThread::t_threadName = "crashed";
                fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
                throw; // rethrow
            }
        }
    };

    void startThread(ThreadData *data) {
        data->runInThread();
        delete data;
    }

    Thread::Thread(Thread &&th) noexcept
            : started_(th.started_),
              joined_(false),
              thread_(std::move(th.thread_)),
              latch_(1),
              func_(std::move(th.func_)),
              name_(std::move(th.name_)) {
        assert(!started_);
    }

    Thread::~Thread() {
        if (started_ && !joined_) {
            thread_.detach();
        }
    }

    void Thread::start() {
        if (__builtin_expect(started_, false))
            return;
        started_ = true;
        auto *data = new ThreadData(func_, name_, &latch_);
        try {
            thread_ = std::thread(startThread, data);
        } catch (...) {
            started_ = false;
            fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
            LOG_SYSFATAL << "Failed in Thread::start";
        }
        latch_.wait();
    }

    void Thread::join() {
        assert(started_);
        assert(!joined_);
        joined_ = true;
        thread_.join();
    }

    void Thread::setDefaultName() {
        int num = numCreated_.fetch_add(1);
        if (name_.empty()) {
            char buf[32];
            snprintf(buf, sizeof buf, "Thread%d", num);
            name_ = buf;
        }
    }

    AtomicInt32 Thread::numCreated_;

    namespace CurrentThread {
        __thread char t_tidString[32];
        __thread int t_tidStringLength = 0;
        __thread const char *t_threadName = "unknown";
        thread_local std::thread::id t_tid;

        void cacheTid() {
            if (t_tidStringLength == 0) {
                t_tid = std::this_thread::get_id();
                t_tidStringLength =
                        snprintf(t_tidString, sizeof t_tidString, "%s ", Thread::tidToString(t_tid).c_str());
            }
        }

        const std::thread::id& tid() {
            if (__builtin_expect(t_tidStringLength == 0, false)) {
                cacheTid();
            }
            return t_tid;
        }
    }

    class ThreadNameInitializer {
    public:
        ThreadNameInitializer() {
            CurrentThread::t_threadName = "main";
            CurrentThread::cacheTid();
            pthread_atfork(nullptr, nullptr, &afterFork);
        }
    };

    ThreadNameInitializer init;
}
