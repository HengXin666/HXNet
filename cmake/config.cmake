# 是否启用 Address Sanitizer
set(HX_DEBUG_BY_ADDRESS_SANITIZER ON)

# 使用 Address Sanitizer
if(HX_DEBUG_BY_ADDRESS_SANITIZER AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
  message("=-=-=-=-=-=-= 启用 Address Sanitizer [-fsanitize=address] =-=-=-=-=-=-=")
endif()