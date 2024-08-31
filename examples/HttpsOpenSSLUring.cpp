#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <poll.h>
#include <sys/epoll.h>
#include <signal.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>  
#include <openssl/err.h>

#include <iostream>
#include <HXWeb/HXApiHelper.h>
#include <HXSTL/utils/FileUtils.h>
#include <HXWeb/protocol/websocket/WebSocket.h>
#include <HXWeb/server/Server.h>

using namespace std::chrono;

#include <HXSTL/coroutine/loop/IoUringLoop.h>
#include <HXSTL/coroutine/loop/AsyncLoop.h>
#include <HXWeb/socket/AddressResolver.h>

using namespace std;

#ifdef HTTPS_OPEN_SSL_URING_MAIN

#define log(...) do { printf(__VA_ARGS__); fflush(stdout); } while(0)
#define check0(x, ...) if(x) do { log(__VA_ARGS__); exit(1); } while(0)
#define check1(x, ...) if(!x) do { log(__VA_ARGS__); exit(1); } while(0)

BIO* errBio; // 用于 SSL 错误输出的 BIO
SSL_CTX* g_sslCtx; // 全局 SSL 上下文

// 初始化 SSL 库和上下文
void initSSL() {
    chdir("../certs"); // 切换到证书目录
    SSL_load_error_strings(); // 加载错误字符串
    int r = SSL_library_init(); // 初始化 SSL 库
    check0(!r, "SSL_library_init failed");
    g_sslCtx = SSL_CTX_new(SSLv23_method()); // 创建新的 SSL 上下文
    check0(g_sslCtx == NULL, "SSL_CTX_new failed");
    errBio = BIO_new_fd(2, BIO_NOCLOSE); // 创建错误输出的 BIO
    string cert = "cert.pem", key = "key.pem";
    r = SSL_CTX_use_certificate_file(g_sslCtx, cert.c_str(), SSL_FILETYPE_PEM); // 加载证书
    check0(r <= 0, "SSL_CTX_use_certificate_file %s failed", cert.c_str());
    r = SSL_CTX_use_PrivateKey_file(g_sslCtx, key.c_str(), SSL_FILETYPE_PEM); // 加载私钥
    check0(r <= 0, "SSL_CTX_use_PrivateKey_file %s failed", key.c_str());
    r = SSL_CTX_check_private_key(g_sslCtx); // 检查私钥
    check0(!r, "SSL_CTX_check_private_key failed");
    log("SSL inited\n");
}

// 设置文件描述符为非阻塞模式
// value 是否 为 非阻塞模式
int setNonBlock(int fd, bool value = true) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return errno;
    }
    if (value) {
        return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
    return fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
}

HX::STL::coroutine::task::TimerTask startConn(
    int fd,
    std::chrono::seconds timeout
) {
    HX::STL::tools::LinuxErrorHandlingTools::convertError<int>(
        setNonBlock(fd)
    ).expect("setNonBlock");

    if (POLLERR == co_await HX::STL::coroutine::loop::IoUringTask().prepPollAdd(
        fd, POLLIN | POLLOUT | POLLERR
    )) {
        printf("发生错误! err: %s\n", strerror(errno));
        co_await HX::STL::coroutine::loop::IoUringTask().prepClose(fd);
        co_return;
    }

    SSL* ssl = SSL_new(g_sslCtx);

    if (ssl == nullptr) {
        printf("ssl == nullptr\n");
        co_await HX::STL::coroutine::loop::IoUringTask().prepClose(fd);
        co_return;
    }

    HX::STL::tools::LinuxErrorHandlingTools::convertError<int>(
        SSL_set_fd(ssl, fd) // 绑定文件描述符
    ).expect("SSL_set_fd");

    SSL_set_accept_state(ssl); // 设置为接受状态

    // 监测 io_uring_prep_poll_add fd状态!!!
    // 1. 握手
    while (true) {
        int res = SSL_do_handshake(ssl); // 执行握手
        if (res == 1)
            break;
        int err = SSL_get_error(ssl, res);
        if (err == SSL_ERROR_WANT_WRITE) {
            // 设置关注写事件
            if (POLLOUT != co_await HX::STL::coroutine::loop::IoUringTask().prepPollAdd(
                fd, POLLOUT | POLLERR
            )) {
                printf("POLLOUT error!\n");
                goto END;
            }
        } else if (err == SSL_ERROR_WANT_READ) {
            // 设置关注读事件
            if (POLLIN != co_await HX::STL::coroutine::loop::IoUringTask().prepPollAdd(
                fd, POLLIN | POLLERR
            )) {
                printf("POLLIN error!\n");
                goto END;
            }
        } else {
            ERR_print_errors(errBio);
            goto END;
        }
    }

    {
        HX::web::protocol::http::Request request;
        HX::web::protocol::http::Response response;
        vector<char> buf(HX::STL::utils::FileUtils::kBufMaxSize);
        while (true) {
            // 2. 读取
            std::size_t n = buf.size();
            while (true) {
                int readLen = SSL_read(ssl, buf.data(), n); // 从 SSL 连接中读取数据
                int err = SSL_get_error(ssl, readLen);
                if (readLen > 0) {
                    if (std::size_t size = request.parserRequest(
                        std::span<char> {buf.data(), (std::size_t) readLen}
                    )) {
                        n = std::min(n, size);
                        if (POLLIN != co_await HX::STL::coroutine::loop::IoUringTask().prepPollAdd(
                            fd, POLLIN | POLLERR
                        )) {
                            printf("SSL_read: (request) POLLIN error!\n");
                            goto END;
                        }
                        continue;
                    }
                    break;
                } else if (readLen == 0) { // 客户端断开连接
                    goto END;
                } else if (err == SSL_ERROR_WANT_READ) {
                    if (POLLIN != co_await HX::STL::coroutine::loop::IoUringTask().prepPollAdd(
                        fd, POLLIN | POLLERR
                    )) {
                        printf("SSL_read: POLLIN error!\n");
                        goto END;
                    }
                } else {
                    printf("SB SSL_read: err is %d is %s\n", err, strerror(errno));
                    goto END;
                }
            }

            auto&& fun = HX::web::router::Router::getSingleton().getEndpointFunc(
                request.getRequesType(),
                request.getRequesPath()
            );

            // 3. 写入
            std::string resBuf = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Length: 13\r\n\r\nHello, world!";
            n = resBuf.size();
            while (true) {
                int writeLen = SSL_write(ssl, resBuf.data(), n); // 从 SSL 连接中读取数据
                int err = SSL_get_error(ssl, writeLen);
                if (writeLen > 0) {
                    n -= writeLen;
                    if (n > 0) {
                        if (POLLOUT != co_await HX::STL::coroutine::loop::IoUringTask().prepPollAdd(
                            fd, POLLOUT | POLLERR
                        )) {
                            printf("SSL_write: (n > 0) POLLOUT error!\n");
                            goto END;
                        }
                        continue;
                    }
                    break;
                } else if (writeLen == 0) { // 客户端断开连接
                    goto END;
                } else if (err == SSL_ERROR_WANT_WRITE) {
                    if (POLLOUT != co_await HX::STL::coroutine::loop::IoUringTask().prepPollAdd(
                        fd, POLLOUT | POLLERR
                    )) {
                        printf("SSL_write: POLLOUT error!\n");
                        goto END;
                    }
                } else {
                    printf("SB SSL_write: err is %d is %s\n", err, strerror(errno));
                    goto END;
                }
            }
        }
    }

    END:
    co_await HX::STL::coroutine::loop::IoUringTask().prepClose(fd);
    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    co_return;
}

HX::STL::coroutine::task::Task<> co_main(
    const string& name, 
    const string& port, 
    std::chrono::seconds timeout
) {
    HX::web::socket::AddressResolver resolver;
    auto entry = resolver.resolve(name, port);

    int serverFd = HX::STL::tools::UringErrorHandlingTools::throwingError(
        co_await HX::STL::coroutine::loop::IoUringTask().prepSocket(
            entry._curr->ai_family,
            entry._curr->ai_socktype,
            entry._curr->ai_protocol,
            0
        )
    );

    auto serve_addr = entry.getAddress();
    int on = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));

    HX::STL::tools::LinuxErrorHandlingTools::convertError<int>(
        ::bind(serverFd, serve_addr._addr, serve_addr._addrlen)
    ).expect("bind");

    HX::STL::tools::LinuxErrorHandlingTools::convertError<int>(
        ::listen(serverFd, SOMAXCONN)
    ).expect("listen");
    
    while (true) {
        int fd = HX::STL::tools::UringErrorHandlingTools::throwingError(
            co_await HX::STL::coroutine::loop::IoUringTask().prepAccept(
                serverFd,
#ifdef CLIENT_ADDRESS_LOGGING
                &_addr._addr,
                &_addr._addrlen,
#else
                nullptr,
                nullptr,
#endif
                0
            )
        );

        LOG_WARNING("有新的连接: %d", fd);
        // 开始读取
        HX::STL::coroutine::loop::AsyncLoop::getLoop().getTimerLoop().addInitiationTask(
            std::make_shared<HX::STL::coroutine::task::TimerTask>(
                startConn(fd, timeout)
            )
        );
    }
}

int main() {
    initSSL();
    HX::STL::coroutine::task::runTask(
        HX::STL::coroutine::loop::AsyncLoop::getLoop(),
        co_main("0.0.0.0", "22333", 10s)
    );
    return 0;
}

#endif // HTTPS_OPEN_SSL_URING_MAIN