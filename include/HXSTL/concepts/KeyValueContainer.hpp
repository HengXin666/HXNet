#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-09-07 15:57:59
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
#ifndef _HX_KEY_VALUE_CONTAINER_H_
#define _HX_KEY_VALUE_CONTAINER_H_

namespace HX { namespace STL { namespace concepts {

// 概念: 判断类型 T 是否是键值对的关联容器
template <typename T>
concept KeyValueContainer = requires(T t) {
    typename T::key_type;
    typename T::mapped_type;
};

}}} // HX::STL::concepts

#endif // !_HX_KEY_VALUE_CONTAINER_H_