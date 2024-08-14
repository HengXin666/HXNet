#include <HXWeb/socket/AddressResolver.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <iostream>

#include <HXSTL/tools/ErrorHandlingTools.h>

namespace HX { namespace web { namespace socket {

int AddressResolver::AddressInfo::createSocketAndBind() const {
    int serverFd = createSocket();
    AddressRef serveAddr = getAddress();
    int on = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));     // 设置端口复用(允许一个套接字在 TIME_WAIT 状态下重新绑定到之前使用的地址和端口)
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));     // 允许多个套接字绑定到相同的地址和端口
    CHECK_CALL(::bind, serverFd, serveAddr._addr, serveAddr._addrlen);   // 将套接字绑定IP和端口用于监听
    CHECK_CALL(::listen, serverFd, SOMAXCONN);                           // 设置监听: 设定可同时排队的客户端最大连接个数
                                                                         // 其中`SOMAXCONN`: Linux可监听的最大数量
    return serverFd;
}

int AddressResolver::AddressInfo::createSocket() const {
    /** 注:
     * @brief `::socket` 创建一个套接字
     * 此函数用于创建一个新的套接字。
     * @param domain 协议族/域，如 AF_INET（IPv4）、AF_INET6（IPv6）、AF_UNIX（Unix 域）等
     * @param type 套接字类型，如 SOCK_STREAM（面向连接的流套接字）、SOCK_DGRAM（无连接的数据报套接字）等
     * @param protocol 协议，通常为 0，表示使用默认协议
     * @return 成功时返回套接字描述符，失败时返回 -1
     * 
     * struct addrinfo {
     *     int ai_flags;             // 标志
     *     int ai_family;            // 地址族: AF_INET（IPv4）或 AF_INET6（IPv6）
     *     int ai_socktype;          // 套接字类型: SOCK_STREAM（流）或 SOCK_DGRAM（数据报）
     *     int ai_protocol;          // 协议: 如 IPPROTO_TCP（TCP）或 IPPROTO_UDP（UDP）
     *     size_t ai_addrlen;        // 地址长度
     *     struct sockaddr *ai_addr; // 指向 sockaddr 结构体的指针
     *     char *ai_canonname;       // 规范化的主机名
     *     struct addrinfo *ai_next; // 指向下一个 addrinfo 结构体的指针
     * };
     */
    return CHECK_CALL(::socket, _curr->ai_family, _curr->ai_socktype, _curr->ai_protocol);
}

AddressResolver::AddressInfo AddressResolver::resolve(
    const std::string& name, 
    const std::string& service
    ) {
    /**
     * hostname: 主机名或地址字符串 IPv4 的点分十进制表示或 IPv6 的十六进制表示
     * service: 服务名可以是十进制的端口号, 也可以是已知的服务名称, 如 ftp、http 等
     * hints: 可以是空指针, 也可以是指向某个 addrinfo 结构体的指针, 包含对所需地址类型的提示
     * result: 该函数通过 result 指针参数返回一个 addrinfo 结构体链表的指针
     */
    int err = getaddrinfo(name.c_str(), service.c_str(), nullptr, &_head);
    if (err) {
        auto ec = std::error_code(err, HX::STL::tools::LinuxErrorHandlingTools::gaiCategory());
        throw std::system_error(ec, name + ":" + service);
    }
    return {_head};
}

}}} // namespace HX::web::socket
