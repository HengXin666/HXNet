option(BUILD_01_REFLECTION "Build 01-reflection" ON)

file(GLOB_RECURSE srcs CONFIGURE_DEPENDS 
    *.cpp 
    *.h
    *.hpp
)

add_executable(01-reflection 
    ${srcs}
)