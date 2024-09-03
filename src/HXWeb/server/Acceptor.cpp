#include <HXWeb/server/Acceptor.h>

#include <HXSTL/tools/ErrorHandlingTools.h>
#include <HXSTL/coroutine/loop/AsyncLoop.h>
#include <HXWeb/server/ConnectionHandler.h>
#include <HXWeb/protocol/http/Http.hpp>

namespace HX { namespace web { namespace server {

/**
 * @brief 创建服务器: 创建套接字并且绑定端口并且启用监听并且设置端口复用和端口平均分配
 * @param entry 
 * @return int 服务器套接字
 */
static inline HX::STL::coroutine::task::Task<int> _createServer(
    const HX::web::socket::AddressResolver::AddressInfo& entry
) {
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

    co_return serverFd;
}

HX::STL::coroutine::task::Task<> Acceptor<HX::web::protocol::http::Http>::start(
    const HX::web::socket::AddressResolver::AddressInfo& entry,
    std::chrono::seconds timeout /*= std::chrono::seconds{30}*/
) {
    int serverFd = co_await _createServer(entry);
    
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
                ConnectionHandler<HX::web::protocol::http::Http>::start(fd, timeout)
            )
        );
    }
}

HX::STL::coroutine::task::Task<> Acceptor<HX::web::protocol::https::Https>::start(
    const HX::web::socket::AddressResolver::AddressInfo& entry,
    std::chrono::seconds timeout /*= std::chrono::seconds{30}*/
) {
    int serverFd = co_await _createServer(entry);
    
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
                ConnectionHandler<HX::web::protocol::https::Https>::start(fd, timeout)
            )
        );
    }
}

}}} // namespace HX::web::server