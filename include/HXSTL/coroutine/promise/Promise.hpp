#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-12 14:45:37
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _HX_PROMISE_H_
#define _HX_PROMISE_H_

#include <HXSTL/container/Uninitialized.hpp>
#include <HXSTL/coroutine/awaiter/RepeatAwaiter.hpp>
#include <HXSTL/coroutine/awaiter/PreviousAwaiter.hpp>

namespace HX { namespace STL { namespace coroutine { namespace promise {

/**
 * @brief 
 * @tparam T 
 * @tparam bool 第一次创建时候是否暂停协程 (默认暂停`true`)
 */
template <class T, bool ifSuspend = true>
struct Promise {
    auto initial_suspend() { 
        if constexpr (ifSuspend)
            return std::suspend_always(); // 第一次创建, 直接挂起
        else
            return std::suspend_never();
    }

    auto final_suspend() noexcept {
        return HX::STL::coroutine::awaiter::PreviousAwaiter(_previous);
    }

    void unhandled_exception() noexcept {
        _exception = std::current_exception();
    }

    void return_value(T&& res) {
        _res.putVal(std::move(res));
    }

    void return_value(const T& res) {
        _res.putVal(res);
    }

    auto yield_value(T&& res) {
        _res.putVal(res);
        return std::suspend_always(); // 挂起协程
    }

    T result() {
        if (_exception) [[unlikely]] {
            std::rethrow_exception(_exception);
        }
        return _res.moveVal();
    }

    auto get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    Promise &operator=(Promise &&) = delete;

    HX::STL::container::Uninitialized<T> _res;
    
    std::coroutine_handle<> _previous {}; // 上一个协程句柄
    std::exception_ptr _exception {}; // 异常信息
};

template <bool ifSuspend>
struct Promise<void, ifSuspend> {
    auto initial_suspend() { 
        if constexpr (ifSuspend)
            return std::suspend_always(); // 第一次创建, 直接挂起
        else
            return std::suspend_never();
    }

    auto final_suspend() noexcept {
        return HX::STL::coroutine::awaiter::PreviousAwaiter(_previous);
    }

    void unhandled_exception() noexcept {
        _exception = std::current_exception();
    }

    void return_void() noexcept {
    }

    void result() {
        if (_exception) [[unlikely]] {
            std::rethrow_exception(_exception);
        }
    }

    auto get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    Promise &operator=(Promise &&) = delete;
    
    std::coroutine_handle<> _previous {}; // 上一个协程句柄
    std::exception_ptr _exception {}; // 异常信息
};

}}}} // namespace HX::STL::coroutine::awaiter

#endif // !_HX_PROMISE_H_