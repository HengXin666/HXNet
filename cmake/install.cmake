# 递归查找, 然后结果存储在`srcs`
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS 
    src/*.cpp
)

add_library(HXLibs STATIC ${SRC_FILES})
add_library(HXLibs::HXLibs ALIAS HXLibs)

# 导入第三方库
include(cmake/includeLib.cmake)

# 包含头文件目录
include_directories(include)

target_include_directories(HXLibs PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(HXLibs PUBLIC cxx_std_20)
