// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/Logging.h"

#include "gg_lib/net/EventLoop.h"
#include "gg_lib/net/Channel.h"
#include "gg_lib/net/TimerQueue.h"
#include "gg_lib/net/Poller.h"
#include "gg_lib/net/SocketsHelper.h"

#include <algorithm>
#include <csignal>

#include <sys/eventfd.h>
#include <unistd.h>

using namespace gg_lib;
using namespace gg_lib::net;

namespace {
    __thread EventLoop *t_loopInThisThread = nullptr;
    const int kPollTimeMs = 1000;

    int createEventFd() {
        int eventFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (eventFd < 0) {
            LOG_SYSFATAL << "Failed in eventFd";
        }
        return eventFd;
    }

    class SetSignal {
    public:
        SetSignal() {
            ::signal(SIGPIPE, SIG_IGN);
        }
    };

    SetSignal setSignal;
}

EventLoop *EventLoop::getEventLoopOfCurrentThread() {
    return t_loopInThisThread;
}

EventLoop::EventLoop()
        : looping_(false),
          quit_(false),
          eventHandling_(false),
          callingPendingFunctors_(false),
          iteration_(0),
          threadId_(CurrentThread::tid()),
          poller_(Poller::newDefaultPoller(this)),
          timerQueue_(new TimerQueue(this)),
          wakeupFd_(createEventFd()),
          wakeupChannel_(new Channel(this, wakeupFd_)),
          currentActiveChannel_(nullptr) {
    LOG_DEBUG << "EventLoop created " << this << " in thread" << CurrentThread::tidString();
    if (t_loopInThisThread) {
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread
                  << " exists in this thread " << CurrentThread::tidString();
    } else {
        t_loopInThisThread = this;
    }
    wakeupChannel_->setReadCallback(
            std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    LOG_DEBUG << "EventLoop " << this << " of thread " << Thread::tidToString(threadId_)
              << " destructs in thread " << CurrentThread::tidString();
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;
    LOG_TRACE << "EventLoop " << this << " start looping";
    while (true) {
        {
            std::lock_guard<std::mutex> lk(mutex_);
            if (quit_)
                break;
        }
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        ++iteration_;
        if (canLevelLog(Logger::TRACE)) {
            printActiveChannels();
        }
        eventHandling_ = true;
        for (Channel *channel: activeChannels_) {
            currentActiveChannel_ = channel;
            currentActiveChannel_->handleEvent(pollReturnTime_);
        }
        currentActiveChannel_ = nullptr;
        eventHandling_ = false;
        doPendingFunctors();
    }
    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::quit() {
    std::lock_guard<std::mutex> lk(mutex_);
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb) {
    {
        std::lock_guard<std::mutex> lk(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }
    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

TimerId EventLoop::runAt(Timestamp time, TimerCallback cb) {
    return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback cb) {
    Timestamp time = Timestamp::addTime(Timestamp::now(), delay);
    return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb) {
    Timestamp time = Timestamp::addTime(Timestamp::now(), interval);
    return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(TimerId timerId) {
    return timerQueue_->cancel(timerId);
}

#define CHANNEL_IN_LOOP(channel) \
    ({assert((channel)->ownerLoop() == this); \
    assertInLoopThread();})

void EventLoop::updateChannel(Channel *channel) {
    CHANNEL_IN_LOOP(channel);
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
    CHANNEL_IN_LOOP(channel);
    if (eventHandling_) {
        assert(currentActiveChannel_ == channel ||
               std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
    }
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel) {
    CHANNEL_IN_LOOP(channel);
    return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << Thread::tidToString(threadId_)
              << ", current thread id = " << CurrentThread::tidString();
}

void EventLoop::wakeup() const {
    uint64_t one = 1;
    ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::handleRead() const {
    uint64_t one = 1;
    ssize_t n = sockets::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::lock_guard<std::mutex> lk(mutex_);
        functors.swap(pendingFunctors_);
    }
    for (const Functor &functor: functors) {
        functor();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels() const {
    for (const Channel *channel: activeChannels_) {
        LOG_TRACE << "{" << channel->reventsToString() << "}";
    }
}
