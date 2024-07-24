#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-24 11:54:00
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
#ifndef _HX_HXBYTESBUFFER_H_
#define _HX_HXBYTESBUFFER_H_

#include <vector>
#include <string_view>

namespace HXSTL {

/**
 * @brief 字节数组 (分配在堆上)
 */
class HXBytesBuffer {
    std::vector<char> _data;
public:
    HXBytesBuffer() = default;
    HXBytesBuffer(HXBytesBuffer &&) = default;
    HXBytesBuffer &operator=(HXBytesBuffer &&) = default;
    explicit HXBytesBuffer(HXBytesBuffer const &) = default;

    explicit HXBytesBuffer(size_t n) : _data(n) {}

    const char *data() const noexcept {
        return _data.data();
    }

    char *data() noexcept {
        return _data.data();
    }

    size_t size() const noexcept {
        return _data.size();
    }

    const char* begin() const noexcept {
        return _data.data();
    }

    char* begin() noexcept {
        return data();
    }

    const char* end() const noexcept {
        return data() + size();
    }

    char* end() noexcept {
        return data() + size();
    }

    operator std::string_view() const noexcept {
        return std::string_view {_data.data(), _data.size()};
    }
};

} // namespace HXSTL

#endif // _HX_HXBYTESBUFFER_H_
