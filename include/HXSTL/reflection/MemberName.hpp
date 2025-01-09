#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-01-09 19:16:01
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
#ifndef _HX_MEMBER_NAME_H_
#define _HX_MEMBER_NAME_H_

#include <string_view>
#include <array>
#include <utility>

#include <HXSTL/reflection/MemberCount.hpp>

namespace HX { namespace STL { namespace reflection {

namespace internal {

// 包裹一层`wrap`的原因：non-type template parameters of scalar type是在clang18才开始的，
// 而Class types as non-type template parameters是在clang12就支持了

/**
 * @brief 获取编译期成员变量指针的符号, 并且提取其成员名称
 * @tparam ptr 编译期成员变量指针
 * @return constexpr std::string_view 
 */
template <auto ptr>
inline constexpr std::string_view getMemberName() {
#if defined(_MSC_VER)
    constexpr std::string_view func_name = __FUNCSIG__;
#else
    constexpr std::string_view func_name = __PRETTY_FUNCTION__;
#endif

#if defined(__clang__)
    auto split = func_name.substr(0, func_name.size() - 1);
    return split.substr(split.find_last_of(":.") + 1);
#elif defined(__GNUC__)
    auto split = func_name.substr(0, func_name.rfind(");"));
    return split.substr(split.find_last_of(":") + 1);
#elif defined(_MSC_VER)
    auto split = func_name.substr(0, func_name.rfind(">"));
    return split.substr(split.rfind("->") + 2);
#else
    static_assert(
        false, "You are using an unsupported compiler. Please use GCC, Clang "
               "or MSVC or switch to the rfl::Field-syntax.");
#endif
}

template <typename T>
struct StaticObj {
    inline static std::decay_t<T> obj;
};

template <typename T>
inline constexpr std::decay_t<T>& getStaticObj() {
    return StaticObj<T>::obj;
}

template <typename T, std::size_t N>
struct ReflectionVisitor {
    static constexpr auto visit() {
        static_assert(
            false, ""); // 不支持实例化主模版
    }
};

template <typename T> 
struct ReflectionVisitor<T, 0> {
    static constexpr auto visit() {
        return std::tuple<>{};
    }
};

#define _HX_GENERATE_TEMPLATES_WITH_SPECIALIZATION_(N, ...) \
template <typename T>                                       \
struct ReflectionVisitor<T, N> {                            \
    static constexpr auto visit() {                         \
        auto&& obj = internal::getStaticObj<T>();           \
        auto&& [__VA_ARGS__] = obj;                         \
        auto&& t = std::tie(__VA_ARGS__);                   \
        auto&& f = [](auto&... fs) {                        \
            return std::make_tuple(&(fs)...);               \
        };                                                  \
        return std::apply(f, t);                            \
    }                                                       \
};

#include <HXSTL/reflection/MemberMacro.hpp>

template <typename T>
inline constexpr auto getStaticObjPtrTuple() {
    return ReflectionVisitor<std::decay_t<T>, membersCountVal<T>>::visit();
}

} // namespace internal

template <typename T>
inline constexpr std::array<std::string_view, membersCountVal<T>> getMembersNames() {
    constexpr auto Cnt = membersCountVal<T>;
    std::array<std::string_view, Cnt> arr;
    constexpr auto tp = internal::getStaticObjPtrTuple<T>(); // 获取 tuple<成员指针...>
    [&] <std::size_t... Ts> (std::index_sequence<Ts...>) {
        ((arr[Ts] = internal::getMemberName<std::get<Ts>(tp)>()), ...);
    } (std::make_index_sequence<Cnt>());
    return arr;
}

}}} // namespace HX::STL::reflection

#endif // !_HX_MEMBER_NAME_H_