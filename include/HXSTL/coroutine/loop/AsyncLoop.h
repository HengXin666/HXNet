#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-10 22:31:19
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
#ifndef _HX_ASYNC_LOOP_H_
#define _HX_ASYNC_LOOP_H_

#include <thread>

#include <HXSTL/coroutine/loop/TimerLoop.h>
#include <HXSTL/coroutine/loop/IoUringLoop.h>

namespace HX { namespace STL { namespace coroutine { namespace loop {

/**
 * @brief 异步循环
 */
class AsyncLoop {
    explicit AsyncLoop() : _timerLoop()
                         , _ioUringLoop()
    {}
public:

    AsyncLoop& operator=(AsyncLoop&&) = delete;

    [[gnu::hot]] static AsyncLoop& getLoop() {
        static AsyncLoop loop;
        return loop;
    }

    void run() {
        while (true) {
            auto timeout = _timerLoop.run();
            if (_ioUringLoop.hasEvent()) {
                _ioUringLoop.run(timeout);
            } else if (timeout) {
                std::this_thread::sleep_for(*timeout);
            } else {
                break;
            }
        }
    }

    [[gnu::hot]] TimerLoop& getTimerLoop() {
        return _timerLoop;
    }

    operator TimerLoop &() {
        return _timerLoop;
    }
    [[gnu::hot]] IoUringLoop& getIoUringLoop() {
        return _ioUringLoop;
    }

    operator IoUringLoop &() {
        return _ioUringLoop;
    }

private:
    TimerLoop _timerLoop;
    IoUringLoop _ioUringLoop;
};

}}}} // namespace HX::STL::coroutine::loop

#endif // !_HX_ASYNC_LOOP_H_