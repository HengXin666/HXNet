#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-10 22:01:49
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
#ifndef _HX_TIMER_LOOP_H_
#define _HX_TIMER_LOOP_H_

#include <queue>
#include <map>
#include <optional>
#include <chrono>
#include <thread>

#include <HXSTL/coroutine/awaiter/Task.hpp>

namespace HX { namespace STL { namespace coroutine { namespace loop {

class TimerLoop {
    /**
     * @brief 添加计时器
     * @param expireTime 计时器结束时间点
     * @param coroutine 结束时候进行执行的协程句柄
     */
    void addTimer(
        std::chrono::system_clock::time_point expireTime, 
        std::coroutine_handle<> coroutine
    ) {
        _timerRBTree.insert({expireTime, coroutine});
    }

public:
    /**
     * @brief 添加任务
     * @param coroutine 协程任务句柄
     */
    void addTask(std::coroutine_handle<> coroutine) {
        _taskQueue.emplace(coroutine);
    }

    /**
     * @brief 执行全部任务
     */
    void runAll();

    std::optional<std::chrono::system_clock::duration> run();

    static TimerLoop& getLoop() { // TODO !
        static TimerLoop loop;
        return loop;
    }

private:
    /**
     * @brief 暂停者
     */
    struct SleepAwaiter { // 使用 co_await 则需要定义这 3 个固定函数
        bool await_ready() const noexcept { // 暂停
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine) const { // `await_ready`后执行: 添加计时器
            TimerLoop::getLoop().addTimer(_expireTime, coroutine);
        }

        void await_resume() const noexcept { // 计时结束
        }

        std::chrono::system_clock::time_point _expireTime; // 过期时间
    };

public:
    /**
     * @brief 暂停指定时间点
     * @param expireTime 时间点, 如 2024-8-4 22:12:23
     */
    HX::STL::coroutine::awaiter::Task<void> static sleep_until(std::chrono::system_clock::time_point expireTime) {
        co_await SleepAwaiter(expireTime);
    }

    /**
     * @brief 暂停一段时间
     * @param duration 比如 3s
     */
    HX::STL::coroutine::awaiter::Task<void> static sleep_for(std::chrono::system_clock::duration duration) {
        co_await SleepAwaiter(std::chrono::system_clock::now() + duration);
    }

private:
    explicit TimerLoop() : _timerRBTree()
                         , _taskQueue()
    {}

    TimerLoop& operator=(TimerLoop&&) = delete;

    /// @brief 计时器红黑树
    std::multimap<std::chrono::system_clock::time_point, std::coroutine_handle<>> _timerRBTree;

    /// @brief 任务队列
    std::queue<std::coroutine_handle<>> _taskQueue;
};

}}}} // namespace HX::STL::coroutine::loop

#endif // !_HX_TIMER_LOOP_H_