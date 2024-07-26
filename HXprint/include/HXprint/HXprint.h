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
#include <format>
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

#ifdef _HX_DEBUG_
#include <cstdio>
#include <cstdarg>
#endif

namespace HXprint { // C++20

// 内部使用的命名空间啊喂!
namespace _ {
/////////////////////////////////////////////////////////

// === 仅DEBUG编译期(未发布)有的日志打印 ===

#ifdef _HX_DEBUG_

enum LogLevel {
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
};

void logMessage(LogLevel level, const char* format, ...);

#define LOG_ERROR(...) logMessage(HXprint::_::LOG_ERROR, __VA_ARGS__)
#define LOG_WARNING(...) logMessage(HXprint::_::LOG_WARNING, __VA_ARGS__)
#define LOG_INFO(...) logMessage(HXprint::_::LOG_INFO, __VA_ARGS__)

#else

#define LOG_ERROR(...)
#define LOG_WARNING(...)
#define LOG_INFO(...)

#endif
/////////////////////////////////////////////////////////

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
                                 std::is_same_v<T, std::multiset<typename T::value_type>> ||
                                 std::is_same_v<T, std::vector<typename T::value_type>> ||
                                 std::is_same_v<T, std::list<typename T::value_type>> ||
                                 std::is_same_v<T, std::deque<typename T::value_type>> ||
                                 std::is_same_v<T, std::unordered_multiset<typename T::value_type>> ||
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

// === 事先声明 ===

// 显式重载
static void _HXprint(const std::nullptr_t& t);
static void _HXprint(const std::nullopt_t& t);
static void _HXprint(const std::monostate& t);
static void _HXprint(bool t);

// 基础类型
template<typename T>
static void _HXprint(const T& t);

// std::optional
template<typename... Ts>
static void _HXprint(const std::optional<Ts...>& t);

// str相关的类型
template<StringType ST>
static void _HXprint(const ST& t);

// std::pair
template<PairContainer Container>
static void _HXprint(const Container& p);

// std::的常见的支持迭代器的单元素容器
template<SingleElementContainer Container>
static void _HXprint(const Container& sc);

// std::的常见的支持迭代器的键值对容器
template <KeyValueContainer Container>
static void _HXprint(const Container& map);

// std::variant 现代共用体
template<typename... Ts>
static void _HXprint(const std::variant<Ts...>& t);

/////////////////////////////////////////////////////////

static void _HXprint(const std::nullptr_t& t) { // 普通指针不行
    _HXprint("nullptr");
}

static void _HXprint(const std::nullopt_t& t) {
    _HXprint("nullopt");
}

static void _HXprint(const std::monostate& t) {
    _HXprint("monostate");
}

static void _HXprint(bool t) {
    if (t)
        _HXprint("true");
    else
        _HXprint("false");
}

template<typename T>
static void _HXprint(const T& t) {
    std::cout << t;
}

template<StringType ST>
static void _HXprint(const ST& t) {
    std::cout << std::quoted(t);
}

template<typename... Ts>
static void _HXprint(const std::optional<Ts...>& t) {
    if (t.has_value())
        _HXprint(*t);
    else
        _HXprint(std::nullopt);
}

template<typename... Ts>
static void _HXprint(const std::variant<Ts...>& t) {
    std::visit([] (const auto &v) -> void { // 访问者模式
        _HXprint(v);
    }, t);
}

template<PrintClassType T>
static void _HXprint(const T& t) {
    t.print();
}

template<PairContainer Container>
static void _HXprint(const Container& p) {
    _HXprint('(');
    _HXprint(std::get<0>(p));
    _HXprint(", ");
    _HXprint(std::get<1>(p));
    _HXprint(')');
}

// 递归打印tuple
template <std::size_t I = 0, typename... Ts>
static void _print(const std::tuple<Ts...>& tup) {
    if constexpr (I == sizeof...(Ts)) { // 因为 I 从 0 开始
        return;
    } else {
        if constexpr (I > 0)
            _HXprint(", ");
        _HXprint(std::get<I>(tup));
        _print<I + 1>(tup);
    }
}

template <std::size_t I = 0, typename... Ts>
static void _HXprint(const std::tuple<Ts...>& tup) {
    _HXprint('(');
    _print(tup);
    _HXprint(')');
}

template <KeyValueContainer Container>
static void _HXprint(const Container& map) {
    _HXprint('{');
    bool once = false;
    for (const auto& [k, v] : map) {
        if (once)
            _HXprint(", ");
        else
            once = true;
        _HXprint(k);
        _HXprint(": ");
        _HXprint(v);
    }
    _HXprint('}');
}

template<SingleElementContainer Container>
static void _HXprint(const Container& sc) {
    _HXprint('[');
    bool once = false;
    for (const auto& it : sc) {
        if (once)
            _HXprint(", ");
        else
            once = true;
        _HXprint(it);
    }
    _HXprint(']');
}

} // namespace _

/**
 * @brief 打印带'\\n'的对象, 多个则使用 ' ' 空格分开.
 */
template <class T0, class ...Ts>
void print(T0 const &t0, Ts const &...ts) {
    _::_HXprint(t0);
    ((std::cout << " ", _::_HXprint(ts)), ...);
    std::cout << "\n";
}

/**
 * @brief 打印不带'\\n'的对象, 多个则使用 ' ' 空格分开.
 */
template <class T0, class ...Ts>
void printnl(T0 const &t0, Ts const &...ts) {
    _::_HXprint(t0);
    ((std::cout << " ", _::_HXprint(ts)), ...);
}
// 注: 建议鸭子类型使用printnl!

#endif // __cpp_lib_concepts

} // namespace HXprint

#endif // _HX_HXPRINT_H_
