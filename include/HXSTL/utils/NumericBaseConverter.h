#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-09-06 23:48:16
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
#ifndef _HX_NUMERIC_BASE_CONVERTER_H_
#define _HX_NUMERIC_BASE_CONVERTER_H_

#include <string>

namespace HX { namespace STL { namespace utils {

/**
 * @brief 进制转化工具类
 */
struct NumericBaseConverter {
    /**
     * @brief 从十进制`num`转化为十六进制字符串
     * @param num 十进制 (非负数)
     * @return std::string 十六进制
     */
    static std::string hexadecimalConversion(std::size_t num) {
        static std::string str = "0123456789ABCDEF";
        std::string res;
        do {
            res += str[num % 16];
            num /= 16;
        } while (num);
        return {res.rbegin(), res.rend()};
    }
};

}}} // namespace HX::STL::utils

#endif // !_HX_NUMERIC_BASE_CONVERTER_H_