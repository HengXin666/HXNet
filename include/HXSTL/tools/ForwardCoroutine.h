#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-11 21:26:06
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
#ifndef _HX_FORWARD_COROUTINE_H_
#define _HX_FORWARD_COROUTINE_H_

#include <coroutine>

namespace HX { namespace STL { namespace tools {

/**
 * @brief 协程转发 (为了防止相互依赖而搞的函数)
 */
struct ForwardCoroutineTools {
    /**
     * @brief 转移句柄执行权到计时器
     * @param coroutine 
     */
    static void TimerLoopAddTask(std::coroutine_handle<> coroutine);
};

}}} // namespace HX::STL::tools

#endif // !_HX_FORWARD_COROUTINE_H_