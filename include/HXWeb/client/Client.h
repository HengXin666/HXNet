#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-16 22:00:48
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
#ifndef _HX_CLIENT_H_
#define _HX_CLIENT_H_

#include <memory>
#include <span>
#include <optional>
#include <chrono>

#include <HXSTL/coroutine/task/Task.hpp>
#include <HXWeb/socket/AddressResolver.h>
#include <HXWeb/client/IO.h>

namespace HX { namespace web { namespace client {

/**
 * @brief 进行连接类
 */
class Client {
    socket::AddressResolver::Address _addr {};  // 用于存放服务端的地址信息
    std::unique_ptr<HX::web::client::IO> _io {};
    using pointer = std::shared_ptr<Client>;
public:
    /**
     * @brief 静态工厂方法
     * @return Acceptor指针
     */
    static pointer make() {
        return std::make_shared<pointer::element_type>();
    }

    /**
     * @brief 开始连接服务器
     * @param url 服务器链接
     * @throw 连接出错
     */
    HX::STL::coroutine::task::Task<> start(const std::string& url);

    /**
     * @brief 读取服务端响应的消息
     * @param timeout 超时时间
     * @return 是否断开连接
     */
    HX::STL::coroutine::task::Task<bool> read(std::chrono::seconds timeout);

    /**
     * @brief 向服务端发送消息
     * @param buf 需要发送的消息
     */
    HX::STL::coroutine::task::Task<> write(std::span<char> buf);

    /**
     * @brief 获取客户端的IO流
     * @return const HX::web::client::IO& 
     */
    const HX::web::client::IO& getIO() const {
        return *_io;
    }
};

}}} // namespace HX::web::client

#endif // !_HX_CLIENT_H_