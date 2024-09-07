#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-09-07 15:54:36
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
#ifndef _HX_STRING_TYPE_H_
#define _HX_STRING_TYPE_H_

#include <type_traits>
#include <string>
#include <string_view>

namespace HX { namespace STL { namespace concepts {

// 概念: 如果这个类型和str沾边, 那么使用""包裹, 注: 普通的const char * 是不会包裹的qwq
template <typename T>
concept StringType = std::is_same_v<T, std::string> ||
                     std::is_same_v<T, std::string_view>;

// 概念: 如果这个类型和wstr沾边, 那么使用""包裹 [不支持...]
// template <typename T>
// concept WStringType = std::is_same_v<T, const wchar_t *> ||
//                      std::is_same_v<T, std::wstring_view> ||
//                      std::is_same_v<T, std::wstring>;

}}} // HX::STL::concepts

#endif // !_HX_STRING_TYPE_H_