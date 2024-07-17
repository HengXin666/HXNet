cmake_minimum_required(VERSION 3.10)

# 设置文件名称
set (appName HXHttp)

# 递归查找
file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)

# 创建静态库: HXHttp
# add_library(HXHttp STATIC ${srcs})

# 编译可执行文件
add_executable (${appName} ${srcs})

# 将include目录添加到HXHttp库的公共包含目录中
target_include_directories(HXHttp PUBLIC include)