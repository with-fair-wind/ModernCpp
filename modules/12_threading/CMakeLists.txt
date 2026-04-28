# modules/12_threading — 多线程（新） / Threading (Modern)
#
# Demos for std::jthread, std::stop_token, mutex/atomic basics.
#
# Threads::Threads is needed by jthread on Linux (-lpthread); on Windows /
# MSVC it's a no-op but linking the imported target is harmless.

find_package(Threads REQUIRED)

mcpp_add_demo(NAME jthread_basics
              SOURCES   demos/jthread_basics.cpp
              LINK_LIBS Threads::Threads)

mcpp_add_test(NAME test_jthread
              SOURCES   tests/test_jthread.cpp
              LINK_LIBS Threads::Threads)
