#include <HXWeb/server/ConnectionHandler.h>

#include <HXWeb/router/Router.h>
#include <HXWeb/server/context/EpollContext.h>
#include <HXSTL/container/Callback.h>
#include <HXSTL/utils/StringUtils.h>

namespace HX { namespace web { namespace server {

void ConnectionHandler::start(int fd) {
    _fd = AsyncFile {fd};
    return read();
}

void ConnectionHandler::read(std::size_t size /*= protocol::http::Request::BUF_SIZE*/) {
    // printf("解析中... %s ~\n", HX::STL::utils::DateTimeFormat::formatWithMilli().c_str());
    context::StopSource stopIO(std::in_place);    // 读写停止程序
    context::StopSource stopTimer(std::in_place); // 计时器停止程序
    // 定时器先完成时, 取消读取
    context::EpollContext::get()._timer.setTimeout(
        std::chrono::seconds(30),  // 定时为 30 s, 后期改为宏定义或者constexpr吧
        [stopIO] {
            stopIO.doRequestStop();
        },
        stopTimer
    );
    return _fd.asyncRead(_buf, size, [self = shared_from_this(), stopTimer] (HX::STL::tools::ErrorHandlingTools::Expected<size_t> ret) {
        stopTimer.doRequestStop();
        
        if (ret.error()) {
            return;
        }

        size_t n = ret.value(); // 读取到的字节数
        if (n == 0) {
            // 断开连接
            LOG_INFO("客户端已断开连接!");
            return;
        }
        
        // 进行解析
        if (std::size_t size = self->_request.parserRequest(HX::STL::container::ConstBytesBufferView {self->_buf.data(), n})) {
            self->read(std::min(size, protocol::http::Request::BUF_SIZE)); // 继续读取
        } else {
            self->handle(); // 开始响应
        }
    }, stopIO);
}

void ConnectionHandler::handle() {
    // 交给路由处理
    auto fun = HX::web::router::Router::getSingleton().getEndpointFunc(_request.getRequesType(), _request.getRequesPath());
    // printf("cli -> url: %s\n", _request.getRequesPath().c_str());
    if (fun) {
        _response = fun(_request);
    } else {
        _response.setResponseLine(HX::web::protocol::http::Response::Status::CODE_404)
                 .setContentType("text/html", "UTF-8")
                 .setBodyData("<h1>404 NOT FIND PATH: [" 
                    + _request.getRequesPath() 
                    + "]</h1><h2>Now Time: " 
                    + HX::STL::utils::DateTimeFormat::format() 
                    + "</h2>");
    }
    _response.createResponseBuffer();
    _request.clear(); // 本次请求使用结束, 清空, 复用
    return write(_response._buf);
}

void ConnectionHandler::write(HX::STL::container::ConstBytesBufferView buf) {
    return _fd.asyncWrite(_response._buf, [self = shared_from_this(), buf] (HX::STL::tools::ErrorHandlingTools::Expected<std::size_t> ret) {
        if (ret.error()) {
            return;
        }

        size_t n = ret.value(); // 已经写入的字节数
        if (n == self->_response._buf.size()) {
            // 全部写入啦
            self->_response.clear();
            return self->read(); // 开始读取
        }

        return self->write(buf.subspan(n));
    });
}

}}} // namespace HX::web::server
