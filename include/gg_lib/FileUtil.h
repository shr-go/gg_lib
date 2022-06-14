// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_FILEUTIL_H
#define GG_LIB_FILEUTIL_H

#include "gg_lib/Utils.h"
#include "gg_lib/noncopyable.h"
#include "gg_lib/FixedBuffer.h"
#include <sys/types.h>

#include <stdexcept>
#include <endian.h>

namespace gg_lib {
    class ReadSmallFile : noncopyable {
    public:
        explicit ReadSmallFile(StringArg filename);

        ~ReadSmallFile();

        template<typename String>
        int readToString(int maxSize,
                         String *content,
                         int64_t *fileSize);

        explicit operator bool() const {
            return fd_ > 0 && !err_;
        }

        static constexpr int kBufferSize = 64 * 1024;
    private:
        int fd_;
        int err_;
        char buf_[kBufferSize];
    };

    template<typename String>
    int readFile(StringArg filename,
                 int maxSize,
                 String *content,
                 int64_t *fileSize = nullptr) {
        ReadSmallFile file(filename);
        return file.readToString(maxSize, content, fileSize);
    }

    class AppendFile : noncopyable {
    public:
        explicit AppendFile(StringArg filename);

        ~AppendFile();

        void append(const char *data, size_t len);

        bool valid() const { return fp_ != nullptr; }

        void flush();

        off_t writtenBytes() const { return writtenBytes_; }

        static constexpr int kBufferSize = 64 * 1024;

    private:
        size_t write(const char *data, size_t len);

        FILE *fp_;
        int err_;
        char buffer_[kBufferSize];
        off_t writtenBytes_;
    };

    class ReadFile : noncopyable {
    public:
        explicit ReadFile(StringArg filename)
                : fp_(::fopen(filename.c_str(), "rb")) {}

        ~ReadFile() {
            if (fp_)
                ::fclose(fp_);
        }

        explicit operator bool() const {
            return fp_;
        }

        void passBytes(int n) {
            fseek(fp_, n, SEEK_CUR);
        }

        string readBytes(int n) {
            string bytes(n, '\0');
            size_t nr = ::fread(&bytes[0], 1, n, fp_);
            if (nr != n)
                throw std::logic_error("no enough data");
            return bytes;
        }

        int32_t readInt32()
        {
            int32_t x = 0;
            size_t nr = ::fread(&x, 1, sizeof(int32_t), fp_);
            if (nr != sizeof(int32_t))
                throw std::logic_error("bad int32_t data");
            return x;
        }

        uint8_t readUInt8()
        {
            uint8_t x = 0;
            size_t nr = ::fread(&x, 1, sizeof(uint8_t), fp_);
            if (nr != sizeof(uint8_t))
                throw std::logic_error("bad uint8_t data");
            return x;
        }

    private:
        FILE *fp_;
    };
}

#endif //GG_LIB_FILEUTIL_H
