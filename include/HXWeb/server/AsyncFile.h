#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-30 23:38:28
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
#ifndef _HX_ASYNC_FILE_H_
#define _HX_ASYNC_FILE_H_

#include <HXSTL/container/Callback.h>
#include <HXSTL/tools/ErrorHandlingTools.h>
#include <HXSTL/container/BytesBuffer.h>
#include <HXWeb/socket/FileDescriptor.h>
#include <HXWeb/socket/AddressResolver.h>
#include <HXWeb/server/context/EpollContext.h>

namespace HX { namespace web { namespace server {

/**
 * @brief 异步文件操作类
 */
class AsyncFile : public HX::web::socket::FileDescriptor {

    void _epollCallback(
        HX::STL::container::Callback<> &&resume, 
        uint32_t events,
        context::StopSource stop
    );
public:
    AsyncFile() = default;

    explicit AsyncFile(int fd);

    AsyncFile(AsyncFile &&) = default;
    AsyncFile &operator=(AsyncFile &&) = default;

    /**
     * @brief 建立连接
     */
    static AsyncFile asyncBind(HX::web::socket::AddressResolver::AddressInfo const &addr) {
        auto sock = AsyncFile{addr.createSocket()};
        auto serve_addr = addr.getAddress();
        int on = 1;
        setsockopt(sock._fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        setsockopt(sock._fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
        HX::STL::tools::ErrorHandlingTools::convertError<int>(
            ::bind(sock._fd, serve_addr._addr, serve_addr._addrlen)
        ).expect("bind");
        HX::STL::tools::ErrorHandlingTools::convertError<int>(
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
        HX::web::socket::AddressResolver::Address& addr, 
        HX::STL::container::Callback<HX::STL::tools::ErrorHandlingTools::Expected<int>> cd,
        context::StopSource stop = context::StopSource {}
    );

    /**
     * @brief 异步读取
     * @param buf 存放所有读取到的数据
     * @param count 期望读取的字节数
     * @param cd 读取成功的回调函数
     */
    void asyncRead(
        HX::STL::container::BytesBuffer& buf,
        std::size_t count,
        HX::STL::container::Callback<HX::STL::tools::ErrorHandlingTools::Expected<size_t>> cd,
        context::StopSource stop = context::StopSource {}
    );

    /**
     * @brief 异步写入
     * @param buf 存放需要写入的数据
     * @param cd 写入成功的回调函数
     */
    void asyncWrite(
        HX::STL::container::ConstBytesBufferView buf,
        HX::STL::container::Callback<HX::STL::tools::ErrorHandlingTools::Expected<size_t>> cd,
        context::StopSource stop = context::StopSource {}
    );

    ~AsyncFile() {
        if (_fd == -1) {
            return;
        }
        ::epoll_ctl(context::EpollContext::get()._epfd, EPOLL_CTL_DEL, _fd, nullptr);
    }

    explicit operator bool() const noexcept {
        return _fd != -1;
    }
};

}}} // namespace HX::web::server

#endif // _HX_ASYNC_FILE_H_