// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_NONCOPYABLE_H
#define GG_LIB_NONCOPYABLE_H

namespace gg_lib {
    class noncopyable {
    public:
        noncopyable(const noncopyable &) = delete;

        noncopyable operator=(const noncopyable &) = delete;

    protected:
        noncopyable() = default;

        ~noncopyable() = default;
    };
}

#endif //GG_LIB_NONCOPYABLE_H
