#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-19 15:14:12
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
#ifndef _HX_HXCONTROLLER_H_
#define _HX_HXCONTROLLER_H_

#include <HXHttp/HXRouter.h>

#include <string_view>

namespace HXHttp {

/**
 * @brief 控制器类
 */
class HXController {

};

/// @brief 测试使用的
class MyWebController : HXController {
    // 请求类型, URL, 端点名称, 请求数据...(可变参数)
    int fun(std::string_view requestType, const std::unordered_map<std::string, std::string>& requestHead) {
        if (requestType == "GET")
            return -1;
        // 内部的 requestHead 处理, 需要的
    }
};

} // namespace HXHttp

#endif // _HX_HXCONTROLLER_H_
