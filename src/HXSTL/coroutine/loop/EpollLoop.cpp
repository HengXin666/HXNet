#include <HXSTL/coroutine/loop/EpollLoop.h>

namespace HX { namespace STL { namespace coroutine { namespace loop {

bool EpollLoop::addListener(EpollFilePromise &promise, EpollEventMask mask, int ctl) {
    struct ::epoll_event event;
    event.events = mask;
    event.data.ptr = &promise;
    
    HX::STL::tools::ErrorHandlingTools::convertError<int>(
        ::epoll_ctl(_epfd, ctl, promise._fd, &event)
    ).expect("epoll_pwait2");

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

}}}} // namespace HX::STL::coroutine::loop