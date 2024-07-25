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
#include <functional>

namespace HXHttp {

class HXController;

class HXRequest;
class HXResponse;

/**
 * @brief 路由类: 懒汉单例
 */
class HXRouter {
    /**
     * don't use a char * as a key
     * std::string keys are never your bottleneck
     * the performance difference between a char * and a std::string is a myth.
     */
    std::unordered_map<std::string, std::function<HXResponse(const HXRequest&)>> _routerMap; // URL - 端点函数 路由映射

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

    /**
     * @brief 添加控制器
     * @param path 挂载的PTAH, 如`"/home/%d"`, 尾部不要`/`
     * @param controller 控制器
     * @return 是否添加成功
     */
    bool addController(const std::string& path, const std::function<HXResponse(const HXRequest&)>& fun) {
        return _routerMap.emplace(path, fun).second;
    }

    /**
     * @brief 获取该URL(PTAH)对于绑定的端点函数
     * @param url 访问的目标地址, 如`"/home/%d"`, 尾部不要`/`
     * @return 存在则返回, 否则为`nullptr`
     */
    std::function<HXResponse(const HXRequest&)> getEndpointFunByURL(const std::string& url) const {
        auto it = _routerMap.find(url);
        if (it != _routerMap.end())
            return it->second;
        return nullptr;
    }
};

} // namespace HXHttp

#endif // _HX_HXROUTER_H_
