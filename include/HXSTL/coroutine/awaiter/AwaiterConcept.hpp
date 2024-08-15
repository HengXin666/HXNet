#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-15 21:54:36
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
#ifndef _HX_AWAITER_CONCEPT_H_
#define _HX_AWAITER_CONCEPT_H_

#include <coroutine>

#include <HXSTL/container/NonVoidHelper.hpp>

namespace HX { namespace STL { namespace coroutine { namespace awaiter {

/**
 * @brief Awaiter概念: 含有这三个方法的类(await_ready/await_suspend/await_resume)
 * @tparam A 
 */
template <class A>
concept Awaiter = requires(A a, std::coroutine_handle<> h) {
    { a.await_ready() };
    { a.await_suspend(h) };
    { a.await_resume() };
};

/**
 * @brief Awaitable概念: 是Awaiter, 并且可以被co_await
 * @tparam A 
 */
template <class A>
concept Awaitable = Awaiter<A> || requires(A a) {
    { a.operator co_await() } -> Awaiter;
};

/**
 * @brief Awaitable特性 (主模版(声明))
 * @tparam A 
 */
template <class A> 
struct AwaitableTraits;

/**
 * @brief Awaitable特性 (模版特化)
 * @tparam A 得是 Awaiter
 */
template <Awaiter A> 
struct AwaitableTraits<A> {
    /// @brief 返回值类型
    using RetType = decltype(std::declval<A>().await_resume());

    /// @brief 擦除void类型的返回值类型
    using NonVoidRetType = HX::STL::container::NonVoidHelper<RetType>::Type;
};

/**
 * @brief Awaitable特性 (条件特化版本), 它在`A`不满足`Awaiter`概念但满足`Awaitable`概念时生效;
 * 其继承 co_await 返回值类型 Awaiter, 以提取 co_await 的返回值类型
 * @tparam A 
 */
template <class A>
    requires(!Awaiter<A> && Awaitable<A>)
struct AwaitableTraits<A>
    : AwaitableTraits<decltype(std::declval<A>().operator co_await())> 
{};

}}}} // namespace HX::STL::coroutine::awaiter

#endif // !_HX_AWAITER_CONCEPT_H_