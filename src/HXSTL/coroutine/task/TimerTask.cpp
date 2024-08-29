#include <HXSTL/coroutine/task/TimerTask.h>

#include <HXSTL/coroutine/loop/AsyncLoop.h>

namespace HX { namespace STL { namespace coroutine { namespace task {

HX::STL::coroutine::awaiter::PreviousAwaiter TimerPromis::final_suspend() noexcept {
    HX::STL::coroutine::loop::AsyncLoop::getLoop().getTimerLoop().addDestructionQueue(
        _ptr
    );
    _ptr = nullptr;
    return HX::STL::coroutine::awaiter::PreviousAwaiter(_previous);
}

}}}} // namespace HX::STL::coroutine::task