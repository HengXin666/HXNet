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
#ifndef _HX_SERVER_IO_H_
#define _HX_SERVER_IO_H_

#include <HXSTL/coroutine/task/Task.hpp>
#include <HXWeb/protocol/http/Http.hpp>
#include <HXWeb/protocol/https/Https.hpp>
#include <HXWeb/socket/IO.h>

namespace HX { namespace web { namespace server {

template <class T>
struct ConnectionHandler;

// template <class T>
// class IOImpl {
//     // 静态断言: 不允许其他非void的实现
//     static_assert(std::is_same<T, void>::value, "Not supported for instantiation");
// public:
//     /**
//      * @brief 读取
//      * @param buf [out] 读取数据到buf里面
//      * @param timeout 超时时间
//      */
//     virtual HX::STL::coroutine::task::Task<> read(
//         std::span<char> buf,
//         struct __kernel_timespec *timeout
//     ) = 0;

//     /**
//      * @brief 写入
//      * @param buf [in] 把buf的数据写入
//      */
//     virtual HX::STL::coroutine::task::Task<> write(std::span<char> buf) = 0;

//     virtual ~IOImpl() {}
// protected:
//     int fd;
// };

// template <>
// class IOImpl<HX::web::protocol::http::Http> : public IOImpl<void> {
// public:
//     HX::STL::coroutine::task::Task<> read(
//         std::span<char> buf,
//         struct __kernel_timespec *timeout
//     ) override;

//     HX::STL::coroutine::task::Task<> write(std::span<char> buf) override;
// };

// template <>
// class IOImpl<HX::web::protocol::https::Https> : public IOImpl<void> {
// public:
//     HX::STL::coroutine::task::Task<> read(
//         std::span<char> buf,
//         struct __kernel_timespec *timeout
//     ) override;

//     HX::STL::coroutine::task::Task<> write(std::span<char> buf) override;
// };

/**
 * @brief 服务连接时候的io
 */
class IO : public HX::web::socket::IO {
public:
    // explicit IO(int fd, const IOImpl<void>& impl) 
    //     : HX::web::socket::IO(fd)
    //     , _ioImpl(impl)
    // {}
    explicit IO(int fd)
        : HX::web::socket::IO(fd)
    {}

    ~IO() noexcept = default;

    /**
     * @brief 立即发送响应
     */
    HX::STL::coroutine::task::Task<> sendResponse() const;
    HX::STL::coroutine::task::Task<> sendResponse(HX::STL::container::NonVoidHelper<>);

    IO& operator=(IO&&) = delete;
protected:
    // === start === 读取相关的函数 === start ===
    /**
     * @brief 解析一条完整的客户端请求
     * @param timeout 超时时间
     * @return bool 是否断开连接
     */
    HX::STL::coroutine::task::Task<bool> _recvRequest(
        struct __kernel_timespec *timeout
    );
    // === end === 读取相关的函数 === end ===

    // === start === 写入相关的函数 === start ===
    /**
     * @brief 写入响应到套接字
     */
    HX::STL::coroutine::task::Task<> _sendResponse() const;
    // === end === 写入相关的函数 === end ===

    friend HX::web::protocol::websocket::WebSocket;
    friend HX::web::protocol::http::Request;
    friend HX::web::protocol::http::Response;
    friend ConnectionHandler<HX::web::protocol::http::Http>;
    friend ConnectionHandler<HX::web::protocol::https::Https>;

    // const IOImpl<void>& _ioImpl;
};

}}} // namespace HX::web::server

#endif // !_HX_SERVER_IO_H_