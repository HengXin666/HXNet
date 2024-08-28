#include <HXWeb/server/Acceptor.h>

#include <HXSTL/tools/ErrorHandlingTools.h>
#include <HXSTL/coroutine/loop/AsyncLoop.h>
#include <HXWeb/server/ConnectionHandler.h>

namespace HX { namespace web { namespace server {

HX::STL::coroutine::task::Task<> Acceptor::start(
    const HX::web::socket::AddressResolver::AddressInfo& entry,
    std::chrono::seconds timeout /*= std::chrono::seconds{30}*/
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
        HX::STL::coroutine::loop::AsyncLoop::getLoop().getTimerLoop().addTask(
            std::make_shared<HX::STL::coroutine::task::TimerTask>(
                ConnectionHandler::start(fd, timeout)
            )
        );
    }
}

}}} // namespace HX::web::server