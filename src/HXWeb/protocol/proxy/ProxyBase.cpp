#include <HXWeb/protocol/proxy/ProxyBase.h>

#include <HXSTL/utils/UrlUtils.h>
#include <HXWeb/protocol/proxy/SocksProxy.h>

namespace HX { namespace web { namespace protocol { namespace proxy {

HX::STL::coroutine::task::Task<bool> ProxyBash::connect(
    const std::string& url,
    const HX::web::client::IO& io
) {
    std::string rmProtocolUrl = url;
    auto protocol = HX::STL::utils::UrlUtils::removeProtocol(rmProtocolUrl);
    if (protocol == "socks5") {
        // co_return co_await HX::web::protocol::proxy::Socks5Proxy()._connect(rmProtocolUrl, io);
    }
    co_return false;
}

}}}} // namespace HX::web::protocol::proxy