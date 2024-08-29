#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-29 21:50:44
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
#ifndef _HX_PROXY_BASE_H_
#define _HX_PROXY_BASE_H_

#include <string>

#include <HXSTL/coroutine/task/Task.hpp>
#include <HXWeb/client/IO.h>

namespace HX { namespace web { namespace protocol { namespace proxy {

/**
 * @brief 代理协议基类
 */
class ProxyBash {
protected:
    /**
     * @brief 连接代理服务器
     * @param url 代理服务器URL(不带协议) (如: hx:R3L9KvC8@127.0.0.1:2233)
     * @param io 客户端IO流
     * @return HX::STL::coroutine::task::Task<bool> 
     */
    virtual HX::STL::coroutine::task::Task<bool> _connect(
        const std::string& url,
        const HX::web::client::IO& io
    ) = 0;
public:
    /**
     * @brief 连接代理服务器
     * @param url 代理服务器URL (如: socks5://hx:R3L9KvC8@127.0.0.1:2233)
     * @param io 客户端IO流
     * @return HX::STL::coroutine::task::Task<bool> 
     */
    static HX::STL::coroutine::task::Task<bool> connect(
        const std::string& url,
        const HX::web::client::IO& io
    );

    virtual ~ProxyBash() {}
};

}}}} // namespace HX::web::protocol::proxy

#endif // !_HX_PROXY_BASE_H_