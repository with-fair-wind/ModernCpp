# CompilerWarnings.cmake
#
# 定义 INTERFACE target `mcpp::warnings`，统一了在 GCC / Clang / MSVC 下的
# 警告集合。任何 demo / test target 链接到它即可获得这套警告。
#
# 受顶层选项 MCPP_WARNINGS_AS_ERRORS（默认 OFF）控制。

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
# 说明：/EHsc 是 MSVC 的默认异常模型 —— CMake 会自动把它注入到 C++ target，
# 这里就不重复添加了。

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
