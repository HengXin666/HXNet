#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-29 14:05:01
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
#ifndef _HX_URL_UTILS_H_
#define _HX_URL_UTILS_H_

#include <string>
#include <optional>

namespace HX { namespace STL { namespace utils {

struct UrlUtils {

    /**
     * @brief 从 URL 中提取出 主机名称(域名或者ip) 和 端口(如果没有直接提供端口则使用协议的默认服务端口, 否则默认http)
     */
    class UrlInfoExtractor {
    public:
        UrlInfoExtractor(const std::string& url) {
            parseUrl(url);
        }

        const std::string& getHostname() const {
            return _hostname;
        }

        const std::string& getService() const {
            return _service;
        }

    private:
        std::string _hostname {};
        std::string _service {};

        void parseUrl(const std::string& url);
    };

    /**
     * @brief 从 URL 从提取出 Ptah 
     * @param url 
     * @return std::string Ptah
     */
    static std::string extractPath(const std::string& url);

    /**
     * @brief 从 URL 从提取出 域名
     * @param url 
     * @return std::string DomainName
     */
    static std::string extractDomainName(const std::string& url);

    /**
     * @brief 从 URL 提取 Protocol(协议, 如`http:// -> http`)
     * @param url 
     * @return std::string Protocol
     * @throw 如果提取失败则会抛出异常
     */
    static std::string extractProtocol(const std::string& url);

    /**
     * @brief 从 URL 中解析出用户名和密码, 如`hx:666@www.loli.com -> hx, 666`
     * @return pair<用户名, 密码>, 如果解析不到, 则返回 std::nullopt
     */
    static std::optional<std::pair<std::string, std::string>> extractUser(const std::string& url);

    /**
     * @brief 从 URL 剔除 Protocol(协议, 如`http:// -> http`)
     * @param url 
     * @return std::string Protocol
     * @throw 如果提取失败则会抛出异常
     */
    static std::string removeProtocol(std::string& url);

    /**
     * @brief 将协议转化为对应的端口, 如果常见的映射没有则假设 protocol 是端口的字符串形式
     * @param protocol 协议, 如`"http" -> 80`
     * @throw 不在常见的协议内
     * @return u_int16_t 
     */
    static u_int16_t getProtocolPort(const std::string& protocol);
};

}}} // namespace HX::STL::utils

#endif // !_HX_URL_UTILS_H_