#include <HXWeb/protocol/http/Response.h>

#include <sys/socket.h>
#include <cstring>

#include <HXSTL/utils/StringUtils.h>
#include <HXSTL/utils/FileUtils.h>
#include <iostream>

namespace HX { namespace web { namespace protocol { namespace http {

Response& Response::setResponseLine(Response::Status statusCode, std::string_view describe /*= ""*/) {
    _statusLine.clear();
    _statusLine.resize(3);
    _statusLine[ResponseLineDataType::ProtocolVersion] = "HTTP/1.1";
    _statusLine[ResponseLineDataType::StatusCode] = std::to_string(static_cast<int>(statusCode));
    if (!describe.size()) {
        switch (statusCode) {
            case Response::Status::CODE_100:
                _statusLine[ResponseLineDataType::StatusMessage] = "Continue";
                break;
            case Response::Status::CODE_101:
                _statusLine[ResponseLineDataType::StatusMessage] = "Switching Protocols";
                break;
            case Response::Status::CODE_102:
                _statusLine[ResponseLineDataType::StatusMessage] = "Processing";
                break;
            case Response::Status::CODE_200:
                _statusLine[ResponseLineDataType::StatusMessage] = "OK";
                break;
            case Response::Status::CODE_201:
                _statusLine[ResponseLineDataType::StatusMessage] = "Created";
                break;
            case Response::Status::CODE_202:
                _statusLine[ResponseLineDataType::StatusMessage] = "Accepted";
                break;
            case Response::Status::CODE_203:
                _statusLine[ResponseLineDataType::StatusMessage] = "Non-Authoritative Information";
                break;
            case Response::Status::CODE_204:
                _statusLine[ResponseLineDataType::StatusMessage] = "No Content";
                break;
            case Response::Status::CODE_205: 
                _statusLine[ResponseLineDataType::StatusMessage] = "Reset Content";
                break;
            case Response::Status::CODE_206:
                _statusLine[ResponseLineDataType::StatusMessage] = "Partial Content";
                break;
            case Response::Status::CODE_207:
                _statusLine[ResponseLineDataType::StatusMessage] = "Multi-Status";
                break;
            case Response::Status::CODE_226:
                _statusLine[ResponseLineDataType::StatusMessage] = "IM Used";
                break;
            case Response::Status::CODE_300:
                _statusLine[ResponseLineDataType::StatusMessage] = "Multiple Choices";
                break;
            case Response::Status::CODE_301:
                _statusLine[ResponseLineDataType::StatusMessage] = "Moved Permanently";
                break;
            case Response::Status::CODE_302: 
                _statusLine[ResponseLineDataType::StatusMessage] = "Moved Temporarily";
                break;
            case Response::Status::CODE_303:
                _statusLine[ResponseLineDataType::StatusMessage] = "See Other";
                break;
            case Response::Status::CODE_304:
                _statusLine[ResponseLineDataType::StatusMessage] = "Not Modified";
                break;
            case Response::Status::CODE_305:
                _statusLine[ResponseLineDataType::StatusMessage] = "Use Proxy";
                break;
            case Response::Status::CODE_306:
                _statusLine[ResponseLineDataType::StatusMessage] = "Reserved";
                break;
            case Response::Status::CODE_307:
                _statusLine[ResponseLineDataType::StatusMessage] = "Temporary Redirect";
                break;
            case Response::Status::CODE_400:
                _statusLine[ResponseLineDataType::StatusMessage] = "Bad Request";
                break;
            case Response::Status::CODE_401:
                _statusLine[ResponseLineDataType::StatusMessage] = "Unauthorized";
                break;
            case Response::Status::CODE_402:
                _statusLine[ResponseLineDataType::StatusMessage] = "Payment Required";
                break;
            case Response::Status::CODE_403:
                _statusLine[ResponseLineDataType::StatusMessage] = "Forbidden";
                break;
            case Response::Status::CODE_404:
                _statusLine[ResponseLineDataType::StatusMessage] = "Not Found";
                break;
            case Response::Status::CODE_405:
                _statusLine[ResponseLineDataType::StatusMessage] = "Method Not Allowed";
                break;
            case Response::Status::CODE_406:
                _statusLine[ResponseLineDataType::StatusMessage] = "Not Acceptable";
                break;
            case Response::Status::CODE_407:
                _statusLine[ResponseLineDataType::StatusMessage] = "Proxy Authentication Required";
                break;
            case Response::Status::CODE_408:
                _statusLine[ResponseLineDataType::StatusMessage] = "Request Timeout";
                break;
            case Response::Status::CODE_409:
                _statusLine[ResponseLineDataType::StatusMessage] = "Conflict";
                break;
            case Response::Status::CODE_410:
                _statusLine[ResponseLineDataType::StatusMessage] = "Gone";
                break;
            case Response::Status::CODE_411:
                _statusLine[ResponseLineDataType::StatusMessage] = "Length Required";
                break;
            case Response::Status::CODE_412:
                _statusLine[ResponseLineDataType::StatusMessage] = "Precondition Failed";
                break;
            case Response::Status::CODE_413:
                _statusLine[ResponseLineDataType::StatusMessage] = "Request Entity Too Large";
                break;
            case Response::Status::CODE_414:
                _statusLine[ResponseLineDataType::StatusMessage] = "Request-URI Too Large";
                break;
            case Response::Status::CODE_415:
                _statusLine[ResponseLineDataType::StatusMessage] = "Unsupported Media Type";
                break;
            case Response::Status::CODE_416:
                _statusLine[ResponseLineDataType::StatusMessage] = "Requested Range Not Satisfiable";
                break;
            case Response::Status::CODE_417:
                _statusLine[ResponseLineDataType::StatusMessage] = "Expectation Failed";
                break;
            case Response::Status::CODE_422:
                _statusLine[ResponseLineDataType::StatusMessage] = "Unprocessable Entity";
                break;
            case Response::Status::CODE_423:
                _statusLine[ResponseLineDataType::StatusMessage] = "Locked";
                break;
            case Response::Status::CODE_424:
                _statusLine[ResponseLineDataType::StatusMessage] = "Failed Dependency";
                break;
            case Response::Status::CODE_425:
                _statusLine[ResponseLineDataType::StatusMessage] = "Unordered Collection";
                break;
            case Response::Status::CODE_426:
                _statusLine[ResponseLineDataType::StatusMessage] = "Upgrade Required";
                break;
            case Response::Status::CODE_428:
                _statusLine[ResponseLineDataType::StatusMessage] = "Precondition Required";
                break;
            case Response::Status::CODE_429:
                _statusLine[ResponseLineDataType::StatusMessage] = "Too Many Requests";
                break;
            case Response::Status::CODE_431:
                _statusLine[ResponseLineDataType::StatusMessage] = "Request Header Fields Too Large";
                break;
            case Response::Status::CODE_434:
                _statusLine[ResponseLineDataType::StatusMessage] = "Requested host unavailable";
                break;
            case Response::Status::CODE_444:
                _statusLine[ResponseLineDataType::StatusMessage] = "Close connection without sending headers";
                break;
            case Response::Status::CODE_449:
                _statusLine[ResponseLineDataType::StatusMessage] = "Retry With";
                break;
            case Response::Status::CODE_451:
                _statusLine[ResponseLineDataType::StatusMessage] = "Unavailable For Legal Reasons";
                break;
            case Response::Status::CODE_500:
                _statusLine[ResponseLineDataType::StatusMessage] = "Internal Server Error";
                break;
            case Response::Status::CODE_501:
                _statusLine[ResponseLineDataType::StatusMessage] = "Not Implemented";
                break;
            case Response::Status::CODE_502:
                _statusLine[ResponseLineDataType::StatusMessage] = "Bad Gateway";
                break;
            case Response::Status::CODE_503:
                _statusLine[ResponseLineDataType::StatusMessage] = "Service Unavailable";
                break;
            case Response::Status::CODE_504:
                _statusLine[ResponseLineDataType::StatusMessage] = "Gateway Timeout";
                break;
            case Response::Status::CODE_505:
                _statusLine[ResponseLineDataType::StatusMessage] = "HTTP Version Not Supported";
                break;
            case Response::Status::CODE_506:
                _statusLine[ResponseLineDataType::StatusMessage] = "Variant Also Negotiates";
                break;
            case Response::Status::CODE_507:
                _statusLine[ResponseLineDataType::StatusMessage] = "Insufficient Storage";
                break;
            case Response::Status::CODE_508:
                _statusLine[ResponseLineDataType::StatusMessage] = "Loop Detected";
                break;
            case Response::Status::CODE_509:
                _statusLine[ResponseLineDataType::StatusMessage] = "Bandwidth Limit Exceeded";
                break;
            case Response::Status::CODE_510:
                _statusLine[ResponseLineDataType::StatusMessage] = "Not Extended";
                break;
            case Response::Status::CODE_511: 
                _statusLine[ResponseLineDataType::StatusMessage] = "Network Authentication Required";
                break;
            default: // 使用可真刁钻呐!
                break;
        }
    } else {
        _statusLine[ResponseLineDataType::StatusMessage] = describe;
    }
    return *this;
}

std::size_t Response::parserResponse(std::string_view buf) {
    if (_buf.size()) {
        _buf += buf;
        buf = _buf;
    }

    if (_statusLine.empty()) { // 响应行还未解析
        std::size_t pos = buf.find("\r\n");
        if (pos == std::string_view::npos) [[unlikely]] { // 不可能事件
            return HX::STL::utils::FileUtils::kBufMaxSize;
        }

        // 解析响应行, 注意 不能按照空格直接切分! 因为 HTTP/1.1 404 NOF FOND\r\n
        _statusLine = HX::STL::utils::StringUtil::split(buf.substr(0, pos), " ");
        if (_statusLine.size() < 3)
            return HX::STL::utils::FileUtils::kBufMaxSize;
        if (_statusLine.size() > 3) {
            for (std::size_t i = 4; i < _statusLine.size(); ++i) {
                _statusLine[ResponseLineDataType::StatusMessage] += _statusLine[i];
            }
            _statusLine.resize(3);
        }
        buf = buf.substr(pos + 2); // 再前进, 以去掉 "\r\n"
    }

    /**
     * @brief 请求头
     * 通过`\r\n`分割后, 取最前面的, 先使用最左的`:`以判断是否是需要作为独立的键值对;
     * -  如果找不到`:`, 并且 非空, 那么它需要接在上一个解析的键值对的值尾
     * -  否则即请求头解析完毕!
     */
    while (!_completeResponseHeader) { // 响应头未解析完
        std::size_t pos = buf.find("\r\n");
        if (pos == std::string_view::npos) { // 没有读取完
            _buf = buf;
            return HX::STL::utils::FileUtils::kBufMaxSize;
        }
        std::string_view subStr = buf.substr(0, pos);
        auto p = HX::STL::utils::StringUtil::splitAtFirst(subStr, ": ");
        if (p.first.empty()) { // 找不到 ": "
            if (subStr.size()) {
                _responseHeadersIt->second.append(subStr);
            } else { // 请求头解析完毕!
                _completeResponseHeader = true;
            }
        } else {
            HX::STL::utils::StringUtil::toSmallLetter(p.first);
            _responseHeadersIt = _responseHeaders.insert(p).first;
        }
        buf = buf.substr(pos + 2);
    }

    if (_responseHeaders.count("content-length")) { // 存在content-length模式接收的响应体
        // 是 空行之后 (\r\n\r\n) 的内容大小(char)
        if (!_remainingBodyLen.has_value()) {
            _responseBody = buf;
            _remainingBodyLen = std::stoll(_responseHeaders["content-length"]) 
                              - _responseBody.size();
        } else {
            *_remainingBodyLen -= buf.size();
            _responseBody.append(buf);
        }

        if (*_remainingBodyLen != 0) {
            _buf.clear();
            return *_remainingBodyLen;
        }
    } else if (_responseHeaders.count("transfer-encoding")) { // 存在响应体以`分块传输编码`
        if (_remainingBodyLen) { // 处理没有读取完的
            if (buf.size() <= *_remainingBodyLen) { // 还没有读取完毕
                _responseBody += buf;
                *_remainingBodyLen -= buf.size();
                return HX::STL::utils::FileUtils::kBufMaxSize;
            } else { // 读取完了
                _responseBody.append(buf, 0, *_remainingBodyLen);
                buf = buf.substr(std::min(*_remainingBodyLen + 2, buf.size()));
                _remainingBodyLen.reset();
            }
        }
        while (true) {
            std::size_t posLen = buf.find("\r\n");
            if (posLen == std::string_view::npos) { // 没有读完
                _buf = buf;
                return HX::STL::utils::FileUtils::kBufMaxSize;
            }
            if (!posLen && buf[0] == '\r') [[unlikely]] { // posLen == 0
                // \r\n 贴脸, 触发原因, std::min(*_remainingBodyLen + 2, buf.size()) 只能 buf.size()
                buf = buf.substr(posLen + 2);
                continue;
            }
            _remainingBodyLen = std::stol(std::string {buf.substr(0, posLen)}, nullptr, 16); // 转换为十进制整数
            if (!*_remainingBodyLen) { // 解析完毕
                return 0;
            }
            buf = buf.substr(posLen + 2);
            if (buf.size() <= *_remainingBodyLen) { // 没有读完
                _responseBody += buf;
                *_remainingBodyLen -= buf.size();
                return HX::STL::utils::FileUtils::kBufMaxSize;
            }
            _responseBody.append(buf.substr(0, *_remainingBodyLen));
            buf = buf.substr(*_remainingBodyLen + 2);
        }
    }

    return 0; // 解析完毕
}

#if 0
std::size_t Response::parserResponse(std::span<char> buf) {
    _buf.append(std::string {buf.data(), buf.size()});
    _buf.push_back('\0'); // HTTP 响应的响应体在传输过程中没有特定的 \0 结束标志, 
                          // 但是 char * 需要
    char *tmp = nullptr;
    char *line = nullptr;
    if (_statusLine.empty()) { // 响应行还未解析
        line = ::strtok_r(_buf.data(), "\r\n", &tmp); // 线程安全
        if (!line)
            return HX::STL::utils::FileUtils::kBufMaxSize;
        // 解析响应行, 注意 不能按照空格直接切分! 因为 HTTP/1.1 404 NOF FOND\r\n
        _statusLine = HX::STL::utils::StringUtil::split(line, " ");
        if (_statusLine.size() < 3)
            return HX::STL::utils::FileUtils::kBufMaxSize;
        if (_statusLine.size() > 3) {
            for (std::size_t i = 4; i < _statusLine.size(); ++i) {
                _statusLine[ResponseLineDataType::StatusMessage] += _statusLine[i];
            }
            _statusLine.resize(3);
        }
    }

    if (!_completeResponseHeader) { // 响应头未解析完
        /**
         * @brief 响应响应头
         * 解析响应报文算法:
         * 响应头\r\n  | 按照 \n 分割, 这样每一个line的后面都会有\r残留, 需要pop掉
         * \r\n (空行) | 只会解析到 \r
         * 响应体
         */
        if (tmp == nullptr) { // 之前就没有解析完
            line = ::strtok_r(_buf.data(), "\n", &tmp);
        } else { // 第一次解析到
            line = ::strtok_r(nullptr, "\n", &tmp);
        }

        /**
         * @brief 假设你有一个非常长的 Content-Security-Policy 头字段
         * 它可能会被分成多个行发送. 每个分片的格式应如下:
         * Content-Security-Policy: default-src 'none'; base-uri 'self'; child-src github.com/... \r\n
         * github.com/webpack/... \r\n
         * github.com/assets/... \r\n
         * ... (其余部分) \r\n
         */

        do {// 解析 响应行
            // 计算当前子字符串的长度
            std::size_t length = (*tmp == '\0' ? ::strlen(line) : tmp - line - 1);
            auto p = HX::STL::utils::StringUtil::splitAtFirst(std::string_view {line, length}, ": ");
            if (p.first == "") { // 解析失败, 说明当前是空行, 也有可能是没有读取完毕
                if (*line != '\r') { 
                    // 应该剩下的参与下次解析
                    printf("继续解析呀~\n"); // (%s)\n", _buf.data());
                    _buf.pop_back();
                    _buf = HX::STL::utils::StringUtil::rfindAndTrim(_buf.data(), "\r\n");
                    std::cout << _buf << '\n';
                    return HX::STL::utils::FileUtils::kBufMaxSize;
                }
                // 是空行
                printf("解析完了~\n");
                _completeResponseHeader = true;
                break;
            }
            HX::STL::utils::StringUtil::toSmallLetter(p.first);
            p.second.pop_back(); // 去掉 '\r'
            _responseHeaders.insert(p);
            printf("%lu -> %lu\n", p.first.size(), p.second.size());
        } while ((line = ::strtok_r(nullptr, "\n", &tmp)));

        if (!_completeResponseHeader) { // 解析完整了, 但是并没有`\r\n`
            _buf.clear();
            printf("\n");
            return HX::STL::utils::FileUtils::kBufMaxSize;
        }
    }

    /**
     * @brief 支持分块传输编码
     * HTTP/1.1 200 OK\r\n
     * Content-Type: text/plain\r\n
     * Transfer-Encoding: chunked\r\n
     * \r\n  <-- 这是请求头和主体之间的空行, 标志着请求头结束
     * 4\r\n
     * This \r\n
     * 7\r\n
     * is a \r\n
     * 9\r\n
     * chunked \r\n
     * 6\r\n
     * message\r\n
     * 0\r\n
     * \r\n
     */
    
    if (_responseHeaders.count("content-length")) { // 存在响应体
        // 是 空行之后 (\r\n\r\n) 的内容大小(char)
        if (!_remainingBodyLen.has_value()) {
            _responseBody = std::string {tmp};
            _remainingBodyLen = std::stoll(_responseHeaders["content-length"]) 
                              - _responseBody.size();
        } else {
            _buf.pop_back();
            *_remainingBodyLen -= buf.size();
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
#endif

void Response::createResponseBuffer() {
    _buf.clear();
    _buf.append(_statusLine[ResponseLineDataType::ProtocolVersion]);
    _buf.append(" ");
    _buf.append(_statusLine[ResponseLineDataType::StatusCode]);
    _buf.append(" ");
    _buf.append(_statusLine[ResponseLineDataType::StatusMessage]);
    _buf.append("\r\n");
    for (const auto& [key, val] : _responseHeaders) {
        _buf.append(key);
        _buf.append(": ");
        _buf.append(val);
        _buf.append("\r\n");
    }
    _buf.append("Content-Length: ");
    _buf.append(std::to_string(_responseBody.size()));
    _buf.append("\r\n\r\n");
    _buf.append(_responseBody);
}

}}}} // namespace HX::web::protocol::http
