#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-18 22:16:09
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
#ifndef _HX_WEB_SOCKET_H_
#define _HX_WEB_SOCKET_H_

#include <memory>
#include <chrono>
#include <functional>

#include <HXSTL/coroutine/task/Task.hpp>
#include <HXWeb/protocol/http/Request.h>

namespace HX { namespace web { namespace protocol { namespace websocket {

/**
 * @brief WebSocket包
 */
struct WebSocketPacket {
    /**
     * @brief 操作码 (协议规定的!)
     */
    enum OpCode : uint8_t {
        Text = 1,   // 文本数据
        Binary = 2, // 二进制数据
        Close = 8,  // 关闭
        Ping = 9,
        Pong = 10,
    } _opCode;

    /// @brief 内容
    std::string _content;
};

/**
 * @brief WebSocket
 */
class WebSocket {
    using pointer = std::shared_ptr<WebSocket>;
    const HX::web::protocol::http::Request& _req;
    std::function<HX::STL::coroutine::task::Task<>(const std::string&)> _onMessage;
    std::function<HX::STL::coroutine::task::Task<>()> _onClose;
    std::function<HX::STL::coroutine::task::Task<>(std::chrono::steady_clock::duration)> _onPong;

    /// @brief 最后一次ping的时间点
    std::chrono::steady_clock::time_point _lastPingTime {};

    /// @brief 是否处于Pong看看对方嘎了没有 阶段
    bool _waitingPong = true;

    /// @brief 是否处于半关闭状态
    bool _halfClosed = false;

    // TODO: 临时方案: 先让他们动起来, 我会重新架构
    // 这是一个混乱的方案, 目前! By Heng_Xin (2024-8-19 15:09:09)

    HX::STL::coroutine::task::Task<> getSpan(std::span<char> s);
    HX::STL::coroutine::task::Task<std::string> getN(std::size_t n);

    HX::STL::coroutine::task::Task<WebSocketPacket> recvPacket();
    HX::STL::coroutine::task::Task<> sendPacket(WebSocketPacket packet, uint32_t mask = 0);

    HX::STL::coroutine::task::Task<> sendPing();
public:
    WebSocket(WebSocket &&) = default;

    explicit WebSocket(const HX::web::protocol::http::Request& req) : _req(req)
    {}

    /**
     * @brief 创建一个WebSocket协议的服务端, 如果可以升级为 WebSocket 则返回 WebSocket指针, 否则是nullptr
     * @return WebSocket指针
     */
    static HX::STL::coroutine::task::Task<pointer> makeServer(
        const HX::web::protocol::http::Request& req
    );

    /**
     * @brief 启动 WebSocket
     * @param pingPongTimeout 心跳包超时时间
     * @return 协程任务 (需要`co_await`)
     */
    HX::STL::coroutine::task::Task<> start(
        std::chrono::steady_clock::duration pingPongTimeout = std::chrono::seconds(5)
    );

    /**
     * @brief 设置收到客户端消息时候触发的回调函数
     * @param onMessage 回调函数
     */
    void setOnMessage(
        std::function<HX::STL::coroutine::task::Task<>(const std::string&)> onMessage
    ) {
        _onMessage = std::move(onMessage);
    }

    /**
     * @brief 设置关闭WebSocket时候触发的回调函数
     * @param onClose 回调函数
     */
    void setOnClose(
        std::function<HX::STL::coroutine::task::Task<>()> onClose
    ) {
        _onClose = std::move(onClose);
    }

    /**
     * @brief 设置收到客户端回复的Pong包的时候触发的回调函数
     * @param onPong 回调函数
     */
    void setOnPong(
        std::function<HX::STL::coroutine::task::Task<>(std::chrono::steady_clock::duration)> onPong
    ) {
        _onPong = std::move(onPong);
    }

    HX::STL::coroutine::task::Task<> send(const std::string& text);
};

}}}} // HX::web::protocol::websocket

#endif // !_HX_WEB_SOCKET_H_