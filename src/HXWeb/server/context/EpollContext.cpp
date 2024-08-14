#include <HXWeb/server/context/EpollContext.h>

namespace HX { namespace web { namespace server { namespace context {

void CallbackFuncTimer::setTimeout(
    std::chrono::steady_clock::duration dt, 
    HX::STL::container::Callback<> cb, 
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

std::chrono::steady_clock::duration CallbackFuncTimer::durationToNextTimer() {
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

void EpollContext::join() {
    std::array<struct ::epoll_event, 128> evs;
    while (1) {
        std::chrono::nanoseconds dt = _timer.durationToNextTimer();
        struct ::timespec timeout, *timeoutp = nullptr;
        if (dt.count() > 0) {
            timeout.tv_sec = dt.count() / 1'000'000'000;
            timeout.tv_nsec = dt.count() % 1'000'000'000;
            timeoutp = &timeout;
        }
        // printf("===阻塞(%d)====================\n", epollCnt);
        int len = HX::STL::tools::LinuxErrorHandlingTools::convertError<int>(
            epoll_pwait2(_epfd, evs.data(), evs.size(), timeoutp, nullptr)
        ).expect("epoll_pwait2");
        // printf("Linux通知: ... %s ~\n", HX::STL::utils::DateTimeFormat::formatWithMilli().c_str());
        for (int i = 0; i < len; ++i) {
            auto cb = HX::STL::container::Callback<>::fromAddress(evs[i].data.ptr);
            cb();
        }
    }
}

}}}} // namespace HX::web::server::context