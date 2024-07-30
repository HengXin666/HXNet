#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-23 16:28:50
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
#ifndef _HX_HXERROR_HANDLING_TOOLS_H_
#define _HX_HXERROR_HANDLING_TOOLS_H_

#include <cerrno>
#include <system_error>
#include <type_traits>
#include <netdb.h>

#include <HXprint/HXprint.h>

namespace HX { namespace STL { namespace tools {

/**
 * @brief 错误处理工具类
 */
struct ErrorHandlingTools {

    /**
     * @brief 安全的错误码模版类
     */
    template <class T>
    struct Expected {
        // @brief 保证是 T 的有符号类型
        std::make_signed_t<T> _res;

        Expected() = default;
        Expected(std::make_signed_t<T> res) noexcept : _res(res) {}

        /**
         * @brief 返回错误码
         * @return 自身, 如果没有错误则返回`0`
         */
        int error() const noexcept {
            if (_res < 0) {
                return -_res;
            }
            return 0;
        }

        /**
         * @brief 判断自身是否为`err`错误码
         * @param err 需要判断的错误码
         * @return `自身 == 错误码`
         */
        bool isError(int err) const noexcept {
            return _res == -err;
        }

        /**
         * @brief 获取转换为系统分类后的错误码
         */
        std::error_code errorCode() const noexcept {
            if (_res < 0) {
                return std::error_code(-_res, std::system_category());
            }
            return std::error_code();
        }

        /**
         * @brief 获取自身值
         * @param what 错误提示
         * @throw 如果自身是错误码, 那么会抛出异常
         */
        T expect(const char *what) const {
            if (_res < 0) {
                auto ec = errorCode();
                LOG_ERROR("%s: %s (%d)", what, ec.message().c_str(), _res);
                throw std::system_error(ec, what);
            }
            return _res;
        }

        /**
         * @brief 获取自身值
         * @throw 如果自身是错误码, 那么会抛出异常
         */
        T value() const {
            if (_res < 0) {
                auto ec = errorCode();
                LOG_ERROR("%s", ec.message().c_str());
                throw std::system_error(ec);
            }
            return _res;
        }

        T valueUnsafe() const {
            assert(_res >= 0);
            return _res;
        }
    };

private:
    /**
     * @brief 获取错误原因, 并且打印后抛出异常
     * @param what 错误提示
     */
    [[noreturn]] static void _throwSystemError(const char *what);

public:
    /**
     * @brief 判断是否是错误: 专门伺候Linux的系统函数(即返回`-1`代表错误, 而错误原因在`erron`)
     * @param what 错误提示 (如: 传入`"HX"`, 那么到时候错误则会打印`HX: ErrorStr...`)
     * @param res 需要检查的可能的错误码
     * @tparam Except 需要排除的错误码
     * @return 当没有`Except`时只是转发`res`, 如果`erron == Except`则会返回`-1`而不会终止程序
     */
    template <int Except = 0, class T>
    static T checkError(const char *what, T res) {
        if (res == -1) {
            if constexpr (Except != 0) {
                if (errno == Except) {
                    return -1;
                }
            }
            _throwSystemError(what);
        }
        return res;
    }

    /**
     * @brief 判断`res`是否是错误, 是则返回对应`错误码的负数`, 使用`if (convertError() < 0)`捕获错误并处理
     * @param res 需要检查的可能的错误码
     * @return 
     */
    template <class U, class T>
    static Expected<U> convertError(T res) {
        if (res == -1) {
            return -errno;
        }
        return res;
    }

    /**
     * @brief 获取错误码: 为了将`getaddrinfo`的错误与标准库的错误处理机制结合起来, 使得处理网络错误更为方便和一致
     */
    static const std::error_category& gaiCategory() {
        static struct final : std::error_category {
            char const *name() const noexcept override {
                return "getaddrinfo";
            }

            std::string message(int err) const override {
                return gai_strerror(err);
            }
        } instance;
        return instance; // 使用: std::error_code(err, gai_category())
    }

};

#define SOURCE_INFO_IMPL_2(file, line) "In " file ":" #line ": "
#define SOURCE_INFO_IMPL(file, line) SOURCE_INFO_IMPL_2(file, line)
#define SOURCE_INFO(...) SOURCE_INFO_IMPL(__FILE__, __LINE__) __VA_ARGS__

/**
 * @brief 判断是否是错误: 专门伺候Linux的系统函数(即返回`-1`代表错误, 而错误原因在`erron`)
 * @package except 需要排除的错误码
 * @param func 错误提示 和 执行的函数名称(func)
 * @param ... func 的参数
 * @return 报错就抛出异常, 如果`erron == Except`则会返回`-1`而不会终止程序, 否则只是转发`func`的`返回值`
 */
#define CHECK_CALL_EXCEPT(except, func, ...) HX::STL::tools::ErrorHandlingTools::checkError<except>(SOURCE_INFO() #func, func(__VA_ARGS__))

/**
 * @brief 判断是否是错误: 专门伺候Linux的系统函数(即返回`-1`代表错误, 而错误原因在`erron`)
 * @param func 错误提示 和 执行的函数名称(func)
 * @param ... func 的参数
 * @return 报错就抛出异常, 否则只是转发`func`的`返回值`
 */
#define CHECK_CALL(func, ...) HX::STL::tools::ErrorHandlingTools::checkError(SOURCE_INFO(#func), func(__VA_ARGS__))

}}} // namespace HX::STL::tools

#endif // _HX_HXERROR_HANDLING_TOOLS_H_