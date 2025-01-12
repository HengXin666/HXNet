project(01-reflection LANGUAGES CXX)

option(BUILD_01_REFLECTION "Build 01-reflection" ON)

if (BUILD_01_REFLECTION)
    # 查找源文件
    file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS 
        *.cpp 
        *.h
        *.hpp
    )

    # 定义目标
    add_executable(01-reflection ${SRC_FILES})

    # 链接 HXLibs 库
    target_link_libraries(01-reflection PUBLIC HXLibs)
endif()
