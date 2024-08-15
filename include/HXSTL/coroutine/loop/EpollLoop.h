#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-10 22:12:01
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
#ifndef _HX_EPOLL_LOOP_H_
#define _HX_EPOLL_LOOP_H_

#include <optional>
#include <vector>
#include <chrono>
#include <sys/epoll.h>

#include <HXSTL/coroutine/task/Task.hpp>
#include <HXSTL/tools/ErrorHandlingTools.h>

namespace HX { namespace STL { namespace coroutine { namespace loop {

/**
 * @brief Epoll 事件掩码
 */
using EpollEventMask = uint32_t;

class EpollLoop {
    EpollLoop& operator=(EpollLoop&&) = delete;
public:
    explicit EpollLoop()
        : _epfd(HX::STL::tools::LinuxErrorHandlingTools::convertError<int>(
            ::epoll_create1(0)).expect("epoll_create1"))
        , _evs()
    {
        _evs.resize(128);
    }

    ~EpollLoop() {
        ::close(_epfd);
    }

    /**
     * @brief 注册套接字到epoll检测
     * @param fd 
     */
    void addEpollCtl(int fd);

    void removeListener(int fd);

    bool addListener(class EpollFilePromise &promise, EpollEventMask mask, int ctl);

    bool run(std::optional<std::chrono::system_clock::duration> timeout);

    bool hasEvent() const noexcept {
        return _count != 0;
    }

    int _epfd = -1;
    int _count = 0;
private:
    std::vector<struct ::epoll_event> _evs;
};

struct EpollFilePromise : HX::STL::coroutine::promise::Promise<EpollEventMask> {
    auto get_return_object() {
        return std::coroutine_handle<EpollFilePromise>::from_promise(*this);
    }

    EpollFilePromise &operator=(EpollFilePromise &&) = delete;

    ~EpollFilePromise();

    int _fd = -1;
};

struct EpollFileAwaiter {
    explicit EpollFileAwaiter(EpollLoop &epollLoop, int fd, EpollEventMask mask, EpollEventMask ctl) 
        : _epollLoop(epollLoop)
        , _fd(fd)
        , _mask(mask)
        , _ctl(ctl)
    {} 

    bool await_ready() const noexcept {
        return false;
    }

    void await_suspend(std::coroutine_handle<EpollFilePromise> coroutine) {
        auto &promise = coroutine.promise();
        // if (_fd <= -1) {
        //     printf(__FILE__": Error _fd %d\n", _fd);
        //     // promise._fd = -1;
        //     coroutine.resume();
        //     return;
        // }
        promise._fd = _fd;
        if (!_epollLoop.addListener(promise, _mask, _ctl)) {
            promise._fd = -1;
            coroutine.resume();
        }
    }

    int await_resume() const noexcept {
        return _fd;
    }

    EpollLoop &_epollLoop;
    int _fd = -1;
    EpollEventMask _mask = 0;
    int _ctl = EPOLL_CTL_MOD;
};

/**
 * @brief 等待文件事件: 添加fd进入epoll检测, 当epoll有事件后结束co_await
 * @param epollLoop Epoll循环对象引用
 * @param fd 文件套接字
 * @param mask Epoll 事件掩码
 * @param ctl 如: EPOLL_CTL_ADD
 * @return HX::STL::coroutine::task::Task<int, EpollFilePromise> 
 */
inline HX::STL::coroutine::task::Task<int, EpollFilePromise> waitFileEvent(
    EpollLoop& epollLoop,
    int fd, 
    EpollEventMask mask, 
    int ctl = EPOLL_CTL_MOD
) {
    co_return co_await EpollFileAwaiter(epollLoop, fd, mask, ctl);
}

}}}} // namespace HX::STL::coroutine::loop

#endif // !_HX_EPOLL_LOOP_H_