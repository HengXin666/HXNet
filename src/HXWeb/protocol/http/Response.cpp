#include <HXWeb/protocol/http/Response.h>

#include <sys/socket.h>

namespace HX { namespace web { namespace protocol { namespace http {

Response& Response::setResponseLine(Response::Status statusCode, std::string_view describe /*= ""*/) {
    _statusLine.append(std::to_string(static_cast<int>(statusCode)) + ' ');
    if (!describe.size()) {
        switch (statusCode) {
            case Response::Status::CODE_100:
                _statusLine.append("Continue");
                break;
            case Response::Status::CODE_101:
                _statusLine.append("Switching Protocols");
                break;
            case Response::Status::CODE_102:
                _statusLine.append("Processing");
                break;
            case Response::Status::CODE_200:
                _statusLine.append("OK");
                break;
            case Response::Status::CODE_201:
                _statusLine.append("Created");
                break;
            case Response::Status::CODE_202:
                _statusLine.append("Accepted");
                break;
            case Response::Status::CODE_203:
                _statusLine.append("Non-Authoritative Information");
                break;
            case Response::Status::CODE_204:
                _statusLine.append("No Content");
                break;
            case Response::Status::CODE_205: 
                _statusLine.append("Reset Content");
                break;
            case Response::Status::CODE_206:
                _statusLine.append("Partial Content");
                break;
            case Response::Status::CODE_207:
                _statusLine.append("Multi-Status");
                break;
            case Response::Status::CODE_226:
                _statusLine.append("IM Used");
                break;
            case Response::Status::CODE_300:
                _statusLine.append("Multiple Choices");
                break;
            case Response::Status::CODE_301:
                _statusLine.append("Moved Permanently");
                break;
            case Response::Status::CODE_302: 
                _statusLine.append("Moved Temporarily");
                break;
            case Response::Status::CODE_303:
                _statusLine.append("See Other");
                break;
            case Response::Status::CODE_304:
                _statusLine.append("Not Modified");
                break;
            case Response::Status::CODE_305:
                _statusLine.append("Use Proxy");
                break;
            case Response::Status::CODE_306:
                _statusLine.append("Reserved");
                break;
            case Response::Status::CODE_307:
                _statusLine.append("Temporary Redirect");
                break;
            case Response::Status::CODE_400:
                _statusLine.append("Bad Request");
                break;
            case Response::Status::CODE_401:
                _statusLine.append("Unauthorized");
                break;
            case Response::Status::CODE_402:
                _statusLine.append("Payment Required");
                break;
            case Response::Status::CODE_403:
                _statusLine.append("Forbidden");
                break;
            case Response::Status::CODE_404:
                _statusLine.append("Not Found");
                break;
            case Response::Status::CODE_405:
                _statusLine.append("Method Not Allowed");
                break;
            case Response::Status::CODE_406:
                _statusLine.append("Not Acceptable");
                break;
            case Response::Status::CODE_407:
                _statusLine.append("Proxy Authentication Required");
                break;
            case Response::Status::CODE_408:
                _statusLine.append("Request Timeout");
                break;
            case Response::Status::CODE_409:
                _statusLine.append("Conflict");
                break;
            case Response::Status::CODE_410:
                _statusLine.append("Gone");
                break;
            case Response::Status::CODE_411:
                _statusLine.append("Length Required");
                break;
            case Response::Status::CODE_412:
                _statusLine.append("Precondition Failed");
                break;
            case Response::Status::CODE_413:
                _statusLine.append("Request Entity Too Large");
                break;
            case Response::Status::CODE_414:
                _statusLine.append("Request-URI Too Large");
                break;
            case Response::Status::CODE_415:
                _statusLine.append("Unsupported Media Type");
                break;
            case Response::Status::CODE_416:
                _statusLine.append("Requested Range Not Satisfiable");
                break;
            case Response::Status::CODE_417:
                _statusLine.append("Expectation Failed");
                break;
            case Response::Status::CODE_422:
                _statusLine.append("Unprocessable Entity");
                break;
            case Response::Status::CODE_423:
                _statusLine.append("Locked");
                break;
            case Response::Status::CODE_424:
                _statusLine.append("Failed Dependency");
                break;
            case Response::Status::CODE_425:
                _statusLine.append("Unordered Collection");
                break;
            case Response::Status::CODE_426:
                _statusLine.append("Upgrade Required");
                break;
            case Response::Status::CODE_428:
                _statusLine.append("Precondition Required");
                break;
            case Response::Status::CODE_429:
                _statusLine.append("Too Many Requests");
                break;
            case Response::Status::CODE_431:
                _statusLine.append("Request Header Fields Too Large");
                break;
            case Response::Status::CODE_434:
                _statusLine.append("Requested host unavailable");
                break;
            case Response::Status::CODE_444:
                _statusLine.append("Close connection without sending headers");
                break;
            case Response::Status::CODE_449:
                _statusLine.append("Retry With");
                break;
            case Response::Status::CODE_451:
                _statusLine.append("Unavailable For Legal Reasons");
                break;
            case Response::Status::CODE_500:
                _statusLine.append("Internal Server Error");
                break;
            case Response::Status::CODE_501:
                _statusLine.append("Not Implemented");
                break;
            case Response::Status::CODE_502:
                _statusLine.append("Bad Gateway");
                break;
            case Response::Status::CODE_503:
                _statusLine.append("Service Unavailable");
                break;
            case Response::Status::CODE_504:
                _statusLine.append("Gateway Timeout");
                break;
            case Response::Status::CODE_505:
                _statusLine.append("HTTP Version Not Supported");
                break;
            case Response::Status::CODE_506:
                _statusLine.append("Variant Also Negotiates");
                break;
            case Response::Status::CODE_507:
                _statusLine.append("Insufficient Storage");
                break;
            case Response::Status::CODE_508:
                _statusLine.append("Loop Detected");
                break;
            case Response::Status::CODE_509:
                _statusLine.append("Bandwidth Limit Exceeded");
                break;
            case Response::Status::CODE_510:
                _statusLine.append("Not Extended");
                break;
            case Response::Status::CODE_511: 
                _statusLine.append("Network Authentication Required");
                break;
            default: // 使用可真刁钻呐!
                break;
        }
    } else {
        _statusLine.append(describe);
    }
    return *this;
}

// Response::Response(
//     Response::Status statusCode, 
//     std::string_view describe /*= ""*/) : _statusLine("HTTP/1.1 ")
//                                         , _responseHeaders()
//                                         , _responseBody() {
//     setResponseLine(statusCode, describe);
// }

void Response::createResponseBuffer() {
    _buf.clear();
    _buf.append(_statusLine);
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
