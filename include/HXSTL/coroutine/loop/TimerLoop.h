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
#include <utility>
#include <thread>

#include <HXSTL/coroutine/task/TimerTask.hpp>
#include <HXSTL/coroutine/task/Task.hpp>

namespace HX { namespace STL { namespace coroutine { namespace loop {

class TimerLoop {
public:
    /**
     * @brief 协程智能指针, 用于维护被`Timer`托管的协程的生命周期
     */
    using CoroutinePtr = std::shared_ptr<HX::STL::coroutine::task::TimerTask>;
private:
    /**
     * @brief 计时器红黑树
     */
    using TimerRBTree = std::multimap<
        std::chrono::system_clock::time_point, // 超时时间点
        std::pair<
            std::coroutine_handle<>, // 协程句柄
            CoroutinePtr
        >
    >;
public:
    /**
     * @brief 添加计时器
     * @param expireTime 计时器结束时间点
     * @param coroutine 结束时候进行执行的协程句柄
     * @param task 需要被`Timer`控制执行权的协程(`TimerTask`)智能指针
     */
    TimerRBTree::iterator addTimer(
        std::chrono::system_clock::time_point expireTime, 
        std::coroutine_handle<> coroutine,
        CoroutinePtr task = {}
    ) {
        auto&& v = std::make_pair<
            std::coroutine_handle<>, 
            CoroutinePtr
        > (
            task != nullptr ? (void)(task->_ptr = task), task->_coroutine : coroutine, 
            std::move(task)
        );
        return _timerRBTree.insert({expireTime, v});
    }

    /**
     * @brief 取消(删除)计时器
     * @param it 需要删除的计时器红黑树迭代器
     */
    void delTimer(TimerRBTree::iterator it) {
        _timerRBTree.erase(it);
    }

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

private:
    /**
     * @brief 暂停者
     */
    struct SleepAwaiter { // 使用 co_await 则需要定义这 3 个固定函数
        explicit SleepAwaiter(
            TimerLoop& timerLoop,
            std::chrono::system_clock::time_point expireTime
        ) : _timerLoop(timerLoop)
          , _expireTime(expireTime)
        {}

        bool await_ready() const noexcept { // 暂停
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine) const { // `await_ready`后执行: 添加计时器
            _it = _timerLoop.addTimer(_expireTime, coroutine);
        }

        void await_resume() const noexcept { // 计时结束
            _it.reset();
        }

        ~SleepAwaiter() { // 如果这个计时还没有到, 但是被析构了, 那么删除任务!
            if (_it.has_value()) {
                _timerLoop.delTimer(*_it);
            }
        }

        TimerLoop& _timerLoop;
        std::chrono::system_clock::time_point _expireTime; // 过期时间
        mutable std::optional<TimerRBTree::iterator> _it;  // 红黑树结点指针, 如果终止计时, 可以从这里删除
    };

public:
    /**
     * @brief 暂停指定时间点
     * @param timerLoop 计时器循环对象
     * @param expireTime 时间点, 如 2024-8-4 22:12:23
     */
    HX::STL::coroutine::task::Task<
        HX::STL::container::NonVoidHelper<>
    > static sleepUntil(
        std::chrono::system_clock::time_point expireTime
    );

    /**
     * @brief 暂停一段时间
     * @param duration 比如 3s
     */
    HX::STL::coroutine::task::Task<
        HX::STL::container::NonVoidHelper<>
    > static sleepFor(
        std::chrono::system_clock::duration duration
    );

    explicit TimerLoop() : _timerRBTree()
                         , _taskQueue()
    {}

private:
    TimerLoop& operator=(TimerLoop&&) = delete;

    /// @brief 计时器红黑树
    TimerRBTree _timerRBTree;

    /// @brief 任务队列
    std::queue<std::coroutine_handle<>> _taskQueue;
};

}}}} // namespace HX::STL::coroutine::loop

#endif // !_HX_TIMER_LOOP_H_