#include <HXWeb/protocol/http/Request.h>

#include <sys/socket.h> // 套接字
#include <cstring>

#include <HXSTL/tools/StringTools.h>

namespace HX { namespace web { namespace protocol { namespace http {

std::size_t protocol::http::Request::parserRequest(HX::STL::container::ConstBytesBufferView buf) {
    _previousData.append(buf);
    char *tmp = nullptr;
    char *line = nullptr;
    if (_requestLine.empty()) { // 请求行还未解析
        line = ::strtok_r(_previousData.data(), "\r\n", &tmp); // 线程安全
        if (!line)
            return protocol::http::Request::BUF_SIZE;
        _requestLine = HX::STL::tools::StringUtil::split(line, " "); // 解析请求头: GET /PTAH HTTP/1.1
        if (_requestLine.size() != 3)
            return protocol::http::Request::BUF_SIZE;
    }

    if (!_completeRequestHeader) { // 请求头未解析完
        /**
         * @brief 解析请求头
         * 解析请求报文算法:
         * 请求头\r\n  | 按照 \n 分割, 这样每一个line的后面都会有\r残留, 需要pop掉
         * \r\n (空行) | 只会解析到 \r
         * 请求体
         */
        if (tmp == nullptr) {
            line = ::strtok_r(_previousData.data(), "\r\n", &tmp);
        } else {
            line = ::strtok_r(nullptr, "\n", &tmp);
        }
        do {// 解析 请求行
            // 计算当前子字符串的长度
            std::size_t length = (*tmp == '\0' ? ::strlen(line) : tmp - line - 1);
            auto p = HX::STL::tools::StringUtil::splitAtFirst(std::string_view {line, length}, ": ");
            if (p.first == "") { // 解析失败, 说明当前是空行, 也有可能是没有读取完毕
                if (*line != '\r') { 
                    // 应该剩下的参与下次解析
                    _previousData = HX::STL::tools::StringUtil::rfindAndTrim(_previousData.data(), "\r\n");
                    return protocol::http::Request::BUF_SIZE;
                }
                // 是空行
                _completeRequestHeader = true;
                break;
            }
            HX::STL::tools::StringUtil::toSmallLetter(p.first);
            p.second.pop_back(); // 去掉 '\r'
            _requestHeaders.insert(p);
            // printf("%s -> %s\n", p.first.c_str(), p.second.c_str());
        } while ((line = ::strtok_r(nullptr, "\n", &tmp)));
    }
    
    if (_requestHeaders.count("content-length")) { // 存在请求体
        // 是 空行之后 (\r\n\r\n) 的内容大小(char)
        if (!_remainingBodyLen.has_value()) {
            _remainingBodyLen = std::stoll(_requestHeaders["content-length"]) 
                              - static_cast<ssize_t>(::strlen(tmp));
            _body = std::string {tmp};
        } else {
            *_remainingBodyLen -= buf.size();
            _body->append(HX::STL::container::ConstBytesBufferView {_previousData.data(), _previousData.size()});
        }

        if (*_remainingBodyLen != 0) {
            _previousData.clear();
            return *_remainingBodyLen;
        }
    }

    // printf("请求体: %s\n", _body->c_str());
    _previousData.clear();
    return 0; // 解析完毕
}

std::unordered_map<std::string, std::string> protocol::http::Request::getParseQueryParameters() const {
    std::string path = getRequesPath();
    std::size_t pos = path.find('?'); // 没必要反向查找
    if (pos == std::string::npos)
        return {};
    std::string parameter = path.substr(pos + 1);
    auto kvArr = HX::STL::tools::StringUtil::split(parameter, "&");
    std::unordered_map<std::string, std::string> res;
    for (const auto& it : kvArr) {
        auto&& kvPair = HX::STL::tools::StringUtil::splitAtFirst(it, "=");
        if (kvPair.first == "")
            res.insert_or_assign(it, "");
        else
            res.insert(std::move(kvPair));
    }
    return res;
}

}}}} // namespace HX::web::protocol::http
