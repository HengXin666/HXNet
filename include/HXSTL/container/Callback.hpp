#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-23 17:20:45
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
#ifndef _HX_CALLBACK_H_
#define _HX_CALLBACK_H_

#include <memory>
#include <cassert>

namespace HX { namespace STL { namespace container {

// 这个`Callback`结构体模板是一种用于存储和调用可变参数回调函数的类模板
// 它主要通过类型擦除和多态来实现这一点

inline constexpr struct multishot_call_t {
    explicit multishot_call_t() = default;
} multishot_call;

/**
 * @brief 回调函数
 */
template <class ...Args>
struct Callback {

    // 基础回调基类
    struct _CallbackBase {
        // 定义一个通用的`_call` 方法, 所有具体的回调实现都会继承自这个基类, 并实现`_call`方法
        virtual void _call(Args... args) = 0;
        virtual ~_CallbackBase() = default;
    };

    // 具体回调实现类模板
    template <class F>
    struct _CallbackImpl final : _CallbackBase {
        F _func;

        template <class ...Ts, 
                  class = std::enable_if_t<std::is_constructible_v<F, Ts...>>>
        _CallbackImpl(Ts &&...ts) : _func(std::forward<Ts>(ts)...) 
        {}

        // 这个类模板继承`_HXCallbackBase`并实现了`_call`方法
        // 它持有一个具体的函数对象`m_func`, 并在`_call`方法中调用该函数对象
        void _call(Args... args) override {
            _func(std::forward<Args>(args)...);
        }
    };

    // 基础回调基类 独享智能指针, 用于持有具体的回调实现
    std::unique_ptr<_CallbackBase> _base;

    // 构造函数
    // 通过模板参数和`std::enable_if`限制, 确保传入的函数对象是可调用的, 并且不是`callback`本身的类型
    // 这个构造函数会创建一个`_CallbackImpl`实例, 并将其存储在`_base`中
    template <class F, 
              class = std::enable_if_t<
                    std::is_invocable_v<F, Args...> && 
                    !std::is_same_v<std::decay_t<F>, Callback>>>
    Callback(F &&f) 
    : _base(std::make_unique<_CallbackImpl<std::decay_t<F>>>(std::forward<F>(f))) 
    {}

/**
 * 注: std::is_invocable<F, Args...> 是一个类型特征模板, 
 * 它会返回一个布尔值（std::true_type 或 std::false_type）, 表示可调用对象 F 是否能够接受参数 Args... 并成功调用
 * std::is_invocable_v<F, Args...> 是 std::is_invocable<F, Args...>::value 的简写, 直接提供布尔值
 * 
 * std::decay_t<F> 是 C++11 中引入的类型特征, 用于获取类型 F 的衍生类型（decayed type）
 * 它会去掉类型的引用、const、volatile 修饰符, 并将数组和函数类型转换为对应的指针类型
 * 例如, std::decay_t<int&> 会是 int, std::decay_t<int[10]> 会是 int*
 * 你可以通过 std::decay 来移除类型的引用和常量修饰符, 并进行其他类型调整
 * 此处, 保证 F 和 callback 不是相同类型及其衍生
 */

    Callback() = default;

    // 允许 = nullptr 作移动赋值构造
    Callback(std::nullptr_t) noexcept {}

    // 不可拷贝
    Callback(Callback const &) = delete;
    Callback &operator=(Callback const &) = delete;

    // 可以移动
    Callback(Callback &&) = default;
    Callback &operator=(Callback &&) = default;

    // @brief 调用存储的回调函数
    void operator()(Args... args) {
        assert(_base);
        _base->_call(std::forward<Args>(args)...);
        _base = nullptr;
    }

    void operator()(multishot_call_t, Args... args) {
        assert(_base);
        _base->_call(std::forward<Args>(args)...);
    }

    // @brief 获取存储的具体回调实现对象
    template <class F>
    F &target() const {
        assert(_base);
        return static_cast<_CallbackImpl<F> &>(*_base);
    }

    // @brief 获取其地址(不取消对指针的托管)
    void *getAddress() {
        return static_cast<void *>(_base.get());
    }

    // @brief 主动内存泄漏(取消对指针的托管)
    void *leakAddress() {
        return static_cast<void *>(_base.release());
    }

    // @brief 恢复回调对象的地址
    static Callback fromAddress(void *addr) {
        Callback cb;
        cb._base = std::unique_ptr<_CallbackBase>(
            static_cast<_CallbackBase *>(addr)
        );
        return cb;
    }

    explicit operator bool() const noexcept {
        return _base != nullptr;
    }
};

}}} // namespace HX::STL::container

#endif // _HX_CALLBACK_H_
