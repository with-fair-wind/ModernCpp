# CompilerWarnings.cmake
#
# Defines the INTERFACE target `mcpp::warnings` which carries a unified
# warning set across GCC, Clang, and MSVC. Link any demo/test target to it
# to pick the warnings up.
#
# Honors the top-level option MCPP_WARNINGS_AS_ERRORS (default OFF).

add_library(mcpp_warnings INTERFACE)
add_library(mcpp::warnings ALIAS mcpp_warnings)

set(_mcpp_gnu_like_flags
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wcast-align
    -Woverloaded-virtual
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
)

set(_mcpp_msvc_flags
    /W4
    /permissive-
    /utf-8
    /Zc:__cplusplus
    /Zc:preprocessor
)
# Note: /EHsc is the MSVC default exception model — CMake injects it for C++
# targets automatically, so we don't repeat it here.

target_compile_options(mcpp_warnings INTERFACE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:${_mcpp_gnu_like_flags}>
    $<$<CXX_COMPILER_ID:MSVC>:${_mcpp_msvc_flags}>
)

target_compile_definitions(mcpp_warnings INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX _CRT_SECURE_NO_WARNINGS>
)

if(MCPP_WARNINGS_AS_ERRORS)
    target_compile_options(mcpp_warnings INTERFACE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Werror>
        $<$<CXX_COMPILER_ID:MSVC>:/WX>
    )
endif()
