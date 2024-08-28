#include <HXWeb/client/Client.h>

#include <HXWeb/protocol/http/Request.h>
#include <HXWeb/protocol/http/Response.h>
#include <HXSTL/tools/ErrorHandlingTools.h>
#include <HXSTL/coroutine/loop/IoUringLoop.h>
#include <HXSTL/utils/FileUtils.h>

#include <regex>
#include <stdexcept>

class UrlParser {
public:
    UrlParser(const std::string& url) {
        parseUrl(url);
    }

    const std::string& getHostname() const {
        return hostname;
    }

    const std::string& getService() const {
        return service;
    }

private:
    std::string hostname;
    std::string service;

    void parseUrl(const std::string& url) {
        // 正则表达式用来解析 URL
        std::regex urlRegex(R"(^(\w+):\/\/([^\/:]+)(?::(\d+))?.*)");
        std::smatch urlMatch;

        if (std::regex_match(url, urlMatch, urlRegex)) {
            // 提取主机名
            hostname = urlMatch[2].str();
            // 提取端口号，如果没有提供端口号，则根据协议提供默认值
            if (urlMatch[3].matched) {
                service = urlMatch[3].str(); // 使用用户提供的端口号
            } else {
                // 根据协议设置默认端口号或服务名称, 不然就默认http进行尝试
                if (urlMatch[1].matched) {
                    service = urlMatch[1].str();
                } else {
                    service = "http";
                }
            }
        } else {
            throw std::invalid_argument("Invalid URL format: " + url);
        }
    }
};

namespace HX { namespace web { namespace client {

HX::STL::coroutine::task::Task<> Client::start(
    const std::string& url
) {
    socket::AddressResolver resolver;
    UrlParser parser(url);
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
}

HX::STL::coroutine::task::Task<bool> Client::read(std::chrono::seconds timeout) {
    auto timespec = HX::STL::coroutine::loop::durationToKernelTimespec(timeout);
    co_return co_await _io->_recvResponse(&timespec);
}

HX::STL::coroutine::task::Task<> Client::write(std::span<char> buf) {
    co_await _io->_sendSpan(buf);
}

}}} // namespace HX::web::client