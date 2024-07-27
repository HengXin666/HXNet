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
#ifndef _HX_HXSTRING_TOOLS_H_
#define _HX_HXSTRING_TOOLS_H_

#include <vector>
#include <string>
#include <string_view>

namespace HXSTL {

/**
 * @brief 字符串操作工具类
 */
struct HXStringUtil final {
    /**
     * @brief 将字符串按`delimiter`分割为数组
     * @param str 需要分割的字符串
     * @param delimiter 分割字符
     * @param res 待返回数组, 你可以事先在前面插入元素
     * @return 分割后的数组
     */
    static std::vector<std::string> split(std::string_view str, std::string_view delim, std::vector<std::string> res = {});

    /**
     * @brief 将字符串按从左到右第一个`delimiter`分割为两半
     * @param str 需要分割的字符串
     * @param delimiter 分割字符
     * @return 分割后的数组, 失败返回: `{"", ""}`
     */
    static std::pair<std::string, std::string> splitAtFirst(std::string_view str, std::string_view delim);

    /**
     * @brief 将字符串转为小写字母
     * @param str [in, out] 待处理字符串
     */
    static void toSmallLetter(std::string& str) {
        std::size_t n = str.size();
        for (std::size_t i = 0; i < n; ++i)
            if ('A' <= str[i] && str[i] <= 'Z')
                str[i] ^= ' ';
    }

    /**
     * @brief 从右到左查找字符并截取后面的子字符串
     * @param 目标字符串
     * @param 模版字符串
     * @return 截取后面的子字符串, 如果没有则返回: 空字符串`""`
     */
    static std::string rfindAndTrim(std::string_view  str, std::string_view delim) {
        std::size_t pos = str.rfind(delim);
        if (pos != std::string::npos) {
            return std::string {str.substr(pos + 1)};
        }
        return ""; // 如果未找到分隔符，返回原字符串
    }
};

/**
 * @brief 时间格式工具类
 */
struct HXDateTimeFormat final {
    /**
     * @brief 格式化当前时间为如: `%Y-%m-%d %H:%M:%S`的格式
     * @param fmt 格式字符串
     * @return 当前时间格式化都的字符串
     */
    static std::string format(const std::string& fmt = "%Y-%m-%d %H:%M:%S");

    /**
     * @brief 格式化当前时间为如: `%Y-%m-%d %H:%M:%S`的格式, 并且带毫秒时间获取
     * @param fmt 格式字符串
     * @param msDelim 毫秒与前部分的分割符，默认是空格
     * @return 当前时间格式化都的字符串
     */
    static std::string formatWithMilli(const std::string& fmt = "%Y-%m-%d %H:%M:%S", const std::string msDelim = " ");
};

} // namespace HXSTL

#endif // _HX_HXSTRING_TOOLS_H_
