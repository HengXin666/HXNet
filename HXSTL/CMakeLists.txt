cmake_minimum_required(VERSION 3.10)

# 设置文件名称
set (appName HXSTL)

# 递归查找
file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)

# 创建静态库: HXSTL
add_library(HXSTL STATIC ${srcs})

# 编译可执行文件
# add_executable (${appName} ${srcs})

# 将include目录添加到HXSTL库的公共包含目录中
target_include_directories(HXSTL PUBLIC include)
