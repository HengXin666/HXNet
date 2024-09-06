#include <HXWeb/HXApiHelper.h>
#include <HXSTL/coroutine/loop/AsyncLoop.h>
#include <HXSTL/utils/FileUtils.h>
#include <HXWeb/server/Server.h>

#ifdef HTTPS_FILE_SERVER_MAIN

class HttpsController {
    ENDPOINT_BEGIN(API_GET, "/", root) {
        RESPONSE_DATA(
            200,
            "<h1>Hello This Heng_Xin 自主研发的 Https 文件服务器!</h1>",
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

    ENDPOINT_BEGIN(API_GET, "/files/**", files) {
        // 使用分块编码
        io.getResponse().setResponseLine(HX::web::protocol::http::Response::Status::CODE_200)
                        .setContentType("text/html", "UTF-8");
        co_await io.sendResponseWithChunkedEncoding("static/text.html");
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
    HX::web::server::Server::startHttps("0.0.0.0", "28205", "certs/cert.pem", "certs/key.pem");
    return 0;
}

#endif // HTTPS_FILE_SERVER_MAIN