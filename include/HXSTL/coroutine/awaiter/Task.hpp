#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-06 21:54:21
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
#ifndef _HX_TASK_H_
#define _HX_TASK_H_

#include <HXSTL/coroutine/awaiter/Uninitialized.hpp>
#include <HXSTL/coroutine/awaiter/RepeatAwaiter.hpp>
#include <HXSTL/coroutine/awaiter/PreviousAwaiter.hpp>

namespace HX { namespace STL { namespace coroutine { namespace awaiter {

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
        return PreviousAwaiter(_previous);
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

    Uninitialized<T> _res;
    
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
        return PreviousAwaiter(_previous);
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

/**
 * @brief 默认的协程控制: 暂停则暂停整个调用链
 */
template <class T, class P>
struct Awaiter {
    using promise_type = P;

    bool await_ready() const noexcept { 
        return false; 
    }

    /**
     * @brief 挂起当前协程
     * @param coroutine 这个是`co_await`的协程句柄 (而不是 _coroutine)
     * @return std::coroutine_handle<promise_type> 
     */
    std::coroutine_handle<promise_type> await_suspend(
        std::coroutine_handle<> coroutine
    ) const noexcept {
        _coroutine.promise()._previous = coroutine; // 此处记录 co_await 之前的协程, 方便恢复
        return _coroutine;
    }

    T await_resume() const noexcept {
        return _coroutine.promise().result();
    }

    std::coroutine_handle<promise_type> _coroutine;
};

/**
 * @brief 协程控制: 暂停则回到父级await进行执行
 */
template <class T, class P>
struct ReturnToParentAwaiter {
    using promise_type = P;
    explicit ReturnToParentAwaiter() = default;

    explicit ReturnToParentAwaiter(std::coroutine_handle<promise_type>) // 适配器模式 (这个实际上是无用参数)
    {}

    bool await_ready() const noexcept { 
        return false; 
    }

    /**
     * @brief 挂起当前协程
     * @param coroutine 这个是`co_await`的协程句柄 (而不是 _coroutine)
     * @return std::coroutine_handle<promise_type> 
     */
    bool await_suspend(std::coroutine_handle<>) const noexcept {
        return false;
    }

    void await_resume() const noexcept {
    }
};

/**
 * @brief 协程任务类: 直接返回, 而不是马上执行
 * @tparam T 协程的返回值类型
 * @tparam P 协程的`promise_type`类型
 */
template <class T = void, class P = Promise<T>, class A = Awaiter<T, P>>
struct [[nodiscard]] Task {
    using promise_type = P;

    Task(std::coroutine_handle<promise_type> coroutine = nullptr) noexcept
        : _coroutine(coroutine) {}

    // Task(Task &&) = delete;

    Task(Task &&that) noexcept : _coroutine(that._coroutine) {
        that._coroutine = nullptr;
    }

    Task &operator=(Task &&that) noexcept {
        std::swap(_coroutine, that._coroutine);
    }

    ~Task() {
        if (_coroutine)
            _coroutine.destroy();
    }

    auto operator co_await() const noexcept {
        return A(_coroutine); // C++20 自动生成构造
    }

    operator std::coroutine_handle<>() const noexcept {
        return _coroutine;
    }

    std::coroutine_handle<promise_type> _coroutine; // 当前协程句柄
private:
};

/**
 * @brief 协程任务类: 会立马开始执行
 * @tparam T 
 * @tparam P 
 * @tparam false
 * @tparam A 
 * @tparam P
 */
template <class T = void, class P = Promise<T, false>, class A = Awaiter<T, P>>
struct [[nodiscard]] TaskSuspend {
    using promise_type = P;

    TaskSuspend(std::coroutine_handle<promise_type> coroutine = nullptr) noexcept
        : _coroutine(coroutine) {}

    // Task(Task &&) = delete;

    TaskSuspend(TaskSuspend &&that) noexcept : _coroutine(that._coroutine) {
        that._coroutine = nullptr;
    }

    TaskSuspend &operator=(TaskSuspend &&that) noexcept {
        std::swap(_coroutine, that._coroutine);
    }

    ~TaskSuspend() {
        if (_coroutine)
            _coroutine.destroy();
    }

    auto operator co_await() const noexcept {
        return A(_coroutine); // C++20 自动生成构造
    }

    operator std::coroutine_handle<>() const noexcept {
        return _coroutine;
    }

    std::coroutine_handle<promise_type> _coroutine; // 当前协程句柄
private:
};

template <class Loop, class T, class P, class A>
T run_task(Loop &loop, Task<T, P, A> const &t) {
    auto a = t.operator co_await();
    a.await_suspend(std::noop_coroutine()).resume();
    loop.run();
    return a.await_resume();
};

}}}} // namespace HX::STL::coroutine::awaiter

#endif // !_HX_TASK_H_