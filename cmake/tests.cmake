# 配置单元测试 (开关在`develop.cmake`中)

# 查找 tests 目录下所有的 .cpp 文件
file(GLOB TEST_FILES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
)

include_directories(tests/include)

# 遍历每个 .cpp 文件, 生成测试目标
foreach(TEST_FILE ${TEST_FILES})
    # 提取 .cpp 文件名作为目标名 (去掉路径和扩展名)
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

    # 添加测试可执行文件
    add_executable(${TEST_NAME} ${TEST_FILE})

    # 链接 HXLibs 库和依赖
    target_link_libraries(${TEST_NAME} PRIVATE HXLibs)

    # 设置可执行文件的输出路径
    set_target_properties(${TEST_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/tests
    )

    # 添加 CTest 测试用例
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()