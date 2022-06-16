// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/FileUtil.h"
#include "gg_lib/Logging.h"
#include "gg_lib/TimeZone.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include <utility>

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
        fprintf(stderr, "AppendFile::append() failed %s\n", strerror_tr(err_));
        return;
    }
    size_t written = 0;
    while (written < len) {
        size_t remain = len - written;
        size_t n = write(data + written, remain);
        if (n != remain) {
            err_ = ferror(fp_);
            if (err_) {
                fprintf(stderr, "AppendFile::append() failed %s\n", strerror_tr(err_));
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


LogFile::LogFile(string basename,
                 off_t rollSize,
                 bool threadSafe,
                 int flushInterval)
        : basename_(std::move(basename)),
          rollSize_(rollSize),
          flushInterval_(flushInterval),
          startOfPeriod_(0),
          lastRoll_(0),
          lastFlush_(0),
          mutex_(threadSafe ? new std::mutex : nullptr) {
    if (g_logTimeZone) {
        offset_ = g_logTimeZone.getTimeOff();
    } else {
        offset_ = 0;
    }
    rollFile();
}

LogFile::~LogFile() = default;

void LogFile::append(const char *logline, int len) {
    if (mutex_) {
        std::lock_guard<std::mutex> lock(*mutex_);
        append_unlocked(logline, len);
    } else {
        append_unlocked(logline, len);
    }
}

void LogFile::flush() {
    if (mutex_) {
        std::lock_guard<std::mutex> lock(*mutex_);
        file_->flush();
    } else {
        file_->flush();
    }
}

void LogFile::append_unlocked(const char* logline, int len) {
    file_->append(logline, len);
    if (file_ -> writtenBytes() > rollSize_) {
        rollFile();
    } else {
        time_t now = ::time(nullptr) + offset_;
        time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
        if (thisPeriod_ != startOfPeriod_) {
            rollFile();
        } else if (now - lastFlush_ > flushInterval_) {
            lastFlush_ = now;
            file_->flush();
        }
    }
}

bool LogFile::rollFile() {
    time_t now = 0;
    string filename = getLogFileName(&now);
    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

    if (now > lastRoll_) {
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        file_.reset(new AppendFile(filename));
        return true;
    }
    return false;
}

string LogFile::getLogFileName(time_t* now) {
    string filename;
    filename.reserve(basename_.size() + 64);
    filename = basename_;

    char timebuf[32];
    struct tm tm{};
    *now = time(nullptr) + offset_;
    gmtime_r(now, &tm);
    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S", &tm);
    filename += timebuf;
    char pidbuf[32];
    snprintf(pidbuf, sizeof pidbuf, ".%d", ::getpid());
    filename += pidbuf;
    filename += ".log";
    return filename;
}


