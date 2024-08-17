#include <iostream>
#include <HXWeb/client/Client.h>

/**
 * @brief 简单的客户端示例
 */

HX::STL::coroutine::task::Task<> startClient() {
    auto ptr = HX::web::client::Client::make();
    co_await ptr->start("www.baidu.com", "80");
    std::string s = "GET / HTTP/1.1\r\n\r\n";
    co_await ptr->write(s);
    auto str = co_await ptr->read();
    std::cout << str << '\n';
    co_await ptr->close();
}