#include <HXWeb/HXApiHelper.h>
#include <HXSTL/coroutine/loop/AsyncLoop.h>
#include <HXSTL/utils/FileUtils.h>
#include <HXWeb/server/Server.h>

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
        co_return true;
    } ENDPOINT_END;
};

int main() {
    chdir("..");
    setlocale(LC_ALL, "zh_CN.UTF-8");
    ROUTER_BIND(HttpsController);

    // 启动服务
    HX::web::server::Server::startHttps("0.0.0.0", "28205", "certs/cert.pem", "certs/key.pem");
    return 0;
}

#endif // HTTPS_SERVER_MAIN