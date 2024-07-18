#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-18 17:40:27
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
#ifndef _HX_HXROUTER_H_
#define _HX_HXROUTER_H_

#include <memory>
#include <unordered_map>
#include <string>

namespace HXHttp {

class HXController;

/**
 * @brief 路由类: 懒汉单例
 */
class HXRouter {
    std::unordered_map<std::string, std::shared_ptr<HXController>> _routerMap; // URL - 控制器 路由映射

    explicit HXRouter() : _routerMap()
    {}

    HXRouter(const HXRouter&) = delete;
    HXRouter& operator=(const HXRouter&) = delete;
public:

    /**
     * @brief 获取路由类单例
     */
    [[nodiscard]] static HXRouter& getSingleton() {
        static HXRouter router{};
        return router;
    }
};

} // namespace HXHttp

#endif // _HX_HXROUTER_H_
