#include <HXHttp/HXRequest.h>

#include <sys/socket.h> // 套接字
#include <cstring>

#include <HXSTL/HXStringTools.h>

namespace HXHttp {

std::size_t HXRequest::parserRequest(HXSTL::HXConstBytesBufferView buf) {
    printf("解析请求: %s...\n", buf.data());
    _previousData.append(buf);
    char *tmp = nullptr;
    char *line = nullptr;
    if (_requestLine.empty()) { // 请求行还未解析
        line = ::strtok_r(_previousData.data(), "\r\n", &tmp); // 线程安全
        if (!line)
            return HXRequest::BUF_SIZE;
        _requestLine = HXSTL::HXStringUtil::split(line, " "); // 解析请求头: GET /PTAH HTTP/1.1
        if (_requestLine.size() != 3)
            return HXRequest::BUF_SIZE;
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
        }
        do {// 解析 请求行
            // 计算当前子字符串的长度
            std::size_t length = (*tmp == '\0' ? ::strlen(line) : tmp - line - 1);
            auto p = HXSTL::HXStringUtil::splitAtFirst(std::string_view {line, length}, ": ");
            if (p.first == "") { // 解析失败, 说明当前是空行, 也有可能是没有读取完毕
                if (*line != '\r') { 
                    // 应该剩下的参与下次解析
                    _previousData = HXSTL::HXStringUtil::rfindAndTrim(_previousData.data(), "\r\n");
                    return HXRequest::BUF_SIZE;
                }
                // 是空行
                _completeRequestHeader = true;
                break;
            }
            HXSTL::HXStringUtil::toSmallLetter(p.first);
            p.second.pop_back(); // 去掉 '\r'
            _requestHeaders.insert(p);
            printf("%s -> %s\n", p.first.c_str(), p.second.c_str());
        } while ((line = ::strtok_r(nullptr, "\n", &tmp)));
    }
    
    if (_requestHeaders.count("content-length")) { // 存在请求体
        // 是 空行之后 (\r\n\r\n) 的内容大小(char)
        if (_remainingBodyLen == -1) {
            _remainingBodyLen = std::stoll(_requestHeaders["content-length"]) 
                              - static_cast<ssize_t>(::strlen(tmp));
            _body = std::string {tmp};
        } else {
            _remainingBodyLen -= buf.size();
            _body->append(_previousData.data());
        }

        if (_remainingBodyLen != 0) {
            _previousData.clear();
            return _remainingBodyLen;
        }
    }

    _previousData.clear();
    return 0; // 解析完毕
}

// @bug [[deprecated]]
int HXRequest::resolutionRequest(int fd, char *str, const std::size_t strLen) {
    char *tmp = NULL;
    char *line = ::strtok_r(str, "\r\n", &tmp); // 线程安全
    if (!line)
        return HXRequest::ParseStatus::NotHttp;
    _requestLine = HXSTL::HXStringUtil::split(line, " "); // 解析请求头: GET /PTAH HTTP/1.1
    if (_requestLine.size() != 3)
        return HXRequest::ParseStatus::NotHttp;
    // requestLine[0] 请求类型
    // requestLine[1] 请求PTAH
    // requestLine[2] 请求协议
    // printf("请求类型: [%s], 请求PTAH: [%s], 请求协议: [%s]\n",
    //     _requestLine[0].c_str(),
    //     _requestLine[1].c_str(),
    //     _requestLine[2].c_str()
    // );

    /// TOOD 如果有多个"\r\n"会直接跳过
    /**
     * @brief 解析请求头和请求体
     * 解析请求报文算法:
     * 请求行\r\n  | 我们按 \r\n 分割
     * 请求头\r\n  | 按照 \n 分割, 这样每一个line的后面都会有\r残留, 需要pop掉
     * \r\n (空行) | 只会解析到 \r
     * 请求体
     */
    while ((line = ::strtok_r(NULL, "\n", &tmp))) { // 解析 请求行
        std::size_t length = (*tmp == '\0' ? ::strlen(line) : tmp - line - 1);
        auto p = HXSTL::HXStringUtil::splitAtFirst(std::string_view {line, length}, ": ");
        if (p.first == "") { // 解析失败, 说明当前是空行
            break;
        }
        HXSTL::HXStringUtil::toSmallLetter(p.first);
        p.second.pop_back(); // 去掉 '\r'
        _requestHeaders.insert(p);
        // printf("%s -> %s\n", p.first.c_str(), p.second.c_str());
    }

    if (_requestHeaders.count("content-length")) { // 存在请求体
        // 是 空行之后 (\r\n\r\n) 的内容大小(char)
        int bodyLen = stoi(_requestHeaders["content-length"]) - strlen(tmp);
        _body = std::string {tmp};
        while (bodyLen > 0) {
            int cvLen = ::recv(fd, str, strLen, 0);
            if (cvLen <= 0) {
                if (bodyLen == 0) // 这个存在疑问qwq..
                    return HXRequest::ParseStatus::ClientOut;
                else if (!(errno == EWOULDBLOCK || errno == EAGAIN))
                    return HXRequest::ParseStatus::RecvError;
                continue;
            }
            _body->append(str, cvLen);
            bodyLen -= cvLen;
        }
        // printf("%s", _body->c_str());
    }
    return HXRequest::ParseStatus::ParseSuccessful;
}

} // namespace HXHttp
