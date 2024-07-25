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

- [x] 重构epoll为基于回调函数的epoll事件循环
- [ ] 开发控制层, 封装简易可使用的端点
- [x] 处理TCP粘包问题 ~~(我连怎么检测出来都不知道...)~~
    - 一般是解析超大请求体才有的问题?!
- [x] 解析请求体 (通过`\r\n\r\n`)区分 (得造轮子了吗?)
- [x] 请求类-请求解析器
- [x] 修复问题: HTTP Headers 不区分大小写, 应该通用一下(使用小写)
- [ ] ~~改用epoll + `零拷贝`技术?~~
- [x] 修复问题: epoll的高并发时候, fd会被争抢: 原因: epoll的模式设置有问题, 应该是边沿模式: EPOLLIN 检查读缓冲区, + 非阻塞
- [x] 新增当前日期时间格式化工具类

- [ ] 路由映射的前置任务:
    - [x] 支持解析Http的请求行/头/体
    - [ ] 支持解析`Query`参数/`PATH`路径
        - [ ] 朴素指定: `/?loli=kawaii&imouto=takusann`
        - [ ] 通配指定: `/%d/%s`, 即支持任意如: `/114514/str` 格式
        - [ ] 骂人指定: `/fxxk/**`, 支持所有的如: `/fxxk/csdn`
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