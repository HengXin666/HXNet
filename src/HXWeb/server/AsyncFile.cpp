#include <HXWeb/server/AsyncFile.h>

#include <fcntl.h>

namespace HX { namespace web { namespace server {

void AsyncFile::_epollCallback(
    HX::STL::container::Callback<> &&resume, 
    uint32_t events,
    context::StopSource stop
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

    HX::STL::tools::ErrorHandlingTools::convertError<int>(
        ::epoll_ctl(context::EpollContext::get()._epfd, EPOLL_CTL_MOD, _fd, &event)
    ).expect("EPOLL_CTL_MOD");

    stop.setStopCallback([resumePtr = resume.leakAddress()] {
        HX::STL::container::Callback<>::fromAddress(resumePtr)();
    });
}

AsyncFile::AsyncFile(int fd) 
    : FileDescriptor(fd) {
    int flags = HX::STL::tools::ErrorHandlingTools::convertError<int>(
        ::fcntl(_fd, F_GETFL)
    ).expect("F_GETFL");
    flags |= O_NONBLOCK;
    HX::STL::tools::ErrorHandlingTools::convertError<int>(
        ::fcntl(_fd, F_SETFL, flags)
    ).expect("F_SETFL");

    struct epoll_event event;
    event.events = EPOLLET;
    event.data.ptr = nullptr;
    HX::STL::tools::ErrorHandlingTools::convertError<int>(
        ::epoll_ctl(context::EpollContext::get()._epfd, EPOLL_CTL_ADD, _fd, &event)
    ).expect("EPOLL_CTL_ADD");
}

// Server::AsyncFile Server::AsyncFile::asyncWrap(int fd) {
//     int flags = CHECK_CALL(::fcntl, fd, F_GETFL);
//     flags |= O_NONBLOCK;
//     CHECK_CALL(::fcntl, fd, F_SETFL, flags);

//     struct ::epoll_event event;
//     event.events = EPOLLET;
//     event.data.ptr = nullptr; // fd 对应 回调函数 (没有)
//     HX::STL::tools::ErrorHandlingTools::convertError<int>(
//         ::epoll_ctl(EpollContext::get()._epfd, EPOLL_CTL_ADD, fd, &event)
//     ).expect("EPOLL_CTL_ADD");

//     return AsyncFile{fd};
// }

void AsyncFile::asyncAccept(
    socket::AddressResolver::Address& addr, 
    HX::STL::container::Callback<HX::STL::tools::ErrorHandlingTools::Expected<int>> cb,
    context::StopSource stop /*= {}*/
    ) {
    if (stop.isStopRequested()) {
        stop.clearStopCallback();
        return cb(-ECANCELED);
    }

    auto ret = HX::STL::tools::ErrorHandlingTools::convertError<int>(::accept(_fd, &addr._addr, &addr._addrlen));

    if (!ret.isError(EAGAIN)) { // 不是EAGAIN错误
        // printf("已连接 %s ~\n", HX::STL::tools::DateTimeFormat::formatWithMilli().c_str());
        stop.clearStopCallback();
        return cb(ret);
    }

    return _epollCallback(
        [this, &addr, cb = std::move(cb), stop] () mutable {
            return asyncAccept(addr, std::move(cb), stop);
        }, EPOLLIN | EPOLLERR | EPOLLET | EPOLLONESHOT, stop
    );
}

void AsyncFile::asyncRead(
    HX::STL::container::BytesBuffer& buf,
    std::size_t count,
    HX::STL::container::Callback<HX::STL::tools::ErrorHandlingTools::Expected<size_t>> cb,
    context::StopSource stop /*= {}*/
    ) {
    if (stop.isStopRequested()) {
        stop.clearStopCallback();
        return cb(-ECANCELED);
    }
        
    auto ret = HX::STL::tools::ErrorHandlingTools::convertError<size_t>(::read(_fd, buf.data(), count));

    if (!ret.isError(EAGAIN)) { // 不是EAGAIN错误
        // printf("开始发送 %s ~\n", HX::STL::tools::DateTimeFormat::formatWithMilli().c_str());
        stop.clearStopCallback();
        return cb(ret);
    }

    return _epollCallback(
        [this, &buf, count, cb = std::move(cb), stop]() mutable {
            return asyncRead(buf, count, std::move(cb), stop);
        }, EPOLLIN | EPOLLET | EPOLLERR | EPOLLONESHOT, stop
    );
}

void AsyncFile::asyncWrite(
    HX::STL::container::ConstBytesBufferView buf,
    HX::STL::container::Callback<HX::STL::tools::ErrorHandlingTools::Expected<size_t>> cb,
    context::StopSource stop /*= {}*/
    ) {
    if (stop.isStopRequested()) {
        stop.clearStopCallback();
        return cb(-ECANCELED);
    }

    auto ret = HX::STL::tools::ErrorHandlingTools::convertError<size_t>(::write(_fd, buf.data(), buf.size()));

    if (!ret.isError(EAGAIN)) { // 不是EAGAIN错误
        // printf("发送完毕 %s ~\n", HX::STL::tools::DateTimeFormat::formatWithMilli().c_str());
        stop.clearStopCallback();
        return cb(ret);
    }

    return _epollCallback(
        [this, &buf, cb = std::move(cb), stop]() mutable {
        return asyncWrite(buf, std::move(cb), stop);
        }, EPOLLOUT | EPOLLERR | EPOLLET | EPOLLONESHOT, stop
    );
}

}}} // namespace HX::web::server
