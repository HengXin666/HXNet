if (HX_NET_FIND_OPENSSL)
    # 查找 OpenSSL 库
    find_package(OpenSSL REQUIRED)

    # 链接 OpenSSL 库
    # 下面这个链接, 需要额外搞?
    target_link_libraries(HXNet OpenSSL::SSL OpenSSL::Crypto)
    # target_include_directories(HXNet PUBLIC lib/openssl/include)
else()
    message(FATAL_ERROR "OpenSSL not found")
endif()