// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_ANY_H
#define GG_LIB_ANY_H

/// This is a simple any class implementation.

#include <typeinfo>
#include <type_traits>
#include <stdexcept>

namespace gg_lib {
    class bad_any_cast : public std::bad_cast {
    public:
        const char *what() const noexcept override {
            return "bad any cast";
        }
    };

    class any final {
    public:

    private:
        class placeholder {
        public:
            virtual ~placeholder(){};
            virtual const std::type_info& type() const = 0;
            virtual placeholder* clone() const = 0;
        };

        placeholder *content;
    };
}

#endif //GG_LIB_ANY_H
