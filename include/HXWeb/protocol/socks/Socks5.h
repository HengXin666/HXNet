#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-27 19:36:38
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
#ifndef _HX_SOCKS5_H_
#define _HX_SOCKS5_H_

namespace HX { namespace web { namespace protocol { namespace socks {

class Socks5 {
public:
    /// @brief 握手
    void handshake();


};

}}}} // namespace HX::web::protocol::socks

#endif // !_HX_SOCKS5_H_