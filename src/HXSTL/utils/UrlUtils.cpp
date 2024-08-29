#include <HXSTL/utils/UrlUtils.h>

#include <regex>
#include <stdexcept>

namespace HX { namespace STL { namespace utils {

std::string UrlUtils::UrlParser::extractPath(const std::string& url) {
    std::size_t protocolFind = url.find("://");
    std::size_t pathFind = url.find("/", protocolFind == std::string::npos ? 0 : protocolFind + 3);
    return pathFind == std::string::npos ? "/" : url.substr(pathFind);
}

void UrlUtils::UrlParser::parseUrl(const std::string& url) {
    // 正则表达式用来解析 URL
    std::regex urlRegex(R"([a-zA-Z0-9][-a-zA-Z0-9]{0,62}(\.[a-zA-Z0-9][-a-zA-Z0-9]{0,62})+\.?)");
    std::smatch urlMatch;
    
    std::size_t protocolFind = url.find("://");
    std::size_t portFind = url.find(":", protocolFind == std::string::npos ? 0 : protocolFind + 1);

    if (protocolFind == std::string::npos && portFind == std::string::npos) {
        // 没有协议和端口，默认服务为 "http"
        _service = "http";
    } else {
        if (portFind != std::string::npos) {
            // 有端口，提取端口号
            std::size_t start = portFind + 1;
            std::size_t end = url.find_first_not_of("0123456789", start);
            _service = url.substr(start, end - start);
        } else if (protocolFind != std::string::npos) {
            // 有协议但没有端口
            _service = url.substr(0, protocolFind);
        }
    }

    if (std::regex_search(url, urlMatch, urlRegex)) {
        _hostname = urlMatch.str();
    } else {
        throw std::invalid_argument("Invalid URL format: " + url);
    }
}

}}} // namespace HX::STL::utils