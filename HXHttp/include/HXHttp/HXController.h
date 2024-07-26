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

#include <string_view>
#include <list>

#include <HXHttp/HXRouter.h>
#include <HXHttp/HXRequest.h>
#include <HXHttp/HXResponse.h>
#include <HXSTL/HXStringTools.h>

namespace HXHttp {

/**
 * @brief 控制器类
 */
class HXController {
protected:
    // std::list<std::string, std::function<void()>> _endpoints;
public:
    // explicit HXController() : _endpoints()
    // {}

    // std::list<std::string, std::function<void()>>&& getEndpoints() {
    //     // return std::move(_endpoints);
    // }

    // 注册端点函数
    // void addEndpoint(const std::string& url, const std::function<void()>& func) {
    //     // _endpoints.emplace_back(url, func);
    // }
};

/// @brief 测试使用的
class MyWebController : HXController {
    const int x = []() -> int {
        HXRouter::getSingleton().addController("GET", "/home", [](const HXHttp::HXRequest& req) -> HXHttp::HXResponse {
            HXHttp::HXResponse response;
            response.setResponseLine(HXHttp::HXResponse::Status::CODE_200)
                .setContentType("text/html", "UTF-8")
                .setBodyData(execQueryHomeData());
            return response;
        });
        return 0;
    }();

public:
    static std::string execQueryHomeData() {
        return "<h1>Heng_Xin Home 哇!</h1><h2>Now Time: " 
                + HXSTL::HXDateTimeFormat::format() 
                + "</h2>";
    }
};

} // namespace HXHttp

#endif // _HX_HXCONTROLLER_H_
