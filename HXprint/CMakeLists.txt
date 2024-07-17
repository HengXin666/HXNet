cmake_minimum_required(VERSION 3.10)

project(HXprint LANGUAGES CXX)

# 递归查找
file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)

# 创建静态库: HXprint
add_library(HXprint STATIC ${srcs})

# 将include目录添加到HXprint库的公共包含目录中
target_include_directories(HXprint PUBLIC include)
