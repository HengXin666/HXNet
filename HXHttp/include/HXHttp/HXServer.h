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
#include <chrono>

#include <HXSTL/HXCallback.h>
#include <HXSTL/HXBytesBuffer.h>
#include <HXHttp/HXAddressResolver.h>
#include <HXHttp/HXErrorHandlingTools.h>
#include <HXHttp/HXRequest.h>
#include <HXHttp/HXResponse.h>

/**
 * @brief 基于回调函数实现异步并依赖epoll以事件促动循环的高并发服务器
 */
namespace HXHttp { namespace HXServer {

    // @brief 停止计时器程序
    class StopSource {
        // @brief 控制块
        struct _ControlBlock {
            bool _stop = false;      // 是否停止
            HXSTL::HXCallback<> _cb; // 停止计时器的回调
        };

        // @brief 控制器
        std::shared_ptr<_ControlBlock> _control;
    public:

        explicit StopSource() = default;

        /**
         * @brief 构造函数
         * @param std::in_place_t 区分构造函数的, 传入`std::in_place`即可
         */
        explicit StopSource(std::in_place_t)
        : _control(std::make_shared<_ControlBlock>()) 
        {}

        /**
         * @brief 是否停止请求
         * @return 是否
         */
        bool isStopRequested() const noexcept {
            return _control && _control->_stop;
        }

        /**
         * @brief 是否可以被停止
         * @return 是否
         */
        bool isStopPossible() const noexcept {
            return _control != nullptr;
        }

        /**
         * @brief 停止请求, 并且执行回调函数
         */
        void doRequestStop() const {
            if (!_control) {
                return;
            }
            _control->_stop = true;
            if (_control->_cb) {
                _control->_cb();
                _control->_cb = nullptr;
            }
        }

        /**
         * @brief 设置停止时执行的回调函数
         * @param cb 停止时执行的回调函数
         */
        void setStopCallback(HXSTL::HXCallback<> cb) const noexcept {
            if (!_control) {
                return;
            }
            _control->_cb = std::move(cb);
        }

        /**
         * @brief 清除停止时执行的回调函数
         */
        void clearStopCallback() const noexcept {
            if (!_control) {
                return;
            }
            _control->_cb = nullptr;
        }
    };

    // @brief 回调函数计时器
    class CallbackFuncTimer {
        struct _TimerEntry {
            HXSTL::HXCallback<> _cb; // 这个是计时到点执行的回调
            StopSource _stop;        // 这个是删除计时器的回调
        };

        // 红黑树: 时间 - 计时项
        std::multimap<std::chrono::steady_clock::time_point, _TimerEntry> _timerHeap;
    public:
        explicit CallbackFuncTimer() : _timerHeap()
        {}

        CallbackFuncTimer(CallbackFuncTimer &&) = delete;

        /**
         * @brief 设置超时时间
         * @param dt 持续时间
         * @param cb 超时后执行的回调函数
         * @param stop 停止计时器的回调
         */
        void setTimeout(
            std::chrono::steady_clock::duration dt, 
            HXSTL::HXCallback<> cb, 
            StopSource stop = StopSource {}
        );

        /**
         * @brief 检查所有的持续时间, 如果超时则执行其回调, 否则返回下一个计时器到点的时间
         * @return 下一个计时器到点的时间, 无则返回`-1`
         */
        std::chrono::steady_clock::duration durationToNextTimer();

        bool empty() const {
            return _timerHeap.empty();
        }

        /**
         * @brief 获取红黑树结点个数
         * @return _timerHeap.size()
         */
        std::size_t size() const {
            return _timerHeap.size();
        }
    };

    // @brief epoll上下文 (半懒汉单例)类
    class EpollContext {
    public:
        int _epfd;
        CallbackFuncTimer _timer;

        /// @brief 全局线程独占单例
        inline static thread_local EpollContext *G_instance = nullptr;

        EpollContext() 
        : _epfd(HXErrorHandlingTools::convertError<int>(::epoll_create1(0)).expect("epoll_create1")) 
        , _timer() {
            G_instance = this;
        }

        /**
         * @brief 开始Epoll事件循环, 等待操作系统消息
         */
        void join();

        ~EpollContext() {
            ::close(_epfd);
            G_instance = nullptr;
        }

        /**
         * @brief 获取全局线程独占单例对象引用
         */
        [[gnu::const]] static EpollContext& get() {
            assert(G_instance);
            return *G_instance;
        }
    };

    /**
     * @brief 文件描述符
     */
    class FileDescriptor {
    protected:
        int _fd = -1;
    public:
        FileDescriptor() = default;

        explicit FileDescriptor(int fd) : _fd(fd) 
        {}

        FileDescriptor(FileDescriptor &&that) noexcept : _fd(that._fd) {
            that._fd = -1;
        }

        FileDescriptor &operator=(FileDescriptor &&that) noexcept {
            std::swap(_fd, that._fd);
            return *this;
        }

        ~FileDescriptor() {
            if (_fd == -1) {
                return;
            }
            ::close(_fd);
        }
    };

    /**
     * @brief 异步文件操作类
     */
    class AsyncFile : public FileDescriptor {

        void _epollCallback(
            HXSTL::HXCallback<> &&resume, 
            uint32_t events,
            StopSource stop
        );
    public:
        AsyncFile() = default;

        explicit AsyncFile(int fd);

        AsyncFile(AsyncFile &&) = default;
        AsyncFile &operator=(AsyncFile &&) = default;

        /**
         * @brief 建立连接
         */
        static AsyncFile asyncBind(HXAddressResolver::addressInfo const &addr) {
            auto sock = AsyncFile{addr.createSocket()};
            auto serve_addr = addr.getAddress();
            int on = 1;
            setsockopt(sock._fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
            setsockopt(sock._fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
            HXErrorHandlingTools::convertError<int>(
                ::bind(sock._fd, serve_addr._addr, serve_addr._addrlen)
            ).expect("bind");
            HXErrorHandlingTools::convertError<int>(
                ::listen(sock._fd, SOMAXCONN)
            ).expect("listen");
            return sock;
        }

        /**
         * @brief 异步建立连接
         * @param addr [out] 用于记录`客户端`信息
         * @param cd 连接成功的回调函数
         */
        void asyncAccept(
            HXAddressResolver::address& addr, 
            HXSTL::HXCallback<HXErrorHandlingTools::Expected<int>> cd,
            StopSource stop = StopSource {}
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
            HXSTL::HXCallback<HXErrorHandlingTools::Expected<size_t>> cd,
            StopSource stop = StopSource {}
        );

        /**
         * @brief 异步写入
         * @param buf 存放需要写入的数据
         * @param cd 写入成功的回调函数
         */
        void asyncWrite(
            HXSTL::HXConstBytesBufferView buf,
            HXSTL::HXCallback<HXErrorHandlingTools::Expected<size_t>> cd,
            StopSource stop = StopSource {}
        );

        ~AsyncFile() {
            if (_fd == -1) {
                return;
            }
            ::epoll_ctl(HXServer::EpollContext::get()._epfd, EPOLL_CTL_DEL, _fd, nullptr);
        }

        explicit operator bool() const noexcept {
            return _fd != -1;
        }
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
} // namespace HXServer

} // namespace HXHttp

#endif // _HX_HXSERVER_H_
