cmake_minimum_required(VERSION 3.15)
# 定义工程名称, 并指定语言为C++
project(HXLibs
    VERSION 0.0.1
    DESCRIPTION "HXLibs"
    HOMEPAGE_URL "https://github.com/HengXin666/HXLibs"
    LANGUAGES CXX
)

# 是否开启 绕过操作系统的缓存, 直接将数据从用户空间读写到磁盘
# set(IO_URING_DIRECT ON) # 有BUG 先不要用

# 启用 搜集客户端地址信息 宏
add_definitions(-DCLIENT_ADDRESS_LOGGING)

#####        ########
# 请选择编译的示例代码#
########       ######
# 部件测试 
# add_definitions(-DTEXT_MAIN_MAIN)  # 文件: TestMain.cpp
# add_definitions(-DJSON_TEST_MAIN)  # json示例
# add_definitions(-DIO_URING_OSTREAM_MAIN)  # 测试自己实现的输出流, 使用协程!
# add_definitions(-DCACHE_TEST_MAIN) # 测试LRU缓存
# add_definitions(-D_HX_REFLECTION_TEST_H_) # 测试反射

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

include(cmake/install.cmake)

if(CMAKE_PROJECT_NAME STREQUAL "HXNet")
    include(cmake/config.cmake)
    include(cmake/develop.cmake)

    add_subdirectory(examples/ChatServer)
else()
    include(cmake/config.cmake)
endif()
