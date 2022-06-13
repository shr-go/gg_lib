// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_COMPARABLE_H
#define GG_LIB_COMPARABLE_H

namespace gg_lib {
    template<typename T>
    class EqualCompareT {
        friend bool operator!=(const T &lhs, const T &rhs) {
            return !(lhs == rhs);
        }
    };

    template<typename T>
    class LessCompareT {
        friend bool operator>(const T &lhs, const T &rhs) {
            return rhs < lhs;
        }

        friend bool operator<=(const T &lhs, const T &rhs) {
            return !(rhs < lhs);
        }

        friend bool operator>=(const T &lhs, const T &rhs) {
            return !(lhs < rhs);
        }
    };
}


#endif //GG_LIB_COMPARABLE_H
