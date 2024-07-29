#include <HXHttp/HXServer.h>

#include <fcntl.h>
#include <array>

#include <HXSTL/HXStringTools.h>
#include <HXHttp/HXRouter.h>

namespace HXHttp {

void HXServer::CallbackFuncTimer::setTimeout(
    std::chrono::steady_clock::duration dt, 
    HXSTL::HXCallback<> cb, 
    StopSource stop /*= {}*/
) {
    auto expireTime = std::chrono::steady_clock::now() + dt;
    auto it = _timerHeap.insert({expireTime, _TimerEntry{std::move(cb), stop}});
    stop.setStopCallback([this, it] {
        auto cb = std::move(it->second._cb);
        _timerHeap.erase(it);
        cb();
    });
}

std::chrono::steady_clock::duration HXServer::CallbackFuncTimer::durationToNextTimer() {
    while (_timerHeap.size()) {
        auto it = _timerHeap.begin();
        auto now = std::chrono::steady_clock::now();
        if (it->first <= now) {
            // 持续时间已经到了, 执行其回调, 并且删除
            it->second._stop.clearStopCallback();
            auto cb = std::move(it->second._cb);
            _timerHeap.erase(it);
            cb();
        } else {
            return it->first - now;
        }
    }
    return std::chrono::nanoseconds(-1);
}

void HXServer::EpollContext::join() {
    std::array<struct ::epoll_event, 128> evs;
    while (1) {
        std::chrono::nanoseconds dt = _timer.durationToNextTimer();
        struct ::timespec timeout, *timeoutp = nullptr;
        if (dt.count() > 0) {
            timeout.tv_sec = dt.count() / 1'000'000'000;
            timeout.tv_nsec = dt.count() % 1'000'000'000;
            timeoutp = &timeout;
        }
        int len = HXErrorHandlingTools::convertError<int>(
            epoll_pwait2(_epfd, evs.data(), evs.size(), timeoutp, nullptr)
        ).expect("epoll_pwait2");
        for (int i = 0; i < len; ++i) {
            auto cb = HXSTL::HXCallback<>::fromAddress(evs[i].data.ptr);
            cb();
        }
    }
}   

void HXServer::AsyncFile::_epollCallback(
    HXSTL::HXCallback<> &&resume, 
    uint32_t events,
    StopSource stop
) {
    // 让操作系统通知我
    /**
     * EPOLLIN: 当有数据可读时, `epoll` 会触发事件
     * EPOLLET: 设置边沿触发模式
     * EPOLLONESHOT: 表示事件只会触发一次: 当一个文件描述符上的一个事件触发并被处理后, 这个文件描述符会从 `epoll` 监控队列中移除
     */
    struct ::epoll_event event;
    event.events = events;
    event.data.ptr = resume.getAddress(); // fd 对应 回调函数

    HXErrorHandlingTools::convertError<int>(
        ::epoll_ctl(HXServer::EpollContext::get()._epfd, EPOLL_CTL_MOD, _fd, &event)
    ).expect("EPOLL_CTL_MOD");
    stop.setStopCallback([resumePtr = resume.leakAddress()] {
        HXSTL::HXCallback<>::fromAddress(resumePtr)();
    });
}

HXServer::AsyncFile HXServer::AsyncFile::asyncWrap(int fd) {
    int flags = CHECK_CALL(::fcntl, fd, F_GETFL);
    flags |= O_NONBLOCK;
    CHECK_CALL(::fcntl, fd, F_SETFL, flags);

    struct ::epoll_event event;
    event.events = EPOLLET;
    event.data.ptr = nullptr; // fd 对应 回调函数 (没有)
    HXErrorHandlingTools::convertError<int>(
        ::epoll_ctl(EpollContext::get()._epfd, EPOLL_CTL_ADD, fd, &event)
    ).expect("EPOLL_CTL_ADD");

    return AsyncFile{fd};
}

void HXServer::AsyncFile::asyncAccept(
    HXAddressResolver::address& addr, 
    HXSTL::HXCallback<HXErrorHandlingTools::Expected<int>> cb,
    StopSource stop /*= {}*/
    ) {
    if (stop.isStopRequested()) {
        stop.clearStopCallback();
        return cb(-ECANCELED);
    }

    auto ret = HXErrorHandlingTools::convertError<int>(::accept(_fd, &addr._addr, &addr._addrlen));

    if (!ret.isError(EAGAIN)) { // 不是EAGAIN错误
        stop.clearStopCallback();
        return cb(ret);
    }

    return _epollCallback(
        [this, &addr, cb = std::move(cb), stop] () mutable {
            return asyncAccept(addr, std::move(cb), stop);
        }, EPOLLIN | EPOLLERR | EPOLLET | EPOLLONESHOT, stop
    );
}

void HXServer::AsyncFile::asyncRead(
    HXSTL::HXBytesBuffer& buf,
    std::size_t count,
    HXSTL::HXCallback<HXErrorHandlingTools::Expected<size_t>> cb,
    StopSource stop /*= {}*/
    ) {
    if (stop.isStopRequested()) {
        stop.clearStopCallback();
        return cb(-ECANCELED);
    }
        
    auto ret = HXErrorHandlingTools::convertError<size_t>(::recv(_fd, buf.data(), count, 0));

    if (!ret.isError(EAGAIN)) { // 不是EAGAIN错误
        stop.clearStopCallback();
        return cb(ret);
    }

    return _epollCallback(
        [this, &buf, count, cb = std::move(cb), stop]() mutable {
            return asyncRead(buf, count, std::move(cb), stop);
        }, EPOLLIN | EPOLLET | EPOLLERR | EPOLLONESHOT, stop
    );
}

void HXServer::AsyncFile::asyncWrite(
    HXSTL::HXConstBytesBufferView buf,
    HXSTL::HXCallback<HXErrorHandlingTools::Expected<size_t>> cb,
    StopSource stop /*= {}*/
    ) {
    if (stop.isStopRequested()) {
        stop.clearStopCallback();
        return cb(-ECANCELED);
    }

    auto ret = HXErrorHandlingTools::convertError<size_t>(::send(_fd, buf.data(), buf.size(), 0));

    if (!ret.isError(EAGAIN)) { // 不是EAGAIN错误
        stop.clearStopCallback();
        return cb(ret);
    }

    return _epollCallback(
        [this, &buf, cb = std::move(cb), stop]() mutable {
        return asyncWrite(buf, std::move(cb), stop);
        }, EPOLLOUT | EPOLLERR | EPOLLET | EPOLLONESHOT, stop
    );
}

void HXServer::ConnectionHandler::start(int fd) {
    _fd = HXServer::AsyncFile::asyncWrap(fd);
    return read();
}

void HXServer::ConnectionHandler::read(std::size_t size /*= HXRequest::BUF_SIZE*/) {
    StopSource stopIO(std::in_place);    // 读写停止程序
    StopSource stopTimer(std::in_place); // 计时器停止程序
    // 定时器先完成时, 取消读取
    // EpollContext::get()._timer.setTimeout(
    //     std::chrono::seconds(2), 
    //     [stopIO] {
    //         stopIO.doRequestStop();
    //     },
    //     stopTimer
    // );
    return _fd.asyncRead(_buf, size, [self = shared_from_this(), stopTimer] (HXErrorHandlingTools::Expected<size_t> ret) {
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
        if (std::size_t size = self->_request.parserRequest(HXSTL::HXConstBytesBufferView {self->_buf.data(), n})) {
            self->read(std::min(size, HXRequest::BUF_SIZE)); // 继续读取
        } else {
            self->handle(); // 开始响应
        }
    }, stopIO);
}

void HXServer::ConnectionHandler::handle() {
    // 交给路由处理
    auto fun = HXRouter::getSingleton().getEndpointFunc(_request.getRequesType(), _request.getRequesPath());
    // printf("cli -> url: %s\n", _request.getRequesPath().c_str());
    if (fun) {
        _response = fun(_request);
    } else {
        _response.setResponseLine(HXResponse::Status::CODE_404)
                 .setContentType("text/html", "UTF-8")
                 .setBodyData("<h1>404 NOT FIND PATH: [" 
                    + _request.getRequesPath() 
                    + "]</h1><h2>Now Time: " 
                    + HXSTL::HXDateTimeFormat::format() 
                    + "</h2>");
    }
    _response.createResponseBuffer();
    _request.clear(); // 本次请求使用结束, 清空, 复用
    return write(_response._buf);
}

void HXServer::ConnectionHandler::write(HXSTL::HXConstBytesBufferView buf) {
    return _fd.asyncWrite(_response._buf, [self = shared_from_this(), buf] (HXErrorHandlingTools::Expected<std::size_t> ret) {
        if (ret.error()) {
            return;
        }

        size_t n = ret.value(); // 已经写入的字节数
        if (n == self->_response._buf.size()) {
            // 全部写入啦
            self->_response.clear();
            return self->read(); // 开始读取
        }

        self->write(buf.subspan(n));
    });
}

void HXServer::Acceptor::start(const std::string& name, const std::string& port) {
    HXAddressResolver resolver;
    auto entry = resolver.resolve(name, port);
    LOG_INFO("====== HXServer start: \033[33m\033]8;;http://%s:%s/\033\\http://%s:%s/\033]8;;\033\\\033[0m\033[1;32m ======", 
        name.c_str(),
        port.c_str(),
        name.c_str(),
        port.c_str()
    );
    int listenfd = entry.createSocketAndBind();
    _serverFd = HXServer::AsyncFile::asyncWrap(listenfd);
    return accept();
}

void HXServer::Acceptor::accept() {
    return _serverFd.asyncAccept(_addr, [self = shared_from_this()] (HXErrorHandlingTools::Expected<int> ret) {
        int fd = ret.expect("accept");
        HXServer::ConnectionHandler::make()->start(fd); // 开始读取
        return self->accept(); // 继续回调(如果没有就挂起, 就返回了)
    });
}

} // namespace HXHttp
