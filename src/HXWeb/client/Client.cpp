#include <HXWeb/client/Client.h>

#include <HXSTL/tools/ErrorHandlingTools.h>
#include <HXSTL/coroutine/loop/IoUringLoop.h>
#include <HXSTL/utils/FileUtils.h>

namespace HX { namespace web { namespace client {

HX::STL::coroutine::task::Task<int> Client::start(
    const std::string& name, const std::string& port
) {
    if (_clientFd != -1) [[unlikely]] {
        throw "start: _clientFd != -1"; // 不要调用多次, 且 _clientFd 非-1
    }

    socket::AddressResolver resolver;
    auto entry = resolver.resolve(name, port);
    auto sockaddr = entry.getAddress();
    
    _clientFd = HX::STL::tools::UringErrorHandlingTools::throwingError(
        co_await HX::STL::coroutine::loop::IoUringTask().prepSocket(
            entry._curr->ai_family,
            entry._curr->ai_socktype,
            entry._curr->ai_protocol,
            0
        )
    );

    co_return co_await HX::STL::coroutine::loop::IoUringTask().prepConnect(
        _clientFd,
        sockaddr._addr,
        sockaddr._addrlen
    );
}

HX::STL::coroutine::task::Task<std::string> Client::read() {
    std::string res;
    std::vector<char> buf(HX::STL::utils::FileUtils::kBufMaxSize);
    std::size_t len = 0;
    while ((len = static_cast<std::size_t>(HX::STL::tools::UringErrorHandlingTools::throwingError(
        co_await HX::STL::coroutine::loop::IoUringTask().prepRecv(
            _clientFd, buf, 0
    )))) == buf.size()) {
        res += std::string_view {buf.data(), buf.size()};
    }
    co_return res += std::string_view {buf.data(), len};
}

HX::STL::coroutine::task::Task<int> Client::write(std::span<char> buf) {
    co_return co_await HX::STL::coroutine::loop::IoUringTask().prepSend(
        _clientFd, buf, 0
    );
}

HX::STL::coroutine::task::Task<int> Client::close() {
    co_return (void)(_clientFd = -1), (
        co_await HX::STL::coroutine::loop::IoUringTask().prepClose(_clientFd)
    );
}

Client::~Client() {
    if (_clientFd == -1) {
        return;
    }
    LOG_ERROR("~Client: _clientFd != -1"); // 请释放 _clientFd
}

}}} // namespace HX::web::client