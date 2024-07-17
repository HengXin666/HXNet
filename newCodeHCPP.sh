#!/bin/bash

# 使用: sh ./newCodeHCPP 项目名称

# 介绍: 会安装你的输入, 在 [newCodeDir] 创建的目录的基础下, 创建 .cpp 和 .h

# 定义一个函数来输出带颜色、下划线和闪烁效果的文本
print_colored_text() {
    local color=$1
    local text=$2
    local style="\033[${color}m"
    local reset="\033[0m"
    echo -e "${style}${text}${reset}"
}

# 定义一个函数来输出带时间和内容的固定格式
print_formatted_message() {
    local time_style=$1
    local content_style=$2
    local message=$3
    local nowTime=$(date "+%Y-%-m-%-d %H:%M:%S")
    
    local time_text="\033[${time_style}m[${nowTime}]\033[0m"
    local content_text="\033[${content_style}m${message}\033[0m"

    echo -e "${time_text}: ${content_text}"
}

# 定义颜色代码
RED="31"
GREEN="32"
YELLOW="33"
BLUE="34"
MAGENTA="35"
CYAN="36"
WHITE="37"
UNDERLINE="4"
BLINK="5"

# 检查是否提供了名称参数
if [ $# -eq 0 ]; then
    print_formatted_message "${RED}" "${YELLOW}" "请提供一个名称作为项目名称, 如果使用下划线, 则会以驼峰命名法命名类!"
    exit 1
fi

name=$1
nowTime=$(date "+%Y-%-m-%-d %H:%M:%S")

read -p "请输入.cpp/.h的名称: " CppFile

# 检查并转换为驼峰命名并确保首字母大写
if [[ "$CppFile" =~ ^[a-zA-Z][a-zA-Z0-9]*$ ]]; then
    # 如果名称符合驼峰命名规则，则不做变换
    camelCaseName="$CppFile"
else
    # 否则，假设名称是下划线分隔的，进行转换
    camelCaseName=$(echo "$CppFile" | sed -r 's/(^|_)([a-zA-Z])/\U\2/g')
fi

# 创建.h文件
cat > ./$name/include/$name/$CppFile.h <<EOL
#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: $nowTime
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
#ifndef _HX_${CppFile^^}_H_
#define _HX_${CppFile^^}_H_

namespace $name {

class $camelCaseName {

};

} // namespace $name

#endif // _HX_${CppFile^^}_H_
EOL

# 创建.cpp文件
cat > ./$name/src/$name/$CppFile.cpp <<EOL
#include <$name/$CppFile.h>

namespace $name {

} // namespace $name
EOL

print_formatted_message "${GREEN}" "${YELLOW}" "项目 $name 中:"
print_formatted_message "${GREEN}" "${BLUE};${UNDERLINE}" "$CppFile.cpp 和 $CppFile.h 已创建!"