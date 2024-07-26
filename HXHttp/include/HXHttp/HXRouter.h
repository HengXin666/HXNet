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
    /// @brief 端点函数
    using EndpointFunc = std::function<HXResponse(const HXRequest&)>;

    /**
     * don't use a char * as a key
     * std::string keys are never your bottleneck
     * the performance difference between a char * and a std::string is a myth.
     */
    /// @brief 请求类型 -> URL -> 端点函数 路由映射
    std::unordered_map<std::string, std::unordered_map<std::string, EndpointFunc>> _routerMap;

    explicit HXRouter() : _routerMap() {
        // 注册请求类型
        _routerMap["GET"];
        _routerMap["POST"];
    }

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
     * @brief 添加端点函数
     * @param requestType 请求类型, 如`"GET"`, `POST` (全大写)
     * @param path 挂载的PTAH, 如`"/home/%d"`, 尾部不要`/`
     * @param func 端点函数
     * @throw 如果请求类型不正确则会抛出
     */
    void addController(const std::string& requestType, const std::string& path, const EndpointFunc& func) {
        auto pathFunMapIt = _routerMap.find(requestType);
        if (pathFunMapIt == _routerMap.end()) {
            throw "There is no such request type available"; // 没有这种请求类型
        }
        pathFunMapIt->second.insert_or_assign(path, func);
    }

    /**
     * @brief 获取该请求类型和URL(PTAH)绑定的端点函数
     * @param requestType 请求类型, 如`"GET"`, `POST` (全大写)
     * @param path 访问的目标地址, 如`"/home/%d"`, 尾部不要`/`
     * @return 存在则返回, 否则为`nullptr`
     */
    EndpointFunc getEndpointFunc(const std::string& requestType, const std::string& path) const {
        if (auto pathFunMapIt = _routerMap.find(requestType); pathFunMapIt != _routerMap.end())
            if (auto pairIt = pathFunMapIt->second.find(path); pairIt != pathFunMapIt->second.end())
                return pairIt->second;
        return nullptr;
    }
};

} // namespace HXHttp

#endif // _HX_HXROUTER_H_
