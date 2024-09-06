<h1 align="center" style="color:yellow">HXNet</h1>

> [!NOTE]
> *Heng_Xin学习现代Cpp的代码存放库,* 包含以下内容:

- 基于`io_uring`+协程的`http/https`服务器, 基于压缩前缀树的路由, 支持`http/https`解析, `WebSocket`协议, 支持`Transfer-Encoding`分块编码传输文件.

- 客户端提供了简单的协程的`request`方法(API), 并且支持使用`socks5`代理. 支持`http/https`, 以及解析`Transfer-Encoding`分块编码的响应体

- `Json`解析, 万用`print`等; 

## 构建要求

- Linux 5.1+ 
- GCC 编译器 (推荐)
- C++20

## 快速开始
> [!TIP]
> 仍然在开发, 非最终产品
>
> - 其他示例:
>   - [基于轮询的聊天室](examples/ChatServer.cpp)
>   - [WebSocket服务端](examples/WsServer.cpp)
>   - [使用`Transfer-Encoding`分块编码传输文件的服务端](examples/HttpsFileServer.cpp)
>   - [支持`socks5`代理的`Http/Https`客户端](examples/Client.cpp)

- 编写端点 (提供了简化使用的 API 宏)
```cpp
#include <HXWeb/HXApiHelper.h> // 使用简化的api

// 用户定义的控制器
class MyWebController {

    // 定义端点函数
    ENDPOINT_BEGIN(API_GET, "/", root) { // 注册get请求, 接收`/`路径请求
        RESPONSE_DATA( // 响应数据
            200, // 状态码
            co_await HX::STL::utils::FileUtils::asyncGetFileContent("index.html"), // (body数据) 异步(协程)读取文件
            "text/html", "UTF-8" // (响应类型), 以及响应编码
        );
        co_return true; // 注意, 端点函数是协程, 得使用 co_return 而不是return (返回值是 bool)
                        // bool 的意思是是否复用连接 (HTTP/1.1 是推荐复用连接的)
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_GET, "/favicon.ico", faviconIco) {
        RESPONSE_DATA(
            200, 
            co_await HX::STL::utils::FileUtils::asyncGetFileContent("favicon.ico"),
            "image/x-icon" // 响应编码 可以不写
        );
        co_return true;
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_GET, "/files/**", files) {
        PARSE_MULTI_LEVEL_PARAM(path);
        RESPONSE_STATUS(200).setContentType("text/html", "UTF-8")
                            .setBodyData("<h1> files URL is " + path + "</h1>"); 
        // 支持直接在端点里面响应 (记得co_await)
        // 响应后, 不会再次在 ConnectionHandler 中再次响应!
        co_await io.sendResponse(); // 立即发送响应
        co_return true;
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_GET, "/home/{id}/{name}", getIdAndNameByHome) {
        START_PARSE_PATH_PARAMS; // 开始解析请求路径参数
        PARSE_PARAM(0, u_int32_t, id, false); // 解析第一个路径参数{id}, 解析为 u_int32_t类型, 命名为 id
                                              // 并且如果解析失败则不复用连接 (false)

        PARSE_PARAM(1, std::string, name);    // 解析第二个路径参数{name} (不写, 则默认复用连接)

        // 解析查询参数为键值对; ?awa=xxx 这种
        GET_PARSE_QUERY_PARAMETERS(queryMap);

        if (queryMap.count("loli")) // 如果解析到 ?loli=xxx
            std::cout << queryMap["loli"] << '\n'; // xxx 的值

        RESPONSE_DATA(
            200, 
            "<h1> Home id 是 " + std::to_string(*id) + ", 而名字是 " 
            + *name + "</h1><h2> 来自 URL: " 
            + io.getRequest().getRequesPath()  + " 的解析</h2>", // 默认`ENDPOINT_BEGIN`会传入 const HX::web::server::IO<>& io, 您可以对其进行更细致的操作
            "text/html", "UTF-8"
        );
        co_return true;
    } ENDPOINT_END;

public: // 控制器成员函数 (请写成`static`方法)
    // todo...
};
```

- 绑定控制器到全局路由
```cpp
#include <HXWeb/HXApiHelper.h> // 宏所在头文件

ROUTER_BIND(MyWebController); // 这个类在上面声明过了
```

- 启动服务器, 并且监听 0.0.0.0:28205, 并且设置路由失败时候返回的界面
    - 可选: 可以设置线程数和超时时间 | 每个线程独享一个`uring`, 但是绑定同一个端口, 由操作系统进行负载均衡
```cpp
#include <HXWeb/HXApiHelper.h> // 宏所在头文件
#include <HXWeb/server/Server.h>

int main() {
    chdir("../static");
    setlocale(LC_ALL, "zh_CN.UTF-8");
    ROUTER_BIND(WSChatController);
    ERROR_ENDPOINT_BEGIN { // 自定义: 设置路由失败时候返回的界面
        RESPONSE_DATA(
            404,
            "<!DOCTYPE html><html><head><meta charset=UTF-8><title>404 Not Found</title><style>body{font-family:Arial,sans-serif;text-align:center;padding:50px;background-color:#f4f4f4}h1{font-size:100px;margin:0;color:#333}p{font-size:24px;color:red}</style><body><h1>404</h1><p>Not Found</p><hr/><p>HXNet</p>",
            "text/html", "UTF-8"
        );
        co_return false;
    } ERROR_ENDPOINT_END;

    // 启动Http服务 [阻塞于此]
    HX::web::server::Server::startHttp("0.0.0.0", "28205", 16 /*可选 线程数(互不相关)*/, 10s /*可选 超时时间*/);

    // 或者, 启动Https服务 [阻塞于此], 需要提供证书和密钥
    HX::web::server::Server::startHttps("0.0.0.0", "28205", "certs/cert.pem", "certs/key.pem");
    return 0;
}
```

## 相关依赖

|依赖库|说明|备注|
|---|---|---|
|liburing|io_uring的封装|https://github.com/axboe/liburing|
|hashlib|用于`WebSocket`构造`SHA-1`信息摘要; 以及进行`Base64`编码|https://create.stephan-brumme.com/hash-library/|
|OpenSSL 3.3.1|用于https的证书/握手|https://github.com/openssl/openssl|

## 代码规范
> --> [C++ 编码规范](documents/CodingStandards/CppStyle.md)

## 开发计划
> --> [开发计划](documents/DevelopmentPlan.md)

## 开发日志
> --> [开发日志](documents/DevelopmentLog.md)

## 性能测试
> [!TIP]
> - 协程版本: (基准: 别人22w/s的并发的程序在我这里一样的参数也就3w+/s..)

```sh
# 读写 index.html
╰─ wrk -c1000 -d15s http://localhost:28205/
Running 15s test @ http://localhost:28205/
  2 threads and 1000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    10.91ms    5.76ms  73.87ms   77.52%
    Req/Sec    38.43k     8.84k   67.78k    69.86%
  1137056 requests in 15.06s, 3.53GB read
Requests/sec:  75521.83
Transfer/sec:    239.91MB

# 没有文件读写
╰─ wrk -c1000 -d15s http://localhost:28205/home/123/123
Running 15s test @ http://localhost:28205/home/123/123
  2 threads and 1000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     5.30ms    3.60ms  40.93ms   76.85%
    Req/Sec    71.23k    22.11k  132.67k    68.15%
  2097727 requests in 15.09s, 412.11MB read
Requests/sec: 139024.19
Transfer/sec:     27.31MB
```
