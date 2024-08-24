#include <HXWeb/server/IO.h>

#include <HXSTL/coroutine/loop/AsyncLoop.h>
#include <HXSTL/coroutine/task/TimerTask.hpp>
#include <HXWeb/protocol/http/Request.h>
#include <HXWeb/protocol/http/Response.h>
#include <HXSTL/tools/ErrorHandlingTools.h>
#include <HXSTL/utils/FileUtils.h>

namespace HX { namespace web { namespace server {

IO::IO(int fd) 
    : _fd(fd)
    , _recvBuf(HX::STL::utils::FileUtils::kBufMaxSize)
    , _request(std::make_unique<HX::web::protocol::http::Request>())
    , _response(std::make_unique<HX::web::protocol::http::Response>())
{}

inline static HX::STL::coroutine::task::TimerTask close(int fd) {
    co_await HX::STL::coroutine::loop::IoUringTask().prepClose(fd);
}

IO::~IO() noexcept {
    // 添加到事件循环, 虽然略微延迟释放fd, 但是RAII呀~
    HX::STL::coroutine::loop::AsyncLoop::getLoop().getTimerLoop().addTimer(
        std::chrono::system_clock::now(),
        nullptr,
        std::make_shared<HX::STL::coroutine::task::TimerTask>(
            close(_fd)
        )
    );
}

HX::STL::coroutine::task::Task<> IO::sendResponse() const {
    co_await _send();
    ++_response->_sendCnt;
}

HX::STL::coroutine::task::Task<> IO::sendResponse(HX::STL::container::NonVoidHelper<>) {
    if (_response->_sendCnt) { // 已经发送过响应了
        _response->_sendCnt = 0;
        co_return;
    }
    co_await _send();
}

HX::STL::coroutine::task::Task<bool> IO::recvRequest(
    struct __kernel_timespec *timeout
) {
    size_t n = co_await recvN(_recvBuf, _recvBuf.size(), timeout); // 读取到的字节数
    while (true) {
        if (n == 0) {
            // 断开连接
            co_return true;
        }

        // LOG_INFO("读取一次结束... (%llu)", n);
        if (std::size_t size = _request->parserRequest(
            HX::STL::container::ConstBytesBufferView {_recvBuf.data(), n}
        )) {
            // LOG_INFO("二次读取中..., 还需要读取 size = %llu", size);
            n = co_await recvN(_recvBuf, size, timeout);
            continue;
        }
        break;
    }
    co_return false;
}

HX::STL::coroutine::task::Task<std::optional<std::string>> IO::recvN(
    std::size_t n
) const {
    std::string s;
    s.resize(n);
    int len = std::max(
        co_await _recvSpan(s), 0
    );

    if (len) {
        co_return s;
    }
    co_return std::nullopt;
}

HX::STL::coroutine::task::Task<std::optional<std::string>> IO::recvN(
    std::size_t n,
    struct __kernel_timespec *timeout
) const {
    std::string s;
    s.resize(n);
    int len = std::max(
        co_await _recvSpan(s, timeout), 0
    );

    if (len) {
        co_return s;
    }
    co_return std::nullopt;
}

HX::STL::coroutine::task::Task<std::size_t> IO::recvN(
    std::span<char> buf,
    std::size_t n,
    struct __kernel_timespec *timeout
) const {
    co_return static_cast<std::size_t>(std::max(
        co_await _recvSpan(std::span{buf.data(), n}, timeout), 0 // ! n
    ));
}

HX::STL::coroutine::task::Task<int> IO::_recvSpan(
    std::span<char> buf
) const {
    co_return co_await HX::STL::coroutine::loop::IoUringTask().prepRecv(
        _fd, buf, 0
    );
}

HX::STL::coroutine::task::Task<int> IO::_recvSpan(
    std::span<char> buf, 
    struct __kernel_timespec *timeout
) const {
    co_return co_await HX::STL::coroutine::loop::IoUringTask::linkOps(
        HX::STL::coroutine::loop::IoUringTask().prepRecv(
            _fd, buf, 0, "125"
        ),
        HX::STL::coroutine::loop::IoUringTask().prepLinkTimeout(
            timeout, IORING_TIMEOUT_BOOTTIME
        )
    );
}

HX::STL::coroutine::task::Task<> IO::_send() const {
    // 本次请求使用结束, 清空, 复用
    _request->clear();
    // 清除响应的缓冲区, 复用
    _response->createResponseBuffer();
    std::span<char> buf = _response->_buf;
    std::size_t n = HX::STL::tools::UringErrorHandlingTools::throwingError(
        co_await HX::STL::coroutine::loop::IoUringTask().prepSend(_fd, buf, 0)
    ); // 已经写入的字节数

    while (true) {
        if (n == buf.size()) {
            // 全部写入啦
            _response->clear();
            break;
        }
        n = HX::STL::tools::UringErrorHandlingTools::throwingError(
            co_await HX::STL::coroutine::loop::IoUringTask().prepSend(
                _fd, buf = buf.subspan(n), 0
            )
        );
    }
}

HX::STL::coroutine::task::Task<> IO::_send(std::span<char> buf) const {
    std::size_t n = HX::STL::tools::UringErrorHandlingTools::throwingError(
        co_await HX::STL::coroutine::loop::IoUringTask().prepSend(_fd, buf, 0)
    ); // 已经写入的字节数

    while (true) {
        if (n == buf.size()) {
            // 全部写入啦
            _response->clear();
            break;
        }
        n = HX::STL::tools::UringErrorHandlingTools::throwingError(
            co_await HX::STL::coroutine::loop::IoUringTask().prepSend(
                _fd, buf = buf.subspan(n), 0
            )
        );
    }
}

}}} // namespace HX::web::server