# 如果需要使用到 <liburing.h>, 则需要添加以下代码
# 因为本库默认开启了很强的编译检测, 而这个库使用了匿名结构体
# 因此, 我们需要暂时的屏蔽警告: -Wpedantic

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

#include <liburing.h>

#pragma GCC diagnostic pop

add_library(HXLibs_Liburing INTERFACE)
target_sources(HXLibs_Liburing INTERFACE liburing.cpp)
target_include_directories(HXLibs_Liburing INTERFACE include)