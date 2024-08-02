#include <unistd.h>

#include <HXWeb/HXApiHelper.h>
#include <HXSTL/utils/FileUtils.h>
#include <HXWeb/server/Acceptor.h>
#include <HXWeb/server/context/EpollContext.h>
#include <HXJson/HXJson.h>
#include <chrono>

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

HX::web::server::context::StopSource recv_timeout_stop = HX::web::server::context::StopSource::make();

class ChatController {

    ENDPOINT_BEGIN(API_GET, "/", root) {
        return req._responsePtr->setResponseLine(HX::web::protocol::http::Response::Status::CODE_200)
            .setContentType("text/html", "UTF-8")
            // .setBodyData("<h1>根目录</h1><h2>Now Time: " 
            //                     + HX::STL::utils::DateTimeFormat::formatWithMilli() 
            //                     + "</h2>");
            .setBodyData(HX::STL::utils::FileUtils::getFileContent("index.html"))
            .writeResponse(req);
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_GET, "/favicon.ico", faviconIco) {
        return req._responsePtr
            ->setResponseLine(HX::web::protocol::http::Response::Status::CODE_200)
            .setContentType("image/x-icon")
            .setBodyData(HX::STL::utils::FileUtils::getFileContent("favicon.ico"))
            .writeResponse(req);
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_POST, "/send", send) { // 客户端发送消息过来
        auto body = req.getRequesBody();
        auto jsonPair = HX::Json::parse(body);
        if (jsonPair.second) {
            messageArr.emplace_back(
                jsonPair.first.get<HX::Json::JsonDict>()["user"].get<std::string>(),
                jsonPair.first.get<HX::Json::JsonDict>()["content"].get<std::string>()
            );
            recv_timeout_stop.doRequestStop();
            recv_timeout_stop = HX::web::server::context::StopSource::make();
            printf("%s\n", Message::toJson(messageArr.begin(), messageArr.end()).c_str());
        }
        
        return req._responsePtr
            ->setResponseLine(HX::web::protocol::http::Response::Status::CODE_200)
            .setContentType("text/plain", "UTF-8")
            .writeResponse(req);
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_POST, "/recv", recv) { // 发送内容给客户端
        using namespace std::chrono_literals;

        auto body = req.getRequesBody();
        printf("recv (%s)\n", body.c_str());
        auto jsonPair = HX::Json::parse(body);

        if (jsonPair.second) {
            int len = jsonPair.first.get<HX::Json::JsonDict>()["first"].get<int>();
            printf("内容是: %d\n", len);
            if (len < (int)messageArr.size())
                return req._responsePtr->setResponseLine(HX::web::protocol::http::Response::Status::CODE_200)
                    .setContentType("text/plain", "UTF-8")
                    .setBodyData(Message::toJson(messageArr.begin() + len, messageArr.end()))
                    .writeResponse(req);
            else
                HX::web::server::context::EpollContext::get()._timer.setTimeout(3s, [&req, len] {
                    std::vector<Message> submessages;
                    printf("回复~\n");
                    return req._responsePtr
                        ->setResponseLine(HX::web::protocol::http::Response::Status::CODE_200)
                        .setContentType("text/plain", "UTF-8")
                        .setBodyData(Message::toJson(messageArr.begin() + len, messageArr.end()))
                        .writeResponse(req);
                }, recv_timeout_stop);
        }

        return req._responsePtr
            ->setResponseLine(HX::web::protocol::http::Response::Status::CODE_200)
            .setContentType("text/plain", "UTF-8")
            .setBodyData("[]")
            .writeResponse(req);
    } ENDPOINT_END;

public:

};

void startChatServer() {
    setlocale(LC_ALL, "zh_CN.UTF-8");
    ROUTER_BIND(ChatController);
    try {
        HX::web::server::context::EpollContext ctx;
        auto ptr = HX::web::server::Acceptor::make();
        ptr->start("0.0.0.0", "28205");
        ctx.join();
    } catch(const std::system_error &e) {
        std::cerr << e.what() << '\n';
    }
}


int main() {
    chdir("../static");
    startChatServer();
    return 0;
}