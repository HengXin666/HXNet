#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-23 17:30:30
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
#ifndef _HX_HXSERVER_H_
#define _HX_HXSERVER_H_

#include <sys/epoll.h>
#include <cassert>
#include <memory>

#include <HXHttp/HXCallback.h>
#include <HXHttp/HXAddressResolver.h>
#include <HXHttp/HXErrorHandlingTools.h>

namespace HXHttp {

/**
 * @brief 基于回调函数实现异步并依赖epoll以事件促动循环的高并发服务器
 */
class HXServer {
public:
    // @brief epoll (半懒汉单例)类
    struct Epoll {
        int _epfd;

        /// @brief 全局线程独占单例
        inline static thread_local Epoll *G_instance = nullptr;

        Epoll() : _epfd(CHECK_CALL(::epoll_create1, 0)) {}

        void join() {

        }

        ~Epoll() {
            ::close(_epfd);
            G_instance = nullptr;
        }

        static Epoll& get() {
            assert(G_instance);
            return *G_instance;
        }
    };

    /**
     * @brief 异步文件操作类
     */
    class asyncFile {
        int _fd = -1;
    public:
        asyncFile() = default;

        explicit asyncFile(int fd) : _fd(fd) {}

        /**
         * @brief 静态工厂方法: 将fd设置为非阻塞, 注册epoll监听 (EPOLLET)
         * @param fd 文件套接字
         * @return asyncFile对象
         */
        static asyncFile asyncWrap(int fd);

        /**
         * @brief 异步建立连接
         */
        void asyncAccept(
            HXAddressResolver::address addr, 
            HXCallback<HXErrorHandlingTools::Expected<int>> cd);
    };

    /**
     * @brief 连接接受类
     */
    class Acceptor : std::enable_shared_from_this<Acceptor> {
        asyncFile _serverFd;              // 服务器套接字
        HXAddressResolver::address _addr; // 用于存放客户端的地址信息 (在::accept中由操作系统填写; 可复用)
        using pointer = std::shared_ptr<Acceptor>;
    public:
        /**
         * @brief 静态工厂方法
         * @return Acceptor指针
         */
        static pointer make() {
            return std::make_shared<pointer::element_type>();
        }

        /**
         * @brief 开始接受连接: 注册服务器套接字, 绑定并监听端口
         * @param name 主机名或地址字符串(IPv4 的点分十进制表示或 IPv6 的十六进制表示)
         * @param port 服务名可以是十进制的端口号, 也可以是已知的服务名称, 如 ftp、http 等
         */
        void start(const std::string& name, const std::string& port);

        /**
         * @brief 进行连接 -> 调用异步的进行连接
         */
        void accept();
    };

public:

};

} // namespace HXHttp

#endif // _HX_HXSERVER_H_
