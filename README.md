# HXNet
学习现代Cpp的代码存放库, 事件循环epoll, http解析, Json解析, 万用print等

> 目标是写一个基于epoll事件循环的 Web Http 后端

## 构建要求

- Linux 系统
- GCC 编译器
- C++20

## 目录结构

```sh
.
|-- CMakeLists.txt
|-- HXCodeTest # 测试代码 (请忽略)
|-- HXJson     # Json解析库
|-- HXprint    # 万能print
|-- HXHttp     # web/http 相关的库, 如epoll
|-- HXSTL      # 自己封装的回调函数/工具类/字节数组(视图)
|-- LICENSE
|-- README.md
|-- build
```

## 工具
我提供了以下的`.sh`脚本, 方便快速构建项目: ~~(我觉得下面的代码还可以复用一下qwq...)~~

- `newCodeDir.sh`: 构建子模块, 默认为静态库, 并且会创建一对cpp/h

- `newCodeHCPP.sh`: 构建子模块的cpp和h, 在上面的基础上创建一对cpp/h

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
    - [ ] 实现基于红黑树实现定时中断, 超时自动终止任务 (类似于Linux内核的“完全公平调度”)
    - [ ] 用户自定义路由 | 控制器
    - [ ] 客户端

---
- [ ] 编写API宏
    - [x] 端点快速定义宏
    - [x] 路径参数解析宏
- [x] 新增`enum`反射, 支持枚举值和字符串相互映射
- [x] 重构epoll为基于回调函数的epoll事件循环
- [x] 开发控制层, 封装简易可使用的端点
- [x] 处理TCP粘包问题 ~~(我连怎么检测出来都不知道...)~~
    - 一般是解析超大请求体才有的问题?!
- [x] 解析请求体 (通过`\r\n\r\n`)区分 (得造轮子了吗?)
- [x] 请求类-请求解析器
- [x] 修复问题: HTTP Headers 不区分大小写, 应该通用一下(使用小写)
- [x] 修复问题: epoll的高并发时候, fd会被争抢: 原因: epoll的模式设置有问题, 应该是边沿模式: EPOLLIN 检查读缓冲区, + 非阻塞
- [x] 新增当前日期时间格式化工具类

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

## 快速开始
> [!TIP]
> 还正在开发, 非最终产品...

- 端点定义 (初步示例)

```cpp
#include <HXHttp/HXApiHelper.h> // 头文件

// 定义端点, 会自动注册到路由
class MyWebController { // 控制器类

    ENDPOINT_BEGIN("GET", "/op", op_fun_endpoint) { // GET请求, 路径是 /op
        HXHttp::HXResponse response;
        // 控制器逻辑...
        response.setResponseLine(HXHttp::HXResponse::Status::CODE_200)
            .setContentType("text/html", "UTF-8")
            .setBodyData(execQueryHomeData()); // 可以调用 控制器(MyWebController) 的静态方法
        return response; // 返回响应
    } ENDPOINT_END;

    ENDPOINT_BEGIN("GET", "/awa/{id}", awa_fun) { // 路径是 /awa/%d
        START_PARSE_PATH_PARAMS;     // 开始解析路径参数
        PARSE_PARAM(0, int32_t, id); // 解析第一个通配符参数, 为int32_t类型, 命名为id
        HXHttp::HXResponse response;
        return response.setResponseLine(HXHttp::HXResponse::Status::CODE_200)
                .setContentType("text/html", "UTF-8")
                .setBodyData("<h1>/awa/{id} 哇!</h1><h2>Now Time: " 
                            + HXSTL::HXDateTimeFormat::formatWithMilli() 
                            + "</h2>");
    } ENDPOINT_END;

    ENDPOINT_BEGIN("GET", "/qwq/**", qwq_fun) { // 路径是 /qwq/** (多级任意, 如 /qwq/file/awa.jpg 这种)
        PARSE_MULTI_LEVEL_PARAM(pathStr); // 解析 ** 的内容, 到 std::string pathStr 中
        HXHttp::HXResponse response;
        return response.setResponseLine(HXHttp::HXResponse::Status::CODE_200)
                .setContentType("text/html", "UTF-8")
                .setBodyData("<h1>"+ pathStr +" 哇!</h1><h2>Now Time: " 
                            + HXSTL::HXDateTimeFormat::formatWithMilli() 
                            + "</h2>");
    } ENDPOINT_END;

public:
    static std::string execQueryHomeData() { // 可惜的是, 必须定义为静态成员函数 (我个人感觉这样没问题吧?~)
        return "<h1>Heng_Xin ll 哇!</h1><h2>Now Time: " 
                + HXSTL::HXDateTimeFormat::format() 
                + "</h2>";
    }
};
```

- 使用: 启动服务端

```cpp
#include <HXHttp/HXServer.h>
#include <HXHttp/HXController.h> // 假装导入头文件qwq..

int main() {
    // setlocale(LC_ALL, "zh_CN.UTF-8"); // 设置Linux错误提示本地化为中文
    try {
        HXHttp::MyWebController {}; // 这个只是暂时这样写qwq
        
        HXHttp::HXServer::Epoll ctx;
        auto ptr = HXHttp::HXServer::Acceptor::make();
        ptr->start("127.0.0.1", "28205"); // 绑定到 127.0.0.1:28205
        ctx.join();
    } catch(const std::system_error &e) {
        std::cerr << e.what() << '\n';
    }
    return 0;
}
```