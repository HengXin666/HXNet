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
    try {
        auto ptr = co_await HX::web::client::Client::request({
            .url = "https://github.com/HengXin666/HXNet",
            .head = {
                {"Host", "github.com"},
                {"User-Agent", "curl/8.8.0"},
                {"Accept", "*/*"}
            },
            .proxy = "socks5://127.0.0.1:2333",
            .verifyBuilder = HX::web::protocol::https::HttpsVerifyBuilder {
                .verifyMod = 0x01
            }
        });
        std::cout << ptr->getStatusCode() << '\n';
        for (auto&& [k, v] : ptr->getResponseHeaders())
            std::cout << k << ' ' << v << '\n';
        std::cout << ptr->getResponseBody() << '\n';
    } catch (const std::system_error& e) {
        std::cerr << e.what() << '\n';
    } catch (const char* e) {
        std::cerr << e << '\n';
    }
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