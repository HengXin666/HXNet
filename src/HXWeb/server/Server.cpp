#include <HXWeb/server/Server.h>

#include <iostream>

#include <HXSTL/coroutine/task/Task.hpp>
#include <HXSTL/coroutine/loop/AsyncLoop.h>
#include <HXWeb/server/Acceptor.h>
#include <HXWeb/protocol/https/Context.h>
#include <HXWeb/protocol/http/Http.hpp>
#include <HXWeb/protocol/https/Https.hpp>
#include <HXprint/HXprint.h>

namespace HX { namespace web { namespace server {

void Server::startHttp(
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
            HX::STL::coroutine::task::runTask(
                HX::STL::coroutine::loop::AsyncLoop::getLoop(),
                [&entry, timeout]() -> HX::STL::coroutine::task::Task<> {
                    try {
                        auto ptr = HX::web::server::Acceptor<HX::web::protocol::http::Http>::make();
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

void Server::startHttps(
    const std::string& name,
    const std::string& port,
    const std::string& certificate,
    const std::string& privateKey,
    std::size_t threadNum /*= std::thread::hardware_concurrency()*/,
    std::chrono::seconds timeout /*= std::chrono::seconds {30}*/
) {
    socket::AddressResolver resolver;
    auto entry = resolver.resolve(name, port);
    std::vector<std::thread> threadArr;

    for (std::size_t i = 0; i < threadNum; ++i) {
        threadArr.emplace_back([&entry, timeout, &certificate, &privateKey]() {
            HX::web::protocol::https::Context::getContext().initSSL(
                certificate,
                privateKey
            );
            HX::STL::coroutine::task::runTask(
                HX::STL::coroutine::loop::AsyncLoop::getLoop(),
                [&entry, timeout]() -> HX::STL::coroutine::task::Task<> {
                    try {
                        auto ptr = HX::web::server::Acceptor<HX::web::protocol::https::Https>::make();
                        co_await ptr->start(entry, timeout);
                    } catch(const std::system_error &e) {
                        std::cerr << e.what() << '\n';
                    }
                    co_return;
                }()
            );
        });
    }

    LOG_INFO("====== HXServer start: \033[33m\033]8;;https://%s:%s/\033\\https://%s:%s/\033]8;;\033\\\033[0m\033[1;32m ======", 
        name.c_str(),
        port.c_str(),
        name.c_str(),
        port.c_str()
    );

    for (auto&& it : threadArr)
        it.join();
}

}}} // namespace HX::web::server