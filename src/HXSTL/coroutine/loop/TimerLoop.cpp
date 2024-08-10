#include <HXSTL/coroutine/loop/TimerLoop.h>

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
                    it->second.resume();
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
            _timerRBTree.erase(it);
            it->second.resume();
        } else {
            return it->first - nowTime;
        }
    }
    return std::nullopt;
}

}}}} // namespace HX::STL::coroutine::loop