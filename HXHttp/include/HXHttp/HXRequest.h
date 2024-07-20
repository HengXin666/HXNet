#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-20 17:04:53
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
#ifndef _HX_HXREQUEST_H_
#define _HX_HXREQUEST_H_

#include <vector>
#include <unordered_map>
#include <string>
#include <optional>

namespace HXHttp {

/**
 * @brief 客户端请求(Request)
 */
class HXRequest {
    std::vector<std::string> _requestLine; // 请求行
    std::unordered_map<std::string, std::string> _requestHead; // 请求头

    // 请求体
    // bool _haveBody = false;
    // char *_bodyStr = NULL;
    std::optional<std::string> _body;
public:
    explicit HXRequest() : _requestLine()
                         , _requestHead()
                         , _body(std::nullopt)
    {}

    int resolutionRequest(int fd, char *str, std::size_t strLen);
};

} // namespace HXHttp

#endif // _HX_HXREQUEST_H_
