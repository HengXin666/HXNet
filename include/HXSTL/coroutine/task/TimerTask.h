#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-12 14:54:19
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
#ifndef _HX_TIMER_TASK_H_
#define _HX_TIMER_TASK_H_

#include <memory>

#include <HXSTL/coroutine/awaiter/PreviousAwaiter.hpp>

namespace HX { namespace STL { namespace coroutine { namespace loop {

// === 前置声明 ===
class TimerLoop;

}}}} // namespace HX::STL::coroutine::loop

namespace HX { namespace STL { namespace coroutine { namespace task {

struct [[nodiscard]] TimerTask;

struct TimerPromis {
    friend TimerTask;

    auto initial_suspend() { 
        return std::suspend_always(); // 第一次创建, 直接挂起
    }

    HX::STL::coroutine::awaiter::PreviousAwaiter final_suspend() noexcept;

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
        return std::coroutine_handle<TimerPromis>::from_promise(*this);
    }

    TimerPromis &operator=(TimerPromis &&) = delete;
    
    std::coroutine_handle<> _previous {}; // 上一个协程句柄
    std::exception_ptr _exception {};     // 异常信息
    std::shared_ptr<TimerTask> _ptr {}; // 自己的指针
};

/**
 * @brief 由`TimerLoop`控制的协程任务.
 * 即使被`co_await`, 也不会暂停原(`co_await`所在)协程, 而是分离`co_await`后面的任务将其控制权交给`TimerLoop`
 * @warning 请保证`co_await`的内容和接下来的无关!, 本任务`无`返回值
 */
struct [[nodiscard]] TimerTask {
    using promise_type = TimerPromis;

    TimerTask(std::coroutine_handle<promise_type> coroutine = nullptr) noexcept
        : _coroutine(coroutine) {}

    // TimerTask(TimerTask &&) = delete;

    TimerTask(TimerTask &&that) noexcept : _coroutine(that._coroutine) {
        that._coroutine = nullptr;
    }

    TimerTask &operator=(TimerTask &&that) noexcept {
        std::swap(_coroutine, that._coroutine);
        return *this;
    }

    ~TimerTask() {
        if (_coroutine) {
            _coroutine.destroy();
        }
    }

    // 不提供
    // auto operator co_await() const noexcept {
    //     return HX::STL::coroutine::awaiter::ExitAwaiter<void, promise_type>(_coroutine);
    // }

    operator std::coroutine_handle<>() const noexcept {
        return _coroutine;
    }

private:
    friend HX::STL::coroutine::loop::TimerLoop;
    std::coroutine_handle<promise_type> _coroutine; // 当前协程句柄
};

}}}} // namespace HX::STL::coroutine::task

#endif // !_HX_TIMER_TASK_H_