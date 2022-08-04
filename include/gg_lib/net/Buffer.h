// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_BUFFER_H
#define GG_LIB_BUFFER_H

#include "gg_lib/copyable.h"
#include "gg_lib/net/SocketsHelper.h"

#include <vector>

namespace gg_lib {
    namespace net {
        class Buffer : public copyable {
        public:
            static constexpr size_t kCheapPrepend = 8;
            static constexpr size_t kInitialSize = 1024;
            static constexpr char kCRLF[] = "\r\n";

            explicit Buffer(size_t initialSize = kInitialSize)
                    : buffer_(kCheapPrepend + initialSize),
                      readerIndex_(kCheapPrepend),
                      writerIndex_(kCheapPrepend) {
                assert(readableBytes() == 0);
                assert(writableBytes() == initialSize);
                assert(prependableBytes() == kCheapPrepend);
            }

            void swap(Buffer &rhs) {
                buffer_.swap(rhs.buffer_);
                std::swap(readerIndex_, rhs.readerIndex_);
                std::swap(writerIndex_, rhs.writerIndex_);
            }

            size_t readableBytes() const { return writerIndex_ - readerIndex_; }

            size_t writableBytes() const { return buffer_.size() - writerIndex_; }

            size_t prependableBytes() const { return readerIndex_; }

            const char *peek() const { return begin() + readerIndex_; }

            const char *findCRLF(const char *begin = nullptr) const {
                const char *start = begin ? begin : peek();
                assert(peek() <= start);
                assert(start <= beginWrite());
                const char *crlf = static_cast<const char *>(
                        memmem(start, beginWrite() - start, kCRLF, 2));
                return crlf == beginWrite() ? nullptr : crlf;
            }

            const char *findEOL(const char *begin = nullptr) const {
                const char *start = begin ? begin : peek();
                assert(peek() <= start);
                assert(start <= beginWrite());
                const char *eol = static_cast<const char *>(
                        memchr(start, '\n', beginWrite() - start));
                return eol;
            }

            void retrieve(size_t len) {
                assert(len <= readableBytes());
                if (len < readableBytes()) {
                    readerIndex_ += len;
                } else {
                    retrieveAll();
                }
            }

            void retrieveUntil(const char *end) {
                assert(peek() <= end);
                assert(end <= beginWrite());
                retrieve(end - peek());
            }

            void retrieveInt64() { retrieve(sizeof(int64_t)); }

            void retrieveInt32() { retrieve(sizeof(int32_t)); }

            void retrieveInt16() { retrieve(sizeof(int16_t)); }

            void retrieveInt8() { retrieve(sizeof(int8_t)); }

            void retrieveAll() {
                readerIndex_ = kCheapPrepend;
                writerIndex_ = kCheapPrepend;
            }

            string retrieveAllAsString() { return retrieveAsString(readableBytes()); }

            string retrieveAsString(size_t len) {
                assert(len <= readableBytes());
                string result(peek(), len);
                retrieve(len);
                return result;
            }

            string_view toStringView() const {
                return {peek(), static_cast<size_t>(readableBytes())};
            }

            void append(const string_view &str) {
                append(str.data(), str.size());
            }

            void append(const char *data, size_t len) {
                ensureWritableBytes(len);
                std::copy(data, data + len, beginWrite());
                hasWritten(len);
            }

            void append(const void *data, size_t len) {
                append(static_cast<const char *>(data), len);
            }

            void ensureWritableBytes(size_t len) {
                if (writableBytes() < len) {
                    makeSpace(len);
                }
                assert(writableBytes() >= len);
            }

            char *beginWrite() { return begin() + writerIndex_; }

            const char *beginWrite() const { return begin() + writerIndex_; }

            void hasWritten(size_t len) {
                assert(len <= writableBytes());
                writerIndex_ += len;
            }

            void unWrite(size_t len) {
                assert(len <= readableBytes());
                writerIndex_ -= len;
            }

            void appendInt64(int64_t x) {
                auto be64 = static_cast<int64_t>(sockets::hostToNetwork64(x));
                append(&be64, sizeof be64);
            }

            void appendInt32(int32_t x) {
                auto be32 = static_cast<int32_t>(sockets::hostToNetwork32(x));
                append(&be32, sizeof be32);
            }

            void appendInt16(int16_t x) {
                auto be16 = static_cast<int16_t>(sockets::hostToNetwork16(x));
                append(&be16, sizeof be16);
            }

            void appendInt8(int8_t x) {
                append(&x, sizeof x);
            }

            int64_t readInt64() {
                int64_t result = peekInt64();
                retrieveInt64();
                return result;
            }

            int32_t readInt32() {
                int32_t result = peekInt32();
                retrieveInt32();
                return result;
            }

            int16_t readInt16() {
                int16_t result = peekInt16();
                retrieveInt16();
                return result;
            }

            int8_t readInt8() {
                int8_t result = peekInt8();
                retrieveInt8();
                return result;
            }

            int64_t peekInt64() const {
                assert(readableBytes() >= sizeof(int64_t));
                int64_t be64 = *reinterpret_cast<const int64_t *>(peek());
                return static_cast<int64_t>(sockets::networkToHost64(be64));
            }

            int32_t peekInt32() const {
                assert(readableBytes() >= sizeof(int32_t));
                int32_t be32 = *reinterpret_cast<const int32_t *>(peek());
                return static_cast<int32_t>(sockets::networkToHost32(be32));
            }

            int16_t peekInt16() const {
                assert(readableBytes() >= sizeof(int16_t));
                int16_t be16 = *reinterpret_cast<const int16_t *>(peek());
                return static_cast<int16_t>(sockets::networkToHost16(be16));
            }

            int8_t peekInt8() const {
                assert(readableBytes() >= sizeof(int8_t));
                int8_t x = *peek();
                return x;
            }

            void prependInt64(int64_t x) {
                auto be64 = static_cast<int64_t>(sockets::hostToNetwork64(x));
                prepend(&be64, sizeof be64);
            }

            void prependInt32(int32_t x) {
                auto be32 = static_cast<int32_t>(sockets::hostToNetwork32(x));
                prepend(&be32, sizeof be32);
            }

            void prependInt16(int16_t x) {
                auto be16 = static_cast<int16_t>(sockets::hostToNetwork16(x));
                prepend(&be16, sizeof be16);
            }

            void prependInt8(int8_t x) {
                prepend(&x, sizeof x);
            }

            void prepend(const void *data, size_t len) {
                assert(len <= prependableBytes());
                readerIndex_ -= len;
                const char *d = static_cast<const char *>(data);
                std::copy(d, d + len, begin() + readerIndex_);
            }

            void shrink(size_t reserve) {
                buffer_.shrink_to_fit();
                Buffer other;
                other.ensureWritableBytes(readableBytes() + reserve);
                other.append(toStringView());
                swap(other);
            }

            size_t internalCapacity() const {
                return buffer_.capacity();
            }

            ssize_t readFd(int fd, int *savedErrno) {
                char extrabuf[65536];
                struct iovec vec[2];
                const size_t writable = writableBytes();
                vec[0].iov_base = begin() + writerIndex_;
                vec[0].iov_len = writable;
                vec[1].iov_base = extrabuf;
                vec[1].iov_len = sizeof extrabuf;
                const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
                const ssize_t n = sockets::readv(fd, vec, iovcnt);
                if (n < 0) {
                    *savedErrno = errno;
                } else if (static_cast<size_t>(n) <= writable) {
                    writerIndex_ += n;
                } else {
                    writerIndex_ = buffer_.size();
                    append(extrabuf, n - writable);
                }
                /// Don't loop read here, cause peer may send lots of data which can crash program.
                return n;
            }

            /// Only for debug use.
            void reset() {
                readerIndex_ = kCheapPrepend;
                writerIndex_ = kCheapPrepend;
                buffer_.clear();
            }

        private:
            char *begin() { return buffer_.data(); }

            const char *begin() const { return buffer_.data(); }

            void makeSpace(size_t len) {
                if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
                    buffer_.resize(writerIndex_ + len);
                }
                /// if possible, move readable data to the front.
                if (kCheapPrepend < readerIndex_) {
                    size_t readable = readableBytes();
                    std::copy(begin() + readerIndex_,
                              begin() + writerIndex_,
                              begin() + kCheapPrepend);
                    readerIndex_ = kCheapPrepend;
                    writerIndex_ = readerIndex_ + readable;
                    assert(readable == readableBytes());
                }

            }

            std::vector<char> buffer_;
            size_t readerIndex_;
            size_t writerIndex_;
        };
    }
}

#endif //GG_LIB_BUFFER_H
