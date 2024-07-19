#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-19 13:49:22
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
#ifndef _HX_HXHTTP_TOOLS_H_
#define _HX_HXHTTP_TOOLS_H_

#include <vector>
#include <string>
#include <string_view>

namespace HXHttp {

/**
 * @brief 字符串操作工具类
 */
struct HXStringUtil final {
    /**
     * @brief 将字符串按`delimiter`分割为数组
     * @param str 需要分割的字符串
     * @param delimiter 分割字符
     * @return 分割后的数组
     */
    static std::vector<std::string> split(std::string_view str, std::string_view delim);

    /**
     * @brief 将字符串按从左到右第一个`delimiter`分割为两半
     * @param str 需要分割的字符串
     * @param delimiter 分割字符
     * @return 分割后的数组
     */
    static std::pair<std::string, std::string> splitAtFirst(std::string_view str, std::string_view delim);

};

} // namespace HXHttp

#endif // _HX_HXHTTP_TOOLS_H_
