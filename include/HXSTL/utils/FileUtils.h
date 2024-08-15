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

#include <string>
#include <string_view>

#include <HXSTL/coroutine/task/Task.hpp>

namespace HX { namespace STL { namespace utils {

/**
 * @brief 文件操作类
 */
struct FileUtils {
    /**
     * @brief [同步的]读取文件内容
     * @param path 文件路径
     * @return std::string 文件内容
     */
    static std::string getFileContent(const std::string& path);

    /**
     * @brief [同步的]向文件写入数据
     * @param path 文件路径
     * @param content 需要写入的数据
     */
    static void putFileContent(const std::string& path, std::string_view content);

    /**
     * @brief [异步的]读取文件内容
     * @param path 文件路径
     * @return std::string 文件内容
     */
    static HX::STL::coroutine::task::Task<std::string> asyncGetFileContent(
        const std::string& path
    );
};

}}} // namespace HX::STL::utils

#endif // _HX_FILE_UTILS_H_