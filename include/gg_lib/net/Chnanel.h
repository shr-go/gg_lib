// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_CHNANEL_H
#define GG_LIB_CHNANEL_H

#include "gg_lib/noncopyable.h"
#include "gg_lib/Timestamp.h"

#include <functional>
#include <memory>
#include <poll.h>

namespace gg_lib {
    namespace net {
        class EventLoop;

        class Channel : noncopyable {
        public:
            typedef std::function<void()> EventCallback;
            typedef std::function<void(Timestamp)> ReadEventCallback;

            Channel(EventLoop *loop, int fd);

            ~Channel();

            void handleEvent(Timestamp receiveTime);

            void setReadCallback(ReadEventCallback cb) {
                readCallback_ = std::move(cb);
            }

            void setWriteCallback(EventCallback cb) {
                writeCallback_ = std::move(cb);
            }

            void setCloseCallback(EventCallback cb) {
                closeCallback_ = std::move(cb);
            }

            void setErrorCallback(EventCallback cb) {
                errorCallback_ = std::move(cb);
            }

            void tie(std::weak_ptr<void> obj);

            int fd() const { return fd_; }

            int events() const { return events_; }

            void set_revents(int revt) { revents_ = revt; }

            bool isNoneEvent() const { return events_ == kNoneEvent; }

            void enableReading() {
                events_ |= kReadEvent;
                update();
            }

            void disableReading() {
                events_ &= ~kReadEvent;
                update();
            }

            void enableWriting() {
                events_ |= kWriteEvent;
                update();
            }

            void disableWriting() {
                events_ &= ~kWriteEvent;
                update();
            }

            void disableAll() {
                events_ = kNoneEvent;
                update();
            }

            bool isWriting() const { return events_ & kWriteEvent; }

            bool isReading() const { return events_ & kReadEvent; }

            int index() const { return index_; }

            void set_index(int idx) { index_ = idx; }

            // for debug
            string reventsToString() const;

            string eventsToString() const;

            void setLogHup(bool logHup) { logHup_ = logHup; }

            EventLoop* ownerLoop() { return loop_; }

            void remove();

        private:

            static string eventsToString(int fd, int ev);

            void update();

            void handleEventWithGuard(Timestamp receiveTime);

            static constexpr int kNoneEvent = 0;
            static constexpr int kReadEvent = POLLIN | POLLPRI;
            static constexpr int kWriteEvent = POLLOUT;

            EventLoop *loop_;
            const int fd_;
            int events_;
            int revents_;
            int index_;
            bool logHup_;

            std::weak_ptr<void> tie_;
            bool tied_;
            bool eventHandling_;
            bool addedToLoop_;
            ReadEventCallback readCallback_;
            EventCallback writeCallback_;
            EventCallback closeCallback_;
            EventCallback errorCallback_;
        };
    }
}

#endif //GG_LIB_CHNANEL_H
