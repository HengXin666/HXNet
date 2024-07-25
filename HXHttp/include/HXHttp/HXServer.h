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

#include <HXSTL/HXCallback.h>
#include <HXSTL/HXBytesBuffer.h>
#include <HXHttp/HXAddressResolver.h>
#include <HXHttp/HXErrorHandlingTools.h>
#include <HXHttp/HXRequest.h>
#include <HXHttp/HXResponse.h>

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

        Epoll() : _epfd(CHECK_CALL(::epoll_create1, 0))  {
            G_instance = this;
        }

        void join();

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
    class AsyncFile {
        int _fd = -1;
    public:
        AsyncFile() = default;

        explicit AsyncFile(int fd) : _fd(fd) {}

        /**
         * @brief 静态工厂方法: 将fd设置为非阻塞, 注册epoll监听 (EPOLLET)
         * @param fd 文件套接字
         * @return asyncFile对象
         */
        static AsyncFile asyncWrap(int fd);

        /**
         * @brief 异步建立连接
         * @param addr [out] 用于记录`客户端`信息
         * @param cd 连接成功的回调函数
         */
        void asyncAccept(
            HXAddressResolver::address& addr, 
            HXSTL::HXCallback<HXErrorHandlingTools::Expected<int>> cd
        );

        /**
         * @brief 异步读取
         * @param buf 存放所有读取到的数据
         * @param count 期望读取的字节数
         * @param cd 读取成功的回调函数
         */
        void asyncRead(
            HXSTL::HXBytesBuffer& buf,
            std::size_t count,
            HXSTL::HXCallback<HXErrorHandlingTools::Expected<size_t>> cd
        );

        /**
         * @brief 异步写入
         * @param buf 存放需要写入的数据
         * @param cd 写入成功的回调函数
         */
        void asyncWrite(
            HXSTL::HXConstBytesBufferView buf,
            HXSTL::HXCallback<HXErrorHandlingTools::Expected<size_t>> cd
        );
    };

    /**
     * @brief 连接处理类
     */
    struct ConnectionHandler : std::enable_shared_from_this<ConnectionHandler> {
        AsyncFile _fd;            // 连接上的客户端的套接字
        // 缓存一次接收到的信息
        HXSTL::HXBytesBuffer _buf{ HXRequest::BUF_SIZE };
        
        using pointer = std::shared_ptr<ConnectionHandler>;

        /// 有空改为模版+组合HXRequest/HXResponse
        HXRequest _request {};    // 客户端请求类

        HXResponse _response {};  // 服务端响应类

        /**
         * @brief 静态工厂方法
         * @return ConnectionHandler指针
         */
        static pointer make() {
            return std::make_shared<pointer::element_type>();
        }

        /**
         * @brief 开始处理连接
         * @param fd 客户端套接字
         */
        void start(int fd);

        /**
         * @brief 开始读取
         * @param size 读取的数据字节大小
         */
        void read(std::size_t size = HXRequest::BUF_SIZE);

        /**
         * @brief 处理请求, 返回响应
         */
        void handle();

        /**
         * @brief 开始写入
         */
        void write(HXSTL::HXConstBytesBufferView buf);
    };

    /**
     * @brief 连接接受类
     */
    class Acceptor : public std::enable_shared_from_this<Acceptor> {
        AsyncFile _serverFd;              // 服务器套接字
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
