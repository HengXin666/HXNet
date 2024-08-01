#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-30 23:45:20
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
 * */
#ifndef _HX_EPOLL_CONTEXT_H_
#define _HX_EPOLL_CONTEXT_H_

#include <sys/epoll.h>
#include <cassert>
#include <chrono>
#include <map>

#include <HXSTL/container/Callback.h>
#include <HXSTL/tools/ErrorHandlingTools.h>

namespace HX { namespace web { namespace server { namespace context {

// @brief 停止计时器程序
class StopSource {
    // @brief 控制块
    struct _ControlBlock {
        bool _stop = false;      // 是否停止
        HX::STL::container::Callback<> _cb; // 停止计时器的回调
    };

    // @brief 控制器
    std::shared_ptr<_ControlBlock> _control;
public:

    StopSource() = default;

    /**
     * @brief 构造函数
     * @param std::in_place_t 区分构造函数的, 传入`std::in_place`即可
     */
    explicit StopSource(std::in_place_t)
    : _control(std::make_shared<_ControlBlock>()) 
    {}

    static StopSource make() {
        return StopSource(std::in_place);
    }

    /**
     * @brief 是否停止请求
     * @return 是否
     */
    bool isStopRequested() const noexcept {
        return _control && _control->_stop;
    }

    /**
     * @brief 是否可以被停止
     * @return 是否
     */
    bool isStopPossible() const noexcept {
        return _control != nullptr;
    }

    /**
     * @brief 停止请求, 并且执行回调函数
     */
    void doRequestStop() const {
        if (!_control) {
            return;
        }
        _control->_stop = true;
        if (_control->_cb) {
            _control->_cb();
            _control->_cb = nullptr;
        }
    }

    /**
     * @brief 设置停止时执行的回调函数
     * @param cb 停止时执行的回调函数
     */
    void setStopCallback(HX::STL::container::Callback<> cb) const noexcept {
        if (!_control) {
            return;
        }
        _control->_cb = std::move(cb);
    }

    /**
     * @brief 清除停止时执行的回调函数
     */
    void clearStopCallback() const noexcept {
        if (!_control) {
            return;
        }
        _control->_cb = nullptr;
    }
};

// @brief 回调函数计时器
class CallbackFuncTimer {
    struct _TimerEntry {
        HX::STL::container::Callback<> _cb; // 这个是计时到点执行的回调
        StopSource _stop;        // 这个是删除计时器的回调
    };

    // 红黑树: 时间 - 计时项
    std::multimap<std::chrono::steady_clock::time_point, _TimerEntry> _timerHeap;
public:
    explicit CallbackFuncTimer() : _timerHeap()
    {}

    CallbackFuncTimer(CallbackFuncTimer &&) = delete;

    /**
     * @brief 设置超时时间
     * @param dt 持续时间
     * @param cb 超时后执行的回调函数
     * @param stop 停止计时器的回调
     */
    void setTimeout(
        std::chrono::steady_clock::duration dt, 
        HX::STL::container::Callback<> cb, 
        StopSource stop = {}
    );

    /**
     * @brief 检查所有的持续时间, 如果超时则执行其回调, 否则返回下一个计时器到点的时间
     * @return 下一个计时器到点的时间, 无则返回`-1`
     */
    std::chrono::steady_clock::duration durationToNextTimer();

    bool empty() const {
        return _timerHeap.empty();
    }

    /**
     * @brief 获取红黑树结点个数
     * @return _timerHeap.size()
     */
    std::size_t size() const {
        return _timerHeap.size();
    }
};

// @brief epoll上下文 (半懒汉单例)类
class EpollContext {
public:
    int _epfd;
    CallbackFuncTimer _timer;

    /// @brief 全局线程独占单例
    inline static thread_local EpollContext *G_instance = nullptr;

    EpollContext() 
    : _epfd(HX::STL::tools::ErrorHandlingTools::convertError<int>(::epoll_create1(0)).expect("epoll_create1")) 
    , _timer() {
        G_instance = this;
    }

    /**
     * @brief 开始Epoll事件循环, 等待操作系统消息
     */
    void join();

    ~EpollContext() {
        ::close(_epfd);
        G_instance = nullptr;
    }

    /**
     * @brief 获取全局线程独占单例对象引用
     */
    [[gnu::const]] static EpollContext& get() {
        assert(G_instance);
        return *G_instance;
    }
};

}}}} // namespace HX::web::server::context

#endif // _HX_EPOLL_CONTEXT_H_