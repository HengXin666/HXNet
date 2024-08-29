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

namespace HX { namespace STL { namespace utils {

struct UrlUtils {
    class UrlParser {
    public:
        UrlParser(const std::string& url) {
            parseUrl(url);
        }

        const std::string& getHostname() const {
            return _hostname;
        }

        const std::string& getService() const {
            return _service;
        }

        /**
         * @brief 从 URL 从提取出 Ptah 
         * @param url 
         * @return std::string Ptah
         */
        static std::string extractPath(const std::string& url);
    private:
        std::string _hostname {};
        std::string _service {};

        void parseUrl(const std::string& url);
    };
};

}}} // namespace HX::STL::utils

#endif // !_HX_URL_UTILS_H_