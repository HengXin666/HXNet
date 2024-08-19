#include <HXWeb/protocol/websocket/WebSocket.h>

#include <hashlib/sha1.h>
#include <hashlib/base64.hpp>

#include <HXWeb/protocol/http/Response.h>
#include <HXSTL/coroutine/task/WhenAny.hpp>
#include <HXSTL/coroutine/loop/TimerLoop.h>
#include <HXSTL/coroutine/loop/IoUringLoop.h>
#include <HXSTL/utils/byteorder.hpp>
#include <HXSTL/tools/ErrorHandlingTools.h>

namespace HX { namespace web { namespace protocol { namespace websocket {

/**
 * @brief 官方要求的神秘仪式
 * @param userKey 用户发来的
 * @return std::string 
 */
inline std::string _webSocketSecretHash(std::string userKey) {
    // websocket 官方要求的神秘仪式
/**
 * Sec-WebSocket-Key是随机的字符串，服务器端会用这些数据来构造出一个SHA-1的信息摘要。
 * 把“Sec-WebSocket-Key”加上一个特殊字符串“258EAFA5-E914-47DA-95CA-C5AB0DC85B11”，
 * 然后计算SHA-1摘要，之后进行Base64编码，将结果做为“Sec-WebSocket-Accept”头的值，返回给客户端。
 * 如此操作，可以尽量避免普通HTTP请求被误认为Websocket协议。
 * By https://zh.wikipedia.org/wiki/WebSocket
 */
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
inline HX::STL::coroutine::task::Task<bool> _httpUpgradeToWebSocket(
    const HX::web::protocol::http::Request& req
) {
    auto& headMap = req.getRequestHeaders();
    if (auto it = headMap.find("upgrade"); it == headMap.end() || it->second != "websocket") {
        co_return false;
    }

    // 是 ws:// 请求
    auto wsKey = headMap.find("sec-websocket-key");
    if (wsKey == headMap.end()) {
        // 怎么会有这种错误?! 什么乐色客户端?!
        co_await req._responsePtr->setResponseLine(HX::web::protocol::http::Response::Status::CODE_400)
                    .setContentType("text/html", "UTF-8")
                    .setBodyData("Not Find: sec-websocket-key")
                    .send();
        co_return false;
    }

    auto wsNewKey = _webSocketSecretHash(wsKey->second);

    co_await req._responsePtr->setResponseLine(HX::web::protocol::http::Response::Status::CODE_101)
                .addHeader("connection", "Upgrade")
                .addHeader("upgrade", "websocket")
                .addHeader("sec-websocket-accept", wsNewKey)
                .setBodyData("")
                .send();
    co_return true;
    // https 的则是 wss:// ?!
}

HX::STL::coroutine::task::Task<WebSocket::pointer> WebSocket::makeServer(
    const HX::web::protocol::http::Request& req
) {
    if (co_await _httpUpgradeToWebSocket(req)) {
        co_return std::make_shared<pointer::element_type>(WebSocket {req});
    }
    co_return nullptr;
}

HX::STL::coroutine::task::Task<> WebSocket::getSpan(std::span<char> s) {
    co_await HX::STL::coroutine::loop::IoUringTask().prepRecv(
        _req._responsePtr->_fd, s, 0
    );
}

HX::STL::coroutine::task::Task<std::string> WebSocket::getN(std::size_t n) {
    std::string s;
    s.reserve(n);
    co_await HX::STL::coroutine::loop::IoUringTask().prepRecv(
        _req._responsePtr->_fd, s, n, 0
    );
    co_return s;
}

HX::STL::coroutine::task::Task<WebSocketPacket> WebSocket::recvPacket() {
    WebSocketPacket packet;
    std::string head = co_await getN(2);
    bool fin;
    do {
        uint8_t head0 = static_cast<uint8_t>(head[0]);
        uint8_t head1 = static_cast<uint8_t>(head[1]);
        fin = (head0 & 0x80) != 0;
        packet._opCode = static_cast<WebSocketPacket::OpCode>(head0 & 0x0F);
        bool masked = (head1 & 0x80) != 0;
        uint8_t payloadLen8 = head1 & 0x7F;
        size_t payloadLen;

        if (packet._opCode >= 8 && packet._opCode <= 10 && payloadLen8 >= 0x7E) [[unlikely]] {
            throw;
        }

        // 解析包的长度
        if (payloadLen8 == 0x7E) {
            uint16_t payloadLen16;
            co_await getSpan(std::span<char>(reinterpret_cast<char *>(&payloadLen16), sizeof(uint16_t)));
            payloadLen16 = HX::STL::utils::byteswapIfLittle(payloadLen16);
            payloadLen = static_cast<size_t>(payloadLen16);
        } else if (payloadLen8 == 0x7F) {
            uint64_t payloadLen64;
            co_await getSpan(std::span<char>(reinterpret_cast<char *>(&payloadLen64), sizeof(uint64_t)));
            payloadLen64 = HX::STL::utils::byteswapIfLittle(payloadLen64);
            payloadLen = static_cast<size_t>(payloadLen64);
            if constexpr (sizeof(uint64_t) > sizeof(size_t)) {
                if (payloadLen64 > std::numeric_limits<size_t>::max()) {
                    throw;
                }
            }
            payloadLen = static_cast<size_t>(payloadLen64);
        } else {
            payloadLen = static_cast<size_t>(payloadLen8);
        }

        std::string mask;
        if (masked) {
            mask = co_await getN(4);
        }
        auto data = co_await getN(payloadLen);
        if (masked) {
            const std::size_t len = data.size();
            for (std::size_t i = 0; i != len; ++i) {
                data[i] ^= mask[i % 4];
            }
        }
        packet._content += data;
    } while(!fin);
    co_return std::move(packet);
}

HX::STL::coroutine::task::Task<> WebSocket::sendPacket(
    WebSocketPacket packet,
    uint32_t mask /*= 0*/
) {
    std::string data;
    const bool fin = true;
    bool masked = mask != 0;
    uint8_t payloadLen8 = 0;

    if (packet._content.size() < 0x7E) {
        payloadLen8 = static_cast<uint8_t>(packet._content.size());
    } else if (packet._content.size() <= 0xFFFF) {
        payloadLen8 = 0x7E;
    } else {
        payloadLen8 = 0x7F;
    }

    uint8_t head0 = (fin ? 1 : 0) << 7 | static_cast<uint8_t>(packet._opCode);
    uint8_t head1 = (masked ? 1 : 0) << 7 | payloadLen8;
    char head[2];
    head[0] = static_cast<uint8_t>(head0);
    head[1] = static_cast<uint8_t>(head1);
    data.append(head, 2);

    if (packet._content.size() > 0x7E) {
        if (packet._content.size() <= 0xFFFF) {
            auto payloadLen16 = static_cast<uint16_t>(packet._content.size());
            payloadLen16 = HX::STL::utils::byteswapIfLittle(payloadLen16);
            std::span<char const> pLen(reinterpret_cast<char const *>(std::addressof(payloadLen16)), sizeof(uint16_t));
            data.append(pLen.data(), pLen.size());
        } else {
            auto payloadLen64 = static_cast<uint64_t>(packet._content.size());
            payloadLen64 = HX::STL::utils::byteswapIfLittle(payloadLen64);
            std::span<char const> pLen(reinterpret_cast<char const *>(std::addressof(payloadLen64)), sizeof(uint64_t));
            data.append(pLen.data(), pLen.size());
        }
    }

    if (masked) {
        char mask_buf[4];
        mask_buf[0] = mask >> 24;
        mask_buf[1] = (mask >> 16) & 0xFF;
        mask_buf[2] = (mask >> 8) & 0xFF;
        mask_buf[3] = mask & 0xFF;
        data.append(head, 4);

        const std::size_t len = packet._content.size();
        for (size_t i = 0; i != len; ++i) {
            packet._content[i] ^= mask_buf[i % 4];
        }
    }

    data += packet._content;

    std::string_view buf = packet._content;

    std::size_t n = HX::STL::tools::UringErrorHandlingTools::throwingError(
        co_await HX::STL::coroutine::loop::IoUringTask().prepSend(_req._responsePtr->_fd, buf, 0)
    ); // 已经写入的字节数

    while (true) {
        if (n == buf.size()) {
            // 全部写入啦
            break;
        }
        n = HX::STL::tools::UringErrorHandlingTools::throwingError(
            co_await HX::STL::coroutine::loop::IoUringTask().prepSend(
                _req._responsePtr->_fd, buf = buf.substr(n), 0
            )
        );
    }

    co_return;
}

HX::STL::coroutine::task::Task<> WebSocket::sendPing() {
    co_await sendPacket( WebSocketPacket {
        ._opCode = WebSocketPacket::Ping,
        ._content = {},
    });
}

HX::STL::coroutine::task::Task<> WebSocket::start(
    std::chrono::steady_clock::duration pingPongTimeout /*= std::chrono::seconds(5)*/
) {
    while (true) {
        auto maybePacket = co_await HX::STL::coroutine::task::WhenAny::whenAny(
            recvPacket(),
            HX::STL::coroutine::loop::TimerLoop::sleepFor(pingPongTimeout)
        );

        if (maybePacket.index()) { // index == 1
            // 主动Ping
            if (_waitingPong) { // 上次ping还没有回复我呢! 对面已经嘎啦!
                break;
            }
            co_await sendPing();
            _waitingPong = true;
            continue;
        }
        _waitingPong = false;

        auto packet = std::get<0>(maybePacket);

        switch (packet._opCode) {
        case WebSocketPacket::OpCode::Text: 
        case WebSocketPacket::OpCode::Binary: {
            // 收到消息
            if (_onMessage) {
                co_await _onMessage(packet._content);
            }
            break;
        }
        case WebSocketPacket::OpCode::Ping: {
            // 收到ping
            packet._opCode = WebSocketPacket::OpCode::Pong;
            co_await sendPacket(packet);
            break;
        }
        case WebSocketPacket::OpCode::Pong: {
            // 收到pong
            auto now = std::chrono::steady_clock::now();
            if (_onPong && _lastPingTime.time_since_epoch().count() != 0) {
                auto dt = now - _lastPingTime;
                co_await _onPong(dt);
            }
            break;
        }
        case WebSocketPacket::OpCode::Close: {
            // 收到关闭请求
            if (_onClose) {
                co_await _onClose();
            }

            if (_halfClosed) {
                co_return; // 退出, 由端点函数继续决定干什么
            }

            co_await sendPacket(packet);
            _halfClosed = true;
            break;
        }
        
        default:
            printf("未知code: %u\n", packet._opCode);
            break;
        }
    }
}

HX::STL::coroutine::task::Task<> WebSocket::send(const std::string& text) {
    if (_halfClosed) [[unlikely]] {
        co_return; // throw !!! ?
    }

    co_await sendPacket( WebSocketPacket {
        ._opCode = WebSocketPacket::Text,
        ._content = std::move(text),
    });
}

}}}} // HX::web::protocol::websocket