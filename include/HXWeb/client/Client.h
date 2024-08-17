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

#include <HXSTL/coroutine/task/Task.hpp>
#include <HXWeb/socket/AddressResolver.h>

namespace HX { namespace web { namespace client {

/**
 * @brief 进行连接类
 */
class Client {
    socket::AddressResolver::Address _addr {};  // 用于存放服务端的地址信息
    int _clientFd = -1;
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
     * @param name 主机名或地址字符串(IPv4 的点分十进制表示或 IPv6 的十六进制表示)
     * @param port 服务名可以是十进制的端口号, 也可以是已知的服务名称, 如 ftp、http 等
     * @return 是否操作成功
     */
    HX::STL::coroutine::task::Task<int> start(const std::string& name, const std::string& port);

    /**
     * @brief 读取服务端响应的消息
     * @return std::string
     * @warning 这个还不算完善: 有时候会丢失消息, 因为可读的时候缓冲区不一定是满的, 
     * 但是服务端又是一段一段的发送导致提前终止(需要解析http才可以完美接收!)
     */
    HX::STL::coroutine::task::Task<std::string> read();

    /**
     * @brief 向服务端发送消息
     * @param buf 需要发送的消息
     * @return 发送的字节 / 错误码`-erron`
     */
    HX::STL::coroutine::task::Task<int> write(std::span<char> buf);

    /**
     * @brief 释放客户端连接
     * @return 是否操作成功
     */
    HX::STL::coroutine::task::Task<int> close();

    /**
     * @brief Destroy the Client object
     * @throw 如果没有调用`close()`, 则会报错! | 抛出异常则代表是用户没有释放连接, 不是代码有问题, 是代码逻辑有问题!
     */
    ~Client();
};

}}} // namespace HX::web::client

#endif // !_HX_CLIENT_H_