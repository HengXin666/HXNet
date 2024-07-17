# 设置CMake的最低版本要求为3.18
cmake_minimum_required(VERSION 3.18)
# 定义工程名称, 并指定语言为C++
project(HXNet LANGUAGES CXX)

# 如果没有定义CMAKE_BUILD_TYPE，则设置为Release
# 可以使用 cmake -DCMAKE_BUILD_TYPE=Debug .. 来指定
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 20) # 设置C++标准为C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 指定C++标准是必需的
set(CMAKE_CXX_EXTENSIONS OFF) # 禁用编译器的扩展

# 设置CMake模块路径, 包含当前目录下的cmake文件夹以及之前的路径
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake;${CMAKE_MODULE_PATH}")


# 定义生成的库的存放目录
set(LIB_PATH ${PROJECT_SOURCE_DIR}/lib)

set(ROOT_PATH "${PROJECT_SOURCE_DIR}") # 设置根项目的根目录
# 在camke .. 的时候会输出提示目录路径 (可选)
message (STATUS "Prefix dir is ${CMAKE_INSTALL_PREFIX}")
message (STATUS "Binary dir is ${PROJECT_BINARY_DIR}")
message (STATUS "Source dir is ${PROJECT_SOURCE_DIR}")
message (STATUS "ROOT_PATH dir is ${ROOT_PATH}")

# 包含自定义的CMake模块MyUsefulFuncs
# include(MyUsefulFuncs)

# ===添加子目录===
add_subdirectory(HXJson)
add_subdirectory(HXprint)
add_subdirectory(HXCodeTest)
add_subdirectory(HXHttp)