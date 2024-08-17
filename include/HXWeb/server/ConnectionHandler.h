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

#include <HXSTL/coroutine/loop/TimerLoop.h>

namespace HX { namespace web { namespace server {

/**
 * @brief 连接处理类
 */
struct ConnectionHandler {

    /**
     * @brief 开始处理连接
     * @param fd 客户端套接字
     * @param timeout 没有收到消息, 自动断开连接的`超时时间`
     */
    static HX::STL::coroutine::task::TimerTask start(
        int fd, 
        std::chrono::seconds timeout
    );
};

}}} // namespace HX::web::server

#endif // _HX_CONNECTION_HANDLER_H_