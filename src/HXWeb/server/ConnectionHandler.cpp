#include <HXWeb/server/ConnectionHandler.h>

#include <HXWeb/router/Router.h>
#include <HXWeb/server/context/EpollContext.h>
#include <HXSTL/container/Callback.h>
#include <HXSTL/utils/StringUtils.h>

namespace HX { namespace web { namespace server {

HX::STL::coroutine::awaiter::Task<
    void,
    HX::STL::coroutine::awaiter::Promise<void>,
    HX::STL::coroutine::awaiter::ReturnToParentAwaiter<void, HX::STL::coroutine::awaiter::Promise<void>>
> ConnectionHandler::start(int fd) {
    _conn = AsyncFile {fd};
    _request._responsePtr = &_response;
    
    while (true) {
        // === 读取 ===
        LOG_INFO("%d 等待消息可读中...", fd);
        size_t n = co_await _conn.asyncRead(_buf, protocol::http::Request::BUF_SIZE); // 读取到的字节数
        while (true) {
            if (n == 0) {
                // 断开连接
                LOG_INFO("客户端已断开连接!");
                co_return;
            }

            if (std::size_t size = _request.parserRequest(HX::STL::container::ConstBytesBufferView {_buf.data(), n})) {
                n = co_await _conn.asyncRead(_buf, std::min(size, protocol::http::Request::BUF_SIZE));
                continue;
            }
            break;
        }

        // === 路由解析 ===
        // 交给路由处理
        auto fun = HX::web::router::Router::getSingleton().getEndpointFunc(_request.getRequesType(), _request.getRequesPath());
        printf("cli -> url: %s\n", _request.getRequesPath().c_str());
        if (fun) {
            co_await fun(_request);
        } else {
            _response.setResponseLine(HX::web::protocol::http::Response::Status::CODE_404)
                    .setContentType("text/html", "UTF-8")
                    .setBodyData("<h1>404 NOT FIND PATH: [" 
                        + _request.getRequesPath() 
                        + "]</h1><h2>Now Time: " 
                        + HX::STL::utils::DateTimeFormat::format() 
                        + "</h2>");
        }
        _response.createResponseBuffer();
        _request.clear(); // 本次请求使用结束, 清空, 复用

        // === 响应 ===
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
