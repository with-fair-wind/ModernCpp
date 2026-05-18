# modules/11_error_handling — 错误处理 / Error Handling
#
# 演示：异常、std::error_code、std::expected（C++23）、断言。
#
# std::expected 需要 libstdc++ 12+ / libc++ 16+ / MSVC 17.4+。
# CI 已经把 Linux 切到 archlinux:base-devel 容器（滚动发行版，当前 GCC 15.x / Clang 22.x），
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

# 不依赖 std::expected 的 demo / test（始终注册）
mcpp_add_demo(NAME optional_basics SOURCES demos/optional_basics.cpp)
mcpp_add_demo(NAME exception_basics SOURCES demos/exception_basics.cpp)
mcpp_add_demo(NAME exception_safety SOURCES demos/exception_safety.cpp)
mcpp_add_demo(NAME noexcept_demo SOURCES demos/noexcept_demo.cpp)
mcpp_add_demo(NAME assertion_demo SOURCES demos/assertion_demo.cpp)
mcpp_add_demo(NAME source_location_demo SOURCES demos/source_location_demo.cpp)

mcpp_add_test(NAME test_optional SOURCES tests/test_optional.cpp)
mcpp_add_test(NAME test_exception SOURCES tests/test_exception.cpp)
mcpp_add_test(NAME test_exception_safety SOURCES tests/test_exception_safety.cpp)
mcpp_add_test(NAME test_noexcept SOURCES tests/test_noexcept.cpp)
mcpp_add_test(NAME test_source_location SOURCES tests/test_source_location.cpp)

mcpp_add_demo(NAME error_code_demo SOURCES demos/error_code_demo.cpp)
mcpp_add_test(NAME test_error_code SOURCES tests/test_error_code.cpp)

if(MCPP_HAS_STD_EXPECTED)
    mcpp_add_demo(NAME expected_basics SOURCES demos/expected_basics.cpp)
    mcpp_add_test(NAME test_expected SOURCES tests/test_expected.cpp)
else()
    message(STATUS
        "11_error_handling: <expected> not available — skipping expected_basics "
        "and test_expected only.")
endif()
