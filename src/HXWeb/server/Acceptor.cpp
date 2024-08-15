#include <HXWeb/server/Acceptor.h>

#include <HXSTL/tools/ErrorHandlingTools.h>
#include <HXSTL/coroutine/loop/AsyncLoop.h>
#include <HXWeb/server/ConnectionHandler.h>

namespace HX { namespace web { namespace server {

HX::STL::coroutine::task::Task<> Acceptor::start(
    const std::string& name,
    const std::string& port
) {
    socket::AddressResolver resolver;
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
    
    LOG_INFO("====== HXServer start: \033[33m\033]8;;http://%s:%s/\033\\http://%s:%s/\033]8;;\033\\\033[0m\033[1;32m ======", 
        name.c_str(),
        port.c_str(),
        name.c_str(),
        port.c_str()
    );

    while (true) {
        int fd = HX::STL::tools::UringErrorHandlingTools::throwingError(
            co_await HX::STL::coroutine::loop::IoUringTask().prepAccept(
                serverFd,
                &_addr._addr,
                &_addr._addrlen,
                0
            )
        );

        LOG_WARNING("有新的连接: %d", fd);
        // 开始读取
        HX::STL::coroutine::loop::AsyncLoop::getLoop().getTimerLoop().addTimer(
            std::chrono::system_clock::now(),
            nullptr,
            std::make_shared<HX::STL::coroutine::task::TimerTask>(
                ConnectionHandler::start(fd)
            )
        );
    }
}

}}} // namespace HX::web::server
