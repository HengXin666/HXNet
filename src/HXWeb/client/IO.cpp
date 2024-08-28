#include <HXWeb/client/IO.h>

#include <HXSTL/coroutine/loop/AsyncLoop.h>
#include <HXWeb/protocol/http/Request.h>
#include <HXWeb/protocol/http/Response.h>
#include <HXSTL/tools/ErrorHandlingTools.h>

namespace HX { namespace web { namespace client {

HX::STL::coroutine::task::Task<bool> IO::_recvResponse(
        struct __kernel_timespec *timeout
) {
    std::size_t n = co_await recvN(_recvBuf, _recvBuf.size(), timeout); // 读取到的字节数
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
            n = co_await recvN(_recvBuf, std::min(size, _recvBuf.size()), timeout);
            continue;
        }
        break;
    }
    co_return false;
}

HX::STL::coroutine::task::Task<> IO::_sendRequest() const {
    // 本次响应使用结束, 清空, 复用
    _response->clear();
    // 生成请求字符串, 用于写入
    _request->createRequestBuffer();
    std::span<char> buf = _request->_buf;
    std::size_t n = HX::STL::tools::UringErrorHandlingTools::throwingError(
        co_await HX::STL::coroutine::loop::IoUringTask().prepSend(_fd, buf, 0)
    ); // 已经写入的字节数

    while (true) {
        if (n == buf.size()) {
            // 全部写入啦
            _request->clear();
            break;
        }
        n = HX::STL::tools::UringErrorHandlingTools::throwingError(
            co_await HX::STL::coroutine::loop::IoUringTask().prepSend(
                _fd, buf = buf.subspan(n), 0
            )
        );
    }
}

}}} // namespace HX::web::client