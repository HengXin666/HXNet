#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-08-15 22:00:38
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
#ifndef _HX_NON_VOID_HELPER_H_
#define _HX_NON_VOID_HELPER_H_

namespace HX { namespace STL { namespace container {

/**
 * @brief 擦除void类型的辅助类 (主模版)
 * @tparam T 
 */
template <class T = void>
struct NonVoidHelper {
    using Type = T;
};

/**
 * @brief 擦除void类型的辅助类 (偏特化模版)
 * @tparam void
 */
template <>
struct NonVoidHelper<void> {
    using Type = NonVoidHelper;

    explicit NonVoidHelper() = default;
};

}}} // namespace HX::STL::container

#endif // !_HX_NON_VOID_HELPER_H_