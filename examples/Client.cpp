#include <iostream>
#include <HXWeb/client/Client.h>
#include <HXWeb/protocol/http/Request.h>
#include <HXWeb/protocol/http/Response.h>
#include <HXSTL/coroutine/loop/AsyncLoop.h>

using namespace std::chrono;

/**
 * @brief 简单的客户端示例
 */

HX::STL::coroutine::task::Task<> startClient() {
    auto ptr = HX::web::client::Client::make();
    co_await ptr->start("http://www.baidu.com");
    std::string s = "GET / HTTP/1.1\r\n\r\n";
    co_await ptr->write(s);
    co_await ptr->read(5s);
    std::cout << "res: " << ptr->getIO().getResponse().getResponseBody() << '\n';
    co_return;
}

#ifdef CLIENT_MAIN
int main() {
    HX::STL::coroutine::task::runTask(
        HX::STL::coroutine::loop::AsyncLoop::getLoop(),
        startClient()
    );
    return 0;
}
#endif