#include <HXSTL/coroutine/loop/AsyncLoop.h>

#include <thread>

namespace HX { namespace STL { namespace coroutine { namespace loop {

void AsyncLoop::run() {
    while (true) {
        auto timeout = _timerLoop.run();
        if (_ioUringLoop.hasEvent()) {
            _ioUringLoop.run(timeout);
        } else if (timeout) {
            std::this_thread::sleep_for(*timeout);
        } else {
            break;
        }
    }
}

}}}} // namespace HX::STL::coroutine::loop