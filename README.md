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
> Heng_Xin 自用

- 模块根文件夹使用驼峰(如`HXWeb`), 分类的子文件夹使用小写(如`tools`), 文件使用驼峰命名.
    - 其中纯头文件(一般里面只有模版等必须只能写在头文件的), 以`.hpp`结尾
    - 剩下的, 实现的代码可以放在`.cpp`里面的, 有`.h`配对
    - `.h`头文件里面应该尽量少的`#include`, 而尽量在`.cpp`里面引入

- #include 规范:
```cpp
#include <HXWeb/socket/AddressResolver.h> // .cpp 与 .h 配对 (然后空行)

#include <sys/types.h>  // 标准库/Linux提供的头文件 (.h在上, 无后缀的在下)
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <iostream>     // (然后空行)

#include <HXSTL/tools/ErrorHandlingTools.h> // 第三方库文件
```

- 命名空间规范: 按照文件夹来, 比如`/HXSTL/coroutine/loop/IoUringLoop.h`, 其中(`HXSTL`是模块根文件夹, 其意思是`HX::STL`)
```cpp
namespace HX { namespace STL { namespace coroutine { namespace loop {

}}}} // namespace HX::STL::coroutine::loop
```

- 类成员命名方式为`_name`(自用函数也是以`_`开头)
- 变量名: 几乎全部都是`驼峰`命名
- 枚举: 首字母大写的驼峰
- 常量: 全大写
- 类的构造函数: 请 **显式** 书写构造函数, 必要可以添加`explicit`关键字!, 而不是依赖`C++20`的自动隐式匹配生成!

> [!TIP]
> 缩进为`4空格`, 并且TAB按键请使用`4空格`而不是`\t`!

## 开发计划/日志

- 阶段性任务:
    - [x] 实现基于红黑树实现定时中断, 超时自动终止任务 (类似于Linux内核的“完全公平调度”)
    - [x] 用户自定义路由 | 控制器
    - [ ] 客户端
    - [x] 重构为基于协程的epoll
    - [x] 修改为使用 io_uring 驱动!!
        - [ ] 支持多线程并且不突出
    - [x] 实现文件的异步读写
    - [ ] 实现协程TimerLoop的对于ABCT任务(其中T是计时任务(?必须吗)), 任意一个完成, 则返回(打断其他), 这个功能

### 协程epoll服务端BUG汇总
1. [x] 读取数据的时候, 有时候无法读取到正确的数据 (某些值被换成了`\0`)
    - 解决方案: 使用`std::span<char>`和`std::vector<char>`配合, 而不是自制`buf`类, 它他妈居然又读取到?!?
2. [x] 无法正确的断开连接: 明明客户端已经关闭, 而服务端却没有反应 | 实际上`::Accept`已经重新复用那个已经关闭的套接字, 但是`co_await`读取, 它没有反应, 一直卡在那里!
    - 解决方案: `include/HXWeb/server/ConnectionHandler.h`实际上`make`创建的是智能指针, 而我们只是需要其协程, 故不需要其对象的成员, 导致`AsyncFile`无法因协程退出而析构
3. [x] 玄学的`include/HXSTL/coroutine/loop/EpollLoop.h`的`await_suspend`的`fd == -1`的问题, 可能和2有关?!?!
    - 离奇的修复啦?!
---
4. 在`AsyncFile::asyncRead`加入了`try`以处理`[ERROR]: 连接被对方重置`, 是否有非`try`的解决方案?!

5. 依旧不能很好的实现html基于轮询的聊天室, 我都怀疑是html+js的问题了...(明明和基于回调的事件循环差不多, 都是这个问题..)
    - 发现啦: http的请求体是不带`\0`作为终止的, 因此解析的时候使用C语言风格的字符串就导致解析失败(越界了)

- 协程版本: (基准: 别人22w/s的并发的程序在我这里一样的参数也就3w+/s..)

```sh
╰─ wrk -c1000 -d15s http://localhost:28205/ # WSL Arth 测试的 (感觉性能还没有跑到尽头)
Running 15s test @ http://localhost:28205/
  2 threads and 1000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    43.61ms   99.59ms   1.78s    98.21%
    Req/Sec    15.30k     0.96k   26.51k    87.96%
  455707 requests in 15.09s, 1.69GB read # 异步的文件读写 | 目前服务端还只是单线程
Requests/sec:  30190.88
Transfer/sec:    114.62MB
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