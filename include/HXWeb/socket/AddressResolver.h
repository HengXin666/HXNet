#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-23 18:05:02
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
#ifndef _HX_ADDRESS_RESOLVER_H_
#define _HX_ADDRESS_RESOLVER_H_

#include <netdb.h>
#include <string>

namespace HX { namespace web { namespace socket {

/**
 * @brief 地址注册类
 */
class AddressResolver {
public:
    /**
     * @brief 用于保存地址引用
     */
    struct AddressRef {
        struct ::sockaddr *_addr; // 指向 sockaddr 结构体的指针
        ::socklen_t _addrlen;     // sockaddr 结构体的长度
    };

    /**
     * @brief 用于保存地址信息
     */
    struct Address {
        union {
            struct ::sockaddr _addr;                 // sockaddr 结构体
            struct ::sockaddr_storage _addr_storage; // 更大的结构体，适应不同的套接字地址类型
        };
        ::socklen_t _addrlen = sizeof(struct ::sockaddr_storage); // 初始化为 sockaddr_storage 的大小

        /**
         * @brief 类型转换操作符重载, 将 Address 转换为 AddressRef
         */
        operator AddressRef() {
            return {&_addr, _addrlen};
        }
    };

    /**
     * @brief 用于处理 addrinfo 结果
     */
    struct AddressInfo {
        struct ::addrinfo *_curr = nullptr; // 指向当前 addrinfo 结构体的指针

        /**
         * @brief 创建套接字、绑定它并监听连接
         * @return 服务器套接字
         */
        int createSocketAndBind() const;

        /**
         * @brief 建立socket套接字
         * @return 服务器套接字
         */
        int createSocket() const;

        /**
         * @brief 获取当前 addrinfo 的地址引用
         * @return 当前 addrinfo 的地址引用
         */
        AddressRef getAddress() const {
            return {_curr->ai_addr, _curr->ai_addrlen};
        }

        /**
         * @brief 移动到下一个 addrinfo 结构体
         * @return 是否可以继续移动
         */
        [[nodiscard]] bool nextEntry() {
            _curr = _curr->ai_next;
            return _curr != nullptr;
        }
    };

private:
    // 指向 addrinfo 链表的头部
    struct ::addrinfo* _head = nullptr;

public:
    /**
     * @brief 解析主机名和服务名为 AddressInfo 链表
     * @param name 主机名或地址字符串(IPv4 的点分十进制表示或 IPv6 的十六进制表示)
     * @param service 服务名可以是十进制的端口号, 也可以是已知的服务名称, 如 ftp、http 等
     * @return 用于处理 addrinfo 结果 的结构体
     */
    AddressInfo resolve(const std::string& name, const std::string& service);
    
    // 默认构造函数
    AddressResolver() = default;

    // 移动构造函数
    AddressResolver(AddressResolver &&that) : _head(that._head) {
        that._head = nullptr;
    }

    /**
     * @brief 析构函数, 释放 addrinfo 链表
     */ 
    ~AddressResolver() {
        if (_head) {
            ::freeaddrinfo(_head);
        }
    }
};

}}} // namespace HX::web::socket

#endif // _HX_ADDRESS_RESOLVER_H_
