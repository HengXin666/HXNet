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
 * @brief 实现一个轮询的聊天室 By Http服务器
 * 
 * 分析一下: 客户端会定时请求, 参数为当前消息的数量
 * 
 * 服务端如果 [客户端当前消息的数量 >= 服务端消息池.size()], 
 *      则会返回null
 * 服务端如果 [客户端当前消息的数量 < 服务端消息池.size()], 
 *      则会返回 消息: index: 服务端消息池.begin() + 客户端当前消息的数量 ~ .end()
 * 
 * 困难点, json解析与封装 (得实现json的反射先...), 那可不行! 反正就那几个, 直接手动得了..
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

std::vector<Message> messageArr;
HX::STL::coroutine::loop::TriggerWaitLoop waitLoop {};

class ChatController {
    ENDPOINT_BEGIN(API_GET, "/", root) {
        RESPONSE_DATA(
            200,
            co_await HX::STL::utils::FileUtils::asyncGetFileContent("index.html"),
            "text/html", "UTF-8"
        );
        co_return true;
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_GET, "/favicon.ico", faviconIco) {
        RESPONSE_DATA(
            200, 
            co_await HX::STL::utils::FileUtils::asyncGetFileContent("favicon.ico"),
            "image/x-icon"
        );
        co_return true;
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_GET, "/files/**", files) {
        PARSE_MULTI_LEVEL_PARAM(path);
        RESPONSE_STATUS(200).setContentType("text/html", "UTF-8")
                            .setBodyData("<h1> files URL is " + path + "</h1>"); // 支持直接在端点里面响应 (记得co_await)
                                     // 响应后, 不会再次在 ConnectionHandler 中再次响应!
        // 多次写回无效! (不懂... 即没有失败, 客户端也没有收到...)
        // co_await RESPONSE_STATUS(200).setContentType("text/html", "UTF-8")
        //                     .setBodyData("<h1> files URL iiiis " + path + "</h1>")
        //                     .send();
        co_return true;
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_GET, "/home/{id}/{name}", getIdAndNameByHome) {
        START_PARSE_PATH_PARAMS; // 开始解析请求路径参数
        PARSE_PARAM(0, u_int32_t, id, false);
        PARSE_PARAM(1, std::string, name);

        // 解析查询参数为键值对; ?awa=xxx 这种
        GET_PARSE_QUERY_PARAMETERS(queryMap);

        if (queryMap.count("loli")) // 如果解析到 ?loli=xxx
            std::cout << queryMap["loli"] << '\n'; // xxx 的值

        RESPONSE_DATA(
            200, 
            "<h1> Home id 是 " 
            + std::to_string(*id) 
            + ", 而名字是 " 
            + *name 
            + "</h1><h2> 来自 URL: " 
            + io.getRequest().getRequesPath() 
            + " 的解析</h2>",
            "text/html", "UTF-8"
        );
        co_return true;
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_POST, "/send", send) { // 客户端发送消息过来
        auto body = io.getRequest().getRequesBody();
        auto jsonPair = HX::Json::parse(body);
        if (jsonPair.second) {
            messageArr.emplace_back(
                jsonPair.first.get<HX::Json::JsonDict>()["user"].get<std::string>(),
                jsonPair.first.get<HX::Json::JsonDict>()["content"].get<std::string>()
            );
            // printf("%s\n", Message::toJson(messageArr.begin(), messageArr.end()).c_str());
            waitLoop.runAllTask();
        } else {
            printf("解析客户端出错\n");
        }
        
        RESPONSE_STATUS(200).setContentType("text/plain", "UTF-8");
        co_return true;
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_POST, "/recv", recv) { // 发送内容给客户端
        using namespace std::chrono_literals;

        auto body = io.getRequest().getRequesBody();
        // printf("recv (%s)\n", body.c_str());
        auto jsonPair = HX::Json::parse(body);

        if (jsonPair.second) {
            int len = jsonPair.first.get<HX::Json::JsonDict>()["first"].get<int>();
            // printf("内容是: %d\n", len);
            if (len < (int)messageArr.size()) {
                // printf("立马回复, 是新消息~\n");
                RESPONSE_DATA(
                    200,
                    Message::toJson(messageArr.begin() + len, messageArr.end()),
                    "text/plain", "UTF-8"
                );
                co_return true;
            }
            else {
                // printf("等我3秒~\n");
                co_await HX::STL::coroutine::task::WhenAny::whenAny(
                    HX::STL::coroutine::loop::TriggerWaitLoop::triggerWait(waitLoop),
                    HX::STL::coroutine::loop::TimerLoop::sleepFor(3s)
                );
                std::vector<Message> submessages;
                // printf("3秒之期已到, 马上回复~\n");
                RESPONSE_DATA(
                    200,
                    Message::toJson(messageArr.begin() + len, messageArr.end()),
                    "text/plain", "UTF-8"
                );
                co_return true;
            }
        } else {
            printf("啥也没有...\n");
        }
        co_return true;
    } ENDPOINT_END;

public: // 控制器成员函数 (请写成`static`方法)

};

HX::STL::coroutine::task::Task<> startChatServer() {
    messageArr.emplace_back("系统", "欢迎来到聊天室!");
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
        startChatServer()
    );
    return 0;
}