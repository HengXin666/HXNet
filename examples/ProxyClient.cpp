#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

struct Proxy {
    std::string ip;
    std::string username;
    std::string password;
    int port;
};

void socks5Handshake(int proxySock, const std::string& username, const std::string& password) {
    unsigned char handshakeRequest[3] = { 0x05, 0x01, 0x02 }; // Version 5, 1 method, username/password authentication
    send(proxySock, handshakeRequest, sizeof(handshakeRequest), 0);

    unsigned char handshakeResponse[2];
    recv(proxySock, handshakeResponse, sizeof(handshakeResponse), 0);
    if (handshakeResponse[0] != 0x05 || handshakeResponse[1] != 0x02) {
        std::cerr << "SOCKS5 handshake failed or no suitable authentication method" << std::endl;
        close(proxySock);
        exit(1);
    }

    // Send username/password authentication
    unsigned char authRequest[3 + username.size() + password.size()];
    authRequest[0] = 0x01; // Sub-negotiation version
    authRequest[1] = username.size();
    memcpy(&authRequest[2], username.c_str(), username.size());
    authRequest[2 + username.size()] = password.size();
    memcpy(&authRequest[3 + username.size()], password.c_str(), password.size());
    send(proxySock, authRequest, sizeof(authRequest), 0);

    unsigned char authResponse[2];
    recv(proxySock, authResponse, sizeof(authResponse), 0);
    if (authResponse[1] != 0x00) {
        std::cerr << "SOCKS5 authentication failed" << std::endl;
        close(proxySock);
        exit(1);
    }
}

/**
 * @brief 协商阶段
 */
void socks5Handshake(int proxySock) {
    unsigned char handshakeRequest[3] = { 
        0x05, // 协议版本号
        0x01, // 客户端支持的方法数量 (决定 METHODS 的长度)
        0x00  // 不进行用户名密码验证
    };
    send(proxySock, handshakeRequest, sizeof(handshakeRequest), 0);

    // 解析服务端响应
    unsigned char handshakeResponse[2];
    recv(proxySock, handshakeResponse, sizeof(handshakeResponse), 0);
    if (handshakeResponse[0] != 0x05 || // 协议版本 (需要一致)
        handshakeResponse[1] != 0x00    // 服务端选择的可用方法
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
        std::cerr << "SOCKS5 handshake failed" << std::endl;
        close(proxySock);
    }
}

/**
 * @brief 子协商阶段 | 如果服务器返回的方法不为 0x00, 则需要进入子协商阶段
 */

/**
 * @brief 代理请求
 */
void socks5ConnectRequest(int proxySock, const std::string& targetHost, int targetPort) {
    unsigned char connectRequest[10];
    connectRequest[0] = 0x05; // 协议版本号 Version 5

    /**
     * @brief 命令类型
     * 0x01 CONNECT         | 代理 TCP 流量
     * 0x02 BIND            | 代理开启监听端口, 接收目标地址的连接
     *                      | (如果 SOCKS5 代理服务器具有公网 IP 地址, 则可以通过 BIND 请求实现内网穿透)
     * 0x03 UDP ASSOCIATE   | 代理 UDP 数据转发
     */
    connectRequest[1] = 0x01;

    connectRequest[2] = 0x00; // 保留字段

    /**
     * @brief 目标地址类型
     * 0x01 IPv4
     * 0x03 域名
     * 0x04 IPv6
     */
    connectRequest[3] = 0x01; // Address type: IPv4

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
    inet_pton(AF_INET, targetHost.c_str(), &connectRequest[4]); // IP address
    *(reinterpret_cast<uint16_t*>(&connectRequest[8])) = htons(targetPort); // Port

    send(proxySock, connectRequest, sizeof(connectRequest), 0);

    // 响应
    unsigned char connectResponse[10];
    recv(proxySock, connectResponse, sizeof(connectResponse), 0);

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
    if (connectResponse[1] != 0x00) { // 失败
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
        std::cerr << "SOCKS5 connection request failed" << std::endl;
        close(proxySock);
        exit(1);
    }
}

void sendHttpRequestsThroughSocks5(const std::string& proxyHost, int proxyPort, const std::string& targetHost, int targetPort) {
    int proxySock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in proxyAddr;
    proxyAddr.sin_family = AF_INET;
    proxyAddr.sin_port = htons(proxyPort);
    inet_pton(AF_INET, proxyHost.c_str(), &proxyAddr.sin_addr);

    if (connect(proxySock, (sockaddr*)&proxyAddr, sizeof(proxyAddr)) < 0) {
        std::cerr << "Failed to connect to proxy server" << std::endl;
        close(proxySock);
        return;
    }

    socks5Handshake(proxySock);
    socks5ConnectRequest(proxySock, targetHost, targetPort);

    // Define multiple HTTP requests
    std::string requests[] = { // 需要 Connection: keep-alive 为复用, 才会复用代理, 不然不会重新连接?!
        "GET / HTTP/1.1\r\nConnection: keep-alive\r\n\r\n",
        "GET / HTTP/1.1\r\nConnection: keep-alive\r\n\r\n"
    };

    for (const auto& request : requests) {
        // Send HTTP request
        send(proxySock, request.c_str(), request.length(), 0);

        // Receive HTTP response
        char buffer[BUFFER_SIZE];
        ssize_t bytesRead;
        std::string response;

        // Read response until it's completely received
        while ((bytesRead = recv(proxySock, buffer, sizeof(buffer), 0)) > 0) {
            response.append(buffer, bytesRead);
            if (bytesRead < BUFFER_SIZE) break; // End of response
        }

        std::cout << response << std::endl; // Output the complete response
    }

    close(proxySock);
}

#ifdef COMPILE_PROXY_CLIENT_MAIN
int main() {
    std::string proxyHost = "127.0.0.1"; // SOCKS5 proxy server IP
    int proxyPort = 2333; // SOCKS5 proxy server port
    std::string targetHost = "183.2.172.42"; // Target server hostname
    int targetPort = 80; // Target server port

    sendHttpRequestsThroughSocks5(proxyHost, proxyPort, targetHost, targetPort);
    return 0;
}
#endif