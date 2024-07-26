#include <HXHttp/HXHttp.h>

namespace HXHttp {

} // namespace HXHttp

#include <HXHttp/HXRouter.h>
#include <HXSTL/HXStringTools.h>
#include <HXHttp/HXRequest.h>
#include <HXHttp/HXResponse.h>
#include <HXprint/HXprint.h>
#include <HXHttp/HXServer.h>

#include <HXSTL/HXMagicEnum.h>

#include <iostream>
#include <unordered_map>
#include <csignal>
#include <cstring>

int _me_main() {
    enum MyEnum {
        LoLi = 1,
        imouto = 8,
    };
    std::cout << HXSTL::MagicEnum::getEnumName<MyEnum>(MyEnum::imouto) << '\n';
    std::cout << HXSTL::MagicEnum::nameFromEnum<MyEnum>("imouto") << '\n';
    return 0;
}

int main() {
    setlocale(LC_ALL, "zh_CN.UTF-8");
    try {
        HXHttp::HXRouter::getSingleton().addController("GET", "/", [](const HXHttp::HXRequest& req) -> HXHttp::HXResponse {
            return std::move(HXHttp::HXResponse {}.setResponseLine(HXHttp::HXResponse::Status::CODE_200)
                .setContentType("text/html", "UTF-8")
                .setBodyData("<h1>Hello, world!</h1><h2>Now Time: " 
                            + HXSTL::HXDateTimeFormat::format() 
                            + "</h2>"));
        });
        HXHttp::HXRouter::getSingleton().addController("POST", "/", [](const HXHttp::HXRequest& req) -> HXHttp::HXResponse {
            return std::move(HXHttp::HXResponse {}.setResponseLine(HXHttp::HXResponse::Status::CODE_200)
                .setContentType("text/html", "UTF-8")
                .setBodyData("<h1>Hello, world! POSH老哥</h1><h2>Now Time: " 
                            + HXSTL::HXDateTimeFormat::format() 
                            + "</h2>"));
        });
        HXHttp::HXServer::Epoll ctx;
        auto ptr = HXHttp::HXServer::Acceptor::make();
        ptr->start("127.0.0.1", "28205");
        ctx.join();
    } catch(const std::system_error &e) {
        std::cerr << e.what() << '\n';
    }
    return 0;
}