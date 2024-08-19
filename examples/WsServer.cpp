#include <iostream>
#include <HXWeb/HXApiHelper.h>
#include <HXSTL/utils/FileUtils.h>
#include <HXSTL/coroutine/loop/AsyncLoop.h>
#include <HXWeb/server/Acceptor.h>
#include <HXJson/HXJson.h>
#include <HXSTL/coroutine/task/WhenAny.hpp>
#include <HXSTL/coroutine/loop/TriggerWaitLoop.h>
#include <HXWeb/protocol/websocket/WebSocket.h>

using namespace std::chrono;

/**
 * @brief 实现一个websocket的聊天室 By Http服务器
 */

class WSChatController {
    ENDPOINT_BEGIN(API_GET, "/favicon.ico", faviconIco) {
        RESPONSE_DATA(
            200, 
            co_await HX::STL::utils::FileUtils::asyncGetFileContent("favicon.ico"),
            "image/x-icon"
        );
        co_return true;
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_GET, "/", root) {
        if (auto ws = co_await HX::web::protocol::websocket::WebSocket::makeServer(req)) {
            // 成功升级为 WebSocket
            printf("成功升级为 WebSocket\n");
            ws->setOnMessage([&](const std::string& message) -> HX::STL::coroutine::task::Task<> {
                printf("收到消息: %s (%lu)\n", message.c_str(), message.size());
                co_await ws->send("收到啦！" + message);
            });

            ws->setOnPong([&](std::chrono::steady_clock::duration dt) -> HX::STL::coroutine::task::Task<> {
                printf("网络延迟: %ld ms\n", dt.count());
                co_return;
            });

            ws->setOnClose([&]() -> HX::STL::coroutine::task::Task<> {
                printf("正在关闭连接...\n");
                co_return;
            });

            co_await ws->start();
            co_return false;
        } else {
            RESPONSE_DATA(
                200,
                co_await HX::STL::utils::FileUtils::asyncGetFileContent("WebSocketIndex.html"),
                "text/html", "UTF-8"
            );
            co_return true;
        }
    } ENDPOINT_END;
};

HX::STL::coroutine::task::Task<> startWsChatServer() {
    ROUTER_BIND(WSChatController);
    try {
        auto ptr = HX::web::server::Acceptor::make();
        co_await ptr->start("127.0.0.1", "28205", 10s);
    } catch(const std::system_error &e) {
        std::cerr << e.what() << '\n';
    }
    co_return;
}

int main() {
    chdir("../static");
    setlocale(LC_ALL, "zh_CN.UTF-8");
    HX::STL::coroutine::task::runTask(
        HX::STL::coroutine::loop::AsyncLoop::getLoop(), 
        startWsChatServer()
    );
    return 0;
}