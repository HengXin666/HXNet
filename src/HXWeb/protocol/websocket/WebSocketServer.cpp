#include <HXWeb/protocol/websocket/WebSocketServer.h>

#include <hashlib/sha1.h>
#include <hashlib/base64.hpp>

#include <HXWeb/protocol/http/Response.h>

namespace HX { namespace web { namespace protocol { namespace websocket {

inline std::string websocketSecretHash(std::string userKey) {
    // websocket 官方要求的神秘仪式
    SHA1 sha1;
    std::string inKey = userKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    sha1.add(inKey.data(), inKey.size());
    uint8_t buf[SHA1::HashBytes];
    sha1.getHash(buf);
    return base64::encode_into<std::string>(buf, buf + SHA1::HashBytes);
}

/**
 * @brief 尝试升级为 WebSocket
 * @param req 
 * @return bool 是否升级成功
 */
inline HX::STL::coroutine::task::Task<bool> httpUpgradeToWebSocket(
    HX::web::protocol::http::Request& req
) {
    auto& headMap = req.getRequestHeaders();
    if (auto it = headMap.find("upgrade"); it == headMap.end() || it->second != "websocket") {
        co_return false;
    }

    // 是 ws:// 请求
    auto wsKey = headMap.find("sec-websocket-key");
    if (wsKey != headMap.end()) {
        // 怎么会有这种错误?! 什么乐色客户端?!
        co_return false;
    }

    auto wsNewKey = websocketSecretHash(wsKey->second);

    co_await req._responsePtr->setResponseLine(HX::web::protocol::http::Response::Status::CODE_101)
                    .addHeader("connection", "Upgrade")
                    .addHeader("upgrade", "websocket")
                    .addHeader("sec-websocket-accept", wsNewKey)
                    .setBodyData("")
                    .send();
                    

    // https 的则是 wss:// ?!
}

inline HX::STL::coroutine::task::Task<WebSocketServer::pointer> WebSocketServer::make(
    HX::web::protocol::http::Request& req
) {
    if (co_await httpUpgradeToWebSocket(req)) {
        co_return std::make_shared<pointer::element_type>();
    }
    co_return nullptr;
}

}}}} // HX::web::protocol::websocket