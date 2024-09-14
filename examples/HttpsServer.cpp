#include <HXWeb/HXApiHelper.h>
#include <HXSTL/coroutine/loop/AsyncLoop.h>
#include <HXSTL/utils/FileUtils.h>
#include <HXWeb/server/Server.h>

class ExitController {
    ENDPOINT_BEGIN(API_GET, "/exit", serExit) {
        GET_PARSE_QUERY_PARAMETERS(map);

        if (map.count("loli") && map["loli"] == "end") {
            RESPONSE_DATA(
                200,
                "<h1>已关机!</h1>",
                "text/html", "UTF-8"
            );
            co_await io.sendResponse();
            exit(0);
        } else {
            RESPONSE_DATA(
                200,
                "<h1>已关机... (请等待)</h1>",
                "text/html", "UTF-8"
            );
        }
        co_return true;
    } ENDPOINT_END;
public:
};

// 为了方便调试是否出现内存泄漏, 这里全局注册了一个关机控制器 只需要访问 /exit?loli=end 即可
// 测试结果如下:
// 如果没有正在处理的连接的情况下关机, 不会出现内存泄漏
// 当且仅当正在处理连接的写入/写出的时候突然关闭, 才会触发内存泄漏(实际上是根本没有来得及处理)
// 解决方案: 在空闲的时候才关闭! 比如关闭后, 不处理请求, 并且所有请求断开
int __init__ = []() -> int {
    ROUTER_BIND(ExitController);
    return 0;
}();

#ifdef HTTPS_SERVER_MAIN

class HttpsController {
    ENDPOINT_BEGIN(API_GET, "/", root) {
        RESPONSE_DATA(
            200,
            "<h1>Hello This Heng_Xin 自主研发的 Https 服务器!</h1>",
            "text/html", "UTF-8"
        );
        co_return true;
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_GET, "/favicon.ico", faviconIco) {
        RESPONSE_DATA(
            200, 
            co_await HX::STL::utils::FileUtils::asyncGetFileContent("static/favicon.ico"),
            "image/x-icon"
        );
        co_return false;
    } ENDPOINT_END;
};

#include <HXSTL/coroutine/task/WhenAny.hpp>
#include <chrono>

HX::STL::coroutine::task::Task<bool> __text() {
    co_return false;
}

HX::STL::coroutine::task::Task<bool> _text() {
    // co_await HX::STL::coroutine::loop::TimerLoop::sleepFor(std::chrono::seconds{0});
    co_return co_await __text();
}

HX::STL::coroutine::task::Task<> test() {
    co_await HX::STL::coroutine::task::WhenAny::whenAny( // 如果下面参数互换, 则会段错误
        HX::STL::coroutine::loop::TimerLoop::sleepFor(std::chrono::seconds{3}),
        _text()
    );
}

int main() {
    chdir("..");
    setlocale(LC_ALL, "zh_CN.UTF-8");
    ROUTER_BIND(HttpsController);
    // HX::STL::coroutine::task::runTask(HX::STL::coroutine::loop::AsyncLoop::getLoop(), test());
    // 启动服务
    HX::web::server::Server::startHttps("127.0.0.1", "28205", "certs/cert.pem", "certs/key.pem");
    return 0;
}

#endif // HTTPS_SERVER_MAIN