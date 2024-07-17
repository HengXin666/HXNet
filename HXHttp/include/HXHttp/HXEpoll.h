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

#include <sys/socket.h>
#include <sys/epoll.h>
#include <cstring>
#include <cerrno>

#include <HXprint/HXprint.h>
#include <HXJson/HXJson.h>

namespace HXHttp {

struct SocketAddrBuilderBase {
    int _port;       // 端口
    int _maxQueue;   // 最大排队数
    int _maxConnect; // 最大连接数
};

template<bool Ready = false>
struct [[nodiscard]] SocketAddrBuilder : SocketAddrBuilderBase {
    [[nodiscard]] SocketAddrBuilder<true> &&withPort(int port) && {
        _port = port;
        // 建立socket套接字
        if ((_epollFd = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            LOG_ERROR("socket Error: %s (errno: %d)", strerror(errno), errno);
        }
        return static_cast<SocketAddrBuilder<true> &&>(static_cast<SocketAddrBuilder &&>(*this));
    }

private:
    int _epollFd;          // epoll实例文件描述符
    struct epoll_event ev; // epoll实例 事件
};

class HXEpoll {
    int maxConnect; // 最大连接数

public:
    /**
     * @brief 初始化服务器参数, 以本机作为服务器终端(监听来自本机和公网的全部信息)
     * @param port 端口
     * @param maxQueue 最大排队数 
     * @param maxConnect 最大连接数
     */
    explicit HXEpoll();
};

} // namespace HXHttp

#endif // _HX_HXEPOLL_H_
