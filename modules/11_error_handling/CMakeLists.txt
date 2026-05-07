# modules/11_error_handling — 错误处理 / Error Handling
#
# 演示：异常、std::error_code、std::expected（C++23）、断言。
#
# std::expected 需要 libstdc++ 12+ / libc++ 16+ / MSVC 17.4+。
# CI 已经把 Linux 升到 ubuntu:25.10 容器（GCC 15 / Clang 20 + libstdc++-15），
# 标准 stdlib 都能看到 <expected>；保留 try_compile 是为了照顾本地 toolchain
# 偏旧（如 Ubuntu 24.04 LTS 默认 g++-13）的开发者，工具链不满足时只打印
# STATUS 并跳过 —— 其余矩阵继续保持绿。

include(CheckCXXSourceCompiles)

set(_mcpp_prev_required_flags "${CMAKE_REQUIRED_FLAGS}")
if(MSVC)
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} /std:c++latest")
else()
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -std=c++23")
endif()
check_cxx_source_compiles("
    #include <expected>
    int main() {
        std::expected<int, int> e{1};
        (void)e;
        return 0;
    }
" MCPP_HAS_STD_EXPECTED)
set(CMAKE_REQUIRED_FLAGS "${_mcpp_prev_required_flags}")

if(MCPP_HAS_STD_EXPECTED)
    mcpp_add_demo(NAME expected_basics SOURCES demos/expected_basics.cpp)
    mcpp_add_test(NAME test_expected   SOURCES tests/test_expected.cpp)
else()
    message(STATUS
        "11_error_handling: <expected> not available on this toolchain "
        "(stdlib lacks std::expected) — skipping demos/tests for this module.")
endif()
