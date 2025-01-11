cmake_minimum_required(VERSION 3.20.0)
# 定义工程名称, 并指定语言为C++
project(HXNet
    VERSION 0.0.1
    DESCRIPTION "HXNet"
    HOMEPAGE_URL "https://github.com/HengXin666/HXNet"
    LANGUAGES CXX
)

# 是否开启 绕过操作系统的缓存, 直接将数据从用户空间读写到磁盘
# set(IO_URING_DIRECT ON) # 有BUG 先不要用

# 启用 搜集客户端地址信息 宏
add_definitions(-DCLIENT_ADDRESS_LOGGING)

# 是否系统已经安装了OpenSSL
set(HX_NET_FIND_OPENSSL ON)

# 改用 clang 编译器
# set(CMAKE_C_COMPILER clang)
# set(CMAKE_CXX_COMPILER clang++)

#####        ########
# 请选择编译的示例代码#
########       ######
# 部件测试 
# add_definitions(-DTEXT_MAIN_MAIN)  # 文件: TestMain.cpp
# add_definitions(-DJSON_TEST_MAIN)  # json示例
# add_definitions(-DIO_URING_OSTREAM_MAIN)  # 测试自己实现的输出流, 使用协程!
# add_definitions(-DCACHE_TEST_MAIN) # 测试LRU缓存
add_definitions(-D_HX_REFLECTION_TEST_H_) # 测试反射

# 客户端
# add_definitions(-DCLIENT_MAIN)  # 客户端

# 服务端
# add_definitions(-DCOMPILE_CHAT_SERVER_MAIN)  # 聊天室服务端
# add_definitions(-DCOMPILE_CHAT_SERVER_BY_JSON_MAIN)  # 聊天室服务端(使用HX/Json反射实现)
# add_definitions(-DCOMPILE_WEB_SOCKET_SERVER_MAIN)  # websocket服务端
# add_definitions(-DHTTPS_SERVER_MAIN)  # https服务端
# add_definitions(-DHTTPS_FILE_SERVER_MAIN)  # https简单的文件服务器

# 如果没有定义CMAKE_BUILD_TYPE, 则设置为Release
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
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines") # 显式启动C++协程

# 设置CMake模块路径, 包含当前目录下的cmake文件夹以及之前的路径
# set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake;${CMAKE_MODULE_PATH}")

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
file(GLOB_RECURSE srcs CONFIGURE_DEPENDS 
    src/*.cpp 
    include/*.h
    include/*.hpp
    examples/*.cpp
)

# 编译可执行文件
add_executable (${appName} ${srcs})

# 设置头文件路径
include_directories(${ROOT_PATH}/include)

# 将include目录添加到HXHttp库的公共包含目录中
target_include_directories(HXNet PUBLIC include)

# 导入第三方库
include(cmake/includeLib/FindUring.cmake)
include(cmake/includeLib/FindOpenSSL.cmake)
include(cmake/includeLib/FindHashLib.cmake)

if(CMAKE_PROJECT_NAME STREQUAL "HXNet")
    include(cmake/config.cmake)
    include(cmake/develop.cmake)
endif()