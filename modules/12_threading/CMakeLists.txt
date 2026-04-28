# modules/12_threading — 多线程（新） / Threading (Modern)
#
# Demos for std::jthread, std::stop_token, mutex/atomic basics.
#
# Threads::Threads is added explicitly: on Linux jthread needs -lpthread, on
# Windows / MSVC it's a no-op but linking the imported target is harmless.

find_package(Threads REQUIRED)

mcpp_add_demo(NAME jthread_basics SOURCES demos/jthread_basics.cpp)
mcpp_add_test(NAME test_jthread   SOURCES tests/test_jthread.cpp)

# Threads has to be linked per-target since helpers don't expose it.
target_link_libraries(12_threading__jthread_basics PRIVATE Threads::Threads)
target_link_libraries(12_threading__test_jthread   PRIVATE Threads::Threads)
