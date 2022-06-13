// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/FileUtil.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

using namespace gg_lib;

ReadSmallFile::ReadSmallFile(StringArg filename)
        : fd_(open(filename.c_str(), O_RDONLY | O_CLOEXEC)),
          err_(0) {
    buf_[0] = '\0';
    if (fd_ < 0) {
        err_ = errno;
    }
}

ReadSmallFile::~ReadSmallFile() {
    if (fd_ >= 0) {
        close(fd_);
    }
}

template<typename String>
int ReadSmallFile::readToString(int maxSize, String *content, int64_t *fileSize) {
    assert(content != NULL);
    if (fd_ < 0 || err_) {
        return err_;
    }
    content->clear();
    int64_t _fileSize;
    struct stat statbuf;
    if (fstat(fd_, &statbuf) == 0) {
        if (S_ISREG(statbuf.st_mode)) {
            _fileSize = statbuf.st_size;
            content->reserve(static_cast<int>(std::min(static_cast<int64_t>(maxSize), _fileSize)));
        } else {
            err_ = EINVAL;
        }
    } else {
        err_ = errno;
    }
    if (err_) {
        return err_;
    }
    if (fileSize) {
        *fileSize = _fileSize;
    }
    while (content->size() < static_cast<size_t>(maxSize)) {
        size_t toRead = std::min(static_cast<size_t>(maxSize) - content->size(), sizeof(buf_));
        ssize_t n = ::read(fd_, buf_, toRead);
        if (n > 0) {
            content->append(buf_, n);
        } else {
            if (n < 0)
                err_ = errno;
            break;
        }
    }
    return err_;
}


AppendFile::AppendFile(StringArg filename)
        : fp_(fopen(filename.c_str(), "ae")),
          err_(0),
          writtenBytes_(0) {
    if (!fp_) {
        err_ = errno;
    } else {
        setbuffer(fp_, buffer_, kBufferSize);
    }
}

AppendFile::~AppendFile() {
    if (fp_) {
        fclose(fp_);
    }
}

void AppendFile::append(const char *data, size_t len) {
    if (err_) {
        fprintf(stderr, "AppendFile::append() failed %s\n", strerrordesc_np(err_));
        return;
    }
    size_t written = 0;
    while (written < len) {
        size_t remain = len - written;
        size_t n = write(data + written, remain);
        if (n != remain) {
            err_ = ferror(fp_);
            if (err_) {
                fprintf(stderr, "AppendFile::append() failed %s\n", strerrordesc_np(err_));
                break;
            }
        }
        written += n;
    }
    writtenBytes_ += static_cast<off_t>(written);
}

void AppendFile::flush() {
    fflush(fp_);
}

size_t AppendFile::write(const char *data, size_t len) {
    return fwrite_unlocked(data, 1, len, fp_);
}
