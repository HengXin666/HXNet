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
#ifndef _HX_REQUEST_H_
#define _HX_REQUEST_H_

#include <vector>
#include <unordered_map>
#include <string>
#include <optional>

#include <HXSTL/container/BytesBuffer.hpp>

namespace HX { namespace web { namespace protocol { namespace http {

class Response;

/**
 * @brief 客户端请求类(Request)
 */
class Request {
    /**
     * @brief 请求行数据分类
     */
    enum RequestLineDataType {
        RequestType = 0,        // 请求类型
        RequestPath = 1,        // 请求路径
        ProtocolVersion = 2,    // 协议版本
    };

    HX::STL::container::BytesBuffer _previousData; // 之前未解析全的数据

    std::vector<std::string> _requestLine; // 请求行
    std::unordered_map<std::string, std::string> _requestHeaders; // 请求头

    // 请求体
    std::optional<std::string> _body;

    // @brief 仍需读取的请求体长度
    std::optional<std::size_t> _remainingBodyLen;

    // @brief 是否解析完成请求头
    bool _completeRequestHeader = false;
public:
    /// @brief 缓冲区大小: 第一次recv的大小, 以及存放其值的缓冲区数组的大小
    static constexpr std::size_t kBufSize = 4096U;

    Response* _responsePtr = nullptr;

    /**
     * @brief 请求类型枚举
     */
    enum class RequestType : int {
        GET = 0,
        POST = 1,
        PUT = 2,
        DELETE = 3,

        // 极少使用
        HEAD = 4,    // 获得报文首部
        OPTIONS = 5, // 询问支持的方法
        PATCH = 6,   // 局部更新文件
        TRACE = 7,   // 追踪路径
        CONNECT = 8, // 要求用隧道协议连接代理
    };

    explicit Request() : _requestLine()
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
    std::size_t parserRequest(HX::STL::container::ConstBytesBufferView buf);

    /**
     * @brief 获取请求头键值对的引用
     * @return std::unordered_map<std::string, std::string>& 
     */
    std::unordered_map<std::string, std::string>& getRequestHeaders() const {
        return const_cast<std::unordered_map<std::string, std::string>&>(_requestHeaders);
    }

    /**
     * @brief 解析查询参数 (解析如: `?name=loli&awa=ok&hitori`)
     * @return 返回解析到的字符串键值对哈希表
     * @warning 如果解析到不是键值对的, 即通过`&`分割后没有`=`的, 默认其全部为Key, 但Val = ""
     */
    std::unordered_map<std::string, std::string> getParseQueryParameters() const;

    /**
     * @brief 获取请求类型
     * @return 请求类型 (如: "GET", "POST"...)
     */
    std::string getRequesType() const {
        return _requestLine[RequestLineDataType::RequestType];
    }

    /**
     * @brief 获取请求体 ( 临时设计的 )
     * @return 如果没有请求体, 则返回`""`
     */
    std::string getRequesBody() const {
        if (_body)
            return *_body;
        return "";
    }

    /**
     * @brief 获取请求PATH
     * @return 请求PATH (如: "/", "/home?loli=watasi"...)
     */
    std::string getRequesPath() const {
        return _requestLine[RequestLineDataType::RequestPath];
    }

    /**
     * @brief 获取请求的纯PATH部分
     * @return 请求PATH (如: "/", "/home?loli=watasi"的"/home"部分)
     */
    std::string getPureRequesPath() const {
        std::string path = getRequesPath();
        std::size_t pos = path.find('?');
        if (pos == std::string::npos)
            return path;
        return path.substr(0, pos);
    }

    /**
     * @brief 获取请求协议版本
     * @return 请求协议版本 (如: "HTTP/1.1", "HTTP/2.0"...)
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

}}}} // namespace HX::web::protocol::http

#endif // _HX_REQUEST_H_
