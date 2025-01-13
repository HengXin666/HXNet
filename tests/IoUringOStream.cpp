#include <thread>
#include <iostream>

#include <HXSTL/coroutine/loop/AsyncLoop.h>
#include <HXprint/AsyncOstream.hpp>
// 单纯使用的性能不如printf甚至std::cout
// 但是如果是在服务器中作为回显, 那么理论上可以减少内核态切换的性能开销!
// 即在

HX::STL::coroutine::task::Task<> ostreamTestByIoUring() {
    std::ostringstream ss;
    ss << std::this_thread::get_id();
    std::string idstr = ss.str();

    co_await HX::print::AsyncOstream::puts("你好\n");

    co_return;
}

int main() {
    std::size_t threadNum = 4;
    std::vector<std::thread> threadArr;

    for (std::size_t i = 0; i < threadNum; ++i) {
        threadArr.emplace_back([](){
            HX::STL::coroutine::task::runTask(
                HX::STL::coroutine::loop::AsyncLoop::getLoop(), 
                ostreamTestByIoUring()
            );
        });
    }

    for (auto&& it : threadArr) {
        it.join();
    }
    
    return 0;
}