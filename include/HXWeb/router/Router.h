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
#ifndef _HX_ROUTER_H_
#define _HX_ROUTER_H_

#include <memory>
#include <unordered_map>
#include <string>
#include <functional>

#include <HXSTL/coroutine/task/Task.hpp>
#include <HXWeb/router/RouteMapPrefixTree.hpp>

namespace HX::web::protocol::http {

class Request;
class Response;

}

namespace HX { namespace web { namespace router {

/**
 * @brief 路由类: 懒汉单例
 */
class Router {
    /// @brief 端点函数
    using EndpointFunc = std::function<HX::STL::coroutine::task::Task<void>(
        const HX::web::protocol::http::Request&
    )>;

    /**
     * don't use a char * as a key
     * std::string keys are never your bottleneck
     * the performance difference between a char * and a std::string is a myth.
     */
    // std::unordered_map<std::string, std::unordered_map<std::string, EndpointFunc>> _routerMap;
    /// @brief 请求类型 -> URL -> 端点函数 路由映射
    RouteMapPrefixTree<EndpointFunc> _routerRadixTree;

    explicit Router() : _routerRadixTree() 
    {}

    Router(const Router&) = delete;
    Router& operator=(const Router&) = delete;
public:

    /**
     * @brief 获取路由类单例
     */
    [[nodiscard]] static Router& getSingleton() {
        static Router router{};
        return router;
    }

    /**
     * @brief 添加端点函数
     * @param requestType 请求类型, 如`"GET"`, `POST` (全大写)
     * @param path 挂载的PTAH, 如`"/home/{id}"`, 尾部不要`/`
     * @param func 端点函数
     */
    void addController(const std::string& requestType, const std::string& path, const EndpointFunc& func);

    /**
     * @brief 获取该请求类型和URL(PTAH)绑定的端点函数
     * @param requestType 请求类型, 如`"GET"`, `POST` (全大写)
     * @param path 访问的目标地址, 如`"/home\**?loli=imouto"`, 尾部不要`/`, 会解析为`?`之前的内容
     * @return 存在则返回, 否则为`nullptr`
     */
    EndpointFunc getEndpointFunc(const std::string& requestType, const std::string& path);
};

}}} // namespace HX::web::router

#endif // _HX_ROUTER_H_
