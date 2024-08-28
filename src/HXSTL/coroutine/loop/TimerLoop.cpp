#include <HXSTL/coroutine/loop/TimerLoop.h>

#include <HXSTL/coroutine/task/Task.hpp>
#include <HXSTL/coroutine/loop/AsyncLoop.h>

namespace HX { namespace STL { namespace coroutine { namespace loop {

std::optional<std::chrono::system_clock::duration> TimerLoop::run() {

    /**
     * 发现bug啦!: 如果 _coroutine 还在 co_await, 那么此时直接 resume 是未定义的!
     * 只需要添加一个额外的变量即可!, 明天实现!
     */
    for (auto it = _taskList.begin(); it != _taskList.end(); ) {
        if ((*it)->_coroutine.done()) {
            auto tmp = it++;
            _taskList.erase(tmp);
            printf("删除(1)\n");
        } else {
            (*it)->_coroutine.resume();
            if ((*it)->_coroutine.done()) {
                auto tmp = it++;
                _taskList.erase(tmp);
                printf("删除(2)\n");
            } else {
                ++it;
            }
        }
    }

    while (_timerRBTree.size()) {
        auto nowTime = std::chrono::system_clock::now();
        auto it = _timerRBTree.begin();
        if (it->first < nowTime) {
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
}

HX::STL::coroutine::task::Task<
    HX::STL::container::NonVoidHelper<>
> TimerLoop::sleepFor(
    std::chrono::system_clock::duration duration
) {
    co_await SleepAwaiter(AsyncLoop::getLoop(), std::chrono::system_clock::now() + duration);
}

}}}} // namespace HX::STL::coroutine::loop