# HXNet
学习现代Cpp的代码存放库, Json解析, 万用print等

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
|-- LICENSE
|-- README.md
|-- build
`-- newCodeDir.sh
```

## 工具
我提供了以下的`.sh`脚本, 方便快速构建项目: ~~(我觉得下面的代码还可以复用一下qwq...)~~

- `newCodeDir.sh`: 构建子模块, 默认为静态库, 并且会创建一对cpp/h

- `newCodeHCPP.sh`: 构建子模块的cpp和h, 在上面的基础上创建一对cpp/h