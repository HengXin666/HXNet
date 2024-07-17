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
#include <iomanip>
#include <tuple>
#include <map>
#include <unordered_map>
#include <optional>
#include <variant>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <unordered_set>
#include <string>
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

// 概念: 鸭子类型: 只需要满足有一个成员函数是print的, 即可
template <typename T>
concept PrintClassType = requires(T t) {
    t.print();
};

// 概念: 如果这个类型和str沾边, 那么使用""包裹, 注: 普通的const char * 是不会包裹的qwq
template <typename T>
concept StringType = std::is_same_v<T, std::string> ||
                     std::is_same_v<T, std::string_view>;

// 概念: 如果这个类型和wstr沾边, 那么使用""包裹 [不支持...]
// template <typename T>
// concept WStringType = std::is_same_v<T, const wchar_t *> ||
//                      std::is_same_v<T, std::wstring_view> ||
//                      std::is_same_v<T, std::wstring>;

/////////////////////////////////////////////////////////

// === 事先声明 === \\ 

// 显式重载
static void print(const std::nullptr_t& t);
static void print(const std::nullopt_t& t);
static void print(const std::monostate& t);
static void print(bool t);

// 基础类型
template<typename T>
static void print(const T& t);

// std::optional
template<typename... Ts>
static void print(const std::optional<Ts...>& t);

// str相关的类型
template<StringType ST>
static void print(const ST& t);

// std::pair
template<PairContainer Container>
static void print(const Container& p);

// std::的常见的支持迭代器的单元素容器
template<SingleElementContainer Container>
static void print(const Container& sc);

// std::的常见的支持迭代器的键值对容器
template <KeyValueContainer Container>
static void print(const Container& map);

// std::variant 现代共用体
template<typename... Ts>
static void print(const std::variant<Ts...>& t);

/////////////////////////////////////////////////////////

static void print(const std::nullptr_t& t) { // 普通指针不行
    print("nullptr");
}

static void print(const std::nullopt_t& t) {
    print("nullopt");
}

static void print(const std::monostate& t) {
    print("monostate");
}

static void print(bool t) {
    if (t)
        print("true");
    else
        print("false");
}

template<typename T>
static void print(const T& t) {
    std::cout << t;
}

template<StringType ST>
static void print(const ST& t) {
    std::cout << std::quoted(t);
}

template<typename... Ts>
static void print(const std::optional<Ts...>& t) {
    if (t.has_value())
        print(*t);
    else
        print(std::nullopt);
}

template<typename... Ts>
static void print(const std::variant<Ts...>& t) {
    std::visit([] (const auto &v) -> void { // 访问者模式
        print(v);
    }, t);
}

template<PrintClassType T>
static void print(const T& t) {
    t.print();
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
