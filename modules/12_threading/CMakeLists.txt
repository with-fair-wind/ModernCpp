# modules/12_threading — 多线程（新） / Threading (Modern)
#
# Demos for std::jthread, std::stop_token, mutex/atomic basics.
#
# Threads::Threads is needed by jthread on Linux (-lpthread); on Windows /
# MSVC it's a no-op but linking the imported target is harmless.
#
# std::jthread / std::stop_token need libstdc++ 11+ / MSVC STL 17.0+ /
# libc++ 18+ (Apple Clang on macos-14 still ships an older libc++ where
# they are missing). Probe and skip if absent — same pattern as the
# <expected> guard in 11_error_handling.

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
else()
    message(STATUS
        "12_threading: <jthread>/<stop_token> not available on this toolchain "
        "— skipping demos/tests for this module.")
endif()
