#include <HXSTL/coroutine/loop/TimerLoop.h>

#include <HXSTL/coroutine/task/Task.hpp>
#include <HXSTL/coroutine/loop/AsyncLoop.h>

namespace HX { namespace STL { namespace coroutine { namespace loop {

std::optional<std::chrono::system_clock::duration> TimerLoop::run() {
    while (_initiationQueue.size()) {
        auto&& task = _initiationQueue.front();
        task->_coroutine.resume();
        task->_coroutine.promise()._ptr = task;
        _initiationQueue.pop();
    }

    while (_destructionQueue.size()) {
        _destructionQueue.pop();
    }

    while (_timerRBTree.size()) {
        auto nowTime = std::chrono::system_clock::now();
        auto it = _timerRBTree.begin();
        if (it->first <= nowTime) {
            it->second.resume();
            _timerRBTree.erase(it);
        } else {
            return it->first - nowTime;
        }
    }
    return std::nullopt;
}

HX::STL::coroutine::task::Task<
    HX::STL::container::NonVoidHelper<>
> TimerLoop::sleepUntil(
    std::chrono::system_clock::time_point expireTime
) {
    co_await SleepAwaiter(AsyncLoop::getLoop(), expireTime);
    co_return HX::STL::container::NonVoidHelper<> {};
}

HX::STL::coroutine::task::Task<
    HX::STL::container::NonVoidHelper<>
> TimerLoop::sleepFor(
    std::chrono::system_clock::duration duration
) {
    co_await SleepAwaiter(AsyncLoop::getLoop(), std::chrono::system_clock::now() + duration);
    co_return HX::STL::container::NonVoidHelper<> {};
}

HX::STL::coroutine::task::Task<
    HX::STL::container::NonVoidHelper<>
> TimerLoop::yield() {
    co_await SleepAwaiter(AsyncLoop::getLoop(), std::chrono::system_clock::now());
    co_return HX::STL::container::NonVoidHelper<> {};
}

}}}} // namespace HX::STL::coroutine::loop