#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-09-07 15:43:57
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
#ifndef _HX_TO_STRING_H_
#define _HX_TO_STRING_H_

#include <iomanip>
#include <optional>
#include <tuple>
#include <map>
#include <unordered_map>
#include <variant>
#include <span>
#include <format>
#include <cmath>

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

namespace HX { namespace STL { namespace utils {

// 内部使用的命名空间啊喂!
namespace internal {
    
// 概念: 鸭子类型: 只需要满足有一个成员函数是toString的, 即可
template <typename T>
concept ToStringClassType = requires(T t) {
    t.toString();
};

// 概念: 鸭子类型: 只需要满足有一个成员函数是toJson的, 即可
template <typename T>
concept ToJsonClassType = requires(T t) {
    t.toJson();
};

// 主模版
template <typename... Ts>
struct ToString {
    static std::string toString(const Ts&...) { // 禁止默认实现
        static_assert(sizeof...(Ts) == 0, "toString is not implemented for this type");
        return {};
    }
};

// ===偏特化 ===
template <>
struct ToString<std::nullptr_t> {
    static std::string toString(const std::nullptr_t&) { // 普通指针不行
        return "nullptr";
    }
};

template <>
struct ToString<std::nullopt_t> {
    static std::string toString(const std::nullopt_t&) {
        return "nullopt";
    }
};

template <>
struct ToString<bool> {
    static std::string toString(bool t) {
        return t ? "true" : "false";
    }
};

template <>
struct ToString<std::monostate> {
    static std::string toString(const std::monostate&) {
        return "monostate";
    }
};

// 数字类型
template <class T>
    requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
struct ToString<T> {
    static std::string toString(const T& t) {
        if constexpr (std::is_floating_point_v<T>) {
            // 如果浮点数是整数（例如 26.0），则不显示小数部分
            if (std::floor(t) == t) {
                return std::format("{:.0f}", t);
            } else {
                return std::format("{}", t);
            }
        } else {
            return std::format("{}", t);
        }
    }
};

// C风格数组类型
template <typename T, std::size_t N>
struct ToString<T[N]> {
    static std::string toString(const T (&arr)[N]) {
        std::string res;
        bool once = false;
        for (const auto& it : arr) {
            if (once)
                res += ',';
            else
                once = true;
            res += ToString<std::decay_t<decltype(it)>>::toString(it);
        }
        res += ']';
        return res;
    }
};

// span视图
template <class T>
struct ToString<std::span<T>> {
    static std::string toString(std::span<T> t) {
        std::string res;
        res += '[';
        bool once = false;
        for (const auto& it : t) {
            if (once)
                res += ',';
            else
                once = true;
            res += ToString<std::decay_t<decltype(it)>>::toString(it);
        }
        res += ']';
        return res;
    }
};

// std::optional
template <typename... Ts>
struct ToString<std::optional<Ts...>> {
    static std::string toString(const std::optional<Ts...>& t) {
        std::string res;
        if (t.has_value())
            res += ToString<std::decay_t<decltype(*t)>>::toString(*t);
        else
            res += ToString<std::nullopt_t>::toString(std::nullopt);
        return res;
    }
};

// std::variant 现代共用体
template <typename... Ts>
struct ToString<std::variant<Ts...>> {
    static std::string toString(const std::variant<Ts...>& t) {
        return std::visit([] (const auto& v) -> std::string { // 访问者模式
            return ToString<std::decay_t<decltype(v)>>::toString(v);
        }, t);
    }
};

// std::pair
template <HX::STL::concepts::PairContainer Container>
struct ToString<Container> {
    static std::string toString(const Container& p) {
        std::string res;
        res += '(';
        res += ToString<std::decay_t<decltype(std::get<0>(p))>>::toString(std::get<0>(p));
        res += ',';
        res += ToString<std::decay_t<decltype(std::get<1>(p))>>::toString(std::get<1>(p));
        res += ')';
        return res;
    }
};

// std::的常见的支持迭代器的单元素容器
template <HX::STL::concepts::SingleElementContainer Container>
struct ToString<Container> {
    static std::string toString(const Container& sc) {
        std::string res;
        res += '[';
        bool once = false;
        for (const auto& it : sc) {
            if (once)
                res += ',';
            else
                once = true;
            res += ToString<std::decay_t<decltype(it)>>::toString(it);
        }
        res += ']';
        return res;
    }
};

// std::的常见的支持迭代器的键值对容器
template <HX::STL::concepts::KeyValueContainer Container>
struct ToString<Container> {
    static std::string toString(const Container& map) {
        std::string res;
        res += '{';
        bool once = false;
        for (const auto& [k, v] : map) {
            if (once)
                res += ',';
            else
                once = true;
            res += ToString<std::decay_t<decltype(k)>>::toString(k);
            res += ':';
            res += ToString<std::decay_t<decltype(v)>>::toString(v);
        }
        res += '}';
        return res;
    }
};

// str相关的类型
template <HX::STL::concepts::StringType ST>
struct ToString<ST> {
    static std::string toString(const ST& t) {
        std::string res;
        res += '"';
        res += t;
        res += '"';
        return res;
    }
};

// C风格字符串
template <typename T, std::size_t N>
    requires (std::same_as<T, char>)
struct ToString<T[N]> {
    static std::string toString(const T (&str)[N]) {
        return std::string {str, N - 1}; // - 1 是为了去掉 '\0'
    }
};

// 鸭子类型: 存在`toString`成员函数
template <ToStringClassType T>
struct ToString<T> {
    static std::string toString(const T& t) {
        return t.toString();
    }
};

// 鸭子类型: 存在`toJson`成员函数
template <ToJsonClassType T>
struct ToString<T> {
    static std::string toString(const T& t) {
        return t.toJson();
    }
};

// 递归打印tuple
template <std::size_t I = 0, typename... Ts>
std::string tupleToString(const std::tuple<Ts...>& tup) {
    std::string res;
    if constexpr (I == sizeof...(Ts)) { // 因为 I 从 0 开始
        return res;
    } else {
        if constexpr (I > 0)
            res += ',';
        res += ToString<std::decay_t<decltype(std::get<I>)>>::toString(std::get<I>(tup));
        res += tupleToString<I + 1>(tup);
        return res;
    }
}

template <typename... Ts>
struct ToString<std::tuple<Ts...>> {
    static std::string toString(const std::tuple<Ts...>& tup) {
        std::string res;
        res += '(';
        res += tupleToString(tup);
        res += ')';
        return res;
    }
};

} // namespace internal

/**
 * @brief toString 紧凑的
 */
template <typename T0, typename ...Ts>
static std::string toString(T0 const& t0, Ts const&... ts) {
    std::string res;
    res += internal::ToString<T0>::toString(t0);
    ((res += internal::ToString<Ts>::toString(ts)), ...);
    return res;
}

}}} // namespace HX::STL::utils

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

#endif // !_HX_TO_STRING_H_