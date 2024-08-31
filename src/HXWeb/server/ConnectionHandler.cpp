#include <HXWeb/server/ConnectionHandler.h>

#include <poll.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>  
#include <openssl/err.h>

#include <HXSTL/coroutine/loop/IoUringLoop.h>
#include <HXSTL/tools/ErrorHandlingTools.h>
#include <HXSTL/utils/FileUtils.h>
#include <HXWeb/router/Router.h>
#include <HXWeb/protocol/http/Request.h>
#include <HXWeb/protocol/http/Response.h>
#include <HXWeb/server/IO.h>
#include <HXprint/HXprint.h>

namespace HX { namespace web { namespace server {

HX::STL::coroutine::task::TimerTask ConnectionHandler<HX::web::protocol::http::Http>::start(
    int fd, 
    std::chrono::seconds timeout
) {
    HX::web::server::IO io {fd};

    // 连接超时
    auto _timeout = HX::STL::coroutine::loop::durationToKernelTimespec(timeout);
    
    bool endpointRes = 0; // 是否复用连接

    while (true) {
        // === 读取 ===
        // LOG_INFO("读取中...");
        if (co_await io._recvRequest(&_timeout)) {
            LOG_INFO("客户端 %d 已断开连接!", fd);
            co_return;
        }
        // === 路由解析 ===
        // 交给路由处理
        // LOG_INFO("路由解析中...");
        auto&& fun = HX::web::router::Router::getSingleton().getEndpointFunc(
            io._request->getRequesType(),
            io._request->getRequesPath()
        );
        try {
            // printf("cli -> url: %s\n", _request.getRequesPath().c_str());
            if (fun) {
                endpointRes = co_await fun(io);
            } else {
                endpointRes = co_await HX::web::router::Router::getSingleton().getErrorEndpointFunc()(io);
            }

            // === 响应 ===
            // LOG_INFO("响应中...");
            co_await io.sendResponse(HX::STL::container::NonVoidHelper<>{});
            if (!endpointRes)
                break;
        } catch (const std::exception& e) {
            LOG_ERROR("向客户端 %d 发送消息时候出错 (e): %s", fd, e.what());
            break;
        } catch (const char* msg) {
            LOG_ERROR("向客户端 %d 发送消息时候出错 (msg): %s", fd, msg);
            break;
        } catch(...) {
            LOG_ERROR("向客户端 %d 发送消息时候发生未知错误", fd);
            break;
        }
    }

    LOG_WARNING("客户端直接给我退出! (%d)", fd);
}

#if 0

HX::STL::coroutine::task::TimerTask ConnectionHandler<HX::web::protocol::https::Https>::start(
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
        std::vector<char> buf(HX::STL::utils::FileUtils::kBufMaxSize);
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

#endif

}}} // namespace HX::web::server
