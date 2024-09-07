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
#ifndef _HX_PRINT_H_
#define _HX_PRINT_H_

#include <iostream>
#include <format>
#include <iomanip>
#include <tuple>
#include <map>
#include <unordered_map>
#include <optional>
#include <variant>
#include <concepts>

#ifdef _HX_DEBUG_
#include <cstdio>
#include <cstdarg>
#endif // _HX_DEBUG_

#include <HXSTL/concepts/KeyValueContainer.hpp>
#include <HXSTL/concepts/PairContainer.hpp>
#include <HXSTL/concepts/SingleElementContainer.hpp>
#include <HXSTL/concepts/StringType.hpp>

// 屏蔽未使用函数、变量和参数的警告
#if defined(_MSC_VER) // MSVC
    #pragma warning(push)
    #pragma warning(disable: 4505) // C4505: 未使用的局部函数
    #pragma warning(push)
    #pragma warning(disable: 4101) // C4101: 未使用的局部变量
    #pragma warning(push)
    #pragma warning(disable: 4456) // C4456: 声明隐藏了一个局部变量
#elif defined(__GNUC__) || defined(__clang__) // GCC 和 Clang
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-function"
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-variable"
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

namespace HX { namespace print { // C++20

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

#define LOG_ERROR(...) logMessage(HX::print::_::LOG_ERROR, __VA_ARGS__)
#define LOG_WARNING(...) logMessage(HX::print::_::LOG_WARNING, __VA_ARGS__)
#define LOG_INFO(...) logMessage(HX::print::_::LOG_INFO, __VA_ARGS__)

#else

#define LOG_ERROR(...)
#define LOG_WARNING(...)
#define LOG_INFO(...)

#endif // _HX_DEBUG_

/////////////////////////////////////////////////////////

// 概念: 鸭子类型: 只需要满足有一个成员函数是print的, 即可
template <typename T>
concept PrintClassType = requires(T t) {
    t.print();
};

/////////////////////////////////////////////////////////

// === 事先声明 ===

// 显式重载
static void _HXprint(const std::nullptr_t& t);
static void _HXprint(const std::nullopt_t& t);
static void _HXprint(const std::monostate& t);
static void _HXprint(bool t);

// 基础类型
template <typename T>
static void _HXprint(const T& t);

// std::optional
template <typename... Ts>
static void _HXprint(const std::optional<Ts...>& t);

// str相关的类型
template <HX::STL::concepts::StringType ST>
static void _HXprint(const ST& t);

// std::pair
template <HX::STL::concepts::PairContainer Container>
static void _HXprint(const Container& p);

// std::的常见的支持迭代器的单元素容器
template <HX::STL::concepts::SingleElementContainer Container>
static void _HXprint(const Container& sc);

// std::的常见的支持迭代器的键值对容器
template <HX::STL::concepts::KeyValueContainer Container>
static void _HXprint(const Container& map);

// std::variant 现代共用体
template <typename... Ts>
static void _HXprint(const std::variant<Ts...>& t);

/////////////////////////////////////////////////////////

static void _HXprint(const std::nullptr_t&) { // 普通指针不行
    _HXprint("nullptr");
}

static void _HXprint(const std::nullopt_t&) {
    _HXprint("nullopt");
}

static void _HXprint(const std::monostate&) {
    _HXprint("monostate");
}

static void _HXprint(bool t) {
    if (t)
        _HXprint("true");
    else
        _HXprint("false");
}

template <typename T>
static void _HXprint(const T& t) {
    std::cout << t;
}

template <HX::STL::concepts::StringType ST>
static void _HXprint(const ST& t) {
    std::cout << std::quoted(t);
}

template <typename... Ts>
static void _HXprint(const std::optional<Ts...>& t) {
    if (t.has_value())
        _HXprint(*t);
    else
        _HXprint(std::nullopt);
}

template <typename... Ts>
static void _HXprint(const std::variant<Ts...>& t) {
    std::visit([] (const auto &v) -> void { // 访问者模式
        _HXprint(v);
    }, t);
}

template <PrintClassType T>
static void _HXprint(const T& t) {
    t.print();
}

template <HX::STL::concepts::PairContainer Container>
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

template <HX::STL::concepts::KeyValueContainer Container>
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

template <HX::STL::concepts::SingleElementContainer Container>
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

}} // namespace HX::print

// 恢复删除的警告
#if defined(_MSC_VER) // MSVC
    #pragma warning(pop)
    #pragma warning(pop)
    #pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__) // GCC 和 Clang
    #pragma GCC diagnostic pop
    #pragma GCC diagnostic pop
    #pragma GCC diagnostic pop
#endif

#endif // _HX_PRINT_H_