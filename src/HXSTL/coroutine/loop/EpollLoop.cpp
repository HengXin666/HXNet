#include <HXSTL/coroutine/loop/EpollLoop.h>

#include <HXSTL/coroutine/loop/AsyncLoop.h>

namespace HX { namespace STL { namespace coroutine { namespace loop {

void EpollLoop::addEpollCtl(int fd) {
    struct epoll_event event;
    event.events = EPOLLET;
    event.data.ptr = nullptr;

    HX::STL::tools::ErrorHandlingTools::convertError<int>(
        ::epoll_ctl(
            _epfd,
            EPOLL_CTL_ADD,
            fd,
            &event
        )
    ).expect("EPOLL_CTL_ADD");

    ++_count;
}

void EpollLoop::removeListener(int fd) {
    HX::STL::tools::ErrorHandlingTools::convertError<int>(
        ::epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, nullptr)
    ).expect("EPOLL_CTL_DEL");
    --_count;
}

bool EpollLoop::addListener(EpollFilePromise &promise, EpollEventMask mask, int ctl) {
    struct ::epoll_event event;
    event.events = mask;
    event.data.ptr = &promise;
    
    // printf("开始侦测: %d\n", promise._fd);
    HX::STL::tools::ErrorHandlingTools::convertError<int>(
        ::epoll_ctl(_epfd, ctl, promise._fd, &event)
    ).expect("addListener: epoll_ctl");

    // if (ctl == EPOLL_CTL_ADD)
    //     ++_count;
    return true;
}

bool EpollLoop::run(std::optional<std::chrono::system_clock::duration> timeout) {
    if (!_count)
        return false;
    
    int epollTimeOut = -1;
    if (timeout) {
        epollTimeOut = timeout->count();
    }

    // TODO 这里是否需要修改为 epoll_pwait2 有待商榷
    int len = HX::STL::tools::ErrorHandlingTools::convertError<int>(
        ::epoll_wait(_epfd, _evs.data(), _evs.size(), epollTimeOut)
    ).expect("epoll_wait");

    for (int i = 0; i < len; ++i) {
        auto& event = _evs[i];
        // ((EpollFilePromise *)event.data.ptr)->_previous.resume(); // 下面的更标准
        auto& promise = *(EpollFilePromise *)event.data.ptr;
        std::coroutine_handle<EpollFilePromise>::from_promise(promise).resume();
    }
    return true;
}

EpollFilePromise::~EpollFilePromise() {
    if (_fd != -1) {
        // printf("结束侦测: %d\n", _fd);
        // HX::STL::coroutine::loop::AsyncLoop::getLoop().getEpollLoop().removeListener(_fd);
    }
}

}}}} // namespace HX::STL::coroutine::loop