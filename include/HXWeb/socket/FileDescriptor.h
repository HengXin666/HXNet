#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-30 23:22:40
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
#ifndef _HX_FILE_DESCRIPTOR_H_
#define _HX_FILE_DESCRIPTOR_H_

#include <unistd.h>
#include <memory>

namespace HX { namespace web { namespace socket {

/**
 * @brief 文件描述符
 */
class FileDescriptor {
protected:
public:
    int _fd = -1;
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

    virtual ~FileDescriptor() {
        if (_fd == -1) {
            return;
        }
        ::close(_fd);
    }
};

}}} // namespace HX::web::socket

#endif // _HX_FILE_DESCRIPTOR_H_
