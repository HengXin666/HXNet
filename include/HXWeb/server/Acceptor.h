#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-31 11:54:38
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
#ifndef _HX_ACCEPTOR_H_
#define _HX_ACCEPTOR_H_

#include <memory>
#include <chrono>

#include <HXSTL/coroutine/task/Task.hpp>
#include <HXWeb/protocol/http/Http.hpp>
#include <HXWeb/protocol/https/Https.hpp>
#include <HXWeb/socket/AddressResolver.h>

namespace HX { namespace web { namespace server {

template <class T = void>
class Acceptor {
    // 禁止默认实现
    static_assert(sizeof(T) == 0, "Acceptor is not implemented for this type");
};

/**
 * @brief 连接接受类
 */
template <>
class Acceptor<HX::web::protocol::http::Http> {
#ifdef CLIENT_ADDRESS_LOGGING
    socket::AddressResolver::Address _addr {}; // 用于存放客户端的地址信息 (在::accept中由操作系统填写; 可复用)
#endif
    using pointer = std::shared_ptr<Acceptor<HX::web::protocol::http::Http>>;
public:
    /**
     * @brief 静态工厂方法
     * @return Acceptor指针
     */
    static pointer make() {
        return std::make_shared<pointer::element_type>();
    }

    /**
     * @brief 开始接受连接: 注册服务器套接字, 绑定并监听端口
     * @param name 主机名或地址字符串(IPv4 的点分十进制表示或 IPv6 的十六进制表示)
     * @param port 服务名可以是十进制的端口号, 也可以是已知的服务名称, 如 ftp、http 等
     * @param timeout 没有收到消息, 超时自动断开的时间 (单位: 秒)
     */
    HX::STL::coroutine::task::Task<> start(
        const HX::web::socket::AddressResolver::AddressInfo& entry,
        std::chrono::seconds timeout = std::chrono::seconds{30}
    );
};

template <>
class Acceptor<HX::web::protocol::https::Https> {
#ifdef CLIENT_ADDRESS_LOGGING
    socket::AddressResolver::Address _addr {}; // 用于存放客户端的地址信息 (在::accept中由操作系统填写; 可复用)
#endif
    using pointer = std::shared_ptr<Acceptor<HX::web::protocol::https::Https>>;
public:
    /**
     * @brief 静态工厂方法
     * @return Acceptor指针
     */
    static pointer make() {
        return std::make_shared<pointer::element_type>();
    }

    /**
     * @brief 开始接受连接: 注册服务器套接字, 绑定并监听端口
     * @param name 主机名或地址字符串(IPv4 的点分十进制表示或 IPv6 的十六进制表示)
     * @param port 服务名可以是十进制的端口号, 也可以是已知的服务名称, 如 ftp、http 等
     * @param timeout 没有收到消息, 超时自动断开的时间 (单位: 秒)
     */
    HX::STL::coroutine::task::Task<> start(
        const HX::web::socket::AddressResolver::AddressInfo& entry,
        std::chrono::seconds timeout = std::chrono::seconds{30}
    );
};

}}} // namespace HX::web::server

#endif // _HX_ACCEPTOR_H_