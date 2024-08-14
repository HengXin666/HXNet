# HXNet
学习现代Cpp的代码存放库, 事件循环epoll, 基于压缩前缀树的路由, http解析, Json解析, 万用print等

> 目标是写一个基于epoll事件循环的 Web Http 后端

## 构建要求

- Linux 系统
- GCC 编译器
- C++20

## 快速开始
> [!TIP]
> 仍然在开发, 非最终产品
>
> 到时候应该使用宏再度封装 return req._responsePtr-> ... .writeResponse(req); 部分, 不过当前版本分支已被弃用, 因为我为了修复bug写了个协程的..然而bug并不是协程修改的部分引发..

- 编写端点
```cpp
#include <HXWeb/HXApiHelper.h> // 包含了控制器编写的宏

/// @brief 编写控制器(端点)
class MywebController {

    ENDPOINT_BEGIN(API_GET, "/", root) {
        return req._responsePtr->setResponseLine(HX::web::protocol::http::Response::Status::CODE_200)
            .setContentType("text/html", "UTF-8")
            // .setBodyData("<h1>根目录</h1><h2>Now Time: " 
            //                     + HX::STL::utils::DateTimeFormat::formatWithMilli() 
            //                     + "</h2>");
            .setBodyData(HX::STL::utils::FileUtils::getFileContent("index.html"))
            .writeResponse(req);
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_GET, "/awa/{id}", awa_fun) { // 支持解析路径参数
        START_PARSE_PATH_PARAMS;     // 开始解析路径参数
        PARSE_PARAM(0, int32_t, id); // 解析第一个路径参数{id}, 命名为id, 类型为 int32_t
        return req._responsePtr->setResponseLine(HX::web::protocol::http::Response::Status::CODE_200)
                .setContentType("text/html", "UTF-8")
                .setBodyData("<h1>awa/" + id + "</h1><h2>Now Time: " 
                                    + HX::STL::utils::DateTimeFormat::formatWithMilli() 
                                    + "</h2>");
                .writeResponse(req);
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_GET, "/qwq/**", qwq_fun) { // 支持解析多级通配符路径参数(/**这种)
        PARSE_MULTI_LEVEL_PARAM(pathStr); // 开始解析多级通配符路径参数, 并且解析得到string, 并且命名为 pathStr

        GET_PARSE_QUERY_PARAMETERS(map);  // 解析请求(如 /?awa=xxx 这种), 返回的是 unordered_map<string, string>
        if (map.count("awa"))
            printf("awa -> %s\n", map["awa"].c_str());
        return req._responsePtr->setResponseLine(HX::web::protocol::http::Response::Status::CODE_200)
                .setContentType("text/html", "UTF-8")
                .setBodyData("<h1>qwq/" + pathStr + "</h1><h2>Now Time: " 
                                    + HX::STL::utils::DateTimeFormat::formatWithMilli() 
                                    + "</h2>");
                .writeResponse(req);
    } ENDPOINT_END;

public:
    static std::string execQueryHomeData() { // 在端点内调用的函数应该是静态函数
        return "<h1>Heng_Xin ll 哇!</h1><h2>Now Time: " 
                + HX::STL::utils::DateTimeFormat::format() 
                + "</h2>";
    }
};
```

- 绑定控制器到全局路由
```cpp
#include <HXWeb/HXApiHelper.h> // 宏所在头文件

ROUTER_BIND(MywebController); // 这个类在上面声明过了
```

- 启动服务器, 并且监听 0.0.0.0:28205
```cpp
#include <HXWeb/server/Acceptor.h>
#include <HXWeb/server/context/EpollContext.h>

try { // TOOD: 到时候应该还会进一步封装这个东西...
    HX::web::server::context::EpollContext ctx;
    auto ptr = HX::web::server::Acceptor::make();
    ptr->start("0.0.0.0", "28205");
    ctx.join();
} catch(const std::system_error &e) {
    std::cerr << e.what() << '\n';
}
```

## 目录结构

```sh
. # 只说明头文件
|-- include
|   |-- HXJson
|   |   `-- HXJson.h # JSON字符串解析为cpp对象
|   |-- HXSTL
|   |   |-- HXSTL.h
|   |   |-- container
|   |   |   |-- BytesBuffer.h        # 字节数组
|   |   |   |-- Callback.h           # 回调函数(函数对象)
|   |   |   `-- RadixTree.h          # 基数树(压缩前缀树)
|   |   `-- tools
|   |       |-- ErrorHandlingTools.h # Linux 错误处理工具
|   |       |-- MagicEnum.h          # 魔法枚举: 支持反射到枚举值对应的字符串, 和通过字符串得到枚举值
|   |       `-- StringTools.h        # 字符串工具类(切分等) / 当前时间格式化到字符串工具类
|   |-- HXWeb
|   |   |-- HXApiHelper.h            # 为用户提供更好体验的API宏定义
|   |   |-- client                   # 客户端
|   |   |-- protocol                 # 协议
|   |   |   |-- http                 # HTTP协议
|   |   |   |   |-- Request.h        # 请求
|   |   |   |   `-- Response.h       # 响应
|   |   |   `-- websocket
|   |   |-- router
|   |   |   |-- RequestParsing.h     # 请求模版解析 (用于给路由解析初始化)
|   |   |   |-- RouteMapPrefixTree.h # 路由的压缩前缀树
|   |   |   `-- Router.h             # 单例 路由类
|   |   |-- server                   # 服务端
|   |   |   |-- Acceptor.h           # 接受连接类
|   |   |   |-- AsyncFile.h          # 文件异步操作类
|   |   |   |-- ConnectionHandler.h  # 连接消息处理类
|   |   |   `-- context
|   |   |       `-- EpollContext.h   # 单例 Epoll 上下文 + 基于红黑树の定时器
|   |   `-- socket
|   |       |-- AddressResolver.h    # 地址(适配器模式封装的LinuxAPI)
|   |       `-- FileDescriptor.h     # 文件描述符类
|   `-- HXprint
|       `-- HXprint.h                # 可打印 STL 容器 | 日志 函数
```

## 代码规范
> 几乎是谷歌C++规范

但, 类成员命名方式为`_name`(自用函数也是以`_`开头)

- 变量名: 几乎全部都是`驼峰`命名
- 枚举: 首字母大写的驼峰
- 常量: 全大写

> [!TIP]
> 缩进为`4空格`, 并且TAB按键请使用`4空格`而不是`\t`!

## 开发计划/日志

- 阶段性任务:
    - [x] 实现基于红黑树实现定时中断, 超时自动终止任务 (类似于Linux内核的“完全公平调度”)
    - [x] 用户自定义路由 | 控制器
    - [ ] 客户端

- 阶段性BUG:
    - [ ] 首次建立连接的延迟居然有300ms, 但是之后的响应才几ms, 而重新连接又是tm的300ms...
        - DE半天了, 找不到bug..
        - 奇怪的问题.. >> tmd 等我以后学了协程的, 重构你!

```sh
╰─ wrk -c200 -d30s http://localhost:28205 # wsl Arth Linux 渣机
Running 30s test @ http://localhost:28205
  2 threads and 200 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     6.25ms  609.54us  47.83ms   89.89%
    Req/Sec    16.08k   747.80    17.86k    69.33%
  959976 requests in 30.05s, 130.00MB read
Requests/sec:  31950.71
Transfer/sec:      4.33MB
```

---
- [ ] 编写API宏
    - [x] 端点快速定义宏
    - [x] 路径参数解析宏

- [ ] 路由映射的前置任务:
    - [x] 支持解析Http的请求行/头/体
    - [x] 支持解析`Query`参数/`PATH`路径 (未支持解析qwq!)
        - [x] 朴素指定: `/?loli=kawaii&imouto=takusann`
        - [x] 通配指定: `/{name}/{idStr}`, 即支持任意如: `/114514/str` 格式
            - 可解析为`bool`/`(u_)int(16、32、64)_t`/`float`/`(long) double`/`std::string`/`std::string_view`
        - [x] 骂人指定: `/fxxk/**`, 支持所有的如: `/fxxk/csdn`
        - 支持以上内容, 得需要使用字典树(前缀树), 应该是基数树(压缩前缀树) | 哈希表不能完全胜任

    - 解析规则: 
        1. 优先级: `静态URL` > `/{name}` > `/**`
        2. 自动忽略尾部多余的`/`, 如`/home`===`/home/`===`/home////////`
        3. 注: 不会被`GET`参数影响: 如`/?loli=kawaii&imouto=takusann`只会先解析`/`.

    - [ ] 支持解析`Body`请求体
        - [ ] none: 没有请求体
        - [ ] form-data: 格式如下:
```http
Content-Type: multipart/form-data; boundary=--------------------------004625337498508003962705
Content-Length: 167

----------------------------004625337498508003962705
Content-Disposition: form-data; name="awa"

hello Heng_Xin
----------------------------004625337498508003962705--
```
-   -   - [ ] urlencoded: 格式如下
```http
Content-Type: application/x-www-form-urlencoded
Content-Length: 91

awa=%E6%92%92%E5%A4%A7%E5%A3%B0%E5%9C%B0&qwq=%E6%81%AD%E5%96%9C%E5%8F%91%E8%B4%A2&0.0=hello
```
-   -   - [ ] raw: json/xml/html/...格式如下
```http
Content-Length: 186
content-type: application/json

{
    "sites": [
        HX_Github : "https://github.com/HengXin666/HXNet/tree/main"
    ]
}
```
-   - [ ] 解析凭证: (怎么搞? 在哪里? 饼干吗? 在请求头`Authorization`似乎可以)

-   - [x] 响应
        - [x] 响应码
        - [x] 响应头/行
        - [x] 响应体