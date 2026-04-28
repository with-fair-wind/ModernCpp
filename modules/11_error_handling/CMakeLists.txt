# modules/11_error_handling — 错误处理 / Error Handling
#
# Demos for exceptions, std::error_code, std::expected (C++23), assertions.
#
# std::expected requires libstdc++ 12+ / libc++ 16+ / MSVC 17.4+.
# On ubuntu-24.04 the apt clang-18 sometimes resolves to a libstdc++ that
# does not yet expose <expected>, so guard the registration with a probe
# rather than failing the build outright. Modules whose feature mix isn't
# satisfied print STATUS and skip — the rest of the matrix keeps green.

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
