#include <iostream>
#include <HXWeb/client/Client.h>
#include <HXWeb/protocol/http/Request.h>
#include <HXWeb/protocol/http/Response.h>
#include <HXSTL/coroutine/loop/AsyncLoop.h>
#include <HXSTL/utils/FileUtils.h>
#include <HXSTL/utils/StringUtils.h>

using namespace std::chrono;

/**
 * @brief 简单的客户端示例
 */

HX::STL::coroutine::task::Task<> startClient() {
    try {
        auto ptr = co_await HX::web::client::Client::request({
            .url = "https://github.com/HengXin666/HXNet",
            .head = { // Host 内部已经自动填写~
                {"User-Agent", "curl/8.8.0"},
                {"Accept", "*/*"}
            },
            .proxy = "socks5://127.0.0.1:2333"
        });
        std::cout << ptr->getStatusCode() << '\n';
        // for (auto&& [k, v] : ptr->getResponseHeaders())
        //     std::cout << k 
        //     << " -> " << v 
        //     << "\n";
        std::string body = ptr->getResponseBody();
        printf("等我写入 (body.size %lu)\n", body.size());
        std::cout << body << '\n';
        printf("写入完毕~\n");
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