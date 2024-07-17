cmake_minimum_required(VERSION 3.10)

project(HXJson LANGUAGES CXX)

# 递归查找
file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)

# 设置头文件路径
include_directories("${ROOT_PATH}/HXprint/include")

# 创建静态库: HXJson
add_library(HXJson STATIC ${srcs})

# 将include目录添加到HXJson库的公共包含目录中
target_include_directories(HXJson PUBLIC include)
