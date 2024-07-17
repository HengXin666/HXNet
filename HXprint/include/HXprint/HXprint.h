#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-16 13:47:40
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
#ifndef _HX_HXPRINT_H_
#define _HX_HXPRINT_H_

#include <iostream>
#include <map>
#include <unordered_map>
#include <list>
#include <set>
#include <unordered_set>
#include <vector>
#include <string>
#include <deque>
#include <tuple>
#include <string_view>
#include <type_traits>
#include <concepts>

namespace HXprint { // C++20

#ifdef __cpp_lib_concepts // C++ >= 20 && concepts

// 概念: 判断类型 T 是否是键值对的关联容器
template <typename T>
concept KeyValueContainer = requires(T t) {
    typename T::key_type;
    typename T::mapped_type;
};

// 概念: 判断类型 T 是否是单元素容器
template <typename T>
concept SingleElementContainer = std::is_same_v<T, std::set<typename T::value_type>> ||
                                 std::is_same_v<T, std::vector<typename T::value_type>> ||
                                 std::is_same_v<T, std::list<typename T::value_type>> ||
                                 std::is_same_v<T, std::deque<typename T::value_type>> ||
                                 std::is_same_v<T, std::unordered_set<typename T::value_type>>;
template <typename T>
concept PairContainer = requires(T t) {
    typename T::first_type;
    typename T::second_type;
};

/////////////////////////////////////////////////////////

// === 事先声明 ===

template<typename T>
static void print(const T& t);

template<PairContainer Container>
static void print(const Container& p);

template<SingleElementContainer Container>
static void print(const Container& sc);

template <KeyValueContainer Container>
static void print(const Container& map);

/////////////////////////////////////////////////////////

template<typename T>
static void print(const T& t) {
    std::cout << t;
}

template<PairContainer Container>
static void print(const Container& p) {
    print('(');
    print(std::get<0>(p));
    print(", ");
    print(std::get<1>(p));
    print(')');
}

// 递归打印tuple
template <std::size_t I = 0, typename... Ts>
static void _print(const std::tuple<Ts...>& tup) {
    if constexpr (I == sizeof...(Ts)) { // 因为 I 从 0 开始
        return;
    } else {
        if constexpr (I > 0)
            print(", ");
        print(std::get<I>(tup));
        _print<I + 1>(tup);
    }
}

template <std::size_t I = 0, typename... Ts>
static void print(const std::tuple<Ts...>& tup) {
    print('(');
    _print(tup);
    print(')');
}

template <KeyValueContainer Container>
static void print(const Container& map) {
    print('{');
    bool once = false;
    for (const auto& [k, v] : map) {
        if (once)
            print(", ");
        else
            once = true;
        print(k);
        print(": ");
        print(v);
    }
    print('}');
}

template<SingleElementContainer Container>
static void print(const Container& sc) {
    print('[');
    bool once = false;
    for (const auto& it : sc) {
        if (once)
            print(", ");
        else
            once = true;
        print(it);
    }
    print(']');
}

#endif // __cpp_lib_concepts

}

#endif // _HX_HXPRINT_H_
