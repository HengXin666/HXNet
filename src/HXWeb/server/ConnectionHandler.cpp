#include <HXWeb/server/ConnectionHandler.h>

#include <HXSTL/container/Callback.h>
#include <HXSTL/utils/StringUtils.h>
#include <HXSTL/tools/ErrorHandlingTools.h>
#include <HXWeb/router/Router.h>
#include <HXWeb/protocol/http/Request.h>
#include <HXWeb/protocol/http/Response.h>

namespace HX { namespace web { namespace server {

HX::STL::coroutine::awaiter::TimerTask ConnectionHandler::start(int fd) {
    AsyncFile _conn {fd};

    HX::web::protocol::http::Request _request {};    // 客户端请求类

    HX::web::protocol::http::Response _response {};  // 服务端响应类

    _request._responsePtr = &_response;
    
    while (true) {
        // === 读取 ===
        std::vector<char> buff(protocol::http::Request::BUF_SIZE);
        // LOG_INFO("读取中...");
        size_t n = co_await _conn.asyncRead(buff, protocol::http::Request::BUF_SIZE); // 读取到的字节数
        while (true) {
            if (n == 0) {
                // 断开连接
                LOG_INFO("客户端 %d 已断开连接!", fd);
                co_return;
            }

            // LOG_INFO("读取一次结束... (%llu)", n);
            if (std::size_t size = _request.parserRequest(
                HX::STL::container::ConstBytesBufferView {buff.data(), n}
            )) {
                // LOG_INFO("二次读取中..., 还需要读取 size = %llu", size);
                n = co_await _conn.asyncRead(buff, std::min(size, protocol::http::Request::BUF_SIZE));
                continue;
            }
            break;
        }

        // === 路由解析 ===
        // 交给路由处理
        // LOG_INFO("路由解析中...");
        auto&& fun = HX::web::router::Router::getSingleton().getEndpointFunc(
            _request.getRequesType(), 
            _request.getRequesPath()
        );
        // printf("cli -> url: %s\n", _request.getRequesPath().c_str());
        if (fun) {
            co_await fun(_request);
        } else {
            _response.setResponseLine(HX::web::protocol::http::Response::Status::CODE_404)
                    .setContentType("text/html", "UTF-8")
                    .setBodyData("<h1>404 NOT FIND PATH: [" 
                        + _request.getRequesPath() 
                        + "]</h1><h2>Now Time: " 
                        + HX::STL::utils::DateTimeFormat::formatWithMilli() 
                        + "</h2>");
        }
        _response.createResponseBuffer();
        _request.clear(); // 本次请求使用结束, 清空, 复用

        // === 响应 ===
        // LOG_INFO("响应中...");
        HX::STL::container::ConstBytesBufferView buf = _response._buf;
        n = co_await _conn.asyncWrite(buf); // 已经写入的字节数
        while (true) {
            if (n == buf.size()) {
                // 全部写入啦
                _response.clear();
                break;
            }
            n = co_await _conn.asyncWrite(buf = buf.subspan(n));
        }
    }
}

}}} // namespace HX::web::server
