#include <HXWeb/client/Client.h>

#include <HXWeb/protocol/http/Request.h>
#include <HXWeb/protocol/http/Response.h>
#include <HXWeb/protocol/proxy/ProxyBase.h>
#include <HXSTL/tools/ErrorHandlingTools.h>
#include <HXSTL/coroutine/loop/IoUringLoop.h>
#include <HXSTL/utils/FileUtils.h>
#include <HXSTL/utils/UrlUtils.h>

namespace HX { namespace web { namespace client {

HX::STL::coroutine::task::Task<
    std::shared_ptr<HX::web::protocol::http::Response>
> Client::request(
    const std::string& method,
    const std::string& url,
    const std::unordered_map<std::string, std::string> head /*= {}*/,
    const std::string& body /*= ""*/,
    std::chrono::milliseconds timeout /*= std::chrono::milliseconds {30 * 1000}*/,
    const std::string& proxy /*= ""*/
) {
    auto ptr = HX::web::client::Client::make();
    co_await ptr->start(url, proxy);
    ptr->_io->_request->setRequestLine(method, HX::STL::utils::UrlUtils::extractPath(url))
                       .setRequestHeaders(head)
                       .setRequestBody(body);
    co_await ptr->_io->_sendRequest();
    co_await ptr->_io->_recvResponse(timeout);
    co_return std::make_shared<HX::web::protocol::http::Response>(
        std::move(ptr->_io->getResponse())
    );
}

HX::STL::coroutine::task::Task<> Client::start(
    const std::string& url,
    const std::string& proxy /*= ""*/
) {
    socket::AddressResolver resolver;
    HX::STL::utils::UrlUtils::UrlInfoExtractor parser(proxy.size() ? proxy : url);
    auto entry = resolver.resolve(parser.getHostname(), parser.getService());
    auto sockaddr = entry.getAddress();
    
    int _clientFd = HX::STL::tools::UringErrorHandlingTools::throwingError(
        co_await HX::STL::coroutine::loop::IoUringTask().prepSocket(
            entry._curr->ai_family,
            entry._curr->ai_socktype,
            entry._curr->ai_protocol,
            0
        )
    );

    co_await HX::STL::coroutine::loop::IoUringTask().prepConnect(
        _clientFd,
        sockaddr._addr,
        sockaddr._addrlen
    );

    _io = std::make_unique<HX::web::client::IO>(_clientFd);
    if (proxy.size()) { // 进行代理连接
        co_await HX::web::protocol::proxy::ProxyBash::connect(proxy, url, *_io);
    }
}

HX::STL::coroutine::task::Task<bool> Client::read(std::chrono::milliseconds timeout) {
    co_return co_await _io->_recvResponse(timeout);
}

HX::STL::coroutine::task::Task<> Client::write(std::span<char> buf) {
    co_await _io->_sendSpan(buf);
}

}}} // namespace HX::web::client