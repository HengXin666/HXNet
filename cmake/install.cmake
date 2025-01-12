# 递归查找, 然后结果存储在`srcs`
file(GLOB_RECURSE srcs 
    CONFIGURE_DEPENDS 
    RELATIVE ${CMAKE_SOURCE_DIR}
    include/*.h
    include/*.hpp
    src/*.cpp
)

add_library(HXLibs ${srcs})
add_library(HXLibs::HXLibs ALIAS HXLibs)

# 导入第三方库
include(cmake/includeLib.cmake)

# 包含头文件目录
include_directories(include)