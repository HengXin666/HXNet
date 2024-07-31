#include <HXWeb/server/Acceptor.h>

#include <HXSTL/tools/ErrorHandlingTools.h>
#include <HXWeb/server/ConnectionHandler.h>

namespace HX { namespace web { namespace server {

void Acceptor::start(const std::string& name, const std::string& port) {
    socket::AddressResolver resolver;
    auto entry = resolver.resolve(name, port);
    _serverFd = AsyncFile::asyncBind(entry);
    LOG_INFO("====== HXServer start: \033[33m\033]8;;http://%s:%s/\033\\http://%s:%s/\033]8;;\033\\\033[0m\033[1;32m ======", 
        name.c_str(),
        port.c_str(),
        name.c_str(),
        port.c_str()
    );
    return accept();
}

void Acceptor::accept() {
    // printf(">>> %s <<<\n", HX::STL::utils::DateTimeFormat::formatWithMilli().c_str());
    return _serverFd.asyncAccept(_addr, [self = shared_from_this()] (HX::STL::tools::ErrorHandlingTools::Expected<int> ret) {
        int fd = ret.expect("accept");
        // printf("建立连接成功... %s ~\n", HX::STL::utils::DateTimeFormat::formatWithMilli().c_str());
        ConnectionHandler::make()->start(fd); // 开始读取
        return self->accept(); // 继续回调(如果没有就挂起, 就返回了)
    });
}

}}} // namespace HX::web::server
