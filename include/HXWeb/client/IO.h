#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-21 21:01:37
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
#ifndef _HX_CLIENT_IO_H_
#define _HX_CLIENT_IO_H_

#include <HXSTL/coroutine/task/Task.hpp>
#include <HXWeb/socket/IO.h>

namespace HX { namespace web { namespace protocol { namespace proxy {

class Socks5Proxy;

}}}} // namespace::HX::web::protocol::proxy

namespace HX { namespace web { namespace client {

class Client;

/**
 * @brief 服务连接时候的io
 */
class IO : public HX::web::socket::IO {
public:
    explicit IO(int fd) : HX::web::socket::IO(fd)
    {}

    ~IO() noexcept = default;

    /**
     * @brief 立即发送请求
     */
    // HX::STL::coroutine::task::Task<> sendRequest() const;

    IO& operator=(IO&&) = delete;
protected:
    // === start === 读取相关的函数 === start ===
    /**
     * @brief 解析一条完整的服务端响应
     * @param timeout 超时时间
     * @return bool 是否断开连接
     */
    HX::STL::coroutine::task::Task<bool> _recvResponse(
        struct __kernel_timespec *timeout
    );
    // === end === 读取相关的函数 === end ===

    // === start === 写入相关的函数 === start ===
    /**
     * @brief 写入请求到套接字
     */
    HX::STL::coroutine::task::Task<> _sendRequest() const;
    // === end === 写入相关的函数 === end ===

    friend HX::web::protocol::websocket::WebSocket;
    friend HX::web::protocol::http::Request;
    friend HX::web::protocol::http::Response;
    friend HX::web::protocol::proxy::Socks5Proxy;
    friend Client;
};

}}} // namespace HX::web::server

#endif // !_HX_CLIENT_IO_H_