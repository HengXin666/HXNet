#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-17 17:39:57
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
#ifndef _HX_HXEPOLL_H_
#define _HX_HXEPOLL_H_

#include <sys/epoll.h> // Epoll
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace HXHttp {

class HXEpoll {
    int _epollFd {-1};             // epoll实例文件描述符
    int _serverFd {-1};            // 服务器套接字
    int _epollEventsSize;          // sizeof(_events)
    struct ::epoll_event _ev;      // epoll实例 事件
    struct ::epoll_event *_events; // 用户的 epoll 事件

    // --- 多线程相关 ---
    std::vector<std::thread> _threads;  // 线程
    std::queue<int> _tasks;             // 存放有消息的 fd 文件描述符
    std::mutex _queueMutex;             // 锁
    std::condition_variable _condition; // 条件变量

    // --- 回调函数 ---
    std::function<void(int)> _newConnectCallbackFunc;                       // 有新连接的回调函数, {客户端fd}
    std::function<int(int, char *, const std::size_t)> _newMsgCallbackFunc; // 有新消息的回调函数, {客户端fd, msg, msgLen} -> bool: 是否释放该fd
    std::function<void(int)> _newUserBreakCallbackFunc;                     // 用户断开连接的回调函数, {客户端fd}
    bool _running; // 这个不用原子吧?

    /**
     * @brief 任务线程函数
     */
    void workerThread();

    /**
     * @brief 报错, 并且释放Fd
     */
    void doError();

    // epoll_wait
    /**
     * @brief 返回就绪事件的信息数量
     * @param timeOut 等待的时间;
     *          -1 阻塞等待;
     *          0  立即返回;
     *          正数 阻塞等待的最长时间(单位: 毫秒)
     * @return 失败时返回 -1, 并设置 errno 错误码;
     *         == 0 无消息;
     *         > 0 有对应的消息数量
     */
    int wait(int timeOut) {
        return ::epoll_wait(_epollFd, _events, _epollEventsSize, timeOut);
    }

    // add
    /**
     * @brief 添加事件到epoll红黑树中
     * @param fd 文件操作符
     * @return 成功时返回 0; 失败时返回 -1, 并设置 errno 错误码 
     */
    int ctlAdd(int fd)  {
        // 检测读缓冲区是否有数据, 并且将文件描述符设置为边沿模式
        _ev.events = EPOLLIN | EPOLLET;
        _ev.data.fd = fd;
        return ::epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &_ev);
    }

    // del
    /**
     * @brief 删除epoll红黑树中的事件
     * @param fd 文件操作符
     * @return 成功时返回 0; 失败时返回 -1, 并设置 errno 错误码 
     */
    int ctlDel(int fd) {
        return ::epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL);
    }

    // mod (Modify)
    /**
     * @brief 修改epoll红黑树中的事件
     * @param fd 文件操作符
     * @return 成功时返回 0; 失败时返回 -1, 并设置 errno 错误码 
     */
    int ctlMod(int fd) {
        return ::epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &_ev);
    }

    /**
     * @brief 设置fd为非阻塞
     * @return 错误为真, 正常为假
     */
    bool setNonBlocking(int fd);

public:
    /**
     * @brief 初始化服务器参数, 以本机作为服务器终端(监听来自本机和公网的全部信息)
     * @param port 端口
     * @param evSize wait就绪列表长度, 如果epoll实例的红黑树中已就绪的文件描述符很多, 并且evs数组无法将这些信息全部传出, 那么这些信息会在下一次`epoll_wait()`函数返回的时候被传出
     * @param maxQueue 最大排队数
     * @param maxConnect 最大连接数 | 只是一个提示, 实际上, 内核会根据需要动态调整大小
     */
    explicit HXEpoll(int port = 28205, int evSize = 1024, int maxQueue = 64, int maxConnect = 20);

    /**
     * @brief 设置有新连接的回调函数
     * @param func 回调函数: {int : 客户端fd}
     * @return *this 可链式调用
     */
    [[nodiscard]] HXEpoll& setNewConnectCallback(std::function<void(int)> func) {
        _newConnectCallbackFunc = func;
        return *this;
    }

    /**
     * @brief 设置有新消息的回调函数
     * @param func 回调函数: {int : 客户端fd, char* : msgStr, size_t : msgStrLen} -> bool(true: 删除该fd, false: 不做处理);
     * @return *this 可链式调用
     */
    [[nodiscard]] HXEpoll& setNewMsgCallback(std::function<int(int, char*, const std::size_t)> func) {
        _newMsgCallbackFunc = func;
        return *this;
    }

    /**
     * @brief 设置用户断开连接的回调函数
     * @param func 回调函数: {int : 客户端fd}
     * @return *this 可链式调用
     */
    [[nodiscard]] HXEpoll& setNewUserBreakCallback(std::function<void(int)> func) {
        _newUserBreakCallbackFunc = func;
        return *this;
    }

    /**
     * @brief 启动Epoll
     * @param timeOut 等待的时间,
     *          -1 阻塞等待;
     *           0 立即返回;
     *          正数 阻塞等待的最长时间(单位: 毫秒)
     * @param conditional 每次主循环迭代时调用的函数, 用于检查服务器是否应继续运行, 返回true让服务器继续运行, 返回false关闭服务器
     */
    void run(int timeOut = -1, const std::function<bool()>& conditional = nullptr);

    ~HXEpoll();
};

} // namespace HXHttp

#endif // _HX_HXEPOLL_H_
