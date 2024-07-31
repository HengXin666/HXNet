#pragma
#include <algorithm>
#include <arpa/inet.h>
#include <array>
#include <cassert>
#include <chrono>
#include <fcntl.h>
#include <map>
#include <netdb.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <system_error>
#include <type_traits>
#include <unistd.h>
#include <utility>
#include <vector>

#include <HXSTL/tools/ErrorHandlingTools.h>
#include <HXSTL/container/Callback.h>
#include <HXWeb/socket/AddressResolver.h>
#include <HXWeb/HXApiHelper.h>
#include <HXWeb/server/context/EpollContext.h>
#include <HXWeb/server/AsyncFile.h>

using namespace HX;
using namespace web;
using namespace STL;
using namespace container;
using namespace socket;
using namespace server;
using namespace context;
using namespace tools;
using namespace protocol::http;

struct http_server : std::enable_shared_from_this<http_server> {
    using pointer = std::shared_ptr<http_server>;

    static pointer make() {
        return std::make_shared<pointer::element_type>();
    }

    struct http_connection_handler
        : std::enable_shared_from_this<http_connection_handler> {
        HX::web::server::AsyncFile m_conn;
        BytesBuffer m_readbuf{1024};
        HX::web::protocol::http::Request _request {};
        HX::web::protocol::http::Response _response {};

        using pointer = std::shared_ptr<http_connection_handler>;

        static pointer make() {
            return std::make_shared<pointer::element_type>();
        }

        void do_start(int connfd) {
            m_conn = HX::web::server::AsyncFile{connfd};
            return do_read(Request::BUF_SIZE);
        }

        void do_read(size_t size) {
            // 注意：TCP 基于流，可能粘包
            // fmt::println("开始读取...");
            // 设置一个 3 秒的定时器，若 3
            // 秒内没有读到任何请求，则视为对方放弃，关闭连接
            StopSource stop_io(std::in_place);
            StopSource stop_timer(std::in_place);
            EpollContext::get()._timer.setTimeout(
                std::chrono::seconds(1),
                [stop_io] {
                    stop_io.doRequestStop(); // 定时器先完成时，取消读取
                },
                stop_timer);
            // 开始读取
            return m_conn.asyncRead(m_readbuf, size, [self = shared_from_this(), stop_timer] (HX::STL::tools::ErrorHandlingTools::Expected<size_t> ret) {
                stop_timer.doRequestStop();
                
                if (ret.error()) {
                    return;
                }

                size_t n = ret.value(); // 读取到的字节数
                if (n == 0) {
                    // 断开连接
                    LOG_INFO("客户端已断开连接!");
                    return;
                }
                
                if (std::size_t size = self->_request.parserRequest(HX::STL::container::ConstBytesBufferView {self->m_readbuf.data(), n})) {
                    self->do_read(std::min(size, protocol::http::Request::BUF_SIZE)); // 继续读取
                } else {
                    self->do_handle(); // 开始响应
                }
                }, stop_io
            );
        }

        void do_handle() {
            // 交给路由处理
            auto fun = HX::web::router::Router::getSingleton().getEndpointFunc(_request.getRequesType(), _request.getRequesPath());
            // printf("cli -> url: %s\n", _request.getRequesPath().c_str());
            if (fun) {
                // _response = fun(_request);
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
            _request.clear();
            return do_write(_response._buf);
        }

        void do_write(ConstBytesBufferView buffer) {
            return m_conn.asyncWrite(buffer, [self = shared_from_this(), buffer](ErrorHandlingTools::Expected<size_t> ret) {
                if (ret.error()) {
                    // fmt::println("写入错误，放弃连接");
                    return;
                }
                auto n = ret.value();

                if (buffer.size() == n) {
                    self->_request.clear();
                    return self->do_read(Request::BUF_SIZE);
                }
                return self->do_write(buffer.subspan(n));
            });
        }
    };

    HX::web::server::AsyncFile m_listening;
    AddressResolver::Address m_addr;

    void do_start(std::string name, std::string port) {
        AddressResolver resolver;
        // fmt::println("正在监听：http://{}:{}", name, port);
        auto entry = resolver.resolve(name, port);
        m_listening = HX::web::server::AsyncFile::asyncBind(entry);
        return do_accept();
    }

    void do_accept() {
        return m_listening.asyncAccept(m_addr, [self = shared_from_this()] (ErrorHandlingTools::Expected<int> ret) {
            auto connfd = ret.expect("accept");
            http_connection_handler::make()->do_start(connfd);
            return self->do_accept();
        });
    }
};

void server() {
    EpollContext ctx;
    auto acceptor = http_server::make();

    acceptor->do_start("127.0.0.1", "28205");
    ctx.join();
}

