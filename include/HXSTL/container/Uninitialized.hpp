#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-06 17:18:51
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
 */
#ifndef _HX_UNINITIALIZED_H_
#define _HX_UNINITIALIZED_H_

#include <utility>
#include <memory>

#include <HXSTL/container/NonVoidHelper.hpp>

namespace HX { namespace STL { namespace container {

/**
 * @brief 一个未初始化的值 (用于延迟初始化)
 * @tparam T 值类型
 */
template <class T>
class Uninitialized {
    union {
        T _val;
    };
public:
    Uninitialized() noexcept {}
    Uninitialized(Uninitialized &&) = delete;
    ~Uninitialized() noexcept {}

    /**
     * @brief 移动值
     * @return T 值
     */
    T moveVal() {
        T res(std::move(_val));
        _val.~T();
        return res;
    }

    /**
     * @brief 初始化值
     * @tparam Ts 值类型
     * @param args 用于初始化的值参数
     */
    template <class... Ts>
    void putVal(Ts&&... args) {
        new (std::addressof(_val)) T(std::forward<Ts>(args)...);
    }
};

template <>
class Uninitialized<void> {
public:
    auto moveVal() {
        return NonVoidHelper<> {};
    }
    
    void putVal(NonVoidHelper<>) {}
};

template <class T>
class Uninitialized<T const> : public Uninitialized<T> {};

template <class T>
class Uninitialized<T &> : public Uninitialized<std::reference_wrapper<T>> {};

template <class T>
class Uninitialized<T &&> : public Uninitialized<T> {};

}}} // namespace HX::STL::container

#endif // !_HX_UNINITIALIZED_H_