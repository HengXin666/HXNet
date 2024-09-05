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
#include <unordered_map>

#include <HXSTL/coroutine/task/Task.hpp>
#include <HXWeb/socket/AddressResolver.h>
#include <HXWeb/protocol/https/Context.h>
#include <HXWeb/client/IO.h>

namespace HX { namespace web { namespace client {

/**
 * @brief 进行连接类
 */
class Client {
    socket::AddressResolver::Address _addr {};  // 用于存放服务端的地址信息
    std::shared_ptr<HX::web::client::IO<void>> _io {};
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
     * @brief 独立的请求的参数包
     */
    struct RequestBuilder {
        std::string method = "GET";
        std::string url;
        std::unordered_map<std::string, std::string> head = {};
        std::string body = "";
        std::chrono::milliseconds timeout = std::chrono::milliseconds {30 * 1000};

        /// @brief 代理, 如 socks5://hx:R3L9KvC8@127.0.0.1:2233
        std::string proxy = "";

        /// @brief 如果是 https 协议, 则可以设置验证参数包
        std::optional<HX::web::protocol::https::HttpsVerifyBuilder> verifyBuilder = std::nullopt;
    };

    /**
     * @brief 发起一次独立的请求
     * @param method 请求类型, 如 "GET"
     * @param url 请求URL
     * @param head 请求头
     * @param body 请求体
     * @param timeout 超时时间 (单位: ms)
     * @param proxy 代理服务器 URL
     * @param verifyBuilder 如果是 https 协议, 则可以设置验证参数包
     * @return std::shared_ptr<HX::web::protocol::http::Response>
     */
    static HX::STL::coroutine::task::Task<
        std::shared_ptr<HX::web::protocol::http::Response>
    > request(
        const std::string& method,
        const std::string& url,
        const std::unordered_map<std::string, std::string> head = {},
        const std::string& body = "",
        std::chrono::milliseconds timeout = std::chrono::milliseconds {30 * 1000},
        const std::string& proxy = "",
        std::optional<HX::web::protocol::https::HttpsVerifyBuilder> verifyBuilder = std::nullopt
    );

    /**
     * @brief 发起一次独立的请求
     * @param reqBuilder 独立的请求的参数包
     * @return std::shared_ptr<HX::web::protocol::http::Response> 
     */
    static HX::STL::coroutine::task::Task<
        std::shared_ptr<HX::web::protocol::http::Response>
    > request(
        const RequestBuilder& reqBuilder
    ) {
        co_return co_await request(
            reqBuilder.method,
            reqBuilder.url,
            reqBuilder.head,
            reqBuilder.body,
            reqBuilder.timeout,
            reqBuilder.proxy,
            reqBuilder.verifyBuilder
        );
    }

    /**
     * @brief 独立的`开始连接服务器`参数包
     */
    struct StartBuilder {
        std::string url;
        std::string proxy = "";
        std::chrono::milliseconds timeout = std::chrono::milliseconds {5 * 1000};
    };

    /**
     * @brief 开始连接服务器
     * @param url 目标服务器URL
     * @param proxy 代理服务器 URL
     * @param timeout 超时时间
     * @param verifyBuilder 如果是 https 协议, 则可以设置验证参数包
     * @throw 连接出错
     */
    HX::STL::coroutine::task::Task<> start(
        const std::string& url = "",
        const std::string& proxy = "",
        std::chrono::milliseconds timeout = std::chrono::milliseconds {5 * 1000},
        std::optional<HX::web::protocol::https::HttpsVerifyBuilder> verifyBuilder = std::nullopt
    );

    /**
     * @brief 开始连接服务器
     * @param startBuilder 独立的`开始连接服务器`参数包
     * @throw 连接出错
     */
    HX::STL::coroutine::task::Task<> start(
        const StartBuilder& startBuilder
    ) {
        if (startBuilder.url.empty()) [[unlikely]] {
            throw std::invalid_argument("Error: startBuilder: url is null");
        }
        co_await start(
            startBuilder.url,
            startBuilder.proxy,
            startBuilder.timeout
        );
    }

    /**
     * @brief 读取服务端响应的消息
     * @param timeout 超时时间
     * @return 是否断开连接
     */
    HX::STL::coroutine::task::Task<bool> read(std::chrono::milliseconds timeout);

    /**
     * @brief 向服务端发送消息
     * @param buf 需要发送的消息
     */
    HX::STL::coroutine::task::Task<> write(std::span<char> buf);

    /**
     * @brief 获取客户端的IO流
     * @return const HX::web::client::IO& 
     */
    const HX::web::client::IO<void>& getIO() const {
        return *_io;
    }
};

}}} // namespace HX::web::client

#endif // !_HX_CLIENT_H_