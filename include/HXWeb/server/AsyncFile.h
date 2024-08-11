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

#include <HXSTL/container/BytesBuffer.h>
#include <HXSTL/coroutine/awaiter/Task.hpp>
#include <HXSTL/coroutine/loop/AsyncLoop.h>
#include <HXWeb/socket/FileDescriptor.h>
#include <HXWeb/socket/AddressResolver.h>

namespace HX { namespace web { namespace server {

/**
 * @brief 异步文件操作类
 */
class AsyncFile : public HX::web::socket::FileDescriptor {
public:
    AsyncFile() = default;

    explicit AsyncFile(int fd);

    AsyncFile(AsyncFile &&) = default;
    AsyncFile &operator=(AsyncFile &&) = default;

    /**
     * @brief 建立连接
     */
    static AsyncFile asyncBind(HX::web::socket::AddressResolver::AddressInfo const &addr);

    /**
     * @brief 异步建立连接
     * @param addr [out] 用于记录`客户端`信息
     * @return 连接成功的客户端fd
     */
    HX::STL::coroutine::awaiter::Task<
        int, 
        HX::STL::coroutine::loop::EpollFilePromise
    > asyncAccept(
        HX::web::socket::AddressResolver::Address& addr
    );

    /**
     * @brief 异步读取
     * @param buf 存放所有读取到的数据
     * @param count 期望读取的字节数
     */
    HX::STL::coroutine::awaiter::Task<
        size_t, 
        HX::STL::coroutine::loop::EpollFilePromise
    > asyncRead(
        HX::STL::container::BytesBuffer& buf,
        std::size_t count
    );

    /**
     * @brief 异步写入
     * @param buf 存放需要写入的数据
     */
    HX::STL::coroutine::awaiter::Task<
        size_t, 
        HX::STL::coroutine::loop::EpollFilePromise
    > asyncWrite(
        HX::STL::container::ConstBytesBufferView buf
    );

    ~AsyncFile();

    explicit operator bool() const noexcept {
        return _fd != -1;
    }
};

}}} // namespace HX::web::server

#endif // _HX_ASYNC_FILE_H_