# modules/12_threading — 多线程（新） / Threading (Modern)
#
# 演示：std::jthread、std::stop_token、mutex / atomic 基础。
#
# 在 Linux 上 jthread 需要 Threads::Threads（-lpthread）；在 Windows / MSVC
# 上它是 no-op，但链接 imported target 没有副作用。
#
# std::jthread / std::stop_token 需要 libstdc++ 11+ / MSVC STL 17.0+ /
# libc++ 18+（macos-14 上的 Apple Clang 仍然附带老版 libc++，没有这些设施）。
# 探测一下，缺就跳过 —— 与 11_error_handling 中 <expected> 的守卫同模式。

include(CheckCXXSourceCompiles)

set(_mcpp_prev_required_flags "${CMAKE_REQUIRED_FLAGS}")
if(MSVC)
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} /std:c++latest")
else()
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -std=c++23")
endif()
check_cxx_source_compiles("
    #include <stop_token>
    #include <thread>
    int main() {
        std::jthread t([](std::stop_token){});
        t.request_stop();
        return 0;
    }
" MCPP_HAS_STD_JTHREAD)
set(CMAKE_REQUIRED_FLAGS "${_mcpp_prev_required_flags}")

if(MCPP_HAS_STD_JTHREAD)
    find_package(Threads REQUIRED)

    mcpp_add_demo(NAME jthread_basics
                  SOURCES   demos/jthread_basics.cpp
                  LINK_LIBS Threads::Threads)

    mcpp_add_test(NAME test_jthread
                  SOURCES   tests/test_jthread.cpp
                  LINK_LIBS Threads::Threads)

    mcpp_add_demo(NAME thread_basics
                  SOURCES   demos/thread_basics.cpp
                  LINK_LIBS Threads::Threads)

    mcpp_add_demo(NAME stop_token_demo
                  SOURCES   demos/stop_token_demo.cpp
                  LINK_LIBS Threads::Threads)

    mcpp_add_demo(NAME semaphore_demo
                  SOURCES   demos/semaphore_demo.cpp
                  LINK_LIBS Threads::Threads)

    mcpp_add_demo(NAME mutex_lock_demo
                  SOURCES   demos/mutex_lock_demo.cpp
                  LINK_LIBS Threads::Threads)

    mcpp_add_demo(NAME shared_mutex_demo
                  SOURCES   demos/shared_mutex_demo.cpp
                  LINK_LIBS Threads::Threads)

    mcpp_add_demo(NAME condition_variable_demo
                  SOURCES   demos/condition_variable_demo.cpp
                  LINK_LIBS Threads::Threads)

    mcpp_add_demo(NAME latch_barrier_demo
                  SOURCES   demos/latch_barrier_demo.cpp
                  LINK_LIBS Threads::Threads)

    mcpp_add_demo(NAME future_promise_demo
                  SOURCES   demos/future_promise_demo.cpp
                  LINK_LIBS Threads::Threads)

    mcpp_add_test(NAME test_thread
                  SOURCES   tests/test_thread.cpp
                  LINK_LIBS Threads::Threads)

    mcpp_add_test(NAME test_stop_token
                  SOURCES   tests/test_stop_token.cpp
                  LINK_LIBS Threads::Threads)

    mcpp_add_test(NAME test_semaphore
                  SOURCES   tests/test_semaphore.cpp
                  LINK_LIBS Threads::Threads)

    mcpp_add_test(NAME test_mutex
                  SOURCES   tests/test_mutex.cpp
                  LINK_LIBS Threads::Threads)

    mcpp_add_test(NAME test_condition_variable
                  SOURCES   tests/test_condition_variable.cpp
                  LINK_LIBS Threads::Threads)

    mcpp_add_test(NAME test_latch_barrier
                  SOURCES   tests/test_latch_barrier.cpp
                  LINK_LIBS Threads::Threads)

    mcpp_add_test(NAME test_future_promise
                  SOURCES   tests/test_future_promise.cpp
                  LINK_LIBS Threads::Threads)
else()
    message(STATUS
        "12_threading: <jthread>/<stop_token> not available on this toolchain "
        "— skipping demos/tests for this module.")
endif()
