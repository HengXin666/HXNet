#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-16 16:19:43
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
#ifndef _HX_TRIGGER_WAIT_LOOP_H_
#define _HX_TRIGGER_WAIT_LOOP_H_

#include <list>
#include <optional>
#include <coroutine>

#include <HXSTL/coroutine/task/Task.hpp>

namespace HX { namespace STL { namespace coroutine { namespace loop {

/**
 * @brief 等待触发循环
 * 用于挂载协程, 可以由其他协程触发, 从而执行本协程的任务
 */
class TriggerWaitLoop {
public:
    using WaitTaskQueue = std::list<std::coroutine_handle<>>;

    /**
     * @brief 析构行为枚举
     * 如果`WaitTaskQueue`还有挂载的协程, 这里将会决定如何处理.
     */
    enum class DestructionBehavior : int {
        Resume,      // 把挂载的任务全部执行
        Cleanup,     // 清除所有挂载的协程 (`coroutine.destroy()`) (不执行)
    };

    /**
     * @brief 创建等待触发循环
     * @param behavior 析构行为
     */
    explicit TriggerWaitLoop(
        DestructionBehavior behavior = DestructionBehavior::Resume
    ) : _waitQueue()
      , _behavior(behavior)
    {}

    TriggerWaitLoop(const TriggerWaitLoop&) = delete;
    TriggerWaitLoop& operator=(const TriggerWaitLoop&) = delete;

    TriggerWaitLoop(TriggerWaitLoop &&that) noexcept 
        : _waitQueue(that._waitQueue)
        , _behavior(that._behavior)
    {
        that._waitQueue.clear();
    }

    TriggerWaitLoop &operator=(TriggerWaitLoop &&that) noexcept {
        std::swap(_waitQueue, that._waitQueue);
        return *this;
    }

    /**
     * @brief 添加挂载任务
     * @param coroutine 协程句柄
     * @return WaitTaskQueue::iterator 挂载的任务的迭代器
     */
    WaitTaskQueue::iterator addWaitTask(std::coroutine_handle<> coroutine) {
        return _waitQueue.insert(_waitQueue.end(), std::move(coroutine));
    }

    /**
     * @brief 继续运行所有挂载的协程
     * @warning 运行后会将所有挂载的协程从`waitQueue`中删除
     */
    void runAllTask() {
        for (auto&& it : _waitQueue)
            it.resume();
        _waitQueue.clear();
    }

    /**
     * @brief 删除一个迭代器
     * @param it 
     */
    void delTask(WaitTaskQueue::iterator it) {
        _waitQueue.erase(it);
    }

    /**
     * @brief 运行单个挂载的协程
     * @param it 需要运行的协程的迭代器
     * @warning 运行后会将迭代器从`waitQueue`中删除
     */
    void runOneTask(WaitTaskQueue::iterator it) {
        it->resume();
        delTask(it);
    }

    ~TriggerWaitLoop();

    struct TriggerWaitTask {
        TriggerWaitTask(TriggerWaitLoop& TWaitLoop) : _TWaitLoop(TWaitLoop) 
        {}

        bool await_ready() const noexcept { // 暂停
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine) const { 
            // `await_ready`后执行: 添加到触发等待队列
            _it = _TWaitLoop.addWaitTask(coroutine);
        }

        void await_resume() const noexcept {
            // 只能挂载一次
            _it.reset();
        }

        ~TriggerWaitTask() { // 如果还没有触发, 但是被析构了, 那么删除任务!
            if (_it.has_value()) {
                _TWaitLoop.delTask(*_it);
            }
        }

    private:
        TriggerWaitLoop& _TWaitLoop;
        mutable std::optional<WaitTaskQueue::iterator> _it;
    };

    HX::STL::coroutine::task::Task<
        HX::STL::container::NonVoidHelper<>
    > static triggerWait(TriggerWaitLoop& TWaitLoop);

private:
    /// @brief 等待触发队列
    WaitTaskQueue _waitQueue;

    /// @brief 析构行为
    DestructionBehavior _behavior;
};

}}}} // namespace HX::STL::coroutine::loop

#endif // !_HX_TRIGGER_WAIT_LOOP_H_