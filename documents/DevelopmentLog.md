# 开发日志

- [2024-8-26 23:02:02] : 添加了调试! 发现问题: (似乎定位到了! 就是135行! 如果改为不定时就不会触发了(似乎))

> - 有`#define DEBUG_MAP`模式:
```C++
==56929==ERROR: AddressSanitizer: heap-use-after-free on address 0x512001b60b70 at pc 0x64838d4680cf bp 0x7edab3efb5d0 sp 0x7edab3efb5c0
WRITE of size 4 at 0x512001b60b70 thread T4
    #0 0x64838d4680ce in HX::STL::coroutine::loop::IoUringLoop::run(std::optional<std::chrono::duration<long, std::ratio<1l, 1000000000l> > >) /home/Heng_Xin/HXcode/HXNet/src/HXSTL/coroutine/loop/IoUringLoop.cpp:51
    #1 0x64838d3ad88b in HX::STL::coroutine::loop::AsyncLoop::run() (/home/Heng_Xin/HXcode/HXNet/build/HXNet+0x4e88b) (BuildId: fc6382419a5718f71d514ebc499e6086c9f2d7f1)
    #2 0x64838d3bd8a1 in void HX::STL::coroutine::task::runTask<HX::STL::coroutine::loop::AsyncLoop, void, HX::STL::coroutine::promise::Promise<void, true>, HX::STL::coroutine::awaiter::ExitAwaiter<void, HX::STL::coroutine::promise::Promise<void, true> > >(HX::STL::coroutine::loop::AsyncLoop&, HX::STL::coroutine::task::Task<void, HX::STL::coroutine::promise::Promise<void, true>, HX::STL::coroutine::awaiter::ExitAwaiter<void, HX::STL::coroutine::promise::Promise<void, true> > > const&) (/home/Heng_Xin/HXcode/HXNet/build/HXNet+0x5e8a1) (BuildId: fc6382419a5718f71d514ebc499e6086c9f2d7f1)
    #3 0x64838d4d4621 in operator() /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/Server.cpp:27
    #4 0x64838d4d5a4f in __invoke_impl<void, HX::web::server::Server::start(const std::string&, const std::string&, std::size_t, std::chrono::seconds)::<lambda()> > /usr/include/c++/14.1.1/bits/invoke.h:61
    #5 0x64838d4d5a12 in __invoke<HX::web::server::Server::start(const std::string&, const std::string&, std::size_t, std::chrono::seconds)::<lambda()> > /usr/include/c++/14.1.1/bits/invoke.h:96
    #6 0x64838d4d59bf in _M_invoke<0> /usr/include/c++/14.1.1/bits/std_thread.h:292
    #7 0x64838d4d5993 in operator() /usr/include/c++/14.1.1/bits/std_thread.h:299
    #8 0x64838d4d5977 in _M_run /usr/include/c++/14.1.1/bits/std_thread.h:244
    #9 0x7edab86e0c83 in execute_native_thread_routine /usr/src/debug/gcc/gcc/libstdc++-v3/src/c++11/thread.cc:104
    #10 0x7edab905cc79 in asan_thread_start /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_interceptors.cpp:234
    #11 0x7edab84a6dec  (/usr/lib/libc.so.6+0x92dec) (BuildId: 32a656aa5562eece8c59a585f5eacd6cf5e2307b)
    #12 0x7edab852a0db  (/usr/lib/libc.so.6+0x1160db) (BuildId: 32a656aa5562eece8c59a585f5eacd6cf5e2307b)

0x512001b60b70 is located 176 bytes inside of 312-byte region [0x512001b60ac0,0x512001b60bf8)
freed by thread T4 here:
    #0 0x7edab90fe25a in operator delete(void*) /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_new_delete.cpp:152
    #1 0x64838d4ce96d in _recvSpan /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/IO.cpp:135
    #2 0x64838d4cebe1 in _recvSpan /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/IO.cpp:120
    #3 0x64838d4453db in std::__n4861::coroutine_handle<HX::STL::coroutine::promise::Promise<int, true> >::destroy() const /usr/include/c++/14.1.1/coroutine:244
    #4 0x64838d444df0 in HX::STL::coroutine::task::Task<int, HX::STL::coroutine::promise::Promise<int, true>, HX::STL::coroutine::awaiter::ExitAwaiter<int, HX::STL::coroutine::promise::Promise<int, true> > >::~Task() /home/Heng_Xin/HXcode/HXNet/include/HXSTL/coroutine/task/Task.hpp:58
    #5 0x64838d4cc88e in recvN /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/IO.cpp:108
    #6 0x64838d4ce8db in _recvSpan /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/IO.cpp:120
    #7 0x64838d3ab4f7 in std::__n4861::coroutine_handle<void>::resume() const /usr/include/c++/14.1.1/coroutine:137
    #8 0x64838d468466 in HX::STL::coroutine::loop::IoUringLoop::run(std::optional<std::chrono::duration<long, std::ratio<1l, 1000000000l> > >) /home/Heng_Xin/HXcode/HXNet/src/HXSTL/coroutine/loop/IoUringLoop.cpp:69
    #9 0x64838d3ad88b in HX::STL::coroutine::loop::AsyncLoop::run() (/home/Heng_Xin/HXcode/HXNet/build/HXNet+0x4e88b) (BuildId: fc6382419a5718f71d514ebc499e6086c9f2d7f1)
    #10 0x64838d3bd8a1 in void HX::STL::coroutine::task::runTask<HX::STL::coroutine::loop::AsyncLoop, void, HX::STL::coroutine::promise::Promise<void, true>, HX::STL::coroutine::awaiter::ExitAwaiter<void, HX::STL::coroutine::promise::Promise<void, true> > >(HX::STL::coroutine::loop::AsyncLoop&, HX::STL::coroutine::task::Task<void, HX::STL::coroutine::promise::Promise<void, true>, HX::STL::coroutine::awaiter::ExitAwaiter<void, HX::STL::coroutine::promise::Promise<void, true> > > const&) (/home/Heng_Xin/HXcode/HXNet/build/HXNet+0x5e8a1) (BuildId: fc6382419a5718f71d514ebc499e6086c9f2d7f1)
    #11 0x64838d4d4621 in operator() /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/Server.cpp:27
    #12 0x64838d4d5a4f in __invoke_impl<void, HX::web::server::Server::start(const std::string&, const std::string&, std::size_t, std::chrono::seconds)::<lambda()> > /usr/include/c++/14.1.1/bits/invoke.h:61
    #13 0x64838d4d5a12 in __invoke<HX::web::server::Server::start(const std::string&, const std::string&, std::size_t, std::chrono::seconds)::<lambda()> > /usr/include/c++/14.1.1/bits/invoke.h:96
    #14 0x64838d4d59bf in _M_invoke<0> /usr/include/c++/14.1.1/bits/std_thread.h:292
    #15 0x64838d4d5993 in operator() /usr/include/c++/14.1.1/bits/std_thread.h:299
    #16 0x64838d4d5977 in _M_run /usr/include/c++/14.1.1/bits/std_thread.h:244
    #17 0x7edab86e0c83 in execute_native_thread_routine /usr/src/debug/gcc/gcc/libstdc++-v3/src/c++11/thread.cc:104
    #18 0x7edab905cc79 in asan_thread_start /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_interceptors.cpp:234
    #19 0x7edab84a6dec  (/usr/lib/libc.so.6+0x92dec) (BuildId: 32a656aa5562eece8c59a585f5eacd6cf5e2307b)

previously allocated by thread T4 here:
    #0 0x7edab90fd682 in operator new(unsigned long) /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_new_delete.cpp:95
    #1 0x64838d4cdd0e in HX::web::server::IO::_recvSpan(std::span<char, 18446744073709551615ul>, __kernel_timespec*) const /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/IO.cpp:135
    #2 0x64838d4cc57b in recvN /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/IO.cpp:109
    #3 0x64838d4c953c in recvRequest /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/IO.cpp:47
    #4 0x64838d4c45be in start /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/ConnectionHandler.cpp:14
    #5 0x64838d4c81a9 in sendResponse /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/IO.cpp:39
    #6 0x64838d4cface in _send /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/IO.cpp:137
    #7 0x64838d3ab4f7 in std::__n4861::coroutine_handle<void>::resume() const /usr/include/c++/14.1.1/coroutine:137
    #8 0x64838d468466 in HX::STL::coroutine::loop::IoUringLoop::run(std::optional<std::chrono::duration<long, std::ratio<1l, 1000000000l> > >) /home/Heng_Xin/HXcode/HXNet/src/HXSTL/coroutine/loop/IoUringLoop.cpp:69
    #9 0x64838d3ad88b in HX::STL::coroutine::loop::AsyncLoop::run() (/home/Heng_Xin/HXcode/HXNet/build/HXNet+0x4e88b) (BuildId: fc6382419a5718f71d514ebc499e6086c9f2d7f1)
    #10 0x64838d3bd8a1 in void HX::STL::coroutine::task::runTask<HX::STL::coroutine::loop::AsyncLoop, void, HX::STL::coroutine::promise::Promise<void, true>, HX::STL::coroutine::awaiter::ExitAwaiter<void, HX::STL::coroutine::promise::Promise<void, true> > >(HX::STL::coroutine::loop::AsyncLoop&, HX::STL::coroutine::task::Task<void, HX::STL::coroutine::promise::Promise<void, true>, HX::STL::coroutine::awaiter::ExitAwaiter<void, HX::STL::coroutine::promise::Promise<void, true> > > const&) (/home/Heng_Xin/HXcode/HXNet/build/HXNet+0x5e8a1) (BuildId: fc6382419a5718f71d514ebc499e6086c9f2d7f1)
    #11 0x64838d4d4621 in operator() /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/Server.cpp:27
    #12 0x64838d4d5a4f in __invoke_impl<void, HX::web::server::Server::start(const std::string&, const std::string&, std::size_t, std::chrono::seconds)::<lambda()> > /usr/include/c++/14.1.1/bits/invoke.h:61
    #13 0x64838d4d5a12 in __invoke<HX::web::server::Server::start(const std::string&, const std::string&, std::size_t, std::chrono::seconds)::<lambda()> > /usr/include/c++/14.1.1/bits/invoke.h:96
    #14 0x64838d4d59bf in _M_invoke<0> /usr/include/c++/14.1.1/bits/std_thread.h:292
    #15 0x64838d4d5993 in operator() /usr/include/c++/14.1.1/bits/std_thread.h:299
    #16 0x64838d4d5977 in _M_run /usr/include/c++/14.1.1/bits/std_thread.h:244
    #17 0x7edab86e0c83 in execute_native_thread_routine /usr/src/debug/gcc/gcc/libstdc++-v3/src/c++11/thread.cc:104
    #18 0x7edab905cc79 in asan_thread_start /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_interceptors.cpp:234
    #19 0x7edab84a6dec  (/usr/lib/libc.so.6+0x92dec) (BuildId: 32a656aa5562eece8c59a585f5eacd6cf5e2307b)

Thread T4 created by T0 here:
    #0 0x7edab90f38fb in pthread_create /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_interceptors.cpp:245
    #1 0x7edab86e0d81 in __gthread_create /usr/src/debug/gcc/gcc-build/x86_64-pc-linux-gnu/libstdc++-v3/include/x86_64-pc-linux-gnu/bits/gthr-default.h:676
    #2 0x7edab86e0d81 in std::thread::_M_start_thread(std::unique_ptr<std::thread::_State, std::default_delete<std::thread::_State> >, void (*)()) /usr/src/debug/gcc/gcc/libstdc++-v3/src/c++11/thread.cc:172
    #3 0x64838d4d56b0 in thread<HX::web::server::Server::start(const std::string&, const std::string&, std::size_t, std::chrono::seconds)::<lambda()> > /usr/include/c++/14.1.1/bits/std_thread.h:164
    #4 0x64838d4d5570 in construct_at<std::thread, HX::web::server::Server::start(const std::string&, const std::string&, std::size_t, std::chrono::seconds)::<lambda()> > /usr/include/c++/14.1.1/bits/stl_construct.h:97
    #5 0x64838d4d4dd3 in construct<std::thread, HX::web::server::Server::start(const std::string&, const std::string&, std::size_t, std::chrono::seconds)::<lambda()> > /usr/include/c++/14.1.1/bits/alloc_traits.h:536
    #6 0x64838d4d4dd3 in emplace_back<HX::web::server::Server::start(const std::string&, const std::string&, std::size_t, std::chrono::seconds)::<lambda()> > /usr/include/c++/14.1.1/bits/vector.tcc:117
    #7 0x64838d4d495b in HX::web::server::Server::start(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, unsigned long, std::chrono::duration<long, std::ratio<1l, 1l> >) /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/Server.cpp:24
    #8 0x64838d459060 in main /home/Heng_Xin/HXcode/HXNet/examples/WsServer.cpp:115
    #9 0x7edab8439c87  (/usr/lib/libc.so.6+0x25c87) (BuildId: 32a656aa5562eece8c59a585f5eacd6cf5e2307b)
    #10 0x7edab8439d4b in __libc_start_main (/usr/lib/libc.so.6+0x25d4b) (BuildId: 32a656aa5562eece8c59a585f5eacd6cf5e2307b)
    #11 0x64838d39fb24 in _start (/home/Heng_Xin/HXcode/HXNet/build/HXNet+0x40b24) (BuildId: fc6382419a5718f71d514ebc499e6086c9f2d7f1)

SUMMARY: AddressSanitizer: heap-use-after-free /home/Heng_Xin/HXcode/HXNet/src/HXSTL/coroutine/loop/IoUringLoop.cpp:51 in HX::STL::coroutine::loop::IoUringLoop::run(std::optional<std::chrono::duration<long, std::ratio<1l, 1000000000l> > >)
Shadow bytes around the buggy address:
  0x512001b60880: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 fa
  0x512001b60900: fa fa fa fa fa fa fa fa fd fd fd fd fd fd fd fd
  0x512001b60980: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x512001b60a00: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fa
  0x512001b60a80: fa fa fa fa fa fa fa fa fd fd fd fd fd fd fd fd
=>0x512001b60b00: fd fd fd fd fd fd fd fd fd fd fd fd fd fd[fd]fd
  0x512001b60b80: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fa
  0x512001b60c00: fa fa fa fa fa fa fa fa fd fd fd fd fd fd fd fd
  0x512001b60c80: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x512001b60d00: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fa
  0x512001b60d80: fa fa fa fa fa fa fa fa fd fd fd fd fd fd fd fd
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==56929==ABORTING
```

> - 非`#define DEBUG_MAP`模式

```C++
==59971==ERROR: AddressSanitizer: heap-use-after-free on address 0x50f000f41600 at pc 0x5807186b9096 bp 0x7f7472cfb760 sp 0x7f7472cfb750
WRITE of size 4 at 0x50f000f41600 thread T4
    #0 0x5807186b9095 in HX::STL::coroutine::loop::IoUringLoop::run(std::optional<std::chrono::duration<long, std::ratio<1l, 1000000000l> > >) /home/Heng_Xin/HXcode/HXNet/src/HXSTL/coroutine/loop/IoUringLoop.cpp:51
    #1 0x5807185fe88b in HX::STL::coroutine::loop::AsyncLoop::run() (/home/Heng_Xin/HXcode/HXNet/build/HXNet+0x4b88b) (BuildId: 6e0f9435176d70d8aad9988f57ae821276eb4124)
    #2 0x58071860e8a1 in void HX::STL::coroutine::task::runTask<HX::STL::coroutine::loop::AsyncLoop, void, HX::STL::coroutine::promise::Promise<void, true>, HX::STL::coroutine::awaiter::ExitAwaiter<void, HX::STL::coroutine::promise::Promise<void, true> > >(HX::STL::coroutine::loop::AsyncLoop&, HX::STL::coroutine::task::Task<void, HX::STL::coroutine::promise::Promise<void, true>, HX::STL::coroutine::awaiter::ExitAwaiter<void, HX::STL::coroutine::promise::Promise<void, true> > > const&) (/home/Heng_Xin/HXcode/HXNet/build/HXNet+0x5b8a1) (BuildId: 6e0f9435176d70d8aad9988f57ae821276eb4124)
    #3 0x5807187199d3 in operator() /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/Server.cpp:27
    #4 0x58071871ae01 in __invoke_impl<void, HX::web::server::Server::start(const std::string&, const std::string&, std::size_t, std::chrono::seconds)::<lambda()> > /usr/include/c++/14.1.1/bits/invoke.h:61
    #5 0x58071871adc4 in __invoke<HX::web::server::Server::start(const std::string&, const std::string&, std::size_t, std::chrono::seconds)::<lambda()> > /usr/include/c++/14.1.1/bits/invoke.h:96
    #6 0x58071871ad71 in _M_invoke<0> /usr/include/c++/14.1.1/bits/std_thread.h:292
    #7 0x58071871ad45 in operator() /usr/include/c++/14.1.1/bits/std_thread.h:299
    #8 0x58071871ad29 in _M_run /usr/include/c++/14.1.1/bits/std_thread.h:244
    #9 0x7f74774e0c83 in execute_native_thread_routine /usr/src/debug/gcc/gcc/libstdc++-v3/src/c++11/thread.cc:104
    #10 0x7f7477e5cc79 in asan_thread_start /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_interceptors.cpp:234
    #11 0x7f74772a6dec  (/usr/lib/libc.so.6+0x92dec) (BuildId: 32a656aa5562eece8c59a585f5eacd6cf5e2307b)
    #12 0x7f747732a0db  (/usr/lib/libc.so.6+0x1160db) (BuildId: 32a656aa5562eece8c59a585f5eacd6cf5e2307b)

0x50f000f41600 is located 112 bytes inside of 176-byte region [0x50f000f41590,0x50f000f41640)
freed by thread T4 here:
    #0 0x7f7477efe25a in operator delete(void*) /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_new_delete.cpp:152
    #1 0x580718713f57 in _recvSpan /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/IO.cpp:135
    #2 0x580718714147 in _recvSpan /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/IO.cpp:120
    #3 0x5807186963db in std::__n4861::coroutine_handle<HX::STL::coroutine::promise::Promise<int, true> >::destroy() const /usr/include/c++/14.1.1/coroutine:244
    #4 0x580718695df0 in HX::STL::coroutine::task::Task<int, HX::STL::coroutine::promise::Promise<int, true>, HX::STL::coroutine::awaiter::ExitAwaiter<int, HX::STL::coroutine::promise::Promise<int, true> > >::~Task() /home/Heng_Xin/HXcode/HXNet/include/HXSTL/coroutine/task/Task.hpp:58
    #5 0x58071871210a in recvN /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/IO.cpp:108
    #6 0x580718713ec5 in _recvSpan /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/IO.cpp:120
    #7 0x5807185fc4f7 in std::__n4861::coroutine_handle<void>::resume() const /usr/include/c++/14.1.1/coroutine:137
    #8 0x5807186b93ae in HX::STL::coroutine::loop::IoUringLoop::run(std::optional<std::chrono::duration<long, std::ratio<1l, 1000000000l> > >) /home/Heng_Xin/HXcode/HXNet/src/HXSTL/coroutine/loop/IoUringLoop.cpp:85
    #9 0x5807185fe88b in HX::STL::coroutine::loop::AsyncLoop::run() (/home/Heng_Xin/HXcode/HXNet/build/HXNet+0x4b88b) (BuildId: 6e0f9435176d70d8aad9988f57ae821276eb4124)
    #10 0x58071860e8a1 in void HX::STL::coroutine::task::runTask<HX::STL::coroutine::loop::AsyncLoop, void, HX::STL::coroutine::promise::Promise<void, true>, HX::STL::coroutine::awaiter::ExitAwaiter<void, HX::STL::coroutine::promise::Promise<void, true> > >(HX::STL::coroutine::loop::AsyncLoop&, HX::STL::coroutine::task::Task<void, HX::STL::coroutine::promise::Promise<void, true>, HX::STL::coroutine::awaiter::ExitAwaiter<void, HX::STL::coroutine::promise::Promise<void, true> > > const&) (/home/Heng_Xin/HXcode/HXNet/build/HXNet+0x5b8a1) (BuildId: 6e0f9435176d70d8aad9988f57ae821276eb4124)
    #11 0x5807187199d3 in operator() /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/Server.cpp:27
    #12 0x58071871ae01 in __invoke_impl<void, HX::web::server::Server::start(const std::string&, const std::string&, std::size_t, std::chrono::seconds)::<lambda()> > /usr/include/c++/14.1.1/bits/invoke.h:61
    #13 0x58071871adc4 in __invoke<HX::web::server::Server::start(const std::string&, const std::string&, std::size_t, std::chrono::seconds)::<lambda()> > /usr/include/c++/14.1.1/bits/invoke.h:96
    #14 0x58071871ad71 in _M_invoke<0> /usr/include/c++/14.1.1/bits/std_thread.h:292
    #15 0x58071871ad45 in operator() /usr/include/c++/14.1.1/bits/std_thread.h:299
    #16 0x58071871ad29 in _M_run /usr/include/c++/14.1.1/bits/std_thread.h:244
    #17 0x7f74774e0c83 in execute_native_thread_routine /usr/src/debug/gcc/gcc/libstdc++-v3/src/c++11/thread.cc:104
    #18 0x7f7477e5cc79 in asan_thread_start /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_interceptors.cpp:234
    #19 0x7f74772a6dec  (/usr/lib/libc.so.6+0x92dec) (BuildId: 32a656aa5562eece8c59a585f5eacd6cf5e2307b)

previously allocated by thread T4 here:
    #0 0x7f7477efd682 in operator new(unsigned long) /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_new_delete.cpp:95
    #1 0x580718713408 in HX::web::server::IO::_recvSpan(std::span<char, 18446744073709551615ul>, __kernel_timespec*) const /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/IO.cpp:135
    #2 0x580718711df7 in recvN /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/IO.cpp:109
    #3 0x58071870edb8 in recvRequest /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/IO.cpp:47
    #4 0x580718709ea0 in start /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/ConnectionHandler.cpp:14
    #5 0x58071870da25 in sendResponse /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/IO.cpp:39
    #6 0x580718714fd3 in _send /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/IO.cpp:137
    #7 0x5807185fc4f7 in std::__n4861::coroutine_handle<void>::resume() const /usr/include/c++/14.1.1/coroutine:137
    #8 0x5807186b93ae in HX::STL::coroutine::loop::IoUringLoop::run(std::optional<std::chrono::duration<long, std::ratio<1l, 1000000000l> > >) /home/Heng_Xin/HXcode/HXNet/src/HXSTL/coroutine/loop/IoUringLoop.cpp:85
    #9 0x5807185fe88b in HX::STL::coroutine::loop::AsyncLoop::run() (/home/Heng_Xin/HXcode/HXNet/build/HXNet+0x4b88b) (BuildId: 6e0f9435176d70d8aad9988f57ae821276eb4124)
    #10 0x58071860e8a1 in void HX::STL::coroutine::task::runTask<HX::STL::coroutine::loop::AsyncLoop, void, HX::STL::coroutine::promise::Promise<void, true>, HX::STL::coroutine::awaiter::ExitAwaiter<void, HX::STL::coroutine::promise::Promise<void, true> > >(HX::STL::coroutine::loop::AsyncLoop&, HX::STL::coroutine::task::Task<void, HX::STL::coroutine::promise::Promise<void, true>, HX::STL::coroutine::awaiter::ExitAwaiter<void, HX::STL::coroutine::promise::Promise<void, true> > > const&) (/home/Heng_Xin/HXcode/HXNet/build/HXNet+0x5b8a1) (BuildId: 6e0f9435176d70d8aad9988f57ae821276eb4124)
    #11 0x5807187199d3 in operator() /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/Server.cpp:27
    #12 0x58071871ae01 in __invoke_impl<void, HX::web::server::Server::start(const std::string&, const std::string&, std::size_t, std::chrono::seconds)::<lambda()> > /usr/include/c++/14.1.1/bits/invoke.h:61
    #13 0x58071871adc4 in __invoke<HX::web::server::Server::start(const std::string&, const std::string&, std::size_t, std::chrono::seconds)::<lambda()> > /usr/include/c++/14.1.1/bits/invoke.h:96
    #14 0x58071871ad71 in _M_invoke<0> /usr/include/c++/14.1.1/bits/std_thread.h:292
    #15 0x58071871ad45 in operator() /usr/include/c++/14.1.1/bits/std_thread.h:299
    #16 0x58071871ad29 in _M_run /usr/include/c++/14.1.1/bits/std_thread.h:244
    #17 0x7f74774e0c83 in execute_native_thread_routine /usr/src/debug/gcc/gcc/libstdc++-v3/src/c++11/thread.cc:104
    #18 0x7f7477e5cc79 in asan_thread_start /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_interceptors.cpp:234
    #19 0x7f74772a6dec  (/usr/lib/libc.so.6+0x92dec) (BuildId: 32a656aa5562eece8c59a585f5eacd6cf5e2307b)

Thread T4 created by T0 here:
    #0 0x7f7477ef38fb in pthread_create /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_interceptors.cpp:245
    #1 0x7f74774e0d81 in __gthread_create /usr/src/debug/gcc/gcc-build/x86_64-pc-linux-gnu/libstdc++-v3/include/x86_64-pc-linux-gnu/bits/gthr-default.h:676
    #2 0x7f74774e0d81 in std::thread::_M_start_thread(std::unique_ptr<std::thread::_State, std::default_delete<std::thread::_State> >, void (*)()) /usr/src/debug/gcc/gcc/libstdc++-v3/src/c++11/thread.cc:172
    #3 0x58071871aa62 in thread<HX::web::server::Server::start(const std::string&, const std::string&, std::size_t, std::chrono::seconds)::<lambda()> > /usr/include/c++/14.1.1/bits/std_thread.h:164
    #4 0x58071871a922 in construct_at<std::thread, HX::web::server::Server::start(const std::string&, const std::string&, std::size_t, std::chrono::seconds)::<lambda()> > /usr/include/c++/14.1.1/bits/stl_construct.h:97
    #5 0x58071871a185 in construct<std::thread, HX::web::server::Server::start(const std::string&, const std::string&, std::size_t, std::chrono::seconds)::<lambda()> > /usr/include/c++/14.1.1/bits/alloc_traits.h:536
    #6 0x58071871a185 in emplace_back<HX::web::server::Server::start(const std::string&, const std::string&, std::size_t, std::chrono::seconds)::<lambda()> > /usr/include/c++/14.1.1/bits/vector.tcc:117
    #7 0x580718719d0d in HX::web::server::Server::start(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, unsigned long, std::chrono::duration<long, std::ratio<1l, 1l> >) /home/Heng_Xin/HXcode/HXNet/src/HXWeb/server/Server.cpp:24
    #8 0x5807186aa060 in main /home/Heng_Xin/HXcode/HXNet/examples/WsServer.cpp:115
    #9 0x7f7477239c87  (/usr/lib/libc.so.6+0x25c87) (BuildId: 32a656aa5562eece8c59a585f5eacd6cf5e2307b)
    #10 0x7f7477239d4b in __libc_start_main (/usr/lib/libc.so.6+0x25d4b) (BuildId: 32a656aa5562eece8c59a585f5eacd6cf5e2307b)
    #11 0x5807185f0b24 in _start (/home/Heng_Xin/HXcode/HXNet/build/HXNet+0x3db24) (BuildId: 6e0f9435176d70d8aad9988f57ae821276eb4124)

SUMMARY: AddressSanitizer: heap-use-after-free /home/Heng_Xin/HXcode/HXNet/src/HXSTL/coroutine/loop/IoUringLoop.cpp:51 in HX::STL::coroutine::loop::IoUringLoop::run(std::optional<std::chrono::duration<long, std::ratio<1l, 1000000000l> > >)
Shadow bytes around the buggy address:
  0x50f000f41380: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x50f000f41400: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x50f000f41480: fa fa fa fa fd fd fd fd fd fd fd fd fd fd fd fd
  0x50f000f41500: fd fd fd fd fd fd fd fd fd fd fa fa fa fa fa fa
  0x50f000f41580: fa fa fd fd fd fd fd fd fd fd fd fd fd fd fd fd
=>0x50f000f41600:[fd]fd fd fd fd fd fd fd fa fa fa fa fa fa fa fa
  0x50f000f41680: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x50f000f41700: fd fd fd fd fd fd fa fa fa fa fa fa fa fa fd fd
  0x50f000f41780: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x50f000f41800: fd fd fd fd fa fa fa fa fa fa fa fa fd fd fd fd
  0x50f000f41880: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==59971==ABORTING
```

```md
> gpt的建议:
这个错误信息表明你遇到了堆内存使用后的释放（heap-use-after-free）问题。具体来说：

1. **问题概述**：地址 `0x50f000f41600` 被释放后仍然被写入。这通常发生在内存块被释放后，程序仍然尝试访问或修改这块内存。

2. **堆栈跟踪**：
   - 错误发生在 `HX::STL::coroutine::loop::IoUringLoop::run` 函数中的写操作，行号是 51。
   - 错误的内存地址 `0x50f000f41600` 位于一个 176 字节的内存块中，该内存块在 `HX::web::server::IO::_recvSpan` 函数中被释放。

3. **内存分配和释放**：
   - 该内存块最初是在 `HX::web::server::IO::_recvSpan` 函数中分配的。
   - 然后在 `HX::STL::coroutine::task::Task<int, ...>` 对象析构时，这块内存被释放。
   - 错误信息表明，之后的操作尝试写入这块已经释放的内存。

4. **解决建议**：
   - 检查 `HX::STL::coroutine::loop::IoUringLoop::run` 函数中是否有对已释放内存的访问。
   - 确保在内存释放后不再访问或操作那块内存。可以使用智能指针来自动管理内存，避免手动释放后出现问题。
   - 仔细检查 `HX::web::server::IO::_recvSpan` 函数及相关的任务和协程代码，确保所有内存的生命周期都得到正确管理。

这种类型的错误可能很复杂，需要详细审查代码中内存的分配和释放逻辑。
```

- [2024-8-25 22:59:08] : 可能需要解决的关键问题: `it.resume();`为野指针! 不知何处注册得...
- [2024-8-25 16:43:33] : 排查了一个下午, 依旧解决不了... 艹!(rnm tq! 越来越玄学了, 先是 `it.resume();` 为野指针?!, 再是`Function not implemented`异常, 以及`corrupted double-linked list`; wdf, 能力不足了!)
- [2024-8-24 23:11:51] : 修复了一个uring阻塞等待没有效果的bug | 仍然在排查问题... 因为似乎所有的协程都有可能段错误qwq...(怎么可能被提前释放了? 明明单线程没有的问题!)
- [2024-8-23 23:08:20] : 发现一个多线程高并发时候会触发的bug, 原因不明! (位于`src/HXSTL/coroutine/loop/IoUringLoop.cpp`的`58行`, 原因是协程指针失效了?访问了野指针; 仍在排查qwq)
- [2024-8-22 23:14:13] : 修改为多线程版本(每个线程独享一个`uring`, 并且仍然是协程的!) | 可能有bug, 还不能复现, 原因不明
- [2024-8-22 21:59:34] : 新增可自定义路由失败时候的端点函数
- [2024-8-22 11:59:48] : 完成对端点函数以及api宏的重构
- [2024-8-22 11:37:50] : 完成对websocket的重构
- [2024-8-21 23:35:04] : 正在重构请求类和响应类, 目前还有websocket和端点函数以及api宏没有修改, 其他已经适配 | 采用一种基于协程事件循环`close fd`于析构函数的方法, 避免了无法在析构函数中使用协程这个问题
- [2024-8-20 22:34:25] : 支持`co_await Task`协程抛出异常, 并且可以被捕获!
- [2024-8-19 22:37:29] : 修复WebSocket的bug (1. 不应该使用whenAny来取消uring, 因为还在监听; 2. 写错变量名, debug半天...)
- [2024-8-19 17:25:09] : 初步完成WebSocket, 正在测试... (可以连接, 但是发现无法接收到消息?!)
- [2024-8-18 23:25:46] : 导入`hashlib`项目且配置CMake, 初步设计WebSocket连接解析(未完成), 修改`Response`支持直接异步写回(提供`send`函数)
- [2024-8-18 21:48:26] : 导入`OpenSSL`项目且配置CMake, 并且新增`certs/GenerateCerts.sh`以生成证书和私钥
- [2024-8-17 18:58:31] : 尝试开发 协程返回值 (`Expected<T>`(具体描述见分支`v5.0`)) 未遂

---

> [!TIP]
> 请忽略下面... | 我将以新的格式重新书写 (2024-8-17 15:46:47起)

### 协程epoll服务端BUG汇总
1. [x] 读取数据的时候, 有时候无法读取到正确的数据 (某些值被换成了`\0`)
    - 解决方案: 使用`std::span<char>`和`std::vector<char>`配合, 而不是自制`buf`类, 它他妈居然又读取到?!?
2. [x] 无法正确的断开连接: 明明客户端已经关闭, 而服务端却没有反应 | 实际上`::Accept`已经重新复用那个已经关闭的套接字, 但是`co_await`读取, 它没有反应, 一直卡在那里!
    - 解决方案: `include/HXWeb/server/ConnectionHandler.h`实际上`make`创建的是智能指针, 而我们只是需要其协程, 故不需要其对象的成员, 导致`AsyncFile`无法因协程退出而析构
3. [x] 玄学的`include/HXSTL/coroutine/loop/EpollLoop.h`的`await_suspend`的`fd == -1`的问题, 可能和2有关?!?!
    - 离奇的修复啦?!
---
4. 在`AsyncFile::asyncRead`加入了`try`以处理`[ERROR]: 连接被对方重置`, 是否有非`try`的解决方案?!

5. 依旧不能很好的实现html基于轮询的聊天室, 我都怀疑是html+js的问题了...(明明和基于回调的事件循环差不多, 都是这个问题..)
    - 发现啦: http的请求体是不带`\0`作为终止的, 因此解析的时候使用C语言风格的字符串就导致解析失败(越界了)