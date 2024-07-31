#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-31 14:36:36
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
#ifndef _HX_FILE_UTILS_H_
#define _HX_FILE_UTILS_H_

#include <fstream>
#include <string>
#include <string_view>
#include <algorithm>

namespace HX { namespace STL { namespace utils {

/**
 * @brief 文件操作类
 */
struct FileUtils {
    static std::string getFileContent(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::system_error(errno, std::generic_category());
        }
        return std::string {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    }

    static void putFileContent(const std::string& path, std::string_view content) {
        std::ofstream file(path);
        if (!file.is_open()) {
            throw std::system_error(errno, std::generic_category());
        }
        std::copy(content.begin(), content.end(), std::ostreambuf_iterator<char>(file));
    }
};

}}} // namespace HX::STL::utils

#endif // _HX_FILE_UTILS_H_