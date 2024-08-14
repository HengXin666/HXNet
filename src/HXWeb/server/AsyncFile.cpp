#include <HXWeb/server/AsyncFile.h>

#include <fcntl.h>
#include <string.h>

#include <HXSTL/tools/ErrorHandlingTools.h>

namespace HX { namespace web { namespace server {

AsyncFile AsyncFile::asyncBind(HX::web::socket::AddressResolver::AddressInfo const &addr) {
    auto sock = AsyncFile{addr.createSocket()};
    auto serve_addr = addr.getAddress();
    int on = 1;
    setsockopt(sock._fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    setsockopt(sock._fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
    HX::STL::tools::ErrorHandlingTools::convertError<int>(
        ::bind(sock._fd, serve_addr._addr, serve_addr._addrlen)
    ).expect("bind");
    HX::STL::tools::ErrorHandlingTools::convertError<int>(
        ::listen(sock._fd, SOMAXCONN)
    ).expect("listen");
    return sock;
}

AsyncFile::AsyncFile(int fd) : FileDescriptor(fd)
{
    int flags = HX::STL::tools::ErrorHandlingTools::convertError<int>(
        ::fcntl(_fd, F_GETFL)
    ).expect("F_GETFL");

    flags |= O_NONBLOCK;

    HX::STL::tools::ErrorHandlingTools::convertError<int>(
        ::fcntl(_fd, F_SETFL, flags)
    ).expect("F_SETFL");

    HX::STL::coroutine::loop::AsyncLoop::getLoop().getEpollLoop().addEpollCtl(_fd);
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
            EPOLLIN | EPOLLERR | EPOLLET | EPOLLONESHOT
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
    std::span<char> buf,
    std::size_t count
) { // EPOLLIN | EPOLLET | EPOLLERR | EPOLLONESHOT
    try {
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
    } catch (...) {
        co_return 0; // 关闭连接
    }
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

AsyncFile::~AsyncFile() {
    if (_fd == -1) {
        return;
    }
    // 有可能它退出了, 但是还在等待吗?
    HX::STL::coroutine::loop::AsyncLoop::getLoop().getEpollLoop().removeListener(_fd);
}

}}} // namespace HX::web::server
