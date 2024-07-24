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
#include <stdexcept>

namespace HXSTL {

/**
 * @brief const字节数组视图
 */
class HXConstBytesBufferView {
    const char* _data;
    std::size_t _size;
public:
    HXConstBytesBufferView(const char* data, std::size_t size) 
    : _data(data)
    , _size(size)
    {}

    char const *data() const noexcept {
        return _data;
    }

    size_t size() const noexcept {
        return _size;
    }

    char const *begin() const noexcept {
        return data();
    }

    char const *end() const noexcept {
        return data() + size();
    }

    /**
     * @brief 获取子视图
     * @param start 起始索引
     * @param len 长度
     * @return 子视图
     */
    HXConstBytesBufferView subspan(size_t start, size_t len = static_cast<size_t>(-1)) const {
        if (start > size())
            throw std::out_of_range("HXConstBytesBufferView::subspan");
        if (len > size() - start)
            len = size() - start;
        return {data() + start, len};
    }

    operator std::string_view() const noexcept {
        return std::string_view{data(), size()};
    }
};

/**
 * @brief 字节数组视图
 */
class HXBytesBufferView {
    char* _data;
    std::size_t _size;
public:
    HXBytesBufferView(char* data, std::size_t size) 
    : _data(data)
    , _size(size)
    {}

    char *data() const noexcept {
        return _data;
    }

    size_t size() const noexcept {
        return _size;
    }

    char *begin() const noexcept {
        return data();
    }

    char *end() const noexcept {
        return data() + size();
    }

    /**
     * @brief 获取子视图
     * @param start 起始索引
     * @param len 长度
     * @return 子视图
     */
    HXBytesBufferView subspan(size_t start, size_t len = static_cast<size_t>(-1)) const {
        if (start > size())
            throw std::out_of_range("HXBytesBufferView::subspan");
        if (len > size() - start)
            len = size() - start;
        return {data() + start, len};
    }

    operator HXConstBytesBufferView() const noexcept {
        return HXConstBytesBufferView{data(), size()};
    }

    operator std::string_view() const noexcept {
        return std::string_view{data(), size()};
    }
};

/**
 * @brief 字节数组 (分配在堆上)
 */
class HXBytesBuffer {
    std::vector<char> _data;
public:
    HXBytesBuffer() = default;
    HXBytesBuffer(HXBytesBuffer &&) = default;
    HXBytesBuffer &operator=(HXBytesBuffer &&) = default;

    HXBytesBuffer &&operator=(std::string && str) {
        return std::move(HXBytesBuffer {str.data(), str.size()});
    }

    explicit HXBytesBuffer(char *str, std::size_t size) : _data(size) {
        for (std::size_t i = 0; i < size; ++i)
            _data[i] = str[i];
    }

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

    void append(HXConstBytesBufferView chunk) {
        _data.insert(_data.end(), chunk.begin(), chunk.end());
    }

    void append(std::string_view chunk) {
        _data.insert(_data.end(), chunk.begin(), chunk.end());
    }

    template <size_t N>
    void append(const char (&chunk)[N]) {
        append(std::string_view{chunk, N - 1});
    }

    operator std::string_view() const noexcept {
        return std::string_view {_data.data(), _data.size()};
    }

    operator HXConstBytesBufferView() const noexcept {
        return HXConstBytesBufferView {_data.data(), _data.size()};
    }

    /**
     * @brief 清楚容器数据
     */
    void clear() {
        _data.clear();
    }

    /**
     * @brief 设置容器的有效大小
     * @param n 设置容器的大小
     */
    void resize(size_t n) {
        _data.resize(n);
    }

    /**
     * @brief 设置容器的空间大小 (只变大小, 不改值, 如果`n < .size()`则什么也不做)
     * @param n 设置容器空间的大小
     */
    void reserve(size_t n) {
        _data.reserve(n);
    }
};

} // namespace HXSTL

#endif // _HX_HXBYTESBUFFER_H_
