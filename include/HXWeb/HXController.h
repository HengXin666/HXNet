#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-19 15:14:12
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
#ifndef _HX_HXCONTROLLER_H_
#define _HX_HXCONTROLLER_H_

#include <HXWeb/HXApiHelper.h>

namespace HX::web {

/**
 * @brief 控制器类
 */
// class Controller {

// };

/// @brief 测试使用的
class MywebController {

    ENDPOINT_BEGIN(R_GET, "/", root) {
        HX::web::protocol::http::Response response;
        response.setResponseLine(HX::web::protocol::http::Response::Status::CODE_200)
            .setContentType("text/html", "UTF-8")
            .setBodyData("<h1>这里是根目录!</h1><h2>Now Time: " 
                + HX::STL::HXDateTimeFormat::format() 
                + "</h2>");
        return response;
    } ENDPOINT_END;

    ENDPOINT_BEGIN(R_GET, "/op", op_fun_endpoint) {
        HX::web::protocol::http::Response response;
        response.setResponseLine(HX::web::protocol::http::Response::Status::CODE_200)
            .setContentType("text/html", "UTF-8")
            .setBodyData(execQueryHomeData());
        return response;
    } ENDPOINT_END;

    ENDPOINT_BEGIN(R_GET, "/awa/{id}", awa_fun) {
        START_PARSE_PATH_PARAMS;
        PARSE_PARAM(0, int32_t, id);
        HX::web::protocol::http::Response response;
        return std::move(HX::web::protocol::http::Response {}.setResponseLine(HX::web::protocol::http::Response::Status::CODE_200)
                .setContentType("text/html", "UTF-8")
                .setBodyData("<h1>/home/{id}/123 哇!</h1><h2>Now Time: " 
                            + HX::STL::HXDateTimeFormat::formatWithMilli() 
                            + "</h2>"));
    } ENDPOINT_END;

    ENDPOINT_BEGIN(R_GET, "/qwq/**", qwq_fun) {
        PARSE_MULTI_LEVEL_PARAM(pathStr);
        GET_PARSE_QUERY_PARAMETERS(map);
        if (map.count("awa"))
            printf("awa -> %s\n", map["awa"].c_str());
        HX::web::protocol::http::Response response;
        return std::move(HX::web::protocol::http::Response {}.setResponseLine(HX::web::protocol::http::Response::Status::CODE_200)
                .setContentType("text/html", "UTF-8")
                .setBodyData("<h1>"+ pathStr +" 哇!</h1><h2>Now Time: " 
                            + HX::STL::HXDateTimeFormat::formatWithMilli() 
                            + "</h2>"));
    } ENDPOINT_END;

public:
    static std::string execQueryHomeData() {
        return "<h1>Heng_Xin ll 哇!</h1><h2>Now Time: " 
                + HX::STL::HXDateTimeFormat::format() 
                + "</h2>";
    }
};

} // namespace HXHttp

#endif // _HX_HXCONTROLLER_H_
