#include <HXWeb/server/Server.h>

#include <iostream>

#include <HXSTL/coroutine/task/Task.hpp>
#include <HXSTL/coroutine/loop/AsyncLoop.h>
#include <HXWeb/server/Acceptor.h>
#include <HXprint/HXprint.h>

namespace HX { namespace web { namespace server {


void Server::start(
    const std::string& name,
    const std::string& port,
    std::size_t threadNum /*= std::thread::hardware_concurrency()*/,
    std::chrono::seconds timeout /*= std::chrono::seconds {30}*/
) {
    socket::AddressResolver resolver;
    auto entry = resolver.resolve(name, port);
    std::vector<std::thread> threadArr;

    for (std::size_t i = 0; i < threadNum; ++i) {
        threadArr.emplace_back([&entry, timeout]() {
            std::cout << std::this_thread::get_id();
            printf(" 启动!\n");
            HX::STL::coroutine::task::runTask(
                HX::STL::coroutine::loop::AsyncLoop::getLoop(),
                [&entry, timeout]() -> HX::STL::coroutine::task::Task<> {
                    try {
                        auto ptr = HX::web::server::Acceptor::make();
                        co_await ptr->start(entry, timeout);
                    } catch(const std::system_error &e) {
                        std::cerr << e.what() << '\n';
                    }
                    co_return;
                }()
            );
        });
    }

    LOG_INFO("====== HXServer start: \033[33m\033]8;;http://%s:%s/\033\\http://%s:%s/\033]8;;\033\\\033[0m\033[1;32m ======", 
        name.c_str(),
        port.c_str(),
        name.c_str(),
        port.c_str()
    );

    for (auto&& it : threadArr)
        it.join();
}

}}} // namespace HX::web::server