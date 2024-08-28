#include <HXWeb/protocol/http/Response.h>

#include <sys/socket.h>
#include <cstring>

#include <HXSTL/utils/StringUtils.h>
#include <HXSTL/utils/FileUtils.h>

namespace HX { namespace web { namespace protocol { namespace http {

Response& Response::setResponseLine(Response::Status statusCode, std::string_view describe /*= ""*/) {
    _statusLine.clear();
    _statusLine.resize(3);
    _statusLine[0] = "HTTP/1.1";
    _statusLine[1] = std::to_string(static_cast<int>(statusCode));
    if (!describe.size()) {
        switch (statusCode) {
            case Response::Status::CODE_100:
                _statusLine[2] = ("Continue");
                break;
            case Response::Status::CODE_101:
                _statusLine[2] = ("Switching Protocols");
                break;
            case Response::Status::CODE_102:
                _statusLine[2] = ("Processing");
                break;
            case Response::Status::CODE_200:
                _statusLine[2] = ("OK");
                break;
            case Response::Status::CODE_201:
                _statusLine[2] = ("Created");
                break;
            case Response::Status::CODE_202:
                _statusLine[2] = ("Accepted");
                break;
            case Response::Status::CODE_203:
                _statusLine[2] = ("Non-Authoritative Information");
                break;
            case Response::Status::CODE_204:
                _statusLine[2] = ("No Content");
                break;
            case Response::Status::CODE_205: 
                _statusLine[2] = ("Reset Content");
                break;
            case Response::Status::CODE_206:
                _statusLine[2] = ("Partial Content");
                break;
            case Response::Status::CODE_207:
                _statusLine[2] = ("Multi-Status");
                break;
            case Response::Status::CODE_226:
                _statusLine[2] = ("IM Used");
                break;
            case Response::Status::CODE_300:
                _statusLine[2] = ("Multiple Choices");
                break;
            case Response::Status::CODE_301:
                _statusLine[2] = ("Moved Permanently");
                break;
            case Response::Status::CODE_302: 
                _statusLine[2] = ("Moved Temporarily");
                break;
            case Response::Status::CODE_303:
                _statusLine[2] = ("See Other");
                break;
            case Response::Status::CODE_304:
                _statusLine[2] = ("Not Modified");
                break;
            case Response::Status::CODE_305:
                _statusLine[2] = ("Use Proxy");
                break;
            case Response::Status::CODE_306:
                _statusLine[2] = ("Reserved");
                break;
            case Response::Status::CODE_307:
                _statusLine[2] = ("Temporary Redirect");
                break;
            case Response::Status::CODE_400:
                _statusLine[2] = ("Bad Request");
                break;
            case Response::Status::CODE_401:
                _statusLine[2] = ("Unauthorized");
                break;
            case Response::Status::CODE_402:
                _statusLine[2] = ("Payment Required");
                break;
            case Response::Status::CODE_403:
                _statusLine[2] = ("Forbidden");
                break;
            case Response::Status::CODE_404:
                _statusLine[2] = ("Not Found");
                break;
            case Response::Status::CODE_405:
                _statusLine[2] = ("Method Not Allowed");
                break;
            case Response::Status::CODE_406:
                _statusLine[2] = ("Not Acceptable");
                break;
            case Response::Status::CODE_407:
                _statusLine[2] = ("Proxy Authentication Required");
                break;
            case Response::Status::CODE_408:
                _statusLine[2] = ("Request Timeout");
                break;
            case Response::Status::CODE_409:
                _statusLine[2] = ("Conflict");
                break;
            case Response::Status::CODE_410:
                _statusLine[2] = ("Gone");
                break;
            case Response::Status::CODE_411:
                _statusLine[2] = ("Length Required");
                break;
            case Response::Status::CODE_412:
                _statusLine[2] = ("Precondition Failed");
                break;
            case Response::Status::CODE_413:
                _statusLine[2] = ("Request Entity Too Large");
                break;
            case Response::Status::CODE_414:
                _statusLine[2] = ("Request-URI Too Large");
                break;
            case Response::Status::CODE_415:
                _statusLine[2] = ("Unsupported Media Type");
                break;
            case Response::Status::CODE_416:
                _statusLine[2] = ("Requested Range Not Satisfiable");
                break;
            case Response::Status::CODE_417:
                _statusLine[2] = ("Expectation Failed");
                break;
            case Response::Status::CODE_422:
                _statusLine[2] = ("Unprocessable Entity");
                break;
            case Response::Status::CODE_423:
                _statusLine[2] = ("Locked");
                break;
            case Response::Status::CODE_424:
                _statusLine[2] = ("Failed Dependency");
                break;
            case Response::Status::CODE_425:
                _statusLine[2] = ("Unordered Collection");
                break;
            case Response::Status::CODE_426:
                _statusLine[2] = ("Upgrade Required");
                break;
            case Response::Status::CODE_428:
                _statusLine[2] = ("Precondition Required");
                break;
            case Response::Status::CODE_429:
                _statusLine[2] = ("Too Many Requests");
                break;
            case Response::Status::CODE_431:
                _statusLine[2] = ("Request Header Fields Too Large");
                break;
            case Response::Status::CODE_434:
                _statusLine[2] = ("Requested host unavailable");
                break;
            case Response::Status::CODE_444:
                _statusLine[2] = ("Close connection without sending headers");
                break;
            case Response::Status::CODE_449:
                _statusLine[2] = ("Retry With");
                break;
            case Response::Status::CODE_451:
                _statusLine[2] = ("Unavailable For Legal Reasons");
                break;
            case Response::Status::CODE_500:
                _statusLine[2] = ("Internal Server Error");
                break;
            case Response::Status::CODE_501:
                _statusLine[2] = ("Not Implemented");
                break;
            case Response::Status::CODE_502:
                _statusLine[2] = ("Bad Gateway");
                break;
            case Response::Status::CODE_503:
                _statusLine[2] = ("Service Unavailable");
                break;
            case Response::Status::CODE_504:
                _statusLine[2] = ("Gateway Timeout");
                break;
            case Response::Status::CODE_505:
                _statusLine[2] = ("HTTP Version Not Supported");
                break;
            case Response::Status::CODE_506:
                _statusLine[2] = ("Variant Also Negotiates");
                break;
            case Response::Status::CODE_507:
                _statusLine[2] = ("Insufficient Storage");
                break;
            case Response::Status::CODE_508:
                _statusLine[2] = ("Loop Detected");
                break;
            case Response::Status::CODE_509:
                _statusLine[2] = ("Bandwidth Limit Exceeded");
                break;
            case Response::Status::CODE_510:
                _statusLine[2] = ("Not Extended");
                break;
            case Response::Status::CODE_511: 
                _statusLine[2] = ("Network Authentication Required");
                break;
            default: // 使用可真刁钻呐!
                break;
        }
    } else {
        _statusLine[2] = (describe);
    }
    return *this;
}

std::size_t Response::parserResponse(HX::STL::container::ConstBytesBufferView buf) {
    _buf.append(buf);
    _buf.push_back('\0'); // HTTP 响应的响应体在传输过程中没有特定的 \0 结束标志, 
                          // 但是 char * 需要
    char *tmp = nullptr;
    char *line = nullptr;
    if (_statusLine.empty()) { // 请求行还未解析
        line = ::strtok_r(_buf.data(), "\r\n", &tmp); // 线程安全
        if (!line)
            return HX::STL::utils::FileUtils::kBufMaxSize;
        _statusLine = HX::STL::utils::StringUtil::split(line, " "); // 解析请求头: GET /PTAH HTTP/1.1
        if (_statusLine.size() != 3)
            return HX::STL::utils::FileUtils::kBufMaxSize;
    }

    if (!_completeResponseHeader) { // 请求头未解析完
        /**
         * @brief 解析请求头
         * 解析请求报文算法:
         * 请求头\r\n  | 按照 \n 分割, 这样每一个line的后面都会有\r残留, 需要pop掉
         * \r\n (空行) | 只会解析到 \r
         * 请求体
         */
        if (tmp == nullptr) {
            line = ::strtok_r(_buf.data(), "\r\n", &tmp);
        } else {
            line = ::strtok_r(nullptr, "\n", &tmp);
        }
        do {// 解析 请求行
            // 计算当前子字符串的长度
            std::size_t length = (*tmp == '\0' ? ::strlen(line) : tmp - line - 1);
            auto p = HX::STL::utils::StringUtil::splitAtFirst(std::string_view {line, length}, ": ");
            if (p.first == "") { // 解析失败, 说明当前是空行, 也有可能是没有读取完毕
                if (*line != '\r') { 
                    // 应该剩下的参与下次解析
                    _buf = HX::STL::utils::StringUtil::rfindAndTrim(_buf.data(), "\r\n");
                    _buf.pop_back();
                    return HX::STL::utils::FileUtils::kBufMaxSize;
                }
                // 是空行
                _completeResponseHeader = true;
                break;
            }
            HX::STL::utils::StringUtil::toSmallLetter(p.first);
            p.second.pop_back(); // 去掉 '\r'
            _responseHeaders.insert(p);
            // printf("%s -> %s\n", p.first.c_str(), p.second.c_str());
        } while ((line = ::strtok_r(nullptr, "\n", &tmp)));
    }
    
    if (_responseHeaders.count("content-length")) { // 存在请求体
        // 是 空行之后 (\r\n\r\n) 的内容大小(char)
        if (!_remainingBodyLen.has_value()) {
            _responseBody = std::string {tmp};
            _remainingBodyLen = std::stoll(_responseHeaders["content-length"]) 
                              - _responseBody.size();
        } else {
            *_remainingBodyLen -= buf.size();
            _buf.pop_back();
            _responseBody.append(
                std::string_view {
                    _buf.data(), 
                    _buf.size()
                }
            );
        }

        if (*_remainingBodyLen != 0) {
            _buf.clear();
            return *_remainingBodyLen;
        }
    }

    _buf.clear();
    return 0; // 解析完毕
}

void Response::createResponseBuffer() {
    _buf.clear();
    printf("长度: %lld\n", _statusLine.size());
    _buf.append(_statusLine[0]);
    _buf.append(" ");
    _buf.append(_statusLine[1]);
    _buf.append(" ");
    _buf.append(_statusLine[2]);
    _buf.append("\r\n");
    for (const auto& [key, val] : _responseHeaders) {
        _buf.append(key);
        _buf.append(": ");
        _buf.append(val);
        _buf.append("\r\n");
    }
    if (_responseBody.size()) {
        _buf.append("Content-Length: ");
        _buf.append(std::to_string(_responseBody.size()));
        _buf.append("\r\n\r\n");
        _buf.append(_responseBody);
    } else {
        _buf.append("\r\n\r\n");
    }
}

}}}} // namespace HX::web::protocol::http
