# HXNet
学习现代Cpp的代码存放库, 多线程epoll, http解析, Json解析, 万用print等

> 目标是写一个基于epoll + 多线程的 Web Http 后端

## 构建要求

- Linux 系统
- GCC 编译器
- C++20

## 目录结构

```sh
.
|-- CMakeLists.txt
|-- HXCodeTest # 测试代码
|-- HXJson     # Json解析库
|-- HXprint    # 万能print
|-- HXHttp     # web/http 相关的库, 如epoll
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

但, 类成员命名方式为`_name`

- 变量名: 几乎全部都是`驼峰`命名
- 常量: 全大写

> [!TIP]
> 缩进为`4空格`, 并且TAB按键请使用`4空格`而不是`\t`!

## 开发计划/日志
- [ ] 开发控制层, 封装简易可使用的端点
- [ ] 处理TCP粘包问题 ~~(我连怎么检测出来都不知道...)~~
- [ ] 解析请求体
- [ ] 请求类(得存放解析的内容吧?)
- [ ] 修复问题: HTTP Headers 不区分大小写, 应该通用一下(使用小写)