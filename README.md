# HXNet
学习现代Cpp的代码存放库, io_uring + 协程的http服务器, 基于压缩前缀树的路由, http解析, Json解析, 万用print等

## 构建要求

- Linux 系统
- GCC 编译器
- C++20

## 快速开始
> [!TIP]
> 仍然在开发, 非最终产品

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
        co_return; // 注意, 端点函数是协程, 得使用 co_return 而不是return (返回值是 void)
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_GET, "/favicon.ico", faviconIco) {
        RESPONSE_DATA(
            200, 
            co_await HX::STL::utils::FileUtils::asyncGetFileContent("favicon.ico"),
            "image/x-icon" // 响应编码 可以不写
        );
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_GET, "/files/**", files) { // 支持通配符路径
        PARSE_MULTI_LEVEL_PARAM(path); // 解析通配符 (**)
        // 另一种响应宏, 只会设置响应编码, 但是返回的是 Response &, 可以链式调用
        RESPONSE_STATUS(200).setContentType("text/html", "UTF-8")
                            .setBodyData("<h1> files URL is " + path + "</h1>");
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_GET, "/home/{id}/{name}", getIdAndNameByHome) {
        START_PARSE_PATH_PARAMS; // 开始解析请求路径参数
        PARSE_PARAM(0, u_int32_t, id);     // 解析第一个路径参数{id}, 解析为 u_int32_t类型, 命名为 id
        PARSE_PARAM(1, std::string, name); // 解析第二个路径参数{name}

        // 解析查询参数为键值对; ?awa=xxx 这种
        GET_PARSE_QUERY_PARAMETERS(queryMap);

        if (queryMap.count("loli")) // 如果解析到 ?loli=xxx
            std::cout << queryMap["loli"] << '\n'; // xxx 的值

        RESPONSE_DATA(
            200, 
            "<h1> Home id 是 " + std::to_string(*id) + ", 而名字是 " 
            + *name + "</h1><h2> 来自 URL: " 
            + req.getRequesPath() + " 的解析</h2>", // 默认`ENDPOINT_BEGIN`会传入 Request& req, 您可以对其进行更细致的操作
            "text/html", "UTF-8"
        );
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

- 启动服务器, 并且监听 0.0.0.0:28205
```cpp
#include <HXSTL/coroutine/loop/AsyncLoop.h>
#include <HXWeb/server/Acceptor.h>

HX::STL::coroutine::task::Task<> startChatServer() {
    ROUTER_BIND(MyWebController); // 绑定端点控制器到路由
    try {
        auto ptr = HX::web::server::Acceptor::make();
        co_await ptr->start("0.0.0.0", "28205");
    } catch(const std::system_error &e) {
        std::cerr << e.what() << '\n';
    }
    co_return;
}

int main() {
    chdir("../static"); // 移动当前工作目录为这个
    HX::STL::coroutine::awaiter::runTask( // 启动协程任务
        HX::STL::coroutine::loop::AsyncLoop::getLoop(), 
        startChatServer()
    );
    return 0;
}
```

## 相关依赖

|依赖库|说明|备注|
|---|---|---|
|liburing|io_uring|https://github.com/axboe/liburing|

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
╰─ wrk -c500 -d15s http://localhost:28205/ # WSL Arth 测试的 (感觉性能还没有跑到尽头 (cpu: wrk + 本程序 才 24%左右的占用..))
Running 15s test @ http://localhost:28205/
  2 threads and 500 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    17.37ms   21.60ms 614.42ms   99.26%
    Req/Sec    15.73k     0.88k   17.68k    75.67%
  469885 requests in 15.08s, 1.74GB read
Requests/sec:  31154.81
Transfer/sec:    118.28MB
```
