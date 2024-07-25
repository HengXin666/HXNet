#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-20 23:18:48
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
#ifndef _HX_HXRESPONSE_H_
#define _HX_HXRESPONSE_H_

#include <string>
#include <string_view>
#include <unordered_map>

#include <HXSTL/HXBytesBuffer.h>

namespace HXHttp {

/**
 * @brief 服务端响应类(Response)
 */
class HXResponse {
    // 注意: 他们的末尾并没有事先包含 \r\n, 具体在to_string才提供
    std::string _statusLine; // 状态行
    std::unordered_map<std::string, std::string> _responseHeaders; // 响应头部
    // 空行
    std::string _responseBody; // 响应体
public:
    // @brief 待写入的内容
    HXSTL::HXBytesBuffer _buf;

    /**
     * @brief 响应状态码
     */
    enum class Status : int {
        CODE_100 = 100, // Continue
        CODE_101 = 101, // Switching Protocols
        CODE_102 = 102, // Processing
        CODE_200 = 200, // OK
        CODE_201 = 201, // Created
        CODE_202 = 202, // Accepted
        CODE_203 = 203, // Non-Authoritative Information
        CODE_204 = 204, // No Content
        CODE_205 = 205, // Reset Content
        CODE_206 = 206, // Partial Content
        CODE_207 = 207, // Multi-Status
        CODE_226 = 226, // IM Used
        CODE_300 = 300, // Multiple Choices
        CODE_301 = 301, // Moved Permanently
        CODE_302 = 302, // Moved Temporarily
        CODE_303 = 303, // See Other
        CODE_304 = 304, // Not Modified
        CODE_305 = 305, // Use Proxy
        CODE_306 = 306, // Reserved
        CODE_307 = 307, // Temporary Redirect
        CODE_400 = 400, // Bad Request
        CODE_401 = 401, // Unauthorized
        CODE_402 = 402, // Payment Required
        CODE_403 = 403, // Forbidden
        CODE_404 = 404, // Not Found
        CODE_405 = 405, // Method Not Allowed
        CODE_406 = 406, // Not Acceptable
        CODE_407 = 407, // Proxy Authentication Required
        CODE_408 = 408, // Request Timeout
        CODE_409 = 409, // Conflict
        CODE_410 = 410, // Gone
        CODE_411 = 411, // Length Required
        CODE_412 = 412, // Precondition Failed
        CODE_413 = 413, // Request Entity Too Large
        CODE_414 = 414, // Request-URI Too Large
        CODE_415 = 415, // Unsupported Media Type
        CODE_416 = 416, // Requested Range Not Satisfiable
        CODE_417 = 417, // Expectation Failed
        CODE_422 = 422, // Unprocessable Entity
        CODE_423 = 423, // Locked
        CODE_424 = 424, // Failed Dependency
        CODE_425 = 425, // Unordered Collection
        CODE_426 = 426, // Upgrade Required
        CODE_428 = 428, // Precondition Required
        CODE_429 = 429, // Too Many Requests
        CODE_431 = 431, // Request Header Fields Too Large
        CODE_434 = 434, // Requested host unavailable
        CODE_444 = 444, // Close connection without sending headers
        CODE_449 = 449, // Retry With
        CODE_451 = 451, // Unavailable For Legal Reasons
        CODE_500 = 500, // Internal Server Error
        CODE_501 = 501, // Not Implemented
        CODE_502 = 502, // Bad Gateway
        CODE_503 = 503, // Service Unavailable
        CODE_504 = 504, // Gateway Timeout
        CODE_505 = 505, // HTTP Version Not Supported
        CODE_506 = 506, // Variant Also Negotiates
        CODE_507 = 507, // Insufficient Storage
        CODE_508 = 508, // Loop Detected
        CODE_509 = 509, // Bandwidth Limit Exceeded
        CODE_510 = 510, // Not Extended
        CODE_511 = 511, // Network Authentication Required
    };

    /**
     * @brief 构造一个响应, 并且初始化状态行 (协议使用HTTP/1.1)
     * @param statusCode 状态码
     * @param describe 状态码描述: 如果为`""`则会使用该状态码对应默认的描述
     * @warning 不需要手动写`/r`或`/n`以及尾部的`/r/n`
     */
    explicit HXResponse(HXResponse::Status statusCode, std::string_view describe = "");

    explicit HXResponse() : _statusLine("HTTP/1.1 ")
                          , _responseHeaders()
                          , _responseBody()
                          , _buf()
    {}

    /**
     * @brief 设置状态行 (协议使用HTTP/1.1)
     * @param statusCode 状态码
     * @param describe 状态码描述: 如果为`""`则会使用该状态码对应默认的描述
     * @warning 不需要手动写`/r`或`/n`以及尾部的`/r/n`
     */
    HXResponse& setResponseLine(HXResponse::Status statusCode, std::string_view describe = "");

    /**
     * @brief 设置响应头部: 设置响应类型, 如果响应体是文本, 你需要指定字符编码(不指定则留空`""`)
     * @param type 响应类型, 如`text/html`
     * @param encoded 字符编码, 如`UTF-8` ~~(如果是图片等就可以不用指定)~~
     * @return [this&] 可以链式调用
     * @warning 不需要手动写`/r`或`/n`以及尾部的`/r/n`
     */
    HXResponse& setContentType(const std::string& type, const std::string& encoded = "") {
        if (encoded == "") // Content-Type: text/html
            _responseHeaders["Content-Type"] = type;
        else // Content-Type: text/html; charset=UTF-8
            _responseHeaders["Content-Type"] = type + ";charset=" + encoded;
        return *this;
    }

    /**
     * @brief 设置响应体
     * @param data 响应体数据
     * @return [this&] 可以链式调用
     * @warning 不需要手动写`/r`或`/n`以及尾部的`/r/n`
     */
    HXResponse& setBodyData(const std::string& data) {
        _responseBody = data;
        return *this;
    }

    /**
     * @brief 生成响应字符串, 用于写入
     */
    void createResponseBuffer();

    /**
     * @brief 清空已写入的响应, 重置状态 (复用)
     */
    void clear() {
        _statusLine = "HTTP/1.1 ";
        _responseHeaders.clear();
        _responseBody.clear();
        _buf.clear();
    }

    /**
     * @brief 发送响应信息给 fd
     * @param fd 客户端套接字
     * @return 发送字节数 或 -1 (错误)
     * @warning 内部会通过Body内容设置`Content-Length`长度, 不需要用户手动设置
     */
    [[deprecated]] ssize_t sendResponse(int fd) const;
};

} // namespace HXHttp

#endif // _HX_HXRESPONSE_H_
