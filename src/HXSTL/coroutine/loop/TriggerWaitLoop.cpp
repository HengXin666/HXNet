#include <HXSTL/coroutine/loop/TriggerWaitLoop.h>

namespace HX { namespace STL { namespace coroutine { namespace loop {

TriggerWaitLoop::~TriggerWaitLoop() {
    switch (_behavior) {
    case DestructionBehavior::Resume: // 把挂载的任务全部执行
        for (auto&& it : _waitQueue)
            it.resume();
        break;
    case DestructionBehavior::Cleanup: // 清除所有挂载的协程
        for (auto&& it : _waitQueue)
            it.destroy();
        break;
    default:
        break;
    }
}

HX::STL::coroutine::task::Task<
    HX::STL::container::NonVoidHelper<>
> TriggerWaitLoop::triggerWait(TriggerWaitLoop& TWaitLoop) {
    co_await TriggerWaitTask(TWaitLoop);
    co_return HX::STL::container::NonVoidHelper<> {};
}

}}}} // namespace HX::STL::coroutine::loop