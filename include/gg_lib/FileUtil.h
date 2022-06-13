// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_FILEUTIL_H
#define GG_LIB_FILEUTIL_H

#include "gg_lib/Utils.h"
#include "gg_lib/noncopyable.h"
#include <sys/types.h>

namespace gg_lib {
    class ReadSmallFile : noncopyable {
    public:
        explicit ReadSmallFile(StringArg filename);

        ~ReadSmallFile();

        template<typename String>
        int readToString(int maxSize,
                         String *content,
                         int64_t *fileSize);

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
}

#endif //GG_LIB_FILEUTIL_H
