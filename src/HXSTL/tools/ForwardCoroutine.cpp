#include <HXSTL/tools/ForwardCoroutine.h>

#include <HXSTL/coroutine/loop/AsyncLoop.h>

namespace HX { namespace STL { namespace tools {

void ForwardCoroutineTools::TimerLoopAddTask(std::coroutine_handle<> coroutine) {
    HX::STL::coroutine::loop::AsyncLoop::getLoop().getTimerLoop().addTimer(
        std::chrono::system_clock::now(),
        coroutine
    );
}

}}} // namespace HX::STL::tools