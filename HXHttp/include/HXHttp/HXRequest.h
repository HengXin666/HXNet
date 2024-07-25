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

#include <HXSTL/HXBytesBuffer.h>

namespace HXHttp {

/**
 * @brief 客户端请求类(Request)
 */
class HXRequest {
    /**
     * @brief 请求行数据分类
     */
    enum RequestLineDataType {
        RequestType = 0,        // 请求类型
        RequestPath = 1,        // 请求路径
        ProtocolVersion = 2,    // 协议版本
    };

    HXSTL::HXBytesBuffer _previousData; // 之前未解析全的数据

    std::vector<std::string> _requestLine; // 请求行
    std::unordered_map<std::string, std::string> _requestHeaders; // 请求头

    // 请求体
    std::optional<std::string> _body;

    // @brief 仍需读取的请求体长度
    std::optional<std::size_t> _remainingBodyLen;

    // @brief 是否解析完成请求头
    bool _completeRequestHeader = false;
public:
    static constexpr std::size_t BUF_SIZE = 1024ULL;

    explicit HXRequest() : _requestLine()
                         , _requestHeaders()
                         , _body(std::nullopt)
                         , _remainingBodyLen(std::nullopt)
    {}

    /**
     * @brief 解析请求
     * @param buf 需要解析的内容
     * @return 是否需要继续解析;
     *         `== 0`: 不需要;
     *         `>  0`: 需要继续解析`size_t`个字节
     * @warning 假定内容是符合Http协议的
     */
    std::size_t parserRequest(HXSTL::HXConstBytesBufferView buf);

    /**
     * @brief 获取请求类型
     * @return 请求类型 (如: "GET", "POST"...)
     * @warning 需要保证`resolutionRequest`为`ParseSuccessful`
     */
    std::string getRequesType() const {
        return _requestLine[RequestLineDataType::RequestType];
    }

    /**
     * @brief 获取请求PATH
     * @return 请求PATH (如: "/", "/home?loli=watasi"...)
     * @warning 需要保证`resolutionRequest`为`ParseSuccessful`
     */
    std::string getRequesPath() const {
        return _requestLine[RequestLineDataType::RequestPath];
    }

    /**
     * @brief 获取请求协议版本
     * @return 请求协议版本 (如: "HTTP/1.1", "HTTP/2.0"...)
     * @warning 需要保证`resolutionRequest`为`ParseSuccessful`
     */
    std::string getRequesProtocolVersion() const {
        return _requestLine[RequestLineDataType::ProtocolVersion];
    }

    /**
     * @brief 清空已有的请求内容, 并且初始化标准
     */
    void clear() {
        _requestLine.clear();
        _requestHeaders.clear();
        _previousData.clear();
        _body.reset();
        _completeRequestHeader = false;
        _remainingBodyLen.reset();
    }
};

} // namespace HXHttp

#endif // _HX_HXREQUEST_H_
