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

#include <HXSTL/HXCallback.h>
#include <HXSTL/HXRadixTree.h>

#include <HXHttp/HXRequestParsing.h>

int _xxx_main() {
    using func = std::function<void()>;
    HXSTL::HXRadixTree<func> tree;
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
    std::cout << HXSTL::MagicEnum::getEnumName<MyEnum>(MyEnum::imouto) << '\n';
    std::cout << HXSTL::MagicEnum::nameFromEnum<MyEnum>("imouto") << '\n';
    return 0;
}

int main() {
    setlocale(LC_ALL, "zh_CN.UTF-8");
    try {
        HXHttp::HXRouter::getSingleton().addController("GET", "/", [](const HXHttp::HXRequest& req) -> HXHttp::HXResponse {
            auto csMap = req.parseQueryParameters();
            for (const auto& [k, v] : csMap) {
                printf("%s = %s\n", k.c_str(), v.c_str());
            }
            return std::move(HXHttp::HXResponse {}.setResponseLine(HXHttp::HXResponse::Status::CODE_200)
                .setContentType("text/html", "UTF-8")
                .setBodyData("<h1>Hello, world!</h1><h2>Now Time: " 
                            + HXSTL::HXDateTimeFormat::formatWithMilli() 
                            + "</h2>"));
        });
        HXHttp::HXRouter::getSingleton().addController("GET", "/home/**", [](const HXHttp::HXRequest& req) -> HXHttp::HXResponse {
            static const auto UWPIndex = HXHttp::HXRequestParsing::getUniversalWildcardPathBeginIndex("/home/**");
            std::string cilPath = req.getPureRequesPath().substr(UWPIndex);
            printf("-> %s\n", cilPath.c_str());
            return std::move(HXHttp::HXResponse {}.setResponseLine(HXHttp::HXResponse::Status::CODE_200)
                .setContentType("text/html", "UTF-8")
                .setBodyData("<h1>这个是**吗</h1><h2>Now Time: " 
                            + HXSTL::HXDateTimeFormat::formatWithMilli() 
                            + "</h2>"));
        });
        HXHttp::HXRouter::getSingleton().addController("GET", "/home/{id}/123", [](const HXHttp::HXRequest& req) -> HXHttp::HXResponse {
            /**
             * 解析 {id}, 是通过什么呢, 用宏, 无需正则, 因为可以映射找到端点, 说明已经是满足条件的了
             * 因此, 只需要解析即可, 那么通过`/`分割然后使用相对位置提取即可
             * 特别的`**`则需要采用其他方法, 如 .find("/home/") 提取出后面的字符串
             */
            static const auto wildcarIndexArr = HXHttp::HXRequestParsing::getPathWildcardAnalysisArr("/home/{id}/123");
            auto pathSplitArr = HXSTL::HXStringUtil::split(req.getPureRequesPath(), "/");
            // 解析第一个通配符, 解析为 某 基础类型
            auto id = HXHttp::TypeInterpretation<bool>::wildcardElementTypeConversion(pathSplitArr[wildcarIndexArr[0]]);
            if (id)
                printf("%d\n", *id);
            return std::move(HXHttp::HXResponse {}.setResponseLine(HXHttp::HXResponse::Status::CODE_200)
                .setContentType("text/html", "UTF-8")
                .setBodyData("<h1>/home/{id}/123 哇!</h1><h2>Now Time: " 
                            + HXSTL::HXDateTimeFormat::formatWithMilli() 
                            + "</h2>"));
        });
        HXHttp::HXRouter::getSingleton().addController("GET", "/home/", [](const HXHttp::HXRequest& req) -> HXHttp::HXResponse {
            return std::move(HXHttp::HXResponse {}.setResponseLine(HXHttp::HXResponse::Status::CODE_200)
                .setContentType("text/html", "UTF-8")
                .setBodyData("<h1>/home/ 哇! 哥们好细!!</h1><h2>Now Time: " 
                            + HXSTL::HXDateTimeFormat::formatWithMilli() 
                            + "</h2>"));
        });
        HXHttp::HXRouter::getSingleton().addController("POST", "/", [](const HXHttp::HXRequest& req) -> HXHttp::HXResponse {
            return std::move(HXHttp::HXResponse {}.setResponseLine(HXHttp::HXResponse::Status::CODE_200)
                .setContentType("text/html", "UTF-8")
                .setBodyData("<h1>Hello, world! POSH老哥</h1><h2>Now Time: " 
                            + HXSTL::HXDateTimeFormat::formatWithMilli() 
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