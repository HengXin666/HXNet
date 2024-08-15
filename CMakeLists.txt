cmake_minimum_required(VERSION 3.20.0)
# 定义工程名称, 并指定语言为C++
project(HXNet LANGUAGES CXX)

# 是否开启 绕过操作系统的缓存, 直接将数据从用户空间读写到磁盘
# set(IO_URING_DIRECT ON)

# 如果没有定义CMAKE_BUILD_TYPE，则设置为Release
# 可以使用 cmake -DCMAKE_BUILD_TYPE=Debug .. 来指定
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message("=-=-=-=-=-=-=当前是 [Release] 模式=-=-=-=-=-=-=")
else()
    message("=-=-=-=-=-=-=当前是 [DEBUG] 模式=-=-=-=-=-=-=")
    add_definitions(-D_HX_DEBUG_) # 添加一个 _HX_DEBUG_ 宏
    message("已添加宏: _HX_DEBUG_")
endif()

set(CMAKE_CXX_STANDARD 20) # 设置C++标准为C++20
set(CMAKE_C_STANDARD 11)   # 设置C语言标准为C11
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 指定C++标准是必需的
set(CMAKE_CXX_EXTENSIONS OFF) # 禁用编译器的扩展
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines") # 显示启动C++协程

# 设置CMake模块路径, 包含当前目录下的cmake文件夹以及之前的路径
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake;${CMAKE_MODULE_PATH}")

add_compile_options(-Wall -Wextra -Werror=return-type)

# 定义生成的库的存放目录
set(LIB_PATH ${PROJECT_SOURCE_DIR}/lib)

set(ROOT_PATH "${PROJECT_SOURCE_DIR}") # 设置根项目的根目录
# 在camke .. 的时候会输出提示目录路径 (可选)
message (STATUS "Prefix dir is ${CMAKE_INSTALL_PREFIX}")
message (STATUS "Binary dir is ${PROJECT_BINARY_DIR}")
message (STATUS "Source dir is ${PROJECT_SOURCE_DIR}")
message (STATUS "ROOT_PATH dir is ${ROOT_PATH}")

# 设置文件名称
set (appName HXNet)

# 递归查找
file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)

# 编译可执行文件
add_executable (${appName} ${srcs})

if (IO_URING_DIRECT)
    target_compile_definitions(HXNet PUBLIC IO_URING_DIRECT)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
if (HX_NET_FIND_LIBURING)
    include(FindPkgConfig)
    pkg_check_modules(LIBURING liburing)
        if (NOT LIBURING_FOUND)
            find_path(LIBURING_INCLUDE_DIR NAMES liburing.h)
            find_library(LIBURING_LIBRARY NAMES liburing.a)
            if (NOT LIBURING_INCLUDE_DIR OR NOT LIBURING_LIBRARY)
                message(FATAL_ERROR "liburing not found")
            endif()
            set(LIBURING_LIBRARIES ${LIBURING_LIBRARY})
            set(LIBURING_INCLUDE_DIRS ${LIBURING_INCLUDE_DIR})
        endif()
        target_link_libraries(HXNet PUBLIC ${LIBURING_LIBRARIES})
        target_include_directories(HXNet PUBLIC ${LIBURING_INCLUDE_DIRS})
    else()
        target_sources(HXNet PRIVATE lib/liburing/liburing.cpp)
        target_include_directories(HXNet PUBLIC lib/liburing/include)
    endif()
endif()

# 设置头文件路径
include_directories(${ROOT_PATH}/include)

# 将include目录添加到HXHttp库的公共包含目录中
target_include_directories(HXNet PUBLIC include)