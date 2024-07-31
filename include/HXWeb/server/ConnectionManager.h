#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-31 21:07:27
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * */
#ifndef _HX_CONNECTION_MANAGER_H_
#define _HX_CONNECTION_MANAGER_H_

#include <memory>

#include <HXSTL/utils/StringUtils.h>
#include <HXSTL/container/Callback.h>
#include <HXSTL/container/BytesBuffer.h>
#include <HXSTL/tools/ErrorHandlingTools.h>
#include <HXWeb/socket/AddressResolver.h>
#include <HXWeb/router/Router.h>
#include <HXWeb/server/AsyncFile.h>
#include <HXWeb/server/context/EpollContext.h>
#include <HXWeb/protocol/http/Request.h>
#include <HXWeb/protocol/http/Response.h>

namespace HX { namespace web { namespace server {

struct ConnectionManager : std::enable_shared_from_this<ConnectionManager> {
    using pointer = std::shared_ptr<ConnectionManager>;

    static pointer make() {
        return std::make_shared<pointer::element_type>();
    }

    struct ConnectionHandler : std::enable_shared_from_this<ConnectionHandler> {
        HX::web::server::AsyncFile m_conn;
        HX::STL::container::BytesBuffer m_readbuf {1024};
        HX::web::protocol::http::Request _request {};
        HX::web::protocol::http::Response _response {};

        using pointer = std::shared_ptr<ConnectionHandler>;

        static pointer make() {
            return std::make_shared<pointer::element_type>();
        }

        void do_start(int connfd) {
            m_conn = HX::web::server::AsyncFile{connfd};
            return do_read(protocol::http::Request::BUF_SIZE);
        }

        void do_read(size_t size) {
            context::StopSource stop_io(std::in_place);
            context::StopSource stop_timer(std::in_place);
            context::EpollContext::get()._timer.setTimeout(
                std::chrono::seconds(10),
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

        void do_write(HX::STL::container::ConstBytesBufferView buffer) {
            return m_conn.asyncWrite(buffer, [self = shared_from_this(), buffer](HX::STL::tools::ErrorHandlingTools::Expected<size_t> ret) {
                if (ret.error()) {
                    return;
                }
                auto n = ret.value();

                if (buffer.size() == n) {
                    self->_request.clear();
                    return self->do_read(HX::web::protocol::http::Request::BUF_SIZE);
                }
                return self->do_write(buffer.subspan(n));
            });
        }
    };

    HX::web::server::AsyncFile m_listening;
    HX::web::socket::AddressResolver::Address m_addr;

    void do_start(const std::string& name, const std::string& port) {
        HX::web::socket::AddressResolver resolver;
        auto entry = resolver.resolve(name, port);
        m_listening = HX::web::server::AsyncFile::asyncBind(entry);
        LOG_INFO("====== HXServer start: \033[33m\033]8;;http://%s:%s/\033\\http://%s:%s/\033]8;;\033\\\033[0m\033[1;32m ======", 
            name.c_str(),
            port.c_str(),
            name.c_str(),
            port.c_str()
        );
        return do_accept();
    }

    void do_accept() {
        return m_listening.asyncAccept(m_addr, [self = shared_from_this()] (HX::STL::tools::ErrorHandlingTools::Expected<int> ret) {
            auto connfd = ret.expect("accept");
            ConnectionHandler::make()->do_start(connfd);
            return self->do_accept();
        });
    }
};

}}} // namespace HX::web::server

#endif // _HX_CONNECTION_MANAGER_H_