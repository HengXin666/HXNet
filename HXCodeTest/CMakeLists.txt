cmake_minimum_required(VERSION 3.10)

# 设置文件名称
set (appName HXCodeTest)

# 递归查找
file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)

# 设置头文件路径
include_directories(${ROOT_PATH}/HXprint/include)
include_directories(${ROOT_PATH}/HXJson/include)

# # 创建静态库: HXCodeTest
# add_library(HXCodeTest STATIC ${srcs})

# 编译可执行文件
add_executable (${appName} ${srcs})

# add_subdirectory(HXprint)
target_link_libraries(HXCodeTest HXprint)
target_link_libraries(HXCodeTest HXJson)

# 将include目录添加到HXCodeTest库的公共包含目录中
target_include_directories(HXCodeTest PUBLIC include)
