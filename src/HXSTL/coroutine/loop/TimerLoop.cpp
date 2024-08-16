#include <HXSTL/coroutine/loop/TimerLoop.h>

#include <HXSTL/coroutine/task/Task.hpp>
#include <HXSTL/coroutine/loop/AsyncLoop.h>

namespace HX { namespace STL { namespace coroutine { namespace loop {

void TimerLoop::runAll() {
    while (_timerRBTree.size() || _taskQueue.size()) {
        while (_taskQueue.size()) { // 执行协程任务
            auto task = std::move(_taskQueue.front());
            _taskQueue.pop();
            task.resume();
        }

        if (_timerRBTree.size()) { // 执行计时器任务
            auto now = std::chrono::system_clock::now();
            auto it = _timerRBTree.begin();
            if (now >= it->first) {
                do {
                    it->second.first.resume();
                    _timerRBTree.erase(it);
                    if (_timerRBTree.empty())
                        break;
                    it = _timerRBTree.begin();
                } while (now >= it->first);
            } else {
                std::this_thread::sleep_until(it->first); // 全场睡大觉 [阻塞]
            }
        }
    }
}

std::optional<std::chrono::system_clock::duration> TimerLoop::run() {
    while (_timerRBTree.size()) {
        auto nowTime = std::chrono::system_clock::now();
        auto it = _timerRBTree.begin();
        if (it->first < nowTime) {
            it->second.first.resume();
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
}

HX::STL::coroutine::task::Task<
    HX::STL::container::NonVoidHelper<>
> TimerLoop::sleepFor(
    std::chrono::system_clock::duration duration
) {
    co_await SleepAwaiter(AsyncLoop::getLoop(), std::chrono::system_clock::now() + duration);
}

}}}} // namespace HX::STL::coroutine::loop