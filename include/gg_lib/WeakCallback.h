// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_WEAKCALLBACK_H
#define GG_LIB_WEAKCALLBACK_H

#include <functional>
#include <memory>
#include <utility>

namespace gg_lib {
    template<typename CLASS, typename... ARGS>
    class WeakCallback {
    public:
        typedef std::weak_ptr<CLASS> WeakPtrType;
        typedef std::function<void(CLASS *, ARGS...)> FuncType;
        WeakCallback(const WeakPtrType &object,
                     const FuncType &function)
                : object_(object), function_(function) {}

        WeakCallback(WeakPtrType &&object,
                     FuncType &&function)
                : object_(std::forward<WeakPtrType>(object)),
                  function_(std::forward<FuncType>(function)) {}

        void operator()(ARGS &&... args) const {
            std::shared_ptr<CLASS> ptr(object_.lock());
            if (ptr) {
                function_(ptr.get(), std::forward<ARGS>(args)...);
            }
        }

    private:
        WeakPtrType object_;
        FuncType function_;
    };

    template<typename CLASS, typename... ARGS>
    WeakCallback<CLASS, ARGS...> makeWeakCallback(const std::shared_ptr<CLASS> &object,
                                                  void (CLASS::*function)(ARGS...)) {
        return WeakCallback<CLASS, ARGS...>(object, function);
    }

    template<typename CLASS, typename... ARGS>
    WeakCallback<CLASS, ARGS...> makeWeakCallback(const std::shared_ptr<CLASS> &object,
                                                  void (CLASS::*function)(ARGS...) const) {
        return WeakCallback<CLASS, ARGS...>(object, function);
    }

    template<typename CLASS, typename... ARGS>
    WeakCallback<CLASS, ARGS...> makeWeakCallback(const std::weak_ptr<CLASS> &object,
                                                  void (CLASS::*function)(ARGS...)) {
        return WeakCallback<CLASS, ARGS...>(object, function);
    }

    template<typename CLASS, typename... ARGS>
    WeakCallback<CLASS, ARGS...> makeWeakCallback(const std::weak_ptr<CLASS> &object,
                                                  void (CLASS::*function)(ARGS...) const) {
        return WeakCallback<CLASS, ARGS...>(object, function);
    }
}


#endif //GG_LIB_WEAKCALLBACK_H
