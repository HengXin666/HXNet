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

#include <sys/epoll.h>  // Epoll

namespace HXHttp {

class HXEpoll {
    int _epollFd;                  // epoll实例文件描述符
    int _maxConnect;               // 最大连接数
    int _serverFd;                 // 服务器套接字
    struct ::epoll_event _ev;      // epoll实例 事件
    struct ::epoll_event *_events; // 用户的 epoll 事件

    // epoll_wait
    /**
     * @brief 返回就绪事件的信息数量
     * @param timeOut 等待的时间;
     *          -1 阻塞等待;
     *          0  立即返回;
     *          正数 阻塞等待的最长时间(单位: 毫秒)
     * @return 失败时返回 -1，并设置 errno 错误码 ;
     *        == 0 无消息;
     *         > 0 有对应的消息数量
     */
    int wait(int timeOut);

    // add
    /**
     * @brief 添加事件到epoll红黑树中
     * @param fd 文件操作符
     * @param ev 指向 epoll_event 结构体的指针，用于指定事件类型和数据
     * @return 成功时返回 0；失败时返回 -1，并设置 errno 错误码 
     */
    int ctl_add(int fd);

    // del
    /**
     * @brief 删除epoll红黑树中的事件
     * @param fd 文件操作符
     * @param ev 指向 epoll_event 结构体的指针，用于指定事件类型和数据
     * @return 成功时返回 0；失败时返回 -1，并设置 errno 错误码 
     */
    int ctl_del(int fd);

    // mod (Modify)
    /**
     * @brief 修改epoll红黑树中的事件
     * @param fd 文件操作符
     * @param ev 指向 epoll_event 结构体的指针，用于指定事件类型和数据
     * @return 成功时返回 0；失败时返回 -1，并设置 errno 错误码 
     */
    int ctl_mod(int fd);

public:
    /**
     * @brief 初始化服务器参数, 以本机作为服务器终端(监听来自本机和公网的全部信息)
     * @param port 端口
     * @param maxQueue 最大排队数 
     * @param maxConnect 最大连接数 | 只是一个提示, 实际上, 内核会根据需要动态调整大小
     */
    explicit HXEpoll(int port = 28205, int maxQueue = 512, int maxConnect = 1);

    ~HXEpoll();
};

} // namespace HXHttp

#endif // _HX_HXEPOLL_H_
