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
#ifndef _HX_WEB_SOCKET_SERVER_H_
#define _HX_WEB_SOCKET_SERVER_H_

#include <memory>

#include <HXSTL/coroutine/task/Task.hpp>
#include <HXWeb/protocol/http/Request.h>

namespace HX { namespace web { namespace protocol { namespace websocket {

/**
 * @brief WebSocket 服务
 */
class WebSocketServer {
    using pointer = std::shared_ptr<WebSocketServer>;
public:
    /**
     * @brief 静态工厂方法, 如果可以升级为 WebSocket 则返回 WebSocketServer指针, 否则是nullptr
     * @return WebSocketServer指针
     */
    inline static HX::STL::coroutine::task::Task<pointer> make(
        HX::web::protocol::http::Request& req
    );
};

}}}} // HX::web::protocol::websocket

#endif // !_HX_WEB_SOCKET_SERVER_H_