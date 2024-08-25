#include <HXSTL/coroutine/loop/IoUringLoop.h>

#include <HXSTL/tools/ErrorHandlingTools.h>
#include <HXSTL/coroutine/loop/AsyncLoop.h>

namespace HX { namespace STL { namespace coroutine { namespace loop {

#ifdef DEBUG_MAP
thread_local DebugMap<> debugMap;
#endif

IoUringLoop::IoUringLoop(unsigned int entries) : _ring() {
    unsigned int flags = 0;

// 绕过操作系统的缓存, 直接将数据从用户空间读写到磁盘
#if IO_URING_DIRECT
    flags |= IORING_SETUP_IOPOLL;
#endif

    HX::STL::tools::UringErrorHandlingTools::throwingError(
        ::io_uring_queue_init(entries, &_ring, flags)
    );
}

bool IoUringLoop::run(std::optional<std::chrono::system_clock::duration> timeout) {
    ::io_uring_cqe* cqe = nullptr;

    __kernel_timespec timespec; // 设置超时为无限阻塞
    __kernel_timespec* timespecPtr = nullptr;
    if (timeout.has_value()) {
        auto duration = timeout.value();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() % 1000000000;
        timespec.tv_sec = static_cast<long>(seconds);
        timespec.tv_nsec = static_cast<long>(nanoseconds);
        timespecPtr = &timespec;
    }

    // 阻塞等待内核, 返回是错误码; cqe是完成队列, 为传出参数
    int res = ::io_uring_submit_and_wait_timeout(&_ring, &cqe, 1, timespecPtr, nullptr);
    if (res == -ETIME) {
        return false;
    } else if (res < 0) [[unlikely]] {
        if (res == -EINTR) {
            return false;
        }
        throw std::system_error(-res, std::system_category());
    }

    unsigned head, numGot = 0;
    std::vector<std::coroutine_handle<>> tasks;
    io_uring_for_each_cqe(&_ring, head, cqe) {
        auto* task = reinterpret_cast<IoUringTask *>(cqe->user_data);
        task->_res = cqe->res;
        tasks.emplace_back(task->_previous);
        ++numGot;
    }

    // 手动前进完成队列的头部 (相当于批量io_uring_cqe_seen)
    ::io_uring_cq_advance(&_ring, numGot);
    _numSqesPending -= static_cast<std::size_t>(numGot);
    auto __ = std::this_thread::get_id();
    for (auto&& it : tasks) {
#ifdef DEBUG_MAP
        std::string _ = "";
        if (debugMap.count(it)) {
            // std::cout << std::this_thread::get_id();
            _ = debugMap.at(it);
            // printf(" %s", debugMap.at(it).c_str());
            debugMap.erase(it);
            // printf("\n");
            it.resume();
        } else if (it == std::noop_coroutine()) {
            _ = "!";
        } else {
            _ = "%%";
            printf("%%");
        }
        // printf("-");
#else
        it.resume();
#endif
    }
    return true;
}

IoUringTask::IoUringTask() {
    _sqe = HX::STL::coroutine::loop::AsyncLoop::getLoop().getIoUringLoop().getSqe();
    ::io_uring_sqe_set_data(_sqe, this);
}

}}}} // namespace HX::STL::coroutine::loop