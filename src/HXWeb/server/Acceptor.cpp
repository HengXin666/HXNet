#include <HXWeb/server/Acceptor.h>

#include <HXSTL/tools/ErrorHandlingTools.h>
#include <HXWeb/server/ConnectionHandler.h>

namespace HX { namespace web { namespace server {

HX::STL::coroutine::awaiter::Task<> Acceptor::start(
    const std::string& name,
    const std::string& port
) {
    socket::AddressResolver resolver;
    auto entry = resolver.resolve(name, port);
    _serverFd = AsyncFile::asyncBind(entry);
    LOG_INFO("====== HXServer start: \033[33m\033]8;;http://%s:%s/\033\\http://%s:%s/\033]8;;\033\\\033[0m\033[1;32m ======", 
        name.c_str(),
        port.c_str(),
        name.c_str(),
        port.c_str()
    );

    while (true) {
        LOG_INFO("等待连接...");
        int fd = co_await _serverFd.asyncAccept(_addr);
        LOG_WARNING("有新的连接: %d", fd);
        auto task = ConnectionHandler::make()->start(fd); // 开始读取 这个要改!!!!
        HX::STL::coroutine::loop::AsyncLoop::getLoop().getTimerLoop().addTimer(
            std::chrono::system_clock::now(),
            task
        );
    }
}

}}} // namespace HX::web::server