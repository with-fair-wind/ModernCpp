# modules/11_error_handling — 错误处理 / Error Handling
#
# 演示：异常、std::error_code、std::expected（C++23）、断言。
#
# std::expected 需要 libstdc++ 12+ / libc++ 16+ / MSVC 17.4+。
# 在 ubuntu-24.04 上，apt 装的 clang-18 有时会拉到尚未暴露 <expected> 的
# libstdc++，所以这里用一次探测而不是直接让构建失败。当工具链不满足
# 所需特性组合时，本模块只打印一条 STATUS 并跳过 —— 其余矩阵继续保持绿。

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
