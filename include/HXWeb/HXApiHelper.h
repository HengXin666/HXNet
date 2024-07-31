#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-27 22:42:16
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
#ifndef _HX_API_HELPER_H_
#define _HX_API_HELPER_H_

#include <HXWeb/router/Router.h>
#include <HXWeb/protocol/http/Request.h>
#include <HXWeb/protocol/http/Response.h>
#include <HXWeb/router/RequestParsing.h>
#include <HXSTL/tools/StringTools.h>

/* 简化用户编写的 API 宏 */

// 定义常用的请求类型
#define API_GET "GET"
#define API_POST "POST"
#define API_PUT "PUT"
#define API_DELETE "DELETE"

/**
 * @brief 定义一个端点, 其中定义了`req`请求信息(HX::web::protocol::http::Request)
 * @param METHOD 请求类型, 如"GET"
 * @param PATH 端点对应的路径, 如"/home/{id}"
 * @param FUNC_NAME 端点函数名称
 */
#define ENDPOINT_BEGIN(METHOD, PATH, FUNC_NAME) \
const int _HX_endpoint_##FUNC_NAME = []() -> int { \
    std::string templatePath = PATH; \
    HX::web::router::Router::getSingleton().addController(METHOD, templatePath, [=](const HX::web::protocol::http::Request& req) -> HX::web::protocol::http::Response

/**
 * @brief 结束端点的定义
 */
#define ENDPOINT_END \
    );\
    return 0;\
}()

/**
 * @brief 开始解析请求路径参数
 * @warning 接下来请使用`PARSE_PARAM`宏
 */
#define START_PARSE_PATH_PARAMS \
static const auto wildcarIndexArr = HX::web::router::RequestTemplateParsing::getPathWildcardAnalysisArr(templatePath); \
auto pathSplitArr = HX::STL::tools::StringUtil::split(req.getPureRequesPath(), "/")

/**
 * @brief 用于解析指定索引的路径参数, 并将其转换为指定类型的变量
 * @param INDEX 模版路径的第几个通配参数 (从`0`开始计算)
 * @param TYPE 需要解析成的类型, 如`bool`/`int32_t`/`double`/`std::string`/`std::string_view`等
 * @param NAME 变量名称
 * @warning 如果解析不到(出错), 则会直接返回错误给客户端
 */
#define PARSE_PARAM(INDEX, TYPE, NAME) \
auto NAME = HX::web::router::TypeInterpretation<TYPE>::wildcardElementTypeConversion(pathSplitArr[wildcarIndexArr[INDEX]]); \
if (!NAME) \
    return HX::web::protocol::http::Response{}.setResponseLine(HX::web::protocol::http::Response::Status::CODE_400).setContentType("application/json", "UTF-8").setBodyData("Missing PATH parameter '"#NAME"'")

/**
 * @brief 解析多级通配符的宏, 如 `/home/ **` 这种
 * @param NAME 解析结果字符串(`std::string`)的变量名称
 */
#define PARSE_MULTI_LEVEL_PARAM(NAME) \
static const auto UWPIndex = HX::web::router::RequestTemplateParsing::getUniversalWildcardPathBeginIndex(templatePath); \
std::string NAME = req.getPureRequesPath().substr(UWPIndex)

/**
 * @brief 解析查询参数键值对Map (解析如: `?name=loli&awa=ok&hitori`)
 * @param NAME 键值对Map的变量名
 */
#define GET_PARSE_QUERY_PARAMETERS(NAME) \
auto NAME = req.getParseQueryParameters()

/**
 * @brief 路由绑定, 将控制器绑定到路由
 * @param NAME 控制器类名
 */
#define ROUTER_BIND(NAME) \
{NAME {};}

#endif // _HX_API_HELPER_H_
