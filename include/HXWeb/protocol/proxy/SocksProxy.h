#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-29 22:18:01
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
#ifndef _HX_SOCKS_PROXY_H_
#define _HX_SOCKS_PROXY_H_

#include <HXWeb/protocol/proxy/ProxyBase.h>

namespace HX { namespace web { namespace protocol { namespace proxy {

class Socks5Proxy : public HX::web::protocol::proxy::ProxyBash {
protected:
    virtual HX::STL::coroutine::task::Task<bool> _connect(
        const std::string& url,
        const HX::web::client::IO& io
    );

    friend HX::web::protocol::proxy::ProxyBash;
private:
    
};

}}}} // namespace HX::web::protocol::proxy

#endif // !_HX_SOCKS_PROXY_H_