#include <iostream>

/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-31 12:13:49
 * @brief: 测试程序, 用于测试代码!
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * */

#include <functional>
#include <HXSTL/container/RadixTree.h>
/// @brief 压缩前缀树测试
void testRadixTree() {
    using func = std::function<void()>;
    HX::STL::container::RadixTree<func> tree;
    tree.insert({"awa", "qwq", "*"}, [](){
        printf("awa");
    });
    tree.insert({"awa", "0.0", "*"}, [](){
        printf("qwq");
    });
    (*tree.find({"awa", "0.0", "*"}))();
    if (!tree.find({"qwq", "0.0"}).has_value())
        printf("\n没有这个\n");
}

#include <HXSTL/utils/MagicEnum.h>
/// @brief 魔法枚举测试
void testMagicEnum() {
    enum MyEnum {
        LoLi = 1,
        imouto = 8,
    };
    std::cout << HX::STL::utils::MagicEnum::getEnumName<MyEnum>(MyEnum::imouto) << '\n';
    std::cout << HX::STL::utils::MagicEnum::nameFromEnum<MyEnum>("imouto") << '\n';
}

#include <HXWeb/HXApiHelper.h>
#include <HXSTL/utils/FileUtils.h>
#include <HXWeb/server/Acceptor.h>
#include <HXWeb/server/context/EpollContext.h>
#include <unistd.h>
/// @brief 服务端测试
void testServer() {
    ::chdir("../static");
    /// @brief 编写控制器(端点)
    class MywebController {

        ENDPOINT_BEGIN(API_GET, "/", root) {
            HX::web::protocol::http::Response response;
            response.setResponseLine(HX::web::protocol::http::Response::Status::CODE_200)
                .setContentType("text/html", "UTF-8")
                .setBodyData(HX::STL::utils::FileUtils::getFileContent("index.html"));
            return response;
        } ENDPOINT_END;

        ENDPOINT_BEGIN(API_GET, "/favicon.ico", faviconIco) {
            HX::web::protocol::http::Response response;
            response.setResponseLine(HX::web::protocol::http::Response::Status::CODE_200)
                .setContentType("image/x-icon")
                .setBodyData(HX::STL::utils::FileUtils::getFileContent("favicon.ico"));
            return response;
        } ENDPOINT_END;

        ENDPOINT_BEGIN(API_GET, "/op", op_fun_endpoint) {
            HX::web::protocol::http::Response response;
            response.setResponseLine(HX::web::protocol::http::Response::Status::CODE_200)
                .setContentType("text/html", "UTF-8")
                .setBodyData(execQueryHomeData());
            return response;
        } ENDPOINT_END;

        ENDPOINT_BEGIN(API_GET, "/awa/{id}", awa_fun) {
            START_PARSE_PATH_PARAMS;
            PARSE_PARAM(0, int32_t, id);
            HX::web::protocol::http::Response response;
            return std::move(HX::web::protocol::http::Response {}.setResponseLine(HX::web::protocol::http::Response::Status::CODE_200)
                    .setContentType("text/html", "UTF-8")
                    .setBodyData("<h1>/home/{id}/123 哇!</h1><h2>Now Time: " 
                                + HX::STL::utils::DateTimeFormat::formatWithMilli() 
                                + "</h2>"));
        } ENDPOINT_END;

        ENDPOINT_BEGIN(API_GET, "/qwq/**", qwq_fun) {
            PARSE_MULTI_LEVEL_PARAM(pathStr);
            GET_PARSE_QUERY_PARAMETERS(map);
            if (map.count("awa"))
                printf("awa -> %s\n", map["awa"].c_str());
            HX::web::protocol::http::Response response;
            return std::move(HX::web::protocol::http::Response {}.setResponseLine(HX::web::protocol::http::Response::Status::CODE_200)
                    .setContentType("text/html", "UTF-8")
                    .setBodyData("<h1>"+ pathStr +" 哇!</h1><h2>Now Time: " 
                                + HX::STL::utils::DateTimeFormat::formatWithMilli() 
                                + "</h2>"));
        } ENDPOINT_END;

    public:
        static std::string execQueryHomeData() {
            return "<h1>Heng_Xin ll 哇!</h1><h2>Now Time: " 
                    + HX::STL::utils::DateTimeFormat::format() 
                    + "</h2>";
        }
    };

    setlocale(LC_ALL, "zh_CN.UTF-8");
    try {
        ROUTER_BIND(MywebController);

        HX::web::server::context::EpollContext ctx;
        auto ptr = HX::web::server::Acceptor::make();
        ptr->start("0.0.0.0", "28205");
        ctx.join();
    } catch(const std::system_error &e) {
        std::cerr << e.what() << '\n';
    }
}

// int main() {
//     testServer();
//     return 0;
// }