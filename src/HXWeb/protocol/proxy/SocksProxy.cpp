#include <HXWeb/protocol/proxy/SocksProxy.h>

#include <netinet/in.h>

#include <HXSTL/utils/UrlUtils.h>

namespace HX { namespace web { namespace protocol { namespace proxy {

HX::STL::coroutine::task::Task<> Socks5Proxy::_connect(
    const std::string& url,
    const std::string& targetUrl
) {
    auto user = HX::STL::utils::UrlUtils::extractUser(url);
    co_await handshake(user.has_value());
    if (user) {
        co_await subNegotiation(user->first, user->second);
    }
    co_await socks5ConnectRequest(targetUrl);
    co_return;
}

HX::STL::coroutine::task::Task<> Socks5Proxy::subNegotiation(
    const std::string& username, 
    const std::string& password
) {
    // 发送用户名密码进行验证
    std::string authRequest;
    authRequest += static_cast<char>(0x01);
    authRequest += static_cast<char>(username.size());
    authRequest += username;
    authRequest += static_cast<char>(password.size());
    authRequest += password;
    co_await _io._sendSpan(authRequest);

    char authResponse[2];
    co_await _io._recvSpan(authResponse);
    if (authResponse[1] != 0x00) {
        throw std::invalid_argument("sub-negotiation: REP is " + std::to_string(authResponse[1]));
    }
}

HX::STL::coroutine::task::Task<> Socks5Proxy::handshake(
    bool authentication
) {
    char handshakeRequest[3] = { 
        0x05, // 协议版本号
        0x01, // 客户端支持的方法数量 (决定 METHODS 的长度)
        static_cast<char>(authentication ? 0x02 : 0x00)  // 不进行用户名密码验证
    };
    co_await _io._sendSpan(handshakeRequest);

    // 解析服务端响应
    char handshakeResponse[2];
    co_await _io._recvSpan(handshakeResponse);
    if (handshakeResponse[0] != 0x05 ||                        // 协议版本 (需要一致)
        handshakeResponse[1] != (authentication ? 0x02 : 0x00) // 服务端选择的可用方法
    ) {
        /**
         * @brief 身份验证方法(METHOD)的全部可选值如下:
         * 0x00 不需要身份验证(NO AUTHENTICATION REQUIRED)
         * 0x01 GSSAPI
         * 0x02 用户名密码(USERNAME/PASSWORD)
         * 0x03 至 0x7F 由 IANA 分配(IANA ASSIGNED)
         * 0x80 至 0xFE 为私人方法保留(RESERVED FOR PRIVATE METHODS)
         * 0xFF 无可接受的方法 (NO ACCEPTABLE METHODS)
         */
        throw std::invalid_argument("handshake: METHOD is " + std::to_string(handshakeResponse[1]));
    }
}

HX::STL::coroutine::task::Task<> Socks5Proxy::socks5ConnectRequest(
    const std::string& targetUrl
) {
    std::string connectRequest;
    connectRequest += static_cast<char>(0x05); // 协议版本号 Version 5

    /**
     * @brief 命令类型
     * 0x01 CONNECT         | 代理 TCP 流量
     * 0x02 BIND            | 代理开启监听端口, 接收目标地址的连接
     *                      | (如果 SOCKS5 代理服务器具有公网 IP 地址, 则可以通过 BIND 请求实现内网穿透)
     * 0x03 UDP ASSOCIATE   | 代理 UDP 数据转发
     */
    connectRequest += static_cast<char>(0x01);
    
    connectRequest += static_cast<char>(0x00); // 保留字段

    /**
     * @brief 目标地址类型
     * 0x01 IPv4
     * 0x03 域名
     * 0x04 IPv6
     */
    connectRequest += static_cast<char>(0x03);

    /**
     * @brief 目标地址
     * 可变长度
     * 4 (IPv4)
     * 16 (IPv6)
     * 域名:
     *      如果 ATYP 字段值是 0x03，则 DST.ADDR 的格式为:
     *      - 域名长度 (一个unsigned char)
     *      - 域名 (unsigned char []) (可变长度)
     */
    HX::STL::utils::UrlUtils::UrlInfoExtractor parser(targetUrl);
    connectRequest += static_cast<char>(parser.getHostname().size());
    connectRequest += parser.getHostname();
    connectRequest.resize(connectRequest.size() + 2);
    *(reinterpret_cast<uint16_t*>(&connectRequest[connectRequest.size() - 2])) = 
        ::htons(HX::STL::utils::UrlUtils::getProtocolPort(parser.getService()));

    co_await _io._sendSpan(connectRequest);
    co_await _io._recvSpan(connectRequest);

    /**
     * @brief 
     * 字段	        描述            类型               长度       例值
     * VER          协议版本号	    unsigned char	    1	    0x05
     * REP          服务器应答	    unsigned char	    1	    0x00 成功
     * RSV          保留字段	    unsigned char	    1	    0x00
     * ATYP         目标地址类型	unsigned char	    1	    0x01 IPv4
     *                                                         0x04 IPv6
     * BND.ADDR	    绑定地址	    unsigned char []   可变长度
     *                                                4 (IPv4)
     *                                                16 (IPv6)	
     * BND.PORT	    绑定端口	    unsigned short	2
     */
    if (connectRequest[1] != 0x00) { // 失败
        /**
         * @brief 服务器响应消息中的 REP 字段如果不为 0x00, 则表示请求失. 不同值的具体含义如下:
         * 0x00 成功
         * 0x01 常规 SOCKS 服务器故障
         * 0x02 规则不允许的链接
         * 0x03 网络无法访问
         * 0x04 主机无法访问
         * 0x05 连接被拒绝
         * 0x06 TTL 过期
         * 0x07 不支持的命令
         * 0x08 不支持的地址类型
         */
        throw std::invalid_argument("Connect Request: REP is " + std::to_string(connectRequest[1]));
    }
}

}}}} // namespace HX::web::protocol::proxy