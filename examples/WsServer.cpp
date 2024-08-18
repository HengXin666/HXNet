#include <iostream>
#include <HXWeb/HXApiHelper.h>
#include <HXSTL/utils/FileUtils.h>
#include <HXSTL/coroutine/loop/AsyncLoop.h>
#include <HXWeb/server/Acceptor.h>
#include <HXJson/HXJson.h>
#include <HXSTL/coroutine/task/WhenAny.hpp>
#include <HXSTL/coroutine/loop/TriggerWaitLoop.h>

using namespace std::chrono;

/**
 * @brief 实现一个websocket的聊天室 By Http服务器
 */

struct Message {
    std::string _user;
    std::string _content;

    explicit Message(
        std::string user, 
        std::string content
    ) : _user(user)
      , _content(content)
    {}

    static std::string toJson(
        std::vector<Message>::iterator beginIt, 
        std::vector<Message>::iterator endIt
    ) {
        std::string res = "[";
        if (beginIt != endIt) {
            for (; beginIt != endIt; ++beginIt)
                res += "{\"user\":\""+ beginIt->_user + "\",\"content\":\"" + beginIt->_content  +"\"},";
            res.pop_back();
        }
        res += "]";
        return res;
    }
};

std::vector<Message> wsMessageArr;
HX::STL::coroutine::loop::TriggerWaitLoop wsWaitLoop {};

class ChatController {
    ENDPOINT_BEGIN(API_GET, "/favicon.ico", faviconIco) {
        RESPONSE_DATA(
            200, 
            co_await HX::STL::utils::FileUtils::asyncGetFileContent("favicon.ico"),
            "image/x-icon"
        );
        co_return true;
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_GET, "/", root) {
        RESPONSE_DATA(
            200,
            co_await HX::STL::utils::FileUtils::asyncGetFileContent("index.html"),
            "text/html", "UTF-8"
        );
        co_return true;
    } ENDPOINT_END;
};

HX::STL::coroutine::task::Task<> startWsChatServer() {
    wsMessageArr.emplace_back("系统", "欢迎来到聊天室!");
    ROUTER_BIND(ChatController);
    try {
        auto ptr = HX::web::server::Acceptor::make();
        co_await ptr->start("0.0.0.0", "28205", 10s);
    } catch(const std::system_error &e) {
        std::cerr << e.what() << '\n';
    }
    co_return;
}

int _main() {
    chdir("../static");
    setlocale(LC_ALL, "zh_CN.UTF-8");
    HX::STL::coroutine::task::runTask(
        HX::STL::coroutine::loop::AsyncLoop::getLoop(), 
        startWsChatServer()
    );
    return 0;
}