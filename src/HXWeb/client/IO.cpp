#include <HXWeb/client/IO.h>

#include <HXSTL/coroutine/loop/AsyncLoop.h>
#include <HXSTL/coroutine/task/WhenAny.hpp>
#include <HXWeb/protocol/http/Request.h>
#include <HXWeb/protocol/http/Response.h>
#include <HXSTL/tools/ErrorHandlingTools.h>

namespace HX { namespace web { namespace client {

HX::STL::coroutine::task::Task<bool> IO<void>::recvResponse(
    std::chrono::milliseconds timeout
) {
    auto&& res = co_await HX::STL::coroutine::task::WhenAny::whenAny(
        HX::STL::coroutine::loop::TimerLoop::sleepFor(timeout),
        _recvResponse()
    );

    if (res.index())
        co_return false;
    co_return true;
}

HX::STL::coroutine::task::Task<> IO<void>::sendRequest() const {
    // 本次响应使用结束, 清空, 复用
    _response->clear();
    // 生成请求字符串, 用于写入
    _request->createRequestBuffer();
    co_await _sendRequest(_request->_buf);
    // 全部写入啦
    _request->clear();
}

HX::STL::coroutine::task::Task<bool> IO<HX::web::protocol::http::Http>::_recvResponse() {
    std::size_t n = co_await recvN(_recvBuf, _recvBuf.size());
    while (true) {
        if (n == 0) {
            // 断开连接
            co_return true;
        }

        // LOG_INFO("读取一次结束... (%llu)", n);
        if (std::size_t size = _response->parserResponse(
            std::span<char> {_recvBuf.data(), n}
        )) {
            // LOG_INFO("二次读取中..., 还需要读取 size = %llu", size);
            n = co_await recvN(_recvBuf, std::min(size, _recvBuf.size()));
            continue;
        }
        break;
    }
    co_return false;
}

HX::STL::coroutine::task::Task<> IO<HX::web::protocol::http::Http>::_sendRequest(
    std::span<char> buf
) const {
    std::size_t n = HX::STL::tools::UringErrorHandlingTools::throwingError(
        co_await HX::STL::coroutine::loop::IoUringTask().prepSend(_fd, buf, 0)
    ); // 已经写入的字节数

    while (true) {
        if (n == buf.size()) {
            co_return;
        }
        n = HX::STL::tools::UringErrorHandlingTools::throwingError(
            co_await HX::STL::coroutine::loop::IoUringTask().prepSend(
                _fd, buf = buf.subspan(n), 0
            )
        );
    }
}

HX::STL::coroutine::task::Task<bool> IO<HX::web::protocol::https::Https>::_recvResponse() {
    std::size_t n = co_await recvN(_recvBuf, _recvBuf.size());
    while (true) {
        if (n == 0) {
            // 断开连接
            co_return true;
        }

        // LOG_INFO("读取一次结束... (%llu)", n);
        if (std::size_t size = _response->parserResponse(
            std::span<char> {_recvBuf.data(), n}
        )) {
            // LOG_INFO("二次读取中..., 还需要读取 size = %llu", size);
            n = co_await recvN(_recvBuf, std::min(size, _recvBuf.size()));
            continue;
        }
        break;
    }
    co_return false;
}

HX::STL::coroutine::task::Task<> IO<HX::web::protocol::https::Https>::_sendRequest(
    std::span<char> buf
) const {
    std::size_t n = HX::STL::tools::UringErrorHandlingTools::throwingError(
        co_await HX::STL::coroutine::loop::IoUringTask().prepSend(_fd, buf, 0)
    ); // 已经写入的字节数

    while (true) {
        if (n == buf.size()) {
            co_return;
        }
        n = HX::STL::tools::UringErrorHandlingTools::throwingError(
            co_await HX::STL::coroutine::loop::IoUringTask().prepSend(
                _fd, buf = buf.subspan(n), 0
            )
        );
    }
}

}}} // namespace HX::web::client