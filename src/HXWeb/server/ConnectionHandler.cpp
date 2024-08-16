#include <HXWeb/server/ConnectionHandler.h>

#include <HXSTL/coroutine/loop/IoUringLoop.h>
#include <HXSTL/utils/StringUtils.h>
#include <HXSTL/tools/ErrorHandlingTools.h>
#include <HXWeb/router/Router.h>
#include <HXWeb/protocol/http/Request.h>
#include <HXWeb/protocol/http/Response.h>

namespace HX { namespace web { namespace server {

HX::STL::coroutine::task::TimerTask ConnectionHandler::start(int fd, std::chrono::seconds timeout) {
    HX::web::protocol::http::Request _request {};    // 客户端请求类
    HX::web::protocol::http::Response _response {};  // 服务端响应类
    std::vector<char> buff(protocol::http::Request::kBufSize);
    _request._responsePtr = &_response;

    // 连接超时
    auto _timeout = HX::STL::coroutine::loop::durationToKernelTimespec(timeout);

    while (true) {
        // === 读取 ===
        // LOG_INFO("读取中...");
        size_t n = std::max(co_await HX::STL::coroutine::loop::IoUringTask::linkOps(
            HX::STL::coroutine::loop::IoUringTask().prepRecv(fd, buff, 0),
            HX::STL::coroutine::loop::IoUringTask().prepLinkTimeout(&_timeout, 0)
        ), 0); // 读取到的字节数
        while (true) {
            if (n == 0) {
                // 断开连接
                LOG_INFO("客户端 %d 已断开连接!", fd);
                co_await HX::STL::coroutine::loop::IoUringTask().prepClose(fd);
                co_return;
            }

            // LOG_INFO("读取一次结束... (%llu)", n);
            if (std::size_t size = _request.parserRequest(
                HX::STL::container::ConstBytesBufferView {buff.data(), n}
            )) {
                // LOG_INFO("二次读取中..., 还需要读取 size = %llu", size);
                n = std::max(co_await HX::STL::coroutine::loop::IoUringTask::linkOps(
                    HX::STL::coroutine::loop::IoUringTask().prepRecv(fd, buff, size, 0),
                    HX::STL::coroutine::loop::IoUringTask().prepLinkTimeout(&_timeout, 0)
                ), 0);
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
        try {
            n = HX::STL::tools::UringErrorHandlingTools::throwingError(
                co_await HX::STL::coroutine::loop::IoUringTask().prepSend(fd, buf, 0)
            ); // 已经写入的字节数
            while (true) {
                if (n == buf.size()) {
                    // 全部写入啦
                    _response.clear();
                    break;
                }
                n = HX::STL::tools::UringErrorHandlingTools::throwingError(
                    co_await HX::STL::coroutine::loop::IoUringTask().prepSend(
                        fd, buf = buf.subspan(n), 0
                    )
                );
            }
        } catch(const std::exception& e) {
            LOG_ERROR("向客户端 %d 发送消息时候出错: %s", e.what());
            break;
        }
    }

    co_await HX::STL::coroutine::loop::IoUringTask().prepClose(fd);
}

}}} // namespace HX::web::server
