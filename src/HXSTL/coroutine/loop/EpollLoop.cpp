#include <HXSTL/coroutine/loop/EpollLoop.h>

#include <HXSTL/coroutine/loop/AsyncLoop.h>

/**
 * @brief 判断是否有 epoll_pwait2
 */
#if defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 28))
#define HAVE_EPOLL_PWAIT2 1
#else
#define HAVE_EPOLL_PWAIT2 0
#endif

namespace HX { namespace STL { namespace coroutine { namespace loop {

void EpollLoop::addEpollCtl(int fd) {
    struct epoll_event event;
    event.events = EPOLLET;
    event.data.ptr = nullptr;

    HX::STL::tools::LinuxErrorHandlingTools::convertError<int>(
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
    HX::STL::tools::LinuxErrorHandlingTools::convertError<int>(
        ::epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, nullptr)
    ).expect("EPOLL_CTL_DEL");
    --_count;
}

bool EpollLoop::addListener(EpollFilePromise &promise, EpollEventMask mask, int ctl) {
    struct ::epoll_event event;
    event.events = mask;
    event.data.ptr = &promise;
    
    // printf("开始侦测: %d\n", promise._fd);
    HX::STL::tools::LinuxErrorHandlingTools::convertError<int>(
        ::epoll_ctl(_epfd, ctl, promise._fd, &event)
    ).expect("addListener: epoll_ctl");

    // if (ctl == EPOLL_CTL_ADD)
    //     ++_count;
    return true;
}

bool EpollLoop::run(std::optional<std::chrono::system_clock::duration> timeout) {
    if (!_count)
        return false;

#if HAVE_EPOLL_PWAIT2
    struct timespec epollTimeOut;
    struct timespec* epollTimeOutPtr = nullptr;
    if (timeout) {
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(*timeout);
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(*timeout - secs);
        epollTimeOut.tv_sec = secs.count();
        epollTimeOut.tv_nsec = ns.count();
        epollTimeOutPtr = &epollTimeOut;
    }

    // TODO 这里是否需要修改为 epoll_pwait2 有待商榷
    int len = HX::STL::tools::LinuxErrorHandlingTools::convertError<int>(
        ::epoll_pwait2(_epfd, _evs.data(), _evs.size(), epollTimeOutPtr, nullptr)
    ).expect("epoll_pwait2");
#else
    int epollTimeOut = -1;
    if (timeout) {
        epollTimeOut = std::chrono::duration_cast<std::chrono::milliseconds>(*timeout).count();
    }
    int len = HX::STL::tools::ErrorHandlingTools::convertError<int>(
        epoll_pwait(_epfd, _evs.data(), _evs.size(), epollTimeOut, nullptr)
    ).expect("epoll_pwait");
#endif
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