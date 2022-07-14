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
        constexpr any() noexcept
                : content_(nullptr) {}

        template<typename ValueType>
        any(const ValueType &value)
                : content_(new holder<typename std::decay<ValueType>::type>(value)) {}

        any(const any &other)
                : content_(other.content_ ? other.content_->clone() : nullptr) {}

        any(any &&other) noexcept
                : content_(other.content_) {
            other.content_ = nullptr;
        }

        template<typename ValueType>
        any(ValueType &&value,
            typename std::enable_if<!std::is_same<any &, ValueType>::value>::type * = 0,
            typename std::enable_if<!std::is_const<ValueType>::value>::type * = 0)
                : content_(new holder<typename std::decay<ValueType>::type>(static_cast<ValueType &&>(value))) {}

        ~any() noexcept {
            delete content_;
        }

        any &swap(any &rhs) noexcept {
            placeholder *tmp = content_;
            content_ = rhs.content_;
            rhs.content_ = tmp;
            return *this;
        }

        any &operator=(const any &rhs) {
            any(rhs).swap(*this);
            return *this;
        }

        any &operator=(any &&rhs) noexcept {
            rhs.swap(*this);
            any().swap(rhs);
            return *this;
        }

        template<typename ValueType>
        any &operator=(ValueType &&rhs) {
            any(std::forward<ValueType>(rhs)).swap(*this);
            return *this;
        }

        bool empty() const noexcept {
            return !content_;
        }

        void clear() noexcept {
            any().swap(*this);
        }

        const std::type_info &type() const noexcept {
            return content_ ? content_->type() : typeid(void);
        }


    private:
        class placeholder {
        public:
            virtual ~placeholder() = default;

            virtual const std::type_info &type() const = 0;

            virtual placeholder *clone() const = 0;
        };

        template<typename ValueType>
        class holder final : public placeholder {
        public:
            holder(const ValueType &value) : held(value) {}

            holder(ValueType &&value) : held(static_cast< ValueType && >(value)) {};

            holder &operator=(const holder &) = delete;

            const std::type_info &type() const noexcept override {
                return typeid(ValueType);
            }

            placeholder *clone() const override {
                return new holder(held);
            }

            ValueType held;

        };

        template<typename ValueType>
        friend ValueType *any_cast(any *) noexcept;

        placeholder *content_;
    };

    inline void swap(any &lhs, any &rhs) noexcept {
        lhs.swap(rhs);
    }

    template<typename ValueType>
    inline ValueType *any_cast(any *operand) noexcept {
        return operand && operand->type() == typeid(ValueType)
               ? std::addressof(
                        static_cast<any::holder<typename std::remove_cv<ValueType>::type> *>(operand->content_)->held)
               : nullptr;
    }

    template<typename ValueType>
    inline const ValueType *any_cast(const any *operand) noexcept {
        return any_cast<ValueType>(const_cast<any *>(operand));
    }

    template<typename ValueType>
    inline ValueType any_cast(any &operand) {
        typedef typename std::remove_reference<ValueType>::type nonref;
        nonref *result = any_cast<nonref>(std::addressof(operand));
        if (!result)
            throw bad_any_cast();
        typedef typename std::conditional<std::is_reference<ValueType>::value,
                ValueType,
                typename std::add_lvalue_reference<ValueType>::type>::type ref_type;
        return static_cast<ref_type>(*result);
    }

    template<typename ValueType>
    inline ValueType any_cast(const any &operand) {
        typedef typename std::remove_reference<ValueType>::type nonref;
        return any_cast<const nonref &>(const_cast<any &>(operand));
    }

    template<typename ValueType>
    inline ValueType any_cast(any &&operand) {
        static_assert(std::is_rvalue_reference<ValueType &&>::value
                      || std::is_const<typename std::remove_reference<ValueType>::type>::value,
                      "any_cast shall not be used for getting nonconst references to temporary objects");
        return any_cast<ValueType>(operand);
    }

    template<typename ValueType, typename... Args>
    inline any make_any(Args &&... args) {
        static_assert(std::is_constructible<typename std::decay<ValueType>::type, Args &&...>::value
                      || std::is_copy_constructible<typename std::decay<ValueType>::type>::value,
                      "make_any can not construct value with these args");
        return any(ValueType(std::forward<Args>(args)...));
    }

    template<typename ValueType, typename U, typename... Args>
    inline any make_any(std::initializer_list<U> il, Args &&... args) {
        static_assert(std::is_constructible<typename std::decay<ValueType>::type, std::initializer_list<U>&, Args &&...>::value
                      || std::is_copy_constructible<typename std::decay<ValueType>::type>::value,
                      "make_any can not construct value with these args");
        return any(ValueType(il, std::forward<Args>(args)...));
    }

}

#endif //GG_LIB_ANY_H
