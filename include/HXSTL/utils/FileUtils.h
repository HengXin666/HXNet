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

#include <fcntl.h>
#include <string>
#include <string_view>

#include <HXSTL/coroutine/task/Task.hpp>

namespace HX { namespace STL { namespace utils {

/**
 * @brief 文件操作类
 */
struct FileUtils {
private:
#if IO_URING_DIRECT
    // O_DIRECT 可以用来减少操作系统内存复制的开销 (但需要注意可能的对齐要求)
    static constexpr int kOpenModeDefaultFlags = O_LARGEFILE | O_CLOEXEC | O_DIRECT;
#else
    // O_LARGEFILE 允许文件大小超过 2GB
    // O_CLOEXEC 确保在执行新程序时, 文件描述符不会继承到子进程
    static constexpr int kOpenModeDefaultFlags = O_LARGEFILE | O_CLOEXEC;
#endif
public:
    enum class OpenMode : int {
        Read = O_RDONLY | kOpenModeDefaultFlags,                        // 只读模式 (r)
        Write = O_WRONLY | O_TRUNC | O_CREAT | kOpenModeDefaultFlags,   // 只写模式 (w)
        ReadWrite = O_RDWR | O_CREAT | kOpenModeDefaultFlags,           // 读写模式 (a+)
        Append = O_WRONLY | O_APPEND | O_CREAT | kOpenModeDefaultFlags, // 追加模式 (w+)
        Directory = O_RDONLY | O_DIRECTORY | kOpenModeDefaultFlags,     // 目录
    };

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
     * @param flags 打开方式: OpenMode (枚举 如: OpenMode::Read | OpenMode::Append)
     * @param mode 文件权限模式, 仅在文件创建时有效 (一般写0644)
     * @return std::string 文件内容
     */
    static HX::STL::coroutine::task::Task<std::string> asyncGetFileContent(
        const std::string& path,
        OpenMode flags = OpenMode::Read,
        mode_t mode = 0644
    );

    /**
     * @brief [异步的]向文件写入内容
     * @param path 文件路径
     * @param content 需要写入的数据
     * @param flags 打开方式: OpenMode (枚举 如: OpenMode::Write | OpenMode::Append)
     * @param mode 文件权限模式, 仅在文件创建时有效 (一般写0644)
     * @return std::string 文件内容
     */
    static HX::STL::coroutine::task::Task<int> asyncPutFileContent(
        const std::string& path,
        std::string_view content,
        OpenMode flags,
        mode_t mode = 0644
    );
};

/**
 * @brief OpenMode 位运算符重载 (目前有意义的只有 Read | Append (r+))
 * @warning 几乎用不到这个! 如果使用了请看看`FileUtils::OpenMode`是什么东西先!
 */
constexpr FileUtils::OpenMode operator|(FileUtils::OpenMode lhs, FileUtils::OpenMode rhs) {
    return static_cast<FileUtils::OpenMode>(
        static_cast<int>(lhs) |
        static_cast<int>(rhs)
        // static_cast<std::underlying_type<FileUtils::OpenMode>::type>(lhs) |
        // static_cast<std::underlying_type<FileUtils::OpenMode>::type>(rhs)
    );
}

}}} // namespace HX::STL::utils

#endif // _HX_FILE_UTILS_H_