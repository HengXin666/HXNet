#include <HXWeb/server/AsyncFile.h>

#include <HXSTL/coroutine/loop/AsyncLoop.h>

#include <fcntl.h>

namespace HX { namespace web { namespace server {

AsyncFile::AsyncFile(int fd) : FileDescriptor(fd)
{
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
        ::epoll_ctl(
            ((HX::STL::coroutine::loop::EpollLoop &)HX::STL::coroutine::loop::AsyncLoop::getLoop())._epfd,
            EPOLL_CTL_ADD,
            _fd,
            &event
        )
    ).expect("EPOLL_CTL_ADD");
}

HX::STL::coroutine::awaiter::Task<
    int, 
    HX::STL::coroutine::loop::EpollFilePromise
> AsyncFile::asyncAccept(
    socket::AddressResolver::Address& addr
) { // EPOLLIN | EPOLLERR | EPOLLET | EPOLLONESHOT
    auto ret = HX::STL::tools::ErrorHandlingTools::convertError<int>(
        ::accept(_fd, &addr._addr, &addr._addrlen)
    );

    while (ret.isError(EAGAIN)) { // 是EAGAIN错误
        co_await HX::STL::coroutine::loop::waitFileEvent(
            HX::STL::coroutine::loop::AsyncLoop::getLoop(),
            _fd,
            EPOLLIN | EPOLLET | EPOLLERR | EPOLLONESHOT
        );
        ret = HX::STL::tools::ErrorHandlingTools::convertError<int>(
            ::accept(_fd, &addr._addr, &addr._addrlen)
        );
    }
    co_return ret.value();
}

HX::STL::coroutine::awaiter::Task<
    size_t, 
    HX::STL::coroutine::loop::EpollFilePromise
> AsyncFile::asyncRead(
    HX::STL::container::BytesBuffer& buf,
    std::size_t count
) { // EPOLLIN | EPOLLET | EPOLLERR | EPOLLONESHOT
    auto ret = HX::STL::tools::ErrorHandlingTools::convertError<size_t>(
        ::read(_fd, buf.data(), count)
    );

    while (ret.isError(EAGAIN)) { // 是EAGAIN错误
        co_await HX::STL::coroutine::loop::waitFileEvent(
            HX::STL::coroutine::loop::AsyncLoop::getLoop(),
            _fd,
            EPOLLIN | EPOLLET | EPOLLERR | EPOLLONESHOT
        );
        ret = HX::STL::tools::ErrorHandlingTools::convertError<size_t>(
            ::read(_fd, buf.data(), count)
        );
    }
    co_return ret.value();
}

HX::STL::coroutine::awaiter::Task<
    size_t, 
    HX::STL::coroutine::loop::EpollFilePromise
> AsyncFile::asyncWrite(
    HX::STL::container::ConstBytesBufferView buf
) { // EPOLLOUT | EPOLLERR | EPOLLET | EPOLLONESHOT
    auto ret = HX::STL::tools::ErrorHandlingTools::convertError<size_t>(
        ::write(_fd, buf.data(), buf.size())
    );

    while (ret.isError(EAGAIN)) { // 是EAGAIN错误
        co_await HX::STL::coroutine::loop::waitFileEvent(
            HX::STL::coroutine::loop::AsyncLoop::getLoop(),
            _fd,
            EPOLLOUT | EPOLLERR | EPOLLET | EPOLLONESHOT
        );
        ret = HX::STL::tools::ErrorHandlingTools::convertError<size_t>(
            ::write(_fd, buf.data(), buf.size())
        );
    }
    co_return ret.value();
}

}}} // namespace HX::web::server
