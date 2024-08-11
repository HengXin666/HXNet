#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-31 11:54:46
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
#ifndef _HX_CONNECTION_HANDLER_H_
#define _HX_CONNECTION_HANDLER_H_

#include <HXSTL/container/BytesBuffer.h>
#include <HXSTL/tools/ErrorHandlingTools.h>
#include <HXWeb/server/AsyncFile.h>
#include <HXWeb/protocol/http/Request.h>
#include <HXWeb/protocol/http/Response.h>

namespace HX { namespace web { namespace server {

/**
 * @brief 连接处理类
 */
struct ConnectionHandler {
    AsyncFile _conn;            // 连接上的客户端的套接字
    // 缓存一次接收到的信息
    HX::STL::container::BytesBuffer _buf{ protocol::http::Request::BUF_SIZE };
    
    using pointer = std::shared_ptr<ConnectionHandler>;

    /// 有空改为模版+组合HXRequest/Response
    HX::web::protocol::http::Request _request {};    // 客户端请求类

    HX::web::protocol::http::Response _response {};  // 服务端响应类

    /**
     * @brief 静态工厂方法
     * @return ConnectionHandler指针
     */
    static pointer make() {
        return std::make_shared<pointer::element_type>();
    }

    /**
     * @brief 开始处理连接
     * @param fd 客户端套接字
     */
    HX::STL::coroutine::awaiter::Task<> start(int fd);
};

}}} // namespace HX::web::server

#endif // _HX_CONNECTION_HANDLER_H_