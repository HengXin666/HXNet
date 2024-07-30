#include <HXWeb/HXHttp.h>

namespace HX::web {

} // namespace HXHttp

#include <HXWeb/router/Router.h>
#include <HXSTL/HXStringTools.h>
#include <HXWeb/protocol/http/Request.h>
#include <HXWeb/protocol/http/Response.h>
#include <HXprint/HXprint.h>
#include <HXWeb/HXServer.h>
#include <HXWeb/HXController.h>

#include <HXSTL/HXMagicEnum.h>

#include <iostream>
#include <unordered_map>
#include <csignal>
#include <cstring>

#include <HXSTL/HXCallback.h>
#include <HXSTL/HXRadixTree.h>

#include <HXWeb/router/RequestParsing.h>

int _xxx_main() {
    using func = std::function<void()>;
    HX::STL::HXRadixTree<func> tree;
    tree.insert({"awa", "qwq", "*"}, [](){
        printf("awa");
    });
    tree.insert({"awa", "0.0", "*"}, [](){
        printf("qwq");
    });
    (*tree.find({"awa", "0.0", "*"}))();
    if (!tree.find({"qwq", "0.0"}).has_value())
        printf("\n没有这个\n");
    return 0;
}

int _me_main() {
    enum MyEnum {
        LoLi = 1,
        imouto = 8,
    };
    std::cout << HX::STL::MagicEnum::getEnumName<MyEnum>(MyEnum::imouto) << '\n';
    std::cout << HX::STL::MagicEnum::nameFromEnum<MyEnum>("imouto") << '\n';
    return 0;
}

int main() {
    setlocale(LC_ALL, "zh_CN.UTF-8");
    try {
        HX::web::MywebController {};

        HX::web::Server::EpollContext ctx;
        auto ptr = HX::web::Server::Acceptor::make();
        ptr->start("127.0.0.1", "28205");
        ctx.join();
    } catch(const std::system_error &e) {
        std::cerr << e.what() << '\n';
    }
    return 0;
}