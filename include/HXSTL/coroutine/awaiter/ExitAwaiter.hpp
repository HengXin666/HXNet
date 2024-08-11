#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-11 21:07:15
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
#ifndef _HX_EXIT_AWAITER_H_
#define _HX_EXIT_AWAITER_H_

#include <coroutine>

namespace HX { namespace STL { namespace coroutine { namespace awaiter {

/**
 * @brief 默认的协程控制: 在协程挂起(`co_await`)时会退出整个协程链条
 */
template <class T, class P>
struct ExitAwaiter {
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

}}}} // namespace HX::STL::coroutine::awaiter

#endif // !_HX_EXIT_AWAITER_H_